#include "ofxOscReceiver.h"
#include <osc/OscPacketListener.h>
#include <osc/OscReceivedElements.h>
#include <ip/UdpSocket.h>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>

// Internal packet listener that converts oscpack messages to ofxOscMessage
class OscListener : public osc::OscPacketListener {
public:
    std::queue<ofxOscMessage> messageQueue;
    std::mutex queueMutex;

protected:
    void ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& /*remoteEndpoint*/) override {
        ofxOscMessage msg;
        msg.setAddress(m.AddressPattern());

        try {
            // Parse all arguments
            for (osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();
                 arg != m.ArgumentsEnd(); ++arg) {

                if (arg->IsInt32()) {
                    msg.addIntArg(arg->AsInt32());
                }
                else if (arg->IsInt64()) {
                    msg.addInt64Arg(arg->AsInt64());
                }
                else if (arg->IsFloat()) {
                    msg.addFloatArg(arg->AsFloat());
                }
                else if (arg->IsDouble()) {
                    msg.addDoubleArg(arg->AsDouble());
                }
                else if (arg->IsString()) {
                    msg.addStringArg(arg->AsString());
                }
                else if (arg->IsBlob()) {
                    const void* blobData;
                    osc::osc_bundle_element_size_t blobSize;
                    arg->AsBlob(blobData, blobSize);
                    msg.addBlobArg(blobData, blobSize);
                }
                else if (arg->IsChar()) {
                    msg.addCharArg(arg->AsChar());
                }
                else if (arg->IsBool()) {
                    msg.addBoolArg(arg->AsBool());
                }
                else if (arg->IsNil()) {
                    msg.addNoneArg();
                }
                else if (arg->IsInfinitum()) {
                    msg.addTriggerArg();
                }
                else if (arg->IsTimeTag()) {
                    msg.addTimeTagArg(arg->AsTimeTag());
                }
                else if (arg->IsRgbaColor()) {
                    msg.addRgbaColorArg(arg->AsRgbaColor());
                }
                else if (arg->IsMidiMessage()) {
                    msg.addMidiMessageArg(arg->AsMidiMessage());
                }
            }

            // Add to queue
            std::lock_guard<std::mutex> lock(queueMutex);
            messageQueue.push(msg);

        } catch (const osc::Exception& e) {
            // Silently ignore malformed messages
        }
    }
};

// Implementation using pImpl pattern to hide oscpack types from public header
class ofxOscReceiver::Impl {
public:
    Impl()
        : socket_(nullptr)
        , listener_(nullptr)
        , isSetup_(false)
        , isListening_(false)
        , port_(0) {
    }

    ~Impl() {
        shutdown();
    }

    bool setup(int port) {
        shutdown();

        try {
            port_ = port;

            // Create listener
            listener_ = new OscListener();

            // Create socket bound to the specified port
            socket_ = new UdpListeningReceiveSocket(
                IpEndpointName(IpEndpointName::ANY_ADDRESS, port),
                listener_
            );

            isSetup_ = true;
            return true;
        } catch (const std::exception& e) {
            shutdown();
            return false;
        }
    }

    void start() {
        if (!isSetup_ || isListening_ || !socket_) {
            return;
        }

        isListening_ = true;
        shouldStop_ = false;

        // Start listener thread
        listenerThread_ = std::thread([this]() {
            try {
                // Run the socket's receive loop
                // This blocks until AsynchronousBreak() is called
                while (!shouldStop_) {
                    socket_->Run();
                    if (shouldStop_) break;
                }
            } catch (const std::exception& e) {
                // Thread terminated
            }
            isListening_ = false;
        });
    }

    void stop() {
        if (!isListening_ || !socket_) {
            return;
        }

        shouldStop_ = true;

        // Break the socket's receive loop
        socket_->AsynchronousBreak();

        // Wait for thread to finish
        if (listenerThread_.joinable()) {
            listenerThread_.join();
        }

        isListening_ = false;
    }

    void shutdown() {
        stop();

        if (socket_) {
            delete socket_;
            socket_ = nullptr;
        }

        if (listener_) {
            std::lock_guard<std::mutex> lock(listener_->queueMutex);
            // Clear the queue
            while (!listener_->messageQueue.empty()) {
                listener_->messageQueue.pop();
            }
            delete listener_;
            listener_ = nullptr;
        }

        isSetup_ = false;
    }

    bool hasWaitingMessages() const {
        if (!listener_) return false;
        std::lock_guard<std::mutex> lock(listener_->queueMutex);
        return !listener_->messageQueue.empty();
    }

    bool getNextMessage(ofxOscMessage& message) {
        if (!listener_) return false;

        std::lock_guard<std::mutex> lock(listener_->queueMutex);
        if (listener_->messageQueue.empty()) {
            return false;
        }

        message = listener_->messageQueue.front();
        listener_->messageQueue.pop();
        return true;
    }

    size_t getNumWaitingMessages() const {
        if (!listener_) return 0;
        std::lock_guard<std::mutex> lock(listener_->queueMutex);
        return listener_->messageQueue.size();
    }

    bool isSetup() const { return isSetup_; }
    bool isListening() const { return isListening_; }
    int getPort() const { return port_; }

private:
    UdpListeningReceiveSocket* socket_;
    OscListener* listener_;
    std::thread listenerThread_;
    std::atomic<bool> shouldStop_;
    bool isSetup_;
    bool isListening_;
    int port_;
};

// Public API implementation
ofxOscReceiver::ofxOscReceiver()
    : impl_(new Impl()) {
}

ofxOscReceiver::~ofxOscReceiver() {
}

bool ofxOscReceiver::setup(int port) {
    return impl_->setup(port);
}

bool ofxOscReceiver::isSetup() const {
    return impl_->isSetup();
}

int ofxOscReceiver::getPort() const {
    return impl_->getPort();
}

void ofxOscReceiver::start() {
    impl_->start();
}

void ofxOscReceiver::stop() {
    impl_->stop();
}

bool ofxOscReceiver::isListening() const {
    return impl_->isListening();
}

bool ofxOscReceiver::hasWaitingMessages() const {
    return impl_->hasWaitingMessages();
}

bool ofxOscReceiver::getNextMessage(ofxOscMessage& message) {
    return impl_->getNextMessage(message);
}

size_t ofxOscReceiver::getNumWaitingMessages() const {
    return impl_->getNumWaitingMessages();
}

void ofxOscReceiver::shutdown() {
    impl_->shutdown();
}
