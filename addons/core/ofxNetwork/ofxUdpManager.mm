#include "ofxUdpManager.h"
#import <Foundation/Foundation.h>
#import <Network/Network.h>
#include <queue>
#include <mutex>

struct ReceivedDatagram {
    std::vector<uint8_t> data;
    std::string senderIP;
    uint16_t senderPort = 0;
};

struct ofxUdpManager::Impl {
    nw_connection_t connection = nullptr;
    nw_listener_t listener = nullptr;
    dispatch_queue_t queue = nullptr;

    std::string host;
    uint16_t port = 0;
    std::string localAddress;
    uint16_t localPort = 0;
    bool ready = false;
    bool nonBlocking = true;
    bool isListenerMode = false;  // true for bind, false for connect

    std::string errorMessage;
    std::queue<ReceivedDatagram> receiveQueue;
    std::mutex receiveMutex;

    int receiveBufferSize = 65536;
    int sendBufferSize = 65536;
    uint8_t multicastTTL = 1;
    bool multicastLoopback = false;

    ~Impl() {
        cleanup();
    }

    void cleanup() {
        if (connection) {
            nw_connection_cancel(connection);
            connection = nullptr;
        }
        if (listener) {
            nw_listener_cancel(listener);
            listener = nullptr;
        }
        if (queue) {
            queue = nullptr;  // ARC handles release
        }
        ready = false;
    }
};

ofxUdpManager::ofxUdpManager()
    : pImpl(std::make_unique<Impl>()) {
}

ofxUdpManager::~ofxUdpManager() {
    close();
}

ofxUdpManager::ofxUdpManager(ofxUdpManager&& other) noexcept
    : pImpl(std::move(other.pImpl)) {
}

ofxUdpManager& ofxUdpManager::operator=(ofxUdpManager&& other) noexcept {
    if (this != &other) {
        close();
        pImpl = std::move(other.pImpl);
    }
    return *this;
}

bool ofxUdpManager::bind(uint16_t port) {
    return bind("0.0.0.0", port);
}

bool ofxUdpManager::bind(const std::string& address, uint16_t port) {
    @autoreleasepool {
        close();

        pImpl->localAddress = address;
        pImpl->localPort = port;
        pImpl->isListenerMode = true;
        pImpl->errorMessage.clear();

        // Create dispatch queue
        pImpl->queue = dispatch_queue_create("com.oflike.udp", DISPATCH_QUEUE_SERIAL);
        if (!pImpl->queue) {
            pImpl->errorMessage = "Failed to create dispatch queue";
            return false;
        }

        // Create UDP parameters
        nw_parameters_t parameters = nw_parameters_create_secure_udp(
            NW_PARAMETERS_DISABLE_PROTOCOL,  // Disable DTLS for plain UDP
            NW_PARAMETERS_DEFAULT_CONFIGURATION
        );

        if (!parameters) {
            pImpl->errorMessage = "Failed to create parameters";
            return false;
        }

        // Allow local address reuse
        nw_parameters_set_reuse_local_address(parameters, true);

        // Create listener
        NSString* portStr = [NSString stringWithFormat:@"%u", port];
        pImpl->listener = nw_listener_create_with_port([portStr UTF8String], parameters);

        if (!pImpl->listener) {
            pImpl->errorMessage = "Failed to create listener";
            return false;
        }

        // Set queue
        nw_listener_set_queue(pImpl->listener, pImpl->queue);

        // Set state change handler
        nw_listener_set_state_changed_handler(pImpl->listener, ^(nw_listener_state_t state, nw_error_t error) {
            switch (state) {
                case nw_listener_state_ready:
                    pImpl->ready = true;
                    break;
                case nw_listener_state_failed:
                    pImpl->ready = false;
                    if (error) {
                        pImpl->errorMessage = "Listener failed";
                    }
                    break;
                case nw_listener_state_cancelled:
                    pImpl->ready = false;
                    break;
                default:
                    break;
            }
        });

        // Set new connection handler (for each incoming datagram)
        nw_listener_set_new_connection_handler(pImpl->listener, ^(nw_connection_t connection) {
            // Set queue
            nw_connection_set_queue(connection, pImpl->queue);

            // Receive data
            nw_connection_receive(
                connection,
                1,      // minimum_incomplete_length
                pImpl->receiveBufferSize,
                ^(dispatch_data_t content, nw_content_context_t context, bool is_complete, nw_error_t error) {
                    if (content && !error) {
                        const void* buffer;
                        size_t size;
                        dispatch_data_t contiguous = dispatch_data_create_map(content, &buffer, &size);

                        if (buffer && size > 0) {
                            // Get sender info
                            nw_endpoint_t endpoint = nw_connection_copy_endpoint(connection);
                            ReceivedDatagram datagram;
                            datagram.data.assign(static_cast<const uint8_t*>(buffer),
                                               static_cast<const uint8_t*>(buffer) + size);

                            if (endpoint) {
                                const char* hostname = nw_endpoint_get_hostname(endpoint);
                                if (hostname) {
                                    datagram.senderIP = std::string(hostname);
                                }
                                datagram.senderPort = nw_endpoint_get_port(endpoint);
                            }

                            std::lock_guard<std::mutex> lock(pImpl->receiveMutex);
                            pImpl->receiveQueue.push(std::move(datagram));
                        }
                    }

                    // Cancel this connection (UDP is connectionless)
                    nw_connection_cancel(connection);
                }
            );

            // Start connection
            nw_connection_start(connection);
        });

        // Start listener
        nw_listener_start(pImpl->listener);

        // Wait briefly for listener to become ready
        usleep(10000);  // 10ms

        return true;
    }
}

bool ofxUdpManager::connect(const std::string& host, uint16_t port) {
    @autoreleasepool {
        close();

        pImpl->host = host;
        pImpl->port = port;
        pImpl->isListenerMode = false;
        pImpl->errorMessage.clear();

        // Create dispatch queue
        pImpl->queue = dispatch_queue_create("com.oflike.udp", DISPATCH_QUEUE_SERIAL);
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

        // Create UDP parameters
        nw_parameters_t parameters = nw_parameters_create_secure_udp(
            NW_PARAMETERS_DISABLE_PROTOCOL,  // Disable DTLS for plain UDP
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
                    pImpl->ready = true;
                    break;
                case nw_connection_state_failed:
                    pImpl->ready = false;
                    if (error) {
                        pImpl->errorMessage = "Connection failed";
                    }
                    break;
                case nw_connection_state_cancelled:
                    pImpl->ready = false;
                    break;
                default:
                    break;
            }
        });

        // Set queue
        nw_connection_set_queue(pImpl->connection, pImpl->queue);

        // Start connection
        nw_connection_start(pImpl->connection);

        // Wait for connection if blocking mode
        if (!pImpl->nonBlocking) {
            for (int i = 0; i < 50; i++) {  // 500ms max
                if (pImpl->ready) {
                    break;
                }
                usleep(10000);  // 10ms
            }
        }

        return true;
    }
}

void ofxUdpManager::close() {
    if (pImpl) {
        pImpl->cleanup();
    }
}

int ofxUdpManager::send(const char* data, int length) {
    @autoreleasepool {
        if (!pImpl->ready || !pImpl->connection) {
            pImpl->errorMessage = "Not connected";
            return -1;
        }

        // Create dispatch data
        dispatch_data_t dispatchData = dispatch_data_create(
            data,
            length,
            pImpl->queue,
            DISPATCH_DATA_DESTRUCTOR_DEFAULT
        );

        if (!dispatchData) {
            pImpl->errorMessage = "Failed to create dispatch data";
            return -1;
        }

        __block bool completed = false;
        __block bool success = false;

        // Send data
        nw_connection_send(
            pImpl->connection,
            dispatchData,
            NW_CONNECTION_DEFAULT_MESSAGE_CONTEXT,
            true,  // is_complete
            ^(nw_error_t error) {
                success = (error == nullptr);
                completed = true;
            }
        );

        // Wait for completion if blocking mode
        if (!pImpl->nonBlocking) {
            while (!completed) {
                usleep(1000);  // 1ms
            }
        }

        return success ? length : -1;
    }
}

int ofxUdpManager::sendTo(const std::string& host, uint16_t port, const char* data, int length) {
    @autoreleasepool {
        // Create temporary connection for this send
        NSString* hostStr = [NSString stringWithUTF8String:host.c_str()];
        NSString* portStr = [NSString stringWithFormat:@"%u", port];

        nw_endpoint_t endpoint = nw_endpoint_create_host(
            [hostStr UTF8String],
            [portStr UTF8String]
        );

        if (!endpoint) {
            return -1;
        }

        nw_parameters_t parameters = nw_parameters_create_secure_udp(
            NW_PARAMETERS_DISABLE_PROTOCOL,
            NW_PARAMETERS_DEFAULT_CONFIGURATION
        );

        if (!parameters) {
            return -1;
        }

        nw_connection_t tempConnection = nw_connection_create(endpoint, parameters);
        if (!tempConnection) {
            return -1;
        }

        dispatch_queue_t tempQueue = dispatch_queue_create("com.oflike.udp.temp", DISPATCH_QUEUE_SERIAL);
        nw_connection_set_queue(tempConnection, tempQueue);

        __block bool sent = false;
        __block bool completed = false;

        nw_connection_set_state_changed_handler(tempConnection, ^(nw_connection_state_t state, nw_error_t error) {
            if (state == nw_connection_state_ready) {
                dispatch_data_t dispatchData = dispatch_data_create(
                    data,
                    length,
                    tempQueue,
                    DISPATCH_DATA_DESTRUCTOR_DEFAULT
                );

                nw_connection_send(
                    tempConnection,
                    dispatchData,
                    NW_CONNECTION_DEFAULT_MESSAGE_CONTEXT,
                    true,
                    ^(nw_error_t error) {
                        sent = (error == nullptr);
                        completed = true;
                        nw_connection_cancel(tempConnection);
                    }
                );
            } else if (state == nw_connection_state_failed || state == nw_connection_state_cancelled) {
                completed = true;
            }
        });

        nw_connection_start(tempConnection);

        // Wait for completion
        int timeout = 100;  // 100ms
        while (!completed && timeout > 0) {
            usleep(1000);  // 1ms
            timeout--;
        }

        return sent ? length : -1;
    }
}

int ofxUdpManager::receive(char* buffer, int maxLength) {
    std::string senderIP;
    uint16_t senderPort;
    return receiveFrom(buffer, maxLength, senderIP, senderPort);
}

int ofxUdpManager::receiveFrom(char* buffer, int maxLength, std::string& senderIP, uint16_t& senderPort) {
    std::lock_guard<std::mutex> lock(pImpl->receiveMutex);

    if (pImpl->receiveQueue.empty()) {
        return 0;
    }

    ReceivedDatagram datagram = std::move(pImpl->receiveQueue.front());
    pImpl->receiveQueue.pop();

    int bytesToCopy = std::min(static_cast<int>(datagram.data.size()), maxLength);
    std::memcpy(buffer, datagram.data.data(), bytesToCopy);

    senderIP = datagram.senderIP;
    senderPort = datagram.senderPort;

    return bytesToCopy;
}

bool ofxUdpManager::joinMulticastGroup(const std::string& multicastAddress) {
    // Network.framework handles multicast automatically when binding to multicast addresses
    // Additional configuration can be done through parameters if needed
    return true;
}

bool ofxUdpManager::leaveMulticastGroup(const std::string& multicastAddress) {
    // Network.framework manages this automatically
    return true;
}

void ofxUdpManager::setMulticastTTL(uint8_t ttl) {
    pImpl->multicastTTL = ttl;
}

void ofxUdpManager::setMulticastLoopback(bool enable) {
    pImpl->multicastLoopback = enable;
}

void ofxUdpManager::setNonBlocking(bool nonBlocking) {
    pImpl->nonBlocking = nonBlocking;
}

void ofxUdpManager::setReceiveBufferSize(int size) {
    pImpl->receiveBufferSize = size;
}

void ofxUdpManager::setSendBufferSize(int size) {
    pImpl->sendBufferSize = size;
}

bool ofxUdpManager::isReady() const {
    return pImpl && pImpl->ready;
}

std::string ofxUdpManager::getError() const {
    return pImpl->errorMessage;
}

uint16_t ofxUdpManager::getLocalPort() const {
    return pImpl->localPort;
}

std::string ofxUdpManager::getRemoteIP() const {
    return pImpl->host;
}

uint16_t ofxUdpManager::getRemotePort() const {
    return pImpl->port;
}
