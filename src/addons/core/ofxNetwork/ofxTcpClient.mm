#include "ofxTcpClient.h"
#import <Foundation/Foundation.h>
#import <Network/Network.h>
#include <queue>
#include <mutex>

struct ofxTcpClient::Impl {
    nw_connection_t connection = nullptr;
    dispatch_queue_t queue = nullptr;

    std::string host;
    uint16_t port = 0;
    bool connected = false;
    bool nonBlocking = true;
    int timeoutSeconds = 30;

    std::string errorMessage;
    std::queue<std::vector<uint8_t>> receiveQueue;
    std::mutex receiveMutex;
    std::string receiveBuffer;  // Buffer for partial messages

    ~Impl() {
        cleanup();
    }

    void cleanup() {
        if (connection) {
            nw_connection_cancel(connection);
            connection = nullptr;
        }
        if (queue) {
            queue = nullptr;  // ARC handles release
        }
        connected = false;
    }
};

ofxTcpClient::ofxTcpClient()
    : pImpl(std::make_unique<Impl>()) {
}

ofxTcpClient::~ofxTcpClient() {
    close();
}

ofxTcpClient::ofxTcpClient(ofxTcpClient&& other) noexcept
    : pImpl(std::move(other.pImpl)) {
}

ofxTcpClient& ofxTcpClient::operator=(ofxTcpClient&& other) noexcept {
    if (this != &other) {
        close();
        pImpl = std::move(other.pImpl);
    }
    return *this;
}

bool ofxTcpClient::setup(const std::string& host, uint16_t port) {
    @autoreleasepool {
        close();

        pImpl->host = host;
        pImpl->port = port;
        pImpl->errorMessage.clear();

        // Create dispatch queue
        pImpl->queue = dispatch_queue_create("com.oflike.tcp.client", DISPATCH_QUEUE_SERIAL);
        if (!pImpl->queue) {
            pImpl->errorMessage = "Failed to create dispatch queue";
            return false;
        }

        // Create endpoint
        NSString* hostStr = [NSString stringWithUTF8String:host.c_str()];
        NSString* portStr = [NSString stringWithFormat:@"%u", port];

        nw_endpoint_t endpoint = nw_endpoint_create_host(
            [hostStr UTF8String],
            [portStr UTF8String]
        );

        if (!endpoint) {
            pImpl->errorMessage = "Failed to create endpoint";
            return false;
        }

        // Create connection parameters
        nw_parameters_t parameters = nw_parameters_create_secure_tcp(
            NW_PARAMETERS_DISABLE_PROTOCOL,  // Disable TLS for plain TCP
            NW_PARAMETERS_DEFAULT_CONFIGURATION
        );

        if (!parameters) {
            pImpl->errorMessage = "Failed to create parameters";
            return false;
        }

        // Create connection
        pImpl->connection = nw_connection_create(endpoint, parameters);
        if (!pImpl->connection) {
            pImpl->errorMessage = "Failed to create connection";
            return false;
        }

        // Set up state handler
        nw_connection_set_state_changed_handler(pImpl->connection, ^(nw_connection_state_t state, nw_error_t error) {
            switch (state) {
                case nw_connection_state_ready:
                    pImpl->connected = true;
                    break;
                case nw_connection_state_failed:
                    pImpl->connected = false;
                    if (error) {
                        pImpl->errorMessage = "Connection failed";
                    }
                    break;
                case nw_connection_state_cancelled:
                    pImpl->connected = false;
                    break;
                default:
                    break;
            }
        });

        // Set queue
        nw_connection_set_queue(pImpl->connection, pImpl->queue);

        // Start connection
        nw_connection_start(pImpl->connection);

        // Wait for connection (with timeout) if blocking mode
        if (!pImpl->nonBlocking) {
            for (int i = 0; i < pImpl->timeoutSeconds * 10; i++) {
                if (pImpl->connected) {
                    break;
                }
                usleep(100000);  // 100ms
            }
        }

        return true;
    }
}

void ofxTcpClient::close() {
    if (pImpl) {
        pImpl->cleanup();
    }
}

bool ofxTcpClient::isConnected() const {
    return pImpl && pImpl->connected;
}

int ofxTcpClient::send(const std::string& data) {
    return sendRawBytes(data.c_str(), static_cast<int>(data.size()));
}

int ofxTcpClient::sendRawBytes(const char* bytes, int size) {
    @autoreleasepool {
        if (!isConnected() || !pImpl->connection) {
            pImpl->errorMessage = "Not connected";
            return -1;
        }

        // Create dispatch data
        dispatch_data_t data = dispatch_data_create(
            bytes,
            size,
            pImpl->queue,
            DISPATCH_DATA_DESTRUCTOR_DEFAULT
        );

        if (!data) {
            pImpl->errorMessage = "Failed to create dispatch data";
            return -1;
        }

        __block bool completed = false;
        __block bool success = false;

        // Send data
        nw_connection_send(
            pImpl->connection,
            data,
            NW_CONNECTION_DEFAULT_MESSAGE_CONTEXT,
            true,  // is_complete
            ^(nw_error_t error) {
                if (error) {
                    pImpl->errorMessage = "Send failed";
                    success = false;
                } else {
                    success = true;
                }
                completed = true;
            }
        );

        // Wait for completion if blocking mode
        if (!pImpl->nonBlocking) {
            while (!completed) {
                usleep(1000);  // 1ms
            }
        }

        return success ? size : -1;
    }
}

int ofxTcpClient::sendMessage(const std::string& message, const std::string& delimiter) {
    std::string fullMessage = message + delimiter;
    return send(fullMessage);
}

std::string ofxTcpClient::receive() {
    @autoreleasepool {
        if (!isConnected() || !pImpl->connection) {
            return "";
        }

        // Schedule receive
        nw_connection_receive(
            pImpl->connection,
            1,      // minimum_incomplete_length
            65536,  // maximum_length
            ^(dispatch_data_t content, nw_content_context_t context, bool is_complete, nw_error_t error) {
                if (content && !error) {
                    const void* buffer;
                    size_t size;
                    dispatch_data_t contiguous = dispatch_data_create_map(content, &buffer, &size);

                    if (buffer && size > 0) {
                        std::lock_guard<std::mutex> lock(pImpl->receiveMutex);
                        std::vector<uint8_t> data(static_cast<const uint8_t*>(buffer),
                                                  static_cast<const uint8_t*>(buffer) + size);
                        pImpl->receiveQueue.push(std::move(data));
                    }
                }
            }
        );

        // Get data from queue
        std::lock_guard<std::mutex> lock(pImpl->receiveMutex);
        if (pImpl->receiveQueue.empty()) {
            return "";
        }

        std::vector<uint8_t> data = std::move(pImpl->receiveQueue.front());
        pImpl->receiveQueue.pop();

        return std::string(data.begin(), data.end());
    }
}

int ofxTcpClient::receiveRawBytes(char* buffer, int maxSize) {
    std::string data = receive();
    if (data.empty()) {
        return 0;
    }

    int bytesToCopy = std::min(static_cast<int>(data.size()), maxSize);
    std::memcpy(buffer, data.c_str(), bytesToCopy);
    return bytesToCopy;
}

std::string ofxTcpClient::receiveMessage(const std::string& delimiter) {
    // Read new data and append to buffer
    std::string newData = receive();
    pImpl->receiveBuffer += newData;

    // Look for delimiter
    size_t pos = pImpl->receiveBuffer.find(delimiter);
    if (pos == std::string::npos) {
        return "";  // Delimiter not found yet
    }

    // Extract message
    std::string message = pImpl->receiveBuffer.substr(0, pos);
    pImpl->receiveBuffer = pImpl->receiveBuffer.substr(pos + delimiter.size());

    return message;
}

int ofxTcpClient::getNumReceivedBytes() const {
    std::lock_guard<std::mutex> lock(pImpl->receiveMutex);
    int total = 0;
    std::queue<std::vector<uint8_t>> temp = pImpl->receiveQueue;
    while (!temp.empty()) {
        total += temp.front().size();
        temp.pop();
    }
    return total + pImpl->receiveBuffer.size();
}

void ofxTcpClient::setTimeout(int seconds) {
    pImpl->timeoutSeconds = seconds;
}

void ofxTcpClient::setNonBlocking(bool nonBlocking) {
    pImpl->nonBlocking = nonBlocking;
}

std::string ofxTcpClient::getError() const {
    return pImpl->errorMessage;
}

std::string ofxTcpClient::getRemoteIP() const {
    @autoreleasepool {
        if (!pImpl->connection) {
            return "";
        }

        nw_endpoint_t endpoint = nw_connection_copy_endpoint(pImpl->connection);
        if (!endpoint) {
            return "";
        }

        const char* hostname = nw_endpoint_get_hostname(endpoint);
        if (!hostname) {
            return "";
        }

        return std::string(hostname);
    }
}

uint16_t ofxTcpClient::getRemotePort() const {
    return pImpl->port;
}
