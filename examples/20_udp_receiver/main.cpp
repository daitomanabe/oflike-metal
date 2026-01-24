/// \file main.cpp
/// \brief Example 20: UDP Receiver
///
/// Demonstrates UDP receiving functionality using ofxUdpManager:
/// - Receiving UDP datagrams
/// - Connectionless communication
/// - Sender identification
/// - Message buffering
/// - Non-blocking reception
///
/// This example receives UDP messages from senders.

#include "../../src/core/AppBase.h"
#include "../../src/oflike/graphics/ofGraphics.h"
#include "../../src/oflike/types/ofColor.h"
#include "../../src/oflike/math/ofMath.h"
#include "../../src/oflike/utils/ofUtils.h"
#include "../../src/oflike/utils/ofLog.h"
#include "../../src/addons/core/ofxNetwork/ofxUdpManager.h"
#include "../../src/core/Context.h"
#include <vector>
#include <deque>

using namespace oflike;

struct ReceivedMessage {
    std::string message;
    std::string senderIP;
    uint16_t senderPort;
    float timestamp;
};

class UdpReceiverApp : public ofBaseApp {
public:
    ofxUdpManager udp;
    uint16_t listenPort = 9000;
    std::deque<ReceivedMessage> messages;
    int maxMessages = 10;
    int totalMessagesReceived = 0;
    int totalBytesReceived = 0;
    bool bound = false;
    float lastReceiveTime = 0.0f;

    void setup() override {
        ofSetFrameRate(60);

        // Bind to port for receiving
        ofLog() << "Binding to port " << listenPort;
        bound = udp.bind(listenPort);

        if (bound) {
            ofLog() << "UDP receiver ready on port " << listenPort;
            ofLog() << "Waiting for messages...";
        } else {
            ofLogError() << "Failed to bind: " << udp.getError();
        }
    }

    void update() override {
        if (!bound) {
            return;
        }

        // Receive all available messages
        char buffer[8192];
        std::string senderIP;
        uint16_t senderPort;

        while (true) {
            int received = udp.receiveFrom(buffer, sizeof(buffer) - 1, senderIP, senderPort);

            if (received > 0) {
                buffer[received] = '\0';  // Null terminate
                std::string message(buffer, received);

                // Store message
                ReceivedMessage msg;
                msg.message = message;
                msg.senderIP = senderIP;
                msg.senderPort = senderPort;
                msg.timestamp = ofGetElapsedTimef();

                messages.push_back(msg);
                if (messages.size() > static_cast<size_t>(maxMessages)) {
                    messages.pop_front();
                }

                totalMessagesReceived++;
                totalBytesReceived += received;
                lastReceiveTime = ofGetElapsedTimef();

                ofLog() << "Received from " << senderIP << ":" << senderPort
                       << " (" << received << " bytes): " << message;
            } else if (received == 0) {
                // No more data available
                break;
            } else {
                // Error
                ofLogError() << "Receive error: " << udp.getError();
                break;
            }
        }
    }

    void draw() override {
        ofBackground(30);

        // Title
        ofSetColor(255);
        ofDrawRectangle(20, 20, 200, 30);

        // Status indicator
        float indicatorX = 250;
        float indicatorY = 35;
        float indicatorSize = 20;

        if (bound) {
            ofSetColor(0, 255, 0);  // Green = listening
        } else {
            ofSetColor(255, 0, 0);  // Red = not bound
        }
        ofDrawCircle(indicatorX, indicatorY, indicatorSize / 2);

        // Info boxes
        float boxY = 80;
        float boxHeight = 60;
        float boxSpacing = 80;

        // Port info box
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

        // Last message box
        boxY += boxSpacing;
        ofSetColor(60);
        ofDrawRectangle(20, boxY, 360, boxHeight);
        ofSetColor(150, 0, 255);
        ofDrawRectangle(20, boxY, 360, 25);

        // Messages list box
        boxY += boxSpacing;
        float messagesBoxHeight = 250;
        ofSetColor(60);
        ofDrawRectangle(20, boxY, 360, messagesBoxHeight);
        ofSetColor(0, 255, 150);
        ofDrawRectangle(20, boxY, 360, 25);

        // Draw message list
        float messageY = boxY + 35;
        int displayCount = 0;
        for (auto it = messages.rbegin(); it != messages.rend() && displayCount < 8; ++it, ++displayCount) {
            const auto& msg = *it;

            // Message background
            float alpha = 150 * (1.0f - displayCount / 8.0f);
            ofSetColor(80, 80, 80, alpha);
            ofDrawRectangle(30, messageY, 340, 25);

            // Sender indicator
            ofSetColor(0, 200, 255, alpha);
            ofDrawCircle(40, messageY + 12, 4);

            messageY += 28;
        }

        // Visual representation: UDP receiver
        if (bound && totalMessagesReceived > 0) {
            // Receiver box (center)
            float receiverX = 500;
            float receiverY = 150;
            ofSetColor(100);
            ofDrawRectangle(receiverX - 30, receiverY - 40, 60, 80);
            ofSetColor(0, 255, 0);
            ofDrawCircle(receiverX, receiverY, 10);

            // Multiple sender boxes (in circle around receiver)
            float time = ofGetElapsedTimef();
            int numSenders = std::min(static_cast<int>(messages.size()), 5);

            for (int i = 0; i < numSenders; i++) {
                float angle = (i / 5.0f) * TWO_PI;
                float radius = 120;
                float senderX = receiverX + cos(angle) * radius;
                float senderY = receiverY + sin(angle) * radius;

                // Sender box
                ofSetColor(80);
                ofDrawRectangle(senderX - 20, senderY - 20, 40, 40);
                ofSetColor(0, 150, 255);
                ofDrawCircle(senderX, senderY, 8);

                // Connection line (dashed)
                ofSetColor(100, 100, 100, 100);
                int numDashes = 8;
                for (int j = 0; j < numDashes; j++) {
                    if (j % 2 == 0) {
                        float t1 = j / (float)numDashes;
                        float t2 = (j + 1) / (float)numDashes;
                        float x1 = senderX + (receiverX - senderX) * t1;
                        float y1 = senderY + (receiverY - senderY) * t1;
                        float x2 = senderX + (receiverX - senderX) * t2;
                        float y2 = senderY + (receiverY - senderY) * t2;
                        ofDrawLine(x1, y1, x2, y2);
                    }
                }

                // Animate incoming packets
                if (i < static_cast<int>(messages.size())) {
                    float msgTime = messages[messages.size() - 1 - i].timestamp;
                    float timeSince = time - msgTime;
                    if (timeSince < 1.0f) {
                        float progress = timeSince;
                        float packetX = senderX + (receiverX - senderX) * progress;
                        float packetY = senderY + (receiverY - senderY) * progress;
                        float alpha = 200 * (1.0f - progress);
                        ofSetColor(0, 255, 150, alpha);
                        ofDrawCircle(packetX, packetY, 8);
                    }
                }
            }

            // UDP label
            ofSetColor(200, 200, 200);
            ofDrawRectangle(receiverX - 25, receiverY + 50, 50, 20);
        }

        // Instructions box
        boxY += messagesBoxHeight + 20;
        ofSetColor(50);
        ofDrawRectangle(20, boxY, 360, 80);
        ofSetColor(200, 200, 200);
        ofDrawRectangle(20, boxY, 360, 25);

        // Activity indicator
        float activityY = boxY + 100;
        if (ofGetElapsedTimef() - lastReceiveTime < 2.0f) {
            // Pulse effect when recently received
            float pulse = sin(ofGetElapsedTimef() * 5.0f) * 0.5f + 0.5f;
            ofSetColor(0, 255, 150, 100 + 155 * pulse);
            ofDrawRectangle(20, activityY, 360, 30);
        } else {
            ofSetColor(60);
            ofDrawRectangle(20, activityY, 360, 30);
        }

        // FPS indicator
        ofSetColor(255, 255, 255, 100);
        float fps = ofGetFrameRate();
        float barWidth = (fps / 60.0f) * 100.0f;
        ofDrawRectangle(ofGetWidth() - 110, 10, barWidth, 5);
    }

    void keyPressed(int key) override {
        if (key == 'c' || key == 'C') {
            // Clear messages
            messages.clear();
            ofLog() << "Message history cleared";
        } else if (key == 'r' || key == 'R') {
            // Reset statistics
            totalMessagesReceived = 0;
            totalBytesReceived = 0;
            ofLog() << "Statistics reset";
        } else if (key == '+' || key == '=') {
            // Increase message buffer
            maxMessages = std::min(100, maxMessages + 5);
            ofLog() << "Max messages: " << maxMessages;
        } else if (key == '-' || key == '_') {
            // Decrease message buffer
            maxMessages = std::max(5, maxMessages - 5);
            while (messages.size() > static_cast<size_t>(maxMessages)) {
                messages.pop_front();
            }
            ofLog() << "Max messages: " << maxMessages;
        }
    }

    void exit() override {
        if (bound) {
            udp.close();
            ofLog() << "UDP receiver closed";
        }
    }
};

// Usage Instructions:
//
// 1. Run this UDP receiver example first
//
// 2. Then send messages using netcat:
//    echo "Hello UDP" | nc -u localhost 9000
//
// 3. Or use the udp_sender example (Example 19)
//
// 4. Replace TestApp in src/platform/bridge/SwiftBridge.mm:
//    #include "../../examples/20_udp_receiver/main.cpp"
//    testApp_ = std::make_unique<UdpReceiverApp>();
//
// 5. Build and run
//
// Controls:
// - C: Clear message history
// - R: Reset statistics
// - +/-: Increase/decrease message buffer size
