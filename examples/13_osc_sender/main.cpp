/// \file main.cpp
/// \brief Example 13: OSC Sender
///
/// Demonstrates ofxOsc sender capabilities:
/// - Setting up an OSC sender
/// - Sending messages with different argument types
/// - Sending bundles (grouped messages)
/// - Real-time parameter transmission
///
/// This example sends OSC messages to localhost:12345
/// Run the osc_receiver example (14) to receive these messages.

#include "../../src/core/AppBase.h"
#include "../../src/oflike/graphics/ofGraphics.h"
#include "../../src/oflike/types/ofColor.h"
#include "../../src/oflike/utils/ofUtils.h"
#include "../../src/core/Context.h"
#include "../../src/addons/core/ofxOsc/ofxOscSender.h"
#include <iostream>
#include <string>

class OscSenderApp : public ofBaseApp {
public:
    ofxOscSender sender;
    float mouseXNormalized = 0.0f;
    float mouseYNormalized = 0.0f;
    int messageCount = 0;

    void setup() override {
        // Set up OSC sender to send to localhost:12345
        sender.setup("127.0.0.1", 12345);

        ofSetFrameRate(60);

        std::cout << "OSC Sender Example" << std::endl;
        std::cout << "Sending to: 127.0.0.1:12345" << std::endl;
        std::cout << "Press keys 1-5 to send different message types" << std::endl;
        std::cout << "Move mouse to send position updates" << std::endl;
    }

    void update() override {
        // Send mouse position every frame
        ofxOscMessage msg;
        msg.setAddress("/mouse");
        msg.addFloatArg(mouseXNormalized);
        msg.addFloatArg(mouseYNormalized);
        sender.sendMessage(msg);

        messageCount++;
    }

    void draw() override {
        ofBackground(40);

        // Draw info
        ofSetColor(255);
        std::string info = "OSC Sender Example\n\n";
        info += "Destination: 127.0.0.1:12345\n";
        info += "Messages sent: " + std::to_string(messageCount) + "\n\n";
        info += "Mouse position (normalized):\n";
        info += "  X: " + std::to_string(mouseXNormalized) + "\n";
        info += "  Y: " + std::to_string(mouseYNormalized) + "\n\n";
        info += "Press keys:\n";
        info += "  1 - Send int32 value\n";
        info += "  2 - Send float value\n";
        info += "  3 - Send string message\n";
        info += "  4 - Send multiple arguments\n";
        info += "  5 - Send OSC bundle\n";

        // Note: ofDrawBitmapString not yet implemented in Phase 19
        // Using console output for now
        // ofDrawBitmapString(info, 20, 30);

        // Draw crosshair at mouse position
        float mouseX = mouseXNormalized * ofGetWidth();
        float mouseY = mouseYNormalized * ofGetHeight();

        ofSetColor(0, 255, 255);
        ofDrawLine(mouseX - 10, mouseY, mouseX + 10, mouseY);
        ofDrawLine(mouseX, mouseY - 10, mouseX, mouseY + 10);
        ofDrawCircle(mouseX, mouseY, 20);
    }

    void mouseMoved(int x, int y) override {
        // Normalize mouse coordinates to 0-1 range
        mouseXNormalized = (float)x / ofGetWidth();
        mouseYNormalized = (float)y / ofGetHeight();
    }

    void keyPressed(int key) override {
        if (key == '1') {
            // Send int32 message
            ofxOscMessage msg;
            msg.setAddress("/int32");
            msg.addIntArg(42);
            sender.sendMessage(msg);
            std::cout << "Sent int32: 42" << std::endl;
        }
        else if (key == '2') {
            // Send float message
            ofxOscMessage msg;
            msg.setAddress("/float");
            msg.addFloatArg(3.14159f);
            sender.sendMessage(msg);
            std::cout << "Sent float: 3.14159" << std::endl;
        }
        else if (key == '3') {
            // Send string message
            ofxOscMessage msg;
            msg.setAddress("/string");
            msg.addStringArg("Hello OSC!");
            sender.sendMessage(msg);
            std::cout << "Sent string: Hello OSC!" << std::endl;
        }
        else if (key == '4') {
            // Send message with multiple arguments
            ofxOscMessage msg;
            msg.setAddress("/multi");
            msg.addIntArg(100);
            msg.addFloatArg(2.718f);
            msg.addStringArg("multi-arg");
            msg.addBoolArg(true);
            sender.sendMessage(msg);
            std::cout << "Sent multi-arg message" << std::endl;
        }
        else if (key == '5') {
            // Send OSC bundle (multiple messages)
            ofxOscBundle bundle;

            ofxOscMessage msg1;
            msg1.setAddress("/bundle/msg1");
            msg1.addStringArg("First message in bundle");
            bundle.addMessage(msg1);

            ofxOscMessage msg2;
            msg2.setAddress("/bundle/msg2");
            msg2.addIntArg(123);
            msg2.addFloatArg(4.56f);
            bundle.addMessage(msg2);

            ofxOscMessage msg3;
            msg3.setAddress("/bundle/msg3");
            msg3.addStringArg("Last message in bundle");
            bundle.addMessage(msg3);

            sender.sendBundle(bundle);
            std::cout << "Sent bundle with 3 messages" << std::endl;
        }
    }
};

// Register with ofApp system
// In production, this would be set in SwiftBridge.mm:
// setGlobalApp(std::make_shared<OscSenderApp>());
// For documentation: See examples/README.md
