#include "ofxTcpServer.h"
#import <Foundation/Foundation.h>
#import <Network/Network.h>
#include <queue>
#include <mutex>

struct ClientConnection {
    nw_connection_t connection = nullptr;
    std::string remoteIP;
    uint16_t remotePort = 0;
    bool connected = false;
    std::queue<std::vector<uint8_t>> receiveQueue;
    std::mutex receiveMutex;
    std::string errorMessage;
};

struct ofxTcpServer::Impl {
    nw_listener_t listener = nullptr;
    dispatch_queue_t queue = nullptr;

    std::string address;
    uint16_t port = 0;
    bool listening = false;
    int maxClients = 10;

    std::vector<std::unique_ptr<ClientConnection>> clients;
    std::mutex clientsMutex;

    std::string errorMessage;

    ~Impl() {
        cleanup();
    }

    void cleanup() {
        // Close all client connections
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            for (auto& client : clients) {
                if (client && client->connection) {
                    nw_connection_cancel(client->connection);
                }
            }
            clients.clear();
        }

        // Stop listener
        if (listener) {
            nw_listener_cancel(listener);
            listener = nullptr;
        }

        if (queue) {
            queue = nullptr;  // ARC handles release
        }

        listening = false;
    }

    void setupClientReceive(ClientConnection* client) {
        if (!client || !client->connection) return;

        nw_connection_receive(
            client->connection,
            1,      // minimum_incomplete_length
            65536,  // maximum_length
            ^(dispatch_data_t content, nw_content_context_t context, bool is_complete, nw_error_t error) {
                if (content && !error) {
                    const void* buffer;
                    size_t size;
                    dispatch_data_t contiguous = dispatch_data_create_map(content, &buffer, &size);

                    if (buffer && size > 0) {
                        std::lock_guard<std::mutex> lock(client->receiveMutex);
                        std::vector<uint8_t> data(static_cast<const uint8_t*>(buffer),
                                                  static_cast<const uint8_t*>(buffer) + size);
                        client->receiveQueue.push(std::move(data));
                    }

                    // Continue receiving
                    setupClientReceive(client);
                }
            }
        );
    }
};

ofxTcpServer::ofxTcpServer()
    : pImpl(std::make_unique<Impl>()) {
}

ofxTcpServer::~ofxTcpServer() {
    close();
}

ofxTcpServer::ofxTcpServer(ofxTcpServer&& other) noexcept
    : pImpl(std::move(other.pImpl)) {
}

ofxTcpServer& ofxTcpServer::operator=(ofxTcpServer&& other) noexcept {
    if (this != &other) {
        close();
        pImpl = std::move(other.pImpl);
    }
    return *this;
}

bool ofxTcpServer::setup(uint16_t port) {
    return setup("0.0.0.0", port);
}

bool ofxTcpServer::setup(const std::string& address, uint16_t port) {
    @autoreleasepool {
        close();

        pImpl->address = address;
        pImpl->port = port;
        pImpl->errorMessage.clear();

        // Create dispatch queue
        pImpl->queue = dispatch_queue_create("com.oflike.tcp.server", DISPATCH_QUEUE_SERIAL);
        if (!pImpl->queue) {
            pImpl->errorMessage = "Failed to create dispatch queue";
            return false;
        }

        // Create listener parameters
        nw_parameters_t parameters = nw_parameters_create_secure_tcp(
            NW_PARAMETERS_DISABLE_PROTOCOL,  // Disable TLS for plain TCP
            NW_PARAMETERS_DEFAULT_CONFIGURATION
        );

        if (!parameters) {
            pImpl->errorMessage = "Failed to create parameters";
            return false;
        }

        // Configure to accept connections on all interfaces
        nw_parameters_set_local_endpoint(
            parameters,
            nw_endpoint_create_host([NSString stringWithFormat:@"%u", port].UTF8String, "0")
        );

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
                    pImpl->listening = true;
                    break;
                case nw_listener_state_failed:
                    pImpl->listening = false;
                    if (error) {
                        pImpl->errorMessage = "Listener failed";
                    }
                    break;
                case nw_listener_state_cancelled:
                    pImpl->listening = false;
                    break;
                default:
                    break;
            }
        });

        // Set new connection handler
        nw_listener_set_new_connection_handler(pImpl->listener, ^(nw_connection_t connection) {
            std::lock_guard<std::mutex> lock(pImpl->clientsMutex);

            // Check if we've reached max clients
            if (pImpl->clients.size() >= static_cast<size_t>(pImpl->maxClients)) {
                nw_connection_cancel(connection);
                return;
            }

            // Create client connection object
            auto client = std::make_unique<ClientConnection>();
            client->connection = connection;

            // Get remote endpoint info
            nw_endpoint_t endpoint = nw_connection_copy_endpoint(connection);
            if (endpoint) {
                const char* hostname = nw_endpoint_get_hostname(endpoint);
                uint16_t clientPort = nw_endpoint_get_port(endpoint);
                if (hostname) {
                    client->remoteIP = std::string(hostname);
                }
                client->remotePort = clientPort;
            }

            // Get raw pointer for block capture (lifetime managed by pImpl->clients)
            ClientConnection* clientPtr = client.get();
            Impl* implPtr = pImpl.get();

            // Set up state handler for client
            nw_connection_set_state_changed_handler(connection, ^(nw_connection_state_t state, nw_error_t error) {
                switch (state) {
                    case nw_connection_state_ready:
                        clientPtr->connected = true;
                        // Start receiving data
                        implPtr->setupClientReceive(clientPtr);
                        break;
                    case nw_connection_state_failed:
                    case nw_connection_state_cancelled:
                        clientPtr->connected = false;
                        break;
                    default:
                        break;
                }
            });

            // Set queue and start
            nw_connection_set_queue(connection, pImpl->queue);
            nw_connection_start(connection);

            pImpl->clients.push_back(std::move(client));
        });

        // Start listener
        nw_listener_start(pImpl->listener);

        // Wait briefly for listener to become ready
        usleep(10000);  // 10ms

        return true;
    }
}

void ofxTcpServer::close() {
    if (pImpl) {
        pImpl->cleanup();
    }
}

bool ofxTcpServer::isListening() const {
    return pImpl && pImpl->listening;
}

bool ofxTcpServer::waitForClient(int timeoutSeconds) {
    if (!isListening()) {
        return false;
    }

    int initialCount = getNumClients();
    int iterations = (timeoutSeconds > 0) ? (timeoutSeconds * 100) : -1;
    int count = 0;

    while (iterations < 0 || count < iterations) {
        if (getNumClients() > initialCount) {
            return true;
        }
        usleep(10000);  // 10ms
        count++;
    }

    return false;
}

int ofxTcpServer::getNumClients() const {
    std::lock_guard<std::mutex> lock(pImpl->clientsMutex);
    return static_cast<int>(pImpl->clients.size());
}

bool ofxTcpServer::isClientConnected(int clientIndex) const {
    std::lock_guard<std::mutex> lock(pImpl->clientsMutex);
    if (clientIndex < 0 || clientIndex >= static_cast<int>(pImpl->clients.size())) {
        return false;
    }
    return pImpl->clients[clientIndex]->connected;
}

void ofxTcpServer::disconnectClient(int clientIndex) {
    std::lock_guard<std::mutex> lock(pImpl->clientsMutex);
    if (clientIndex < 0 || clientIndex >= static_cast<int>(pImpl->clients.size())) {
        return;
    }

    auto& client = pImpl->clients[clientIndex];
    if (client->connection) {
        nw_connection_cancel(client->connection);
        client->connection = nullptr;
    }
    client->connected = false;

    // Remove client from list
    pImpl->clients.erase(pImpl->clients.begin() + clientIndex);
}

std::string ofxTcpServer::getClientIP(int clientIndex) const {
    std::lock_guard<std::mutex> lock(pImpl->clientsMutex);
    if (clientIndex < 0 || clientIndex >= static_cast<int>(pImpl->clients.size())) {
        return "";
    }
    return pImpl->clients[clientIndex]->remoteIP;
}

uint16_t ofxTcpServer::getClientPort(int clientIndex) const {
    std::lock_guard<std::mutex> lock(pImpl->clientsMutex);
    if (clientIndex < 0 || clientIndex >= static_cast<int>(pImpl->clients.size())) {
        return 0;
    }
    return pImpl->clients[clientIndex]->remotePort;
}

int ofxTcpServer::send(int clientIndex, const std::string& data) {
    return sendRawBytes(clientIndex, data.c_str(), static_cast<int>(data.size()));
}

int ofxTcpServer::sendRawBytes(int clientIndex, const char* bytes, int size) {
    @autoreleasepool {
        std::lock_guard<std::mutex> lock(pImpl->clientsMutex);

        if (clientIndex < 0 || clientIndex >= static_cast<int>(pImpl->clients.size())) {
            return -1;
        }

        auto& client = pImpl->clients[clientIndex];
        if (!client->connected || !client->connection) {
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
            return -1;
        }

        __block bool completed = false;
        __block bool success = false;

        // Send data
        nw_connection_send(
            client->connection,
            data,
            NW_CONNECTION_DEFAULT_MESSAGE_CONTEXT,
            true,  // is_complete
            ^(nw_error_t error) {
                success = (error == nullptr);
                completed = true;
            }
        );

        return success ? size : -1;
    }
}

void ofxTcpServer::sendToAll(const std::string& data) {
    sendRawBytesToAll(data.c_str(), static_cast<int>(data.size()));
}

void ofxTcpServer::sendRawBytesToAll(const char* bytes, int size) {
    int numClients = getNumClients();
    for (int i = 0; i < numClients; i++) {
        sendRawBytes(i, bytes, size);
    }
}

std::string ofxTcpServer::receive(int clientIndex) {
    std::lock_guard<std::mutex> lock(pImpl->clientsMutex);

    if (clientIndex < 0 || clientIndex >= static_cast<int>(pImpl->clients.size())) {
        return "";
    }

    auto& client = pImpl->clients[clientIndex];

    // Get data from queue
    std::lock_guard<std::mutex> recvLock(client->receiveMutex);
    if (client->receiveQueue.empty()) {
        return "";
    }

    std::vector<uint8_t> data = std::move(client->receiveQueue.front());
    client->receiveQueue.pop();

    return std::string(data.begin(), data.end());
}

int ofxTcpServer::receiveRawBytes(int clientIndex, char* buffer, int maxSize) {
    std::string data = receive(clientIndex);
    if (data.empty()) {
        return 0;
    }

    int bytesToCopy = std::min(static_cast<int>(data.size()), maxSize);
    std::memcpy(buffer, data.c_str(), bytesToCopy);
    return bytesToCopy;
}

int ofxTcpServer::getNumReceivedBytes(int clientIndex) const {
    std::lock_guard<std::mutex> lock(pImpl->clientsMutex);

    if (clientIndex < 0 || clientIndex >= static_cast<int>(pImpl->clients.size())) {
        return 0;
    }

    auto& client = pImpl->clients[clientIndex];
    std::lock_guard<std::mutex> recvLock(client->receiveMutex);

    int total = 0;
    std::queue<std::vector<uint8_t>> temp = client->receiveQueue;
    while (!temp.empty()) {
        total += temp.front().size();
        temp.pop();
    }

    return total;
}

void ofxTcpServer::setMaxClients(int maxClients) {
    pImpl->maxClients = maxClients;
}

int ofxTcpServer::getMaxClients() const {
    return pImpl->maxClients;
}

std::string ofxTcpServer::getError() const {
    return pImpl->errorMessage;
}

uint16_t ofxTcpServer::getPort() const {
    return pImpl->port;
}
