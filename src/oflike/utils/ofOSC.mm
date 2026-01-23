#import "ofOSC.h"
#import <Foundation/Foundation.h>
#import <Network/Network.h>
#import <map>
#import <mutex>
#import <cstring>
#import <arpa/inet.h>

namespace oflike {

// OSC パケット解析ユーティリティ
namespace OSCUtil {
    // 4バイトアライメント
    inline size_t alignTo4(size_t size) {
        return (size + 3) & ~3;
    }

    // Int32 エンコード（ビッグエンディアン）
    inline void encodeInt32(std::vector<uint8_t>& buffer, int32_t value) {
        uint32_t netValue = htonl(static_cast<uint32_t>(value));
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&netValue);
        buffer.insert(buffer.end(), bytes, bytes + 4);
    }

    // Float エンコード（ビッグエンディアン）
    inline void encodeFloat(std::vector<uint8_t>& buffer, float value) {
        uint32_t intValue;
        std::memcpy(&intValue, &value, 4);
        uint32_t netValue = htonl(intValue);
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&netValue);
        buffer.insert(buffer.end(), bytes, bytes + 4);
    }

    // String エンコード（NULL終端 + 4バイトアライメント）
    inline void encodeString(std::vector<uint8_t>& buffer, const std::string& str) {
        buffer.insert(buffer.end(), str.begin(), str.end());
        buffer.push_back(0);  // NULL終端
        // パディング
        size_t padding = alignTo4(str.size() + 1) - (str.size() + 1);
        for (size_t i = 0; i < padding; ++i) {
            buffer.push_back(0);
        }
    }

    // Int32 デコード
    inline int32_t decodeInt32(const uint8_t* data) {
        uint32_t netValue;
        std::memcpy(&netValue, data, 4);
        return static_cast<int32_t>(ntohl(netValue));
    }

    // Float デコード
    inline float decodeFloat(const uint8_t* data) {
        uint32_t netValue;
        std::memcpy(&netValue, data, 4);
        uint32_t hostValue = ntohl(netValue);
        float result;
        std::memcpy(&result, &hostValue, 4);
        return result;
    }

    // String デコード
    inline std::string decodeString(const uint8_t* data, size_t maxLen, size_t& outLen) {
        size_t len = strnlen(reinterpret_cast<const char*>(data), maxLen);
        outLen = alignTo4(len + 1);
        return std::string(reinterpret_cast<const char*>(data), len);
    }
}

// OSCMessage::Impl
struct OSCMessage::Impl {
    std::string address;
    std::vector<OSCArg> args;
};

OSCMessage::OSCMessage() : impl_(std::make_unique<Impl>()) {}
OSCMessage::OSCMessage(const std::string& address) : impl_(std::make_unique<Impl>()) {
    impl_->address = address;
}
OSCMessage::~OSCMessage() = default;

void OSCMessage::setAddress(const std::string& address) {
    impl_->address = address;
}

const std::string& OSCMessage::getAddress() const {
    return impl_->address;
}

void OSCMessage::addInt(int32_t value) {
    impl_->args.push_back(OSCArg(value));
}

void OSCMessage::addFloat(float value) {
    impl_->args.push_back(OSCArg(value));
}

void OSCMessage::addString(const std::string& value) {
    impl_->args.push_back(OSCArg(value));
}

size_t OSCMessage::getArgCount() const {
    return impl_->args.size();
}

OSCArgType OSCMessage::getArgType(size_t index) const {
    if (index < impl_->args.size()) {
        return impl_->args[index].type;
    }
    return OSCArgType::INT32;
}

int32_t OSCMessage::getArgAsInt(size_t index) const {
    if (index < impl_->args.size()) {
        return impl_->args[index].asInt();
    }
    return 0;
}

float OSCMessage::getArgAsFloat(size_t index) const {
    if (index < impl_->args.size()) {
        return impl_->args[index].asFloat();
    }
    return 0.0f;
}

const std::string& OSCMessage::getArgAsString(size_t index) const {
    static const std::string empty;
    if (index < impl_->args.size()) {
        return impl_->args[index].asString();
    }
    return empty;
}

const OSCArg& OSCMessage::getArg(size_t index) const {
    static const OSCArg empty;
    if (index < impl_->args.size()) {
        return impl_->args[index];
    }
    return empty;
}

void OSCMessage::clear() {
    impl_->address.clear();
    impl_->args.clear();
}

// OSCReceiver::Impl
struct OSCReceiver::Impl {
    nw_listener_t listener = nullptr;
    nw_connection_t connection = nullptr;
    dispatch_queue_t queue = nullptr;
    std::map<std::string, OSCMessageCallback> handlers;
    std::mutex mutex;
    bool listening = false;
    uint16_t port = 0;

    void parseOSCPacket(const uint8_t* data, size_t length);
    void parseOSCMessage(const uint8_t* data, size_t length);
};

void OSCReceiver::Impl::parseOSCMessage(const uint8_t* data, size_t length) {
    if (length < 4) return;

    // アドレス解読
    size_t addrLen = 0;
    std::string address = OSCUtil::decodeString(data, length, addrLen);
    if (address.empty() || address[0] != '/') return;

    size_t offset = addrLen;
    if (offset >= length) return;

    // タイプタグ解読
    size_t typeTagLen = 0;
    std::string typeTags = OSCUtil::decodeString(data + offset, length - offset, typeTagLen);
    if (typeTags.empty() || typeTags[0] != ',') return;

    offset += typeTagLen;

    // メッセージ作成
    OSCMessage message(address);

    // 引数解読
    for (size_t i = 1; i < typeTags.size(); ++i) {
        if (offset + 4 > length) break;

        char tag = typeTags[i];
        switch (tag) {
            case 'i': {
                int32_t value = OSCUtil::decodeInt32(data + offset);
                message.addInt(value);
                offset += 4;
                break;
            }
            case 'f': {
                float value = OSCUtil::decodeFloat(data + offset);
                message.addFloat(value);
                offset += 4;
                break;
            }
            case 's': {
                size_t strLen = 0;
                std::string value = OSCUtil::decodeString(data + offset, length - offset, strLen);
                message.addString(value);
                offset += strLen;
                break;
            }
            default:
                return;  // 未対応タイプ
        }
    }

    // ハンドラー呼び出し
    std::lock_guard<std::mutex> lock(mutex);
    auto it = handlers.find(address);
    if (it != handlers.end() && it->second) {
        it->second(message);
    }
}

void OSCReceiver::Impl::parseOSCPacket(const uint8_t* data, size_t length) {
    if (length == 0) return;

    // OSC Bundle（#bundle で始まる）またはメッセージ（/ で始まる）
    if (data[0] == '/') {
        parseOSCMessage(data, length);
    } else if (length >= 8 && std::memcmp(data, "#bundle", 7) == 0) {
        // Bundle 対応（簡易版：タイムスタンプスキップ）
        size_t offset = 16;  // "#bundle" + NULL + timetag (8 bytes)
        while (offset + 4 <= length) {
            int32_t elementSize = OSCUtil::decodeInt32(data + offset);
            offset += 4;
            if (elementSize > 0 && offset + elementSize <= length) {
                parseOSCMessage(data + offset, elementSize);
                offset += elementSize;
            } else {
                break;
            }
        }
    }
}

OSCReceiver::OSCReceiver() : impl_(std::make_unique<Impl>()) {}
OSCReceiver::~OSCReceiver() {
    close();
}

bool OSCReceiver::setup(uint16_t port) {
    @autoreleasepool {
        if (impl_->listening) {
            close();
        }

        impl_->port = port;
        impl_->queue = dispatch_queue_create("com.oflike.osc.receiver", DISPATCH_QUEUE_SERIAL);

        // UDP listener 作成
        nw_parameters_t parameters = nw_parameters_create_secure_udp(
            NW_PARAMETERS_DISABLE_PROTOCOL,
            NW_PARAMETERS_DEFAULT_CONFIGURATION
        );

        NSString* portStr = [NSString stringWithFormat:@"%u", port];
        nw_endpoint_t endpoint = nw_endpoint_create_host("::", [portStr UTF8String]);

        impl_->listener = nw_listener_create_with_port([portStr UTF8String], parameters);

        if (!impl_->listener) {
            NSLog(@"[OSCReceiver] Failed to create listener on port %u", port);
            return false;
        }

        // 接続受信ハンドラー
        nw_listener_set_new_connection_handler(impl_->listener, ^(nw_connection_t connection) {
            nw_connection_set_queue(connection, impl_->queue);
            nw_connection_start(connection);

            // データ受信
            nw_connection_receive(connection, 1, 65536, ^(dispatch_data_t content, nw_content_context_t context, bool is_complete, nw_error_t error) {
                if (content) {
                    const void* buffer = nullptr;
                    size_t size = 0;
                    dispatch_data_t contiguous = dispatch_data_create_map(content, &buffer, &size);
                    if (buffer && size > 0) {
                        impl_->parseOSCPacket(static_cast<const uint8_t*>(buffer), size);
                    }
                    if (contiguous) {
                        // データは自動解放される
                    }
                }

                // 次のパケット受信を継続
                if (!is_complete && !error) {
                    nw_connection_receive(connection, 1, 65536, ^(dispatch_data_t c2, nw_content_context_t ctx2, bool complete2, nw_error_t err2) {
                        // 再帰的に受信継続
                    });
                }
            });
        });

        // リスナー状態ハンドラー
        nw_listener_set_state_changed_handler(impl_->listener, ^(nw_listener_state_t state, nw_error_t error) {
            switch (state) {
                case nw_listener_state_ready:
                    impl_->listening = true;
                    NSLog(@"[OSCReceiver] Listening on port %u", port);
                    break;
                case nw_listener_state_failed:
                    impl_->listening = false;
                    NSLog(@"[OSCReceiver] Failed: %@", error ? @(nw_error_get_error_code(error)) : @"Unknown");
                    break;
                case nw_listener_state_cancelled:
                    impl_->listening = false;
                    NSLog(@"[OSCReceiver] Cancelled");
                    break;
                default:
                    break;
            }
        });

        nw_listener_set_queue(impl_->listener, impl_->queue);
        nw_listener_start(impl_->listener);

        return true;
    }
}

void OSCReceiver::close() {
    if (impl_->listener) {
        nw_listener_cancel(impl_->listener);
        impl_->listener = nullptr;
    }
    if (impl_->queue) {
        impl_->queue = nullptr;
    }
    impl_->listening = false;
}

bool OSCReceiver::isListening() const {
    return impl_->listening;
}

void OSCReceiver::addMessageHandler(const std::string& address, OSCMessageCallback callback) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->handlers[address] = callback;
}

void OSCReceiver::removeMessageHandler(const std::string& address) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->handlers.erase(address);
}

void OSCReceiver::update() {
    // Network framework は非同期なので、ここでは何もしない
}

// OSCSender::Impl
struct OSCSender::Impl {
    nw_connection_t connection = nullptr;
    dispatch_queue_t queue = nullptr;
    bool connected = false;
    std::string host;
    uint16_t port = 0;
};

OSCSender::OSCSender() : impl_(std::make_unique<Impl>()) {}
OSCSender::~OSCSender() {
    close();
}

bool OSCSender::setup(const std::string& host, uint16_t port) {
    @autoreleasepool {
        if (impl_->connected) {
            close();
        }

        impl_->host = host;
        impl_->port = port;
        impl_->queue = dispatch_queue_create("com.oflike.osc.sender", DISPATCH_QUEUE_SERIAL);

        NSString* hostStr = [NSString stringWithUTF8String:host.c_str()];
        NSString* portStr = [NSString stringWithFormat:@"%u", port];

        nw_endpoint_t endpoint = nw_endpoint_create_host([hostStr UTF8String], [portStr UTF8String]);
        nw_parameters_t parameters = nw_parameters_create_secure_udp(
            NW_PARAMETERS_DISABLE_PROTOCOL,
            NW_PARAMETERS_DEFAULT_CONFIGURATION
        );

        impl_->connection = nw_connection_create(endpoint, parameters);
        if (!impl_->connection) {
            NSLog(@"[OSCSender] Failed to create connection to %s:%u", host.c_str(), port);
            return false;
        }

        nw_connection_set_queue(impl_->connection, impl_->queue);

        // 接続状態ハンドラー
        nw_connection_set_state_changed_handler(impl_->connection, ^(nw_connection_state_t state, nw_error_t error) {
            switch (state) {
                case nw_connection_state_ready:
                    impl_->connected = true;
                    NSLog(@"[OSCSender] Connected to %s:%u", impl_->host.c_str(), impl_->port);
                    break;
                case nw_connection_state_failed:
                    impl_->connected = false;
                    NSLog(@"[OSCSender] Failed: %@", error ? @(nw_error_get_error_code(error)) : @"Unknown");
                    break;
                case nw_connection_state_cancelled:
                    impl_->connected = false;
                    NSLog(@"[OSCSender] Cancelled");
                    break;
                default:
                    break;
            }
        });

        nw_connection_start(impl_->connection);

        return true;
    }
}

void OSCSender::close() {
    if (impl_->connection) {
        nw_connection_cancel(impl_->connection);
        impl_->connection = nullptr;
    }
    if (impl_->queue) {
        impl_->queue = nullptr;
    }
    impl_->connected = false;
}

bool OSCSender::isConnected() const {
    return impl_->connected;
}

bool OSCSender::sendMessage(const OSCMessage& message) {
    if (!impl_->connected || !impl_->connection) {
        return false;
    }

    // OSC パケット生成
    std::vector<uint8_t> packet;

    // アドレス
    OSCUtil::encodeString(packet, message.getAddress());

    // タイプタグ
    std::string typeTags = ",";
    for (size_t i = 0; i < message.getArgCount(); ++i) {
        OSCArgType type = message.getArgType(i);
        switch (type) {
            case OSCArgType::INT32:  typeTags += 'i'; break;
            case OSCArgType::FLOAT:  typeTags += 'f'; break;
            case OSCArgType::STRING: typeTags += 's'; break;
            default: break;
        }
    }
    OSCUtil::encodeString(packet, typeTags);

    // 引数
    for (size_t i = 0; i < message.getArgCount(); ++i) {
        const OSCArg& arg = message.getArg(i);
        switch (arg.type) {
            case OSCArgType::INT32:
                OSCUtil::encodeInt32(packet, arg.asInt());
                break;
            case OSCArgType::FLOAT:
                OSCUtil::encodeFloat(packet, arg.asFloat());
                break;
            case OSCArgType::STRING:
                OSCUtil::encodeString(packet, arg.asString());
                break;
            default:
                break;
        }
    }

    // 送信
    @autoreleasepool {
        dispatch_data_t data = dispatch_data_create(packet.data(), packet.size(), impl_->queue, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
        nw_connection_send(impl_->connection, data, NW_CONNECTION_DEFAULT_MESSAGE_CONTEXT, true, ^(nw_error_t error) {
            if (error) {
                NSLog(@"[OSCSender] Send failed: %@", @(nw_error_get_error_code(error)));
            }
        });
    }

    return true;
}

// OSCParameterMapper::Impl
struct OSCParameterMapper::Impl {
    struct MappingInfo {
        enum Type { FLOAT, INT, BOOL } type;
        void* ptr;
        float minVal;
        float maxVal;
    };

    std::map<std::string, MappingInfo> mappings;
    std::mutex mutex;
    OSCReceiver* receiver = nullptr;

    void handleMessage(const OSCMessage& message);
};

void OSCParameterMapper::Impl::handleMessage(const OSCMessage& message) {
    std::lock_guard<std::mutex> lock(mutex);

    auto it = mappings.find(message.getAddress());
    if (it == mappings.end()) return;

    const MappingInfo& info = it->second;

    if (message.getArgCount() == 0) return;

    switch (info.type) {
        case MappingInfo::FLOAT: {
            float value = message.getArgAsFloat(0);
            // 正規化（TouchOSC等は 0.0-1.0 で送る）
            float mapped = info.minVal + value * (info.maxVal - info.minVal);
            *static_cast<float*>(info.ptr) = mapped;
            break;
        }
        case MappingInfo::INT: {
            int32_t value = message.getArgAsInt(0);
            *static_cast<int32_t*>(info.ptr) = value;
            break;
        }
        case MappingInfo::BOOL: {
            int32_t value = message.getArgAsInt(0);
            *static_cast<bool*>(info.ptr) = (value != 0);
            break;
        }
    }
}

OSCParameterMapper::OSCParameterMapper() : impl_(std::make_unique<Impl>()) {}
OSCParameterMapper::~OSCParameterMapper() = default;

void OSCParameterMapper::mapFloat(const std::string& oscAddress, float* targetPtr, float minVal, float maxVal) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    OSCParameterMapper::Impl::MappingInfo info;
    info.type = OSCParameterMapper::Impl::MappingInfo::FLOAT;
    info.ptr = targetPtr;
    info.minVal = minVal;
    info.maxVal = maxVal;
    impl_->mappings[oscAddress] = info;
}

void OSCParameterMapper::mapInt(const std::string& oscAddress, int32_t* targetPtr, int32_t minVal, int32_t maxVal) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    OSCParameterMapper::Impl::MappingInfo info;
    info.type = OSCParameterMapper::Impl::MappingInfo::INT;
    info.ptr = targetPtr;
    info.minVal = static_cast<float>(minVal);
    info.maxVal = static_cast<float>(maxVal);
    impl_->mappings[oscAddress] = info;
}

void OSCParameterMapper::mapBool(const std::string& oscAddress, bool* targetPtr) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    OSCParameterMapper::Impl::MappingInfo info;
    info.type = OSCParameterMapper::Impl::MappingInfo::BOOL;
    info.ptr = targetPtr;
    info.minVal = 0.0f;
    info.maxVal = 1.0f;
    impl_->mappings[oscAddress] = info;
}

void OSCParameterMapper::unmapParameter(const std::string& oscAddress) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->mappings.erase(oscAddress);
}

void OSCParameterMapper::clearAllMappings() {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->mappings.clear();
}

void OSCParameterMapper::attachToReceiver(OSCReceiver& receiver) {
    impl_->receiver = &receiver;

    // 全てのマッピングアドレスにハンドラー登録
    std::lock_guard<std::mutex> lock(impl_->mutex);
    for (const auto& pair : impl_->mappings) {
        receiver.addMessageHandler(pair.first, [this](const OSCMessage& msg) {
            impl_->handleMessage(msg);
        });
    }
}

} // namespace oflike
