/// \file main.cpp
/// \brief Example 19: UDP Sender
///
/// Demonstrates UDP sending functionality using ofxUdpManager:
/// - Sending UDP datagrams
/// - Connectionless communication
/// - Broadcasting data
/// - Non-blocking transmission
///
/// This example sends UDP messages to a receiver.

#include "../../src/core/AppBase.h"
#include "../../src/oflike/graphics/ofGraphics.h"
#include "../../src/oflike/types/ofColor.h"
#include "../../src/oflike/math/ofMath.h"
#include "../../src/oflike/utils/ofUtils.h"
#include "../../src/oflike/utils/ofLog.h"
#include "../../src/addons/core/ofxNetwork/ofxUdpManager.h"
#include "../../src/core/Context.h"

using namespace oflike;

class UdpSenderApp : public ofBaseApp {
public:
    ofxUdpManager udp;
    std::string remoteHost = "localhost";
    uint16_t remotePort = 9000;
    float lastSendTime = 0.0f;
    float sendInterval = 1.0f;  // Send message every second
    int messageCount = 0;
    int totalBytesSent = 0;
    bool connected = false;

    void setup() override {
        ofSetFrameRate(60);

        // Connect to remote host
        ofLog() << "Connecting to " << remoteHost << ":" << remotePort;
        connected = udp.connect(remoteHost, remotePort);

        if (connected) {
            ofLog() << "UDP sender ready";
            ofLog() << "Target: " << remoteHost << ":" << remotePort;
        } else {
            ofLogError() << "Failed to setup UDP: " << udp.getError();
        }
    }

    void update() override {
        if (!connected) {
            return;
        }

        // Send periodic messages
        float currentTime = ofGetElapsedTimef();
        if (currentTime - lastSendTime > sendInterval) {
            sendMessage();
            lastSendTime = currentTime;
        }
    }

    void sendMessage() {
        // Create message with timestamp
        std::string message = "UDP Message #" + std::to_string(messageCount) +
                            " | Time: " + std::to_string(ofGetElapsedTimef());
        messageCount++;

        // Send message
        int sent = udp.send(message.c_str(), message.length());

        if (sent > 0) {
            totalBytesSent += sent;
            ofLog() << "Sent: " << message << " (" << sent << " bytes)";
        } else {
            ofLogError() << "Send failed: " << udp.getError();
        }
    }

    void draw() override {
        ofBackground(30);

        // Title
        ofSetColor(255);
        ofDrawRectangle(20, 20, 200, 30);

        // Connection status indicator
        float indicatorX = 250;
        float indicatorY = 35;
        float indicatorSize = 20;

        if (connected) {
            ofSetColor(0, 255, 0);  // Green = connected
        } else {
            ofSetColor(255, 0, 0);  // Red = disconnected
        }
        ofDrawCircle(indicatorX, indicatorY, indicatorSize / 2);

        // Info boxes
        float boxY = 80;
        float boxHeight = 60;
        float boxSpacing = 80;

        // Target info box
        ofSetColor(60);
        ofDrawRectangle(20, boxY, 360, boxHeight);
        ofSetColor(255, 200, 0);
        ofDrawRectangle(20, boxY, 360, 25);

        // Statistics box
        boxY += boxSpacing;
        ofSetColor(60);
        ofDrawRectangle(20, boxY, 360, boxHeight);
        ofSetColor(0, 150, 255);
        ofDrawRectangle(20, boxY, 360, 25);

        // Message info box
        boxY += boxSpacing;
        ofSetColor(60);
        ofDrawRectangle(20, boxY, 360, boxHeight);
        ofSetColor(150, 0, 255);
        ofDrawRectangle(20, boxY, 360, 25);

        // Visual representation: UDP sender
        if (connected && messageCount > 0) {
            // Animate UDP packets
            float time = ofGetElapsedTimef();
            float progress = fmod(time, sendInterval) / sendInterval;

            // Sender box (left)
            float senderX = 450;
            float senderY = 150;
            ofSetColor(100);
            ofDrawRectangle(senderX - 30, senderY - 40, 60, 80);
            ofSetColor(0, 255, 0);
            ofDrawCircle(senderX, senderY, 10);

            // Receiver box (right)
            float receiverX = 600;
            float receiverY = 150;
            ofSetColor(80);
            ofDrawRectangle(receiverX - 30, receiverY - 40, 60, 80);
            ofSetColor(0, 150, 255);
            ofDrawCircle(receiverX, receiverY, 10);

            // Connection line (dashed for UDP)
            ofSetColor(100, 100, 100, 100);
            int numDashes = 10;
            for (int i = 0; i < numDashes; i++) {
                if (i % 2 == 0) {
                    float x1 = senderX + (receiverX - senderX) * (i / (float)numDashes);
                    float x2 = senderX + (receiverX - senderX) * ((i + 1) / (float)numDashes);
                    ofDrawLine(x1, senderY, x2, receiverY);
                }
            }

            // Animated packets
            ofSetColor(0, 255, 150, 200);
            float packetX = senderX + (receiverX - senderX) * progress;
            float packetY = senderY + (receiverY - senderY) * progress;
            ofDrawCircle(packetX, packetY, 8);

            // Multiple packets at different positions
            for (int i = 1; i <= 3; i++) {
                float offset = i * 0.2f;
                float multiProgress = fmod(progress - offset, 1.0f);
                if (multiProgress > 0) {
                    float alpha = 200 * (1.0f - multiProgress);
                    ofSetColor(0, 255, 150, alpha);
                    float mpX = senderX + (receiverX - senderX) * multiProgress;
                    float mpY = senderY + (receiverY - senderY) * multiProgress;
                    ofDrawCircle(mpX, mpY, 6);
                }
            }

            // UDP label
            ofSetColor(200, 200, 200);
            ofDrawRectangle(senderX + 50, senderY - 30, 50, 20);
        }

        // Instructions box
        boxY += boxSpacing;
        ofSetColor(50);
        ofDrawRectangle(20, boxY, 360, 100);
        ofSetColor(200, 200, 200);
        ofDrawRectangle(20, boxY, 360, 25);

        // Rate visualization
        float rateY = boxY + 120;
        ofSetColor(100);
        ofDrawRectangle(20, rateY, 360, 40);

        // Rate bar
        float rateProgress = fmod(ofGetElapsedTimef() - lastSendTime, sendInterval) / sendInterval;
        ofSetColor(0, 255, 150);
        ofDrawRectangle(25, rateY + 5, 350 * rateProgress, 30);

        // FPS indicator
        ofSetColor(255, 255, 255, 100);
        float fps = ofGetFrameRate();
        float barWidth = (fps / 60.0f) * 100.0f;
        ofDrawRectangle(ofGetWidth() - 110, 10, barWidth, 5);
    }

    void keyPressed(int key) override {
        if (key == ' ') {
            // Send manual message on spacebar
            if (connected) {
                sendMessage();
            }
        } else if (key == '+' || key == '=') {
            // Increase send rate
            sendInterval = std::max(0.1f, sendInterval - 0.5f);
            ofLog() << "Send interval: " << sendInterval << "s";
        } else if (key == '-' || key == '_') {
            // Decrease send rate
            sendInterval = std::min(10.0f, sendInterval + 0.5f);
            ofLog() << "Send interval: " << sendInterval << "s";
        } else if (key == 'r' || key == 'R') {
            // Reset statistics
            messageCount = 0;
            totalBytesSent = 0;
            ofLog() << "Statistics reset";
        } else if (key == 'b' || key == 'B') {
            // Send burst of messages
            if (connected) {
                for (int i = 0; i < 10; i++) {
                    sendMessage();
                }
                ofLog() << "Sent burst of 10 messages";
            }
        }
    }

    void exit() override {
        if (connected) {
            udp.close();
            ofLog() << "UDP sender closed";
        }
    }
};

// Usage Instructions:
//
// 1. Start a UDP receiver first (Example 20: UDP Receiver or netcat):
//    nc -u -l 9000
//
// 2. Then run this UDP sender example
//
// 3. Replace TestApp in src/platform/bridge/SwiftBridge.mm:
//    #include "../../examples/19_udp_sender/main.cpp"
//    testApp_ = std::make_unique<UdpSenderApp>();
//
// 4. Build and run
//
// Controls:
// - Space: Send manual message immediately
// - +/-: Increase/decrease send rate
// - B: Send burst of 10 messages
// - R: Reset statistics
