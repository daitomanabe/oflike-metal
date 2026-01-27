#pragma once

#include <string>
#include <memory>
#include <cstdint>

/// \brief UDP Manager using Network.framework
///
/// ofxUdpManager provides a simple interface for UDP communication.
/// It uses Apple's Network.framework for asynchronous, non-blocking I/O.
///
/// Example usage (receiver):
/// \code
/// ofxUdpManager udp;
/// udp.bind(8080);
/// if (udp.receive(buffer, bufferSize) > 0) {
///     // Process received data
/// }
/// \endcode
///
/// Example usage (sender):
/// \code
/// ofxUdpManager udp;
/// udp.connect("192.168.1.100", 8080);
/// udp.send("Hello UDP", 9);
/// \endcode
class ofxUdpManager {
public:
    ofxUdpManager();
    ~ofxUdpManager();

    // Non-copyable, movable
    ofxUdpManager(const ofxUdpManager&) = delete;
    ofxUdpManager& operator=(const ofxUdpManager&) = delete;
    ofxUdpManager(ofxUdpManager&&) noexcept;
    ofxUdpManager& operator=(ofxUdpManager&&) noexcept;

    // Setup
    /// \brief Bind to a port (for receiving)
    /// \param port Port number to bind to
    /// \return true if successful
    bool bind(uint16_t port);

    /// \brief Bind to a specific address and port (for receiving)
    /// \param address IP address to bind to (e.g., "0.0.0.0" for all interfaces)
    /// \param port Port number to bind to
    /// \return true if successful
    bool bind(const std::string& address, uint16_t port);

    /// \brief Connect to a remote host (for sending)
    /// \param host Hostname or IP address
    /// \param port Port number
    /// \return true if successful
    bool connect(const std::string& host, uint16_t port);

    /// \brief Close the connection
    void close();

    // Sending
    /// \brief Send data to connected host
    /// \param data Data buffer
    /// \param length Number of bytes to send
    /// \return Number of bytes sent, or -1 on error
    int send(const char* data, int length);

    /// \brief Send data to specific host (unconnected mode)
    /// \param host Hostname or IP address
    /// \param port Port number
    /// \param data Data buffer
    /// \param length Number of bytes to send
    /// \return Number of bytes sent, or -1 on error
    int sendTo(const std::string& host, uint16_t port, const char* data, int length);

    // Receiving
    /// \brief Receive data (non-blocking)
    /// \param buffer Buffer to store received data
    /// \param maxLength Maximum number of bytes to receive
    /// \return Number of bytes received, 0 if no data, -1 on error
    int receive(char* buffer, int maxLength);

    /// \brief Receive data with sender info (non-blocking)
    /// \param buffer Buffer to store received data
    /// \param maxLength Maximum number of bytes to receive
    /// \param senderIP Output parameter for sender IP
    /// \param senderPort Output parameter for sender port
    /// \return Number of bytes received, 0 if no data, -1 on error
    int receiveFrom(char* buffer, int maxLength, std::string& senderIP, uint16_t& senderPort);

    // Multicast
    /// \brief Join a multicast group
    /// \param multicastAddress Multicast IP address (e.g., "224.0.0.1")
    /// \return true if successful
    bool joinMulticastGroup(const std::string& multicastAddress);

    /// \brief Leave a multicast group
    /// \param multicastAddress Multicast IP address
    /// \return true if successful
    bool leaveMulticastGroup(const std::string& multicastAddress);

    /// \brief Set multicast TTL (Time To Live)
    /// \param ttl TTL value (default 1 = local network only)
    void setMulticastTTL(uint8_t ttl);

    /// \brief Enable multicast loopback
    /// \param enable true to enable, false to disable
    void setMulticastLoopback(bool enable);

    // Settings
    /// \brief Set non-blocking mode (default true)
    /// \param nonBlocking true for non-blocking, false for blocking
    void setNonBlocking(bool nonBlocking);

    /// \brief Set receive buffer size
    /// \param size Buffer size in bytes
    void setReceiveBufferSize(int size);

    /// \brief Set send buffer size
    /// \param size Buffer size in bytes
    void setSendBufferSize(int size);

    // Status
    /// \brief Check if bound or connected
    /// \return true if ready
    bool isReady() const;

    /// \brief Get last error message
    /// \return Error message
    std::string getError() const;

    /// \brief Get local port
    /// \return Port number
    uint16_t getLocalPort() const;

    /// \brief Get remote IP (if connected)
    /// \return IP address as string
    std::string getRemoteIP() const;

    /// \brief Get remote port (if connected)
    /// \return Port number
    uint16_t getRemotePort() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
