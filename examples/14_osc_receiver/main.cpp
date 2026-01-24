/// \file main.cpp
/// \brief Example 14: OSC Receiver
///
/// Demonstrates ofxOsc receiver capabilities:
/// - Setting up an OSC receiver
/// - Receiving messages with different argument types
/// - Parsing and displaying received data
/// - Thread-safe message queue
///
/// This example listens on port 12345
/// Run the osc_sender example (13) to send messages to this receiver.

#include "../../src/core/AppBase.h"
#include "../../src/oflike/graphics/ofGraphics.h"
#include "../../src/oflike/types/ofColor.h"
#include "../../src/oflike/utils/ofUtils.h"
#include "../../src/core/Context.h"
#include "../../src/addons/core/ofxOsc/ofxOscReceiver.h"
#include "../../src/addons/core/ofxOsc/ofxOscMessage.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

class OscReceiverApp : public ofBaseApp {
public:
    ofxOscReceiver receiver;
    float mouseX = 0.5f;
    float mouseY = 0.5f;
    int messageCount = 0;

    std::vector<std::string> recentMessages;
    const int maxMessages = 10;

    void setup() override {
        // Set up OSC receiver to listen on port 12345
        receiver.setup(12345);
        receiver.start(); // Start listener thread

        ofSetFrameRate(60);

        std::cout << "OSC Receiver Example" << std::endl;
        std::cout << "Listening on port: 12345" << std::endl;
        std::cout << "Run the osc_sender example (13) to send messages" << std::endl;
    }

    void update() override {
        // Check for waiting messages
        while (receiver.hasWaitingMessages()) {
            ofxOscMessage msg;
            receiver.getNextMessage(msg);

            messageCount++;

            // Handle different message types
            std::string address = msg.getAddress();

            if (address == "/mouse") {
                // Mouse position message
                if (msg.getNumArgs() >= 2) {
                    mouseX = msg.getArgAsFloat(0);
                    mouseY = msg.getArgAsFloat(1);
                }
                addMessage("Mouse: " + std::to_string(mouseX) + ", " + std::to_string(mouseY));
            }
            else if (address == "/int32") {
                int value = msg.getArgAsInt32(0);
                addMessage("Int32: " + std::to_string(value));
            }
            else if (address == "/float") {
                float value = msg.getArgAsFloat(0);
                addMessage("Float: " + std::to_string(value));
            }
            else if (address == "/string") {
                std::string value = msg.getArgAsString(0);
                addMessage("String: " + value);
            }
            else if (address == "/multi") {
                std::stringstream ss;
                ss << "Multi: ";
                ss << msg.getArgAsInt32(0) << ", ";
                ss << msg.getArgAsFloat(1) << ", ";
                ss << msg.getArgAsString(2) << ", ";
                ss << (msg.getArgAsBool(3) ? "true" : "false");
                addMessage(ss.str());
            }
            else if (address.find("/bundle/") == 0) {
                // Bundle message
                std::stringstream ss;
                ss << "Bundle: " << address << " (";
                for (size_t i = 0; i < msg.getNumArgs(); i++) {
                    if (msg.getArgType(i) == ofxOscMessage::OFX_OSC_TYPE_INT32) {
                        ss << msg.getArgAsInt32(i);
                    } else if (msg.getArgType(i) == ofxOscMessage::OFX_OSC_TYPE_FLOAT) {
                        ss << msg.getArgAsFloat(i);
                    } else if (msg.getArgType(i) == ofxOscMessage::OFX_OSC_TYPE_STRING) {
                        ss << "\"" << msg.getArgAsString(i) << "\"";
                    }
                    if (i < msg.getNumArgs() - 1) ss << ", ";
                }
                ss << ")";
                addMessage(ss.str());
            }
            else {
                // Unknown message type
                addMessage("Unknown: " + address);
            }
        }
    }

    void draw() override {
        ofBackground(40);

        // Draw info
        ofSetColor(255);
        std::string info = "OSC Receiver Example\n\n";
        info += "Listening on port: 12345\n";
        info += "Messages received: " + std::to_string(messageCount) + "\n";
        info += "Queue size: " + std::to_string(receiver.getNumWaitingMessages()) + "\n\n";
        info += "Recent messages:\n";

        // Note: ofDrawBitmapString not yet implemented in Phase 19
        // Using console output for now
        // ofDrawBitmapString(info, 20, 30);

        // Draw recent messages
        // int y = 150;
        // for (const auto& msg : recentMessages) {
        //     ofDrawBitmapString(msg, 20, y);
        //     y += 15;
        // }

        // Draw crosshair at received mouse position
        float screenX = mouseX * ofGetWidth();
        float screenY = mouseY * ofGetHeight();

        ofSetColor(0, 255, 255);
        ofDrawLine(screenX - 10, screenY, screenX + 10, screenY);
        ofDrawLine(screenX, screenY - 10, screenX, screenY + 10);
        ofDrawCircle(screenX, screenY, 20);
    }

    void addMessage(const std::string& msg) {
        recentMessages.insert(recentMessages.begin(), msg);
        if (recentMessages.size() > maxMessages) {
            recentMessages.pop_back();
        }
        std::cout << msg << std::endl;
    }

    void exit() override {
        // Clean shutdown
        receiver.stop();
        receiver.shutdown();
    }
};

// Register with ofApp system
// In production, this would be set in SwiftBridge.mm:
// setGlobalApp(std::make_shared<OscReceiverApp>());
// For documentation: See examples/README.md
