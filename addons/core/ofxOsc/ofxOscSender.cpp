#include "ofxOscSender.h"
#include <osc/OscOutboundPacketStream.h>
#include <ip/UdpSocket.h>
#include <cstring>

// Implementation using pImpl pattern to hide oscpack types from public header
class ofxOscSender::Impl {
public:
    Impl() : socket_(nullptr), isSetup_(false), port_(0) {}

    ~Impl() {
        shutdown();
    }

    bool setup(const std::string& hostname, int port) {
        shutdown();

        try {
            hostname_ = hostname;
            port_ = port;

            // Create UDP socket
            socket_ = new UdpTransmitSocket(IpEndpointName(hostname.c_str(), port));
            isSetup_ = true;
            return true;
        } catch (const std::exception& e) {
            shutdown();
            return false;
        }
    }

    void shutdown() {
        if (socket_) {
            delete socket_;
            socket_ = nullptr;
        }
        isSetup_ = false;
    }

    bool sendMessage(const ofxOscMessage& message, bool wrapInBundle) {
        if (!isSetup_ || !socket_) {
            return false;
        }

        try {
            char buffer[4096];
            osc::OutboundPacketStream p(buffer, sizeof(buffer));

            if (wrapInBundle) {
                // Wrap in bundle with immediate time tag
                p << osc::BeginBundleImmediate;
                serializeMessage(p, message);
                p << osc::EndBundle;
            } else {
                serializeMessage(p, message);
            }

            socket_->Send(p.Data(), p.Size());
            return true;
        } catch (const std::exception& e) {
            return false;
        }
    }

    bool sendBundle(const ofxOscBundle& bundle) {
        if (!isSetup_ || !socket_) {
            return false;
        }

        try {
            char buffer[16384];
            osc::OutboundPacketStream p(buffer, sizeof(buffer));

            serializeBundle(p, bundle);

            socket_->Send(p.Data(), p.Size());
            return true;
        } catch (const std::exception& e) {
            return false;
        }
    }

    bool isSetup() const { return isSetup_; }
    const std::string& getHostname() const { return hostname_; }
    int getPort() const { return port_; }

private:
    UdpTransmitSocket* socket_;
    bool isSetup_;
    std::string hostname_;
    int port_;

    void serializeMessage(osc::OutboundPacketStream& p, const ofxOscMessage& message) {
        p << osc::BeginMessage(message.getAddress().c_str());

        for (size_t i = 0; i < message.getNumArgs(); ++i) {
            auto type = message.getArgType(i);
            const auto& arg = message.getArgument(i);

            switch (type) {
                case ofxOscMessage::OFX_OSC_TYPE_INT32:
                    p << arg.asInt32;
                    break;
                case ofxOscMessage::OFX_OSC_TYPE_INT64:
                    p << (osc::int64)arg.asInt64;
                    break;
                case ofxOscMessage::OFX_OSC_TYPE_FLOAT:
                    p << arg.asFloat;
                    break;
                case ofxOscMessage::OFX_OSC_TYPE_DOUBLE:
                    p << arg.asDouble;
                    break;
                case ofxOscMessage::OFX_OSC_TYPE_STRING:
                    p << arg.asString.c_str();
                    break;
                case ofxOscMessage::OFX_OSC_TYPE_BLOB:
                    p << osc::Blob(arg.asBlob.data(), arg.asBlob.size());
                    break;
                case ofxOscMessage::OFX_OSC_TYPE_TRUE:
                    p << true;
                    break;
                case ofxOscMessage::OFX_OSC_TYPE_FALSE:
                    p << false;
                    break;
                case ofxOscMessage::OFX_OSC_TYPE_NONE:
                    p << osc::OscNil;
                    break;
                case ofxOscMessage::OFX_OSC_TYPE_TRIGGER:
                    p << osc::Infinitum;
                    break;
                case ofxOscMessage::OFX_OSC_TYPE_TIMETAG:
                    p << osc::TimeTag(arg.asTimeTag);
                    break;
                case ofxOscMessage::OFX_OSC_TYPE_CHAR:
                    p << arg.asChar;
                    break;
                case ofxOscMessage::OFX_OSC_TYPE_RGBA_COLOR:
                    p << osc::RgbaColor(arg.asRgbaColor);
                    break;
                case ofxOscMessage::OFX_OSC_TYPE_MIDI_MESSAGE:
                    p << osc::MidiMessage(arg.asMidiMessage);
                    break;
            }
        }

        p << osc::EndMessage;
    }

    void serializeBundle(osc::OutboundPacketStream& p, const ofxOscBundle& bundle) {
        // Convert time tag to oscpack format
        osc::TimeTag timeTag(bundle.getTimeTag());
        p << osc::BeginBundle(timeTag);

        // Add messages
        for (size_t i = 0; i < bundle.getMessageCount(); ++i) {
            serializeMessage(p, bundle.getMessageAt(i));
        }

        // Add nested bundles
        for (size_t i = 0; i < bundle.getBundleCount(); ++i) {
            serializeBundle(p, bundle.getBundleAt(i));
        }

        p << osc::EndBundle;
    }
};

// Public API implementation
ofxOscSender::ofxOscSender()
    : impl_(new Impl()) {
}

ofxOscSender::~ofxOscSender() {
}

bool ofxOscSender::setup(const std::string& hostname, int port) {
    return impl_->setup(hostname, port);
}

bool ofxOscSender::isSetup() const {
    return impl_->isSetup();
}

std::string ofxOscSender::getHostname() const {
    return impl_->getHostname();
}

int ofxOscSender::getPort() const {
    return impl_->getPort();
}

bool ofxOscSender::sendMessage(const ofxOscMessage& message, bool wrapInBundle) {
    return impl_->sendMessage(message, wrapInBundle);
}

bool ofxOscSender::sendBundle(const ofxOscBundle& bundle) {
    return impl_->sendBundle(bundle);
}

void ofxOscSender::shutdown() {
    impl_->shutdown();
}
