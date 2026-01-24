#pragma once

#include "ofxOscMessage.h"
#include <memory>

/// \brief Receives OSC messages and bundles over UDP.
///
/// ofxOscReceiver allows you to receive OSC messages from a remote host.
/// You can either poll for messages using hasWaitingMessages()/getNextMessage()
/// or run a blocking listener thread.
///
/// Example usage (polling):
/// \code
/// ofxOscReceiver receiver;
/// receiver.setup(12345);
///
/// void update() {
///     while (receiver.hasWaitingMessages()) {
///         ofxOscMessage msg;
///         receiver.getNextMessage(msg);
///         if (msg.getAddress() == "/test") {
///             int value = msg.getArgAsInt32(0);
///         }
///     }
/// }
/// \endcode
///
/// Example usage (listener thread):
/// \code
/// ofxOscReceiver receiver;
/// receiver.setup(12345);
/// receiver.start();  // Start listener thread
///
/// // Messages are queued automatically
/// // Call stop() to stop the thread
/// \endcode
class ofxOscReceiver {
public:
    ofxOscReceiver();
    ~ofxOscReceiver();

    // Disable copy (socket and thread management)
    ofxOscReceiver(const ofxOscReceiver&) = delete;
    ofxOscReceiver& operator=(const ofxOscReceiver&) = delete;

    // Setup
    /// \brief Set up the receiver to listen on a specific port
    /// \param port Port number to listen on (1-65535)
    /// \return true on success, false on failure
    bool setup(int port);

    /// \brief Check if the receiver is set up and ready
    bool isSetup() const;

    /// \brief Get the port number we're listening on
    int getPort() const;

    // Listener thread control
    /// \brief Start the listener thread (blocking in background thread)
    /// Messages are automatically queued and can be retrieved with getNextMessage()
    void start();

    /// \brief Stop the listener thread
    void stop();

    /// \brief Check if the listener thread is running
    bool isListening() const;

    // Message retrieval (polling mode or after start())
    /// \brief Check if there are messages waiting in the queue
    bool hasWaitingMessages() const;

    /// \brief Get the next message from the queue
    /// \param message Reference to store the retrieved message
    /// \return true if a message was retrieved, false if queue was empty
    bool getNextMessage(ofxOscMessage& message);

    /// \brief Get the number of messages waiting in the queue
    size_t getNumWaitingMessages() const;

    // Shutdown
    /// \brief Stop listening and clean up
    void shutdown();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
