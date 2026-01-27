#pragma once

#include <string>
#include <memory>
#include <vector>
#include <cstdint>

/// \brief TCP Client using Network.framework
///
/// ofxTcpClient provides a simple interface for TCP client connections.
/// It uses Apple's Network.framework for asynchronous, non-blocking I/O.
///
/// Example usage:
/// \code
/// ofxTcpClient client;
/// if (client.setup("localhost", 8080)) {
///     client.send("Hello Server");
///     std::string response = client.receive();
/// }
/// client.close();
/// \endcode
class ofxTcpClient {
public:
    ofxTcpClient();
    ~ofxTcpClient();

    // Non-copyable, movable
    ofxTcpClient(const ofxTcpClient&) = delete;
    ofxTcpClient& operator=(const ofxTcpClient&) = delete;
    ofxTcpClient(ofxTcpClient&&) noexcept;
    ofxTcpClient& operator=(ofxTcpClient&&) noexcept;

    // Connection
    /// \brief Connect to a TCP server
    /// \param host Hostname or IP address
    /// \param port Port number
    /// \return true if connection initiated successfully
    bool setup(const std::string& host, uint16_t port);

    /// \brief Close the connection
    void close();

    /// \brief Check if connected
    /// \return true if connected
    bool isConnected() const;

    // Sending
    /// \brief Send data
    /// \param data Data to send
    /// \return Number of bytes sent, or -1 on error
    int send(const std::string& data);

    /// \brief Send raw bytes
    /// \param bytes Data buffer
    /// \param size Number of bytes to send
    /// \return Number of bytes sent, or -1 on error
    int sendRawBytes(const char* bytes, int size);

    /// \brief Send message with delimiter
    /// \param message Message to send
    /// \param delimiter Delimiter to append (default "\n")
    /// \return Number of bytes sent, or -1 on error
    int sendMessage(const std::string& message, const std::string& delimiter = "\n");

    // Receiving
    /// \brief Receive data (non-blocking)
    /// \return Received data as string
    std::string receive();

    /// \brief Receive raw bytes (non-blocking)
    /// \param buffer Buffer to store received data
    /// \param maxSize Maximum number of bytes to receive
    /// \return Number of bytes received, 0 if no data, -1 on error
    int receiveRawBytes(char* buffer, int maxSize);

    /// \brief Receive message until delimiter (non-blocking)
    /// \param delimiter Delimiter to wait for (default "\n")
    /// \return Received message without delimiter, or empty if not complete
    std::string receiveMessage(const std::string& delimiter = "\n");

    /// \brief Get number of bytes available to read
    /// \return Number of bytes available
    int getNumReceivedBytes() const;

    // Settings
    /// \brief Set connection timeout
    /// \param seconds Timeout in seconds (default 30)
    void setTimeout(int seconds);

    /// \brief Set non-blocking mode (default true)
    /// \param nonBlocking true for non-blocking, false for blocking
    void setNonBlocking(bool nonBlocking);

    // Status
    /// \brief Get last error message
    /// \return Error message
    std::string getError() const;

    /// \brief Get remote IP address
    /// \return IP address as string
    std::string getRemoteIP() const;

    /// \brief Get remote port
    /// \return Port number
    uint16_t getRemotePort() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
