#pragma once

#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include <functional>

/// \brief TCP Server using Network.framework
///
/// ofxTcpServer provides a simple interface for TCP server connections.
/// It uses Apple's Network.framework for asynchronous, non-blocking I/O.
///
/// Example usage:
/// \code
/// ofxTcpServer server;
/// if (server.setup(8080)) {
///     server.waitForClient();
///     if (server.isClientConnected(0)) {
///         std::string data = server.receive(0);
///         server.send(0, "Response");
///     }
/// }
/// server.close();
/// \endcode
class ofxTcpServer {
public:
    ofxTcpServer();
    ~ofxTcpServer();

    // Non-copyable, movable
    ofxTcpServer(const ofxTcpServer&) = delete;
    ofxTcpServer& operator=(const ofxTcpServer&) = delete;
    ofxTcpServer(ofxTcpServer&&) noexcept;
    ofxTcpServer& operator=(ofxTcpServer&&) noexcept;

    // Server setup
    /// \brief Setup server on specified port
    /// \param port Port number to listen on
    /// \return true if server started successfully
    bool setup(uint16_t port);

    /// \brief Setup server on specified address and port
    /// \param address IP address to bind to (e.g., "0.0.0.0" for all interfaces)
    /// \param port Port number to listen on
    /// \return true if server started successfully
    bool setup(const std::string& address, uint16_t port);

    /// \brief Close server and all connections
    void close();

    /// \brief Check if server is listening
    /// \return true if listening
    bool isListening() const;

    // Client management
    /// \brief Wait for a client connection (blocking)
    /// \param timeoutSeconds Timeout in seconds (default 0 = no timeout)
    /// \return true if client connected
    bool waitForClient(int timeoutSeconds = 0);

    /// \brief Get number of connected clients
    /// \return Number of clients
    int getNumClients() const;

    /// \brief Check if specific client is connected
    /// \param clientIndex Client index
    /// \return true if connected
    bool isClientConnected(int clientIndex) const;

    /// \brief Disconnect specific client
    /// \param clientIndex Client index
    void disconnectClient(int clientIndex);

    /// \brief Get client IP address
    /// \param clientIndex Client index
    /// \return IP address as string
    std::string getClientIP(int clientIndex) const;

    /// \brief Get client port
    /// \param clientIndex Client index
    /// \return Port number
    uint16_t getClientPort(int clientIndex) const;

    // Sending
    /// \brief Send data to specific client
    /// \param clientIndex Client index
    /// \param data Data to send
    /// \return Number of bytes sent, or -1 on error
    int send(int clientIndex, const std::string& data);

    /// \brief Send raw bytes to specific client
    /// \param clientIndex Client index
    /// \param bytes Data buffer
    /// \param size Number of bytes to send
    /// \return Number of bytes sent, or -1 on error
    int sendRawBytes(int clientIndex, const char* bytes, int size);

    /// \brief Send to all connected clients
    /// \param data Data to send
    void sendToAll(const std::string& data);

    /// \brief Send raw bytes to all connected clients
    /// \param bytes Data buffer
    /// \param size Number of bytes to send
    void sendRawBytesToAll(const char* bytes, int size);

    // Receiving
    /// \brief Receive data from specific client (non-blocking)
    /// \param clientIndex Client index
    /// \return Received data as string
    std::string receive(int clientIndex);

    /// \brief Receive raw bytes from specific client (non-blocking)
    /// \param clientIndex Client index
    /// \param buffer Buffer to store received data
    /// \param maxSize Maximum number of bytes to receive
    /// \return Number of bytes received, 0 if no data, -1 on error
    int receiveRawBytes(int clientIndex, char* buffer, int maxSize);

    /// \brief Get number of bytes available from specific client
    /// \param clientIndex Client index
    /// \return Number of bytes available
    int getNumReceivedBytes(int clientIndex) const;

    // Settings
    /// \brief Set maximum number of clients
    /// \param maxClients Maximum clients (default 10)
    void setMaxClients(int maxClients);

    /// \brief Get maximum number of clients
    /// \return Maximum clients
    int getMaxClients() const;

    // Status
    /// \brief Get last error message
    /// \return Error message
    std::string getError() const;

    /// \brief Get server port
    /// \return Port number
    uint16_t getPort() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
