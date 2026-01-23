#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cstdint>
#include <memory>

namespace oflike {

// OSC データ型
enum class OSCArgType {
    INT32,
    FLOAT,
    STRING,
    BLOB
};

// OSC 引数
struct OSCArg {
    OSCArgType type;
    union {
        int32_t i;
        float f;
    };
    std::string s;  // STRING or BLOB

    OSCArg() : type(OSCArgType::INT32), i(0) {}
    explicit OSCArg(int32_t val) : type(OSCArgType::INT32), i(val) {}
    explicit OSCArg(float val) : type(OSCArgType::FLOAT), f(val) {}
    explicit OSCArg(const std::string& val) : type(OSCArgType::STRING), s(val) {}
    explicit OSCArg(const char* val) : type(OSCArgType::STRING), s(val) {}

    int32_t asInt() const { return (type == OSCArgType::INT32) ? i : 0; }
    float asFloat() const { return (type == OSCArgType::FLOAT) ? f : 0.0f; }
    const std::string& asString() const { return s; }
};

// OSC メッセージ
class OSCMessage {
public:
    OSCMessage();
    explicit OSCMessage(const std::string& address);
    ~OSCMessage();

    // アドレス設定
    void setAddress(const std::string& address);
    const std::string& getAddress() const;

    // 引数追加
    void addInt(int32_t value);
    void addFloat(float value);
    void addString(const std::string& value);

    // 引数取得
    size_t getArgCount() const;
    OSCArgType getArgType(size_t index) const;
    int32_t getArgAsInt(size_t index) const;
    float getArgAsFloat(size_t index) const;
    const std::string& getArgAsString(size_t index) const;
    const OSCArg& getArg(size_t index) const;

    // クリア
    void clear();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// OSC 受信コールバック
using OSCMessageCallback = std::function<void(const OSCMessage&)>;

// OSC 受信機
class OSCReceiver {
public:
    OSCReceiver();
    ~OSCReceiver();

    // ポート設定
    bool setup(uint16_t port);
    void close();
    bool isListening() const;

    // メッセージハンドラー登録
    void addMessageHandler(const std::string& address, OSCMessageCallback callback);
    void removeMessageHandler(const std::string& address);

    // 内部処理（フレーム毎に呼ばれる）
    void update();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// OSC 送信機
class OSCSender {
public:
    OSCSender();
    ~OSCSender();

    // 送信先設定
    bool setup(const std::string& host, uint16_t port);
    void close();
    bool isConnected() const;

    // メッセージ送信
    bool sendMessage(const OSCMessage& message);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// パラメータマッピング（DebugOverlay連携用）
class OSCParameterMapper {
public:
    OSCParameterMapper();
    ~OSCParameterMapper();

    // Float パラメータマッピング
    void mapFloat(const std::string& oscAddress, float* targetPtr, float minVal, float maxVal);

    // Int パラメータマッピング
    void mapInt(const std::string& oscAddress, int32_t* targetPtr, int32_t minVal, int32_t maxVal);

    // Bool パラメータマッピング
    void mapBool(const std::string& oscAddress, bool* targetPtr);

    // マッピング解除
    void unmapParameter(const std::string& oscAddress);
    void clearAllMappings();

    // OSC Receiver と接続
    void attachToReceiver(OSCReceiver& receiver);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace oflike
