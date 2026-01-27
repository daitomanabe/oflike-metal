#include "ofxOscParameterSync.h"
#include "ofxOscMessage.h"
#include <map>
#include <functional>

// MARK: - ofxOscParameterSync::Impl

class ofxOscParameterSync::Impl {
public:
    ofxOscSender sender;
    ofxOscReceiver receiver;

    bool hasSender = false;
    bool hasReceiver = false;
    bool autoSend = true;
    bool autoReceive = true;

    // Map OSC addresses to parameter update callbacks
    std::map<std::string, std::function<void(const ofxOscMessage&)>> receiveCallbacks;

    // Map OSC addresses to parameter send functions
    std::map<std::string, std::function<void()>> sendCallbacks;
};

// MARK: - ofxOscParameterSync

ofxOscParameterSync::ofxOscParameterSync()
    : impl_(std::make_unique<Impl>()) {
}

ofxOscParameterSync::~ofxOscParameterSync() = default;

void ofxOscParameterSync::setup(const std::string& sendHost, int sendPort, int receivePort) {
    setupSender(sendHost, sendPort);
    setupReceiver(receivePort);
}

void ofxOscParameterSync::setupSender(const std::string& sendHost, int sendPort) {
    impl_->sender.setup(sendHost, sendPort);
    impl_->hasSender = true;
}

void ofxOscParameterSync::setupReceiver(int receivePort) {
    impl_->receiver.setup(receivePort);
    impl_->hasReceiver = true;
}

template<typename T>
void ofxOscParameterSync::add(ofParameter<T>& param, const std::string& oscAddress) {
    // Setup sender callback
    if (impl_->hasSender) {
        auto sendCallback = [this, &param, oscAddress]() {
            if (!impl_->autoSend) return;

            ofxOscMessage msg;
            msg.setAddress(oscAddress);

            // Add argument based on type
            T value = param.get();
            if constexpr (std::is_same_v<T, float>) {
                msg.addFloatArg(value);
            } else if constexpr (std::is_same_v<T, int>) {
                msg.addIntArg(value);
            } else if constexpr (std::is_same_v<T, bool>) {
                msg.addBoolArg(value);
            } else if constexpr (std::is_same_v<T, std::string>) {
                msg.addStringArg(value);
            }

            impl_->sender.sendMessage(msg);
        };

        impl_->sendCallbacks[oscAddress] = sendCallback;

        // Add listener to parameter for automatic sending
        param.addListener([sendCallback](T&) {
            sendCallback();
        });
    }

    // Setup receiver callback
    if (impl_->hasReceiver) {
        auto receiveCallback = [&param](const ofxOscMessage& msg) {
            if (msg.getNumArgs() > 0) {
                T value;
                if constexpr (std::is_same_v<T, float>) {
                    value = msg.getArgAsFloat(0);
                } else if constexpr (std::is_same_v<T, int>) {
                    value = static_cast<int>(msg.getArgAsInt32(0));
                } else if constexpr (std::is_same_v<T, bool>) {
                    value = msg.getArgAsBool(0);
                } else if constexpr (std::is_same_v<T, std::string>) {
                    value = msg.getArgAsString(0);
                }
                param.set(value);
            }
        };

        impl_->receiveCallbacks[oscAddress] = receiveCallback;
    }
}

// Explicit template instantiations
template void ofxOscParameterSync::add<float>(ofParameter<float>&, const std::string&);
template void ofxOscParameterSync::add<int>(ofParameter<int>&, const std::string&);
template void ofxOscParameterSync::add<bool>(ofParameter<bool>&, const std::string&);
template void ofxOscParameterSync::add<std::string>(ofParameter<std::string>&, const std::string&);

void ofxOscParameterSync::add(ofParameterGroup& group, const std::string& oscAddressPrefix) {
    for (size_t i = 0; i < group.size(); ++i) {
        auto param = group.get(i);
        if (!param) continue;

        std::string oscAddress = oscAddressPrefix + "/" + param->getName();

        // Try to cast to known parameter types
        if (auto floatParam = std::dynamic_pointer_cast<ofParameter<float>>(param)) {
            add(*floatParam, oscAddress);
        }
        else if (auto intParam = std::dynamic_pointer_cast<ofParameter<int>>(param)) {
            add(*intParam, oscAddress);
        }
        else if (auto boolParam = std::dynamic_pointer_cast<ofParameter<bool>>(param)) {
            add(*boolParam, oscAddress);
        }
        else if (auto stringParam = std::dynamic_pointer_cast<ofParameter<std::string>>(param)) {
            add(*stringParam, oscAddress);
        }
        else if (auto subGroup = std::dynamic_pointer_cast<ofParameterGroup>(param)) {
            // Recursively add nested groups
            add(*subGroup, oscAddress);
        }
    }
}

void ofxOscParameterSync::update() {
    if (!impl_->hasReceiver || !impl_->autoReceive) return;

    // Process all pending OSC messages
    while (impl_->receiver.hasWaitingMessages()) {
        ofxOscMessage msg;
        impl_->receiver.getNextMessage(msg);

        std::string address = msg.getAddress();

        // Find and call the appropriate callback
        auto it = impl_->receiveCallbacks.find(address);
        if (it != impl_->receiveCallbacks.end()) {
            it->second(msg);
        }
    }
}

void ofxOscParameterSync::sendAll() {
    if (!impl_->hasSender) return;

    for (auto& [address, sendCallback] : impl_->sendCallbacks) {
        sendCallback();
    }
}

void ofxOscParameterSync::setAutoSend(bool autoSend) {
    impl_->autoSend = autoSend;
}

void ofxOscParameterSync::setAutoReceive(bool autoReceive) {
    impl_->autoReceive = autoReceive;
}

void ofxOscParameterSync::clear() {
    impl_->receiveCallbacks.clear();
    impl_->sendCallbacks.clear();
}
