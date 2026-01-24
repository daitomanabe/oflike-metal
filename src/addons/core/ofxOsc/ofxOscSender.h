#pragma once

#include "ofxOscMessage.h"
#include "ofxOscBundle.h"
#include <string>
#include <memory>

/// \brief Sends OSC messages and bundles over UDP.
///
/// ofxOscSender allows you to send OSC messages to a remote host/port.
/// Messages can be sent individually or grouped in bundles.
///
/// Example usage:
/// \code
/// ofxOscSender sender;
/// sender.setup("localhost", 12345);
///
/// ofxOscMessage msg;
/// msg.setAddress("/test");
/// msg.addIntArg(42);
/// sender.sendMessage(msg);
/// \endcode
class ofxOscSender {
public:
    ofxOscSender();
    ~ofxOscSender();

    // Disable copy (socket management)
    ofxOscSender(const ofxOscSender&) = delete;
    ofxOscSender& operator=(const ofxOscSender&) = delete;

    // Setup
    /// \brief Set up the sender with destination host and port
    /// \param hostname Destination hostname or IP address (e.g. "localhost", "192.168.1.100")
    /// \param port Destination port number (1-65535)
    /// \return true on success, false on failure
    bool setup(const std::string& hostname, int port);

    /// \brief Check if the sender is set up and ready
    bool isSetup() const;

    /// \brief Get the current hostname
    std::string getHostname() const;

    /// \brief Get the current port
    int getPort() const;

    // Send methods
    /// \brief Send an OSC message
    /// \param message The message to send
    /// \param wrapInBundle If true (default), wrap message in a time-tagged bundle
    /// \return true on success, false on failure
    bool sendMessage(const ofxOscMessage& message, bool wrapInBundle = true);

    /// \brief Send an OSC bundle
    /// \param bundle The bundle to send
    /// \return true on success, false on failure
    bool sendBundle(const ofxOscBundle& bundle);

    // Shutdown
    /// \brief Close the socket and clean up
    void shutdown();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
