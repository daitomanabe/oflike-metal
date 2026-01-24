/// \file main.cpp
/// \brief Example 17: TCP Client
///
/// Demonstrates TCP client functionality using ofxTcpClient:
/// - Connecting to a TCP server
/// - Sending messages
/// - Receiving responses
/// - Connection status monitoring
/// - Error handling
///
/// This example connects to a TCP server and exchanges messages.

#include "../../src/core/AppBase.h"
#include "../../src/oflike/graphics/ofGraphics.h"
#include "../../src/oflike/types/ofColor.h"
#include "../../src/oflike/utils/ofUtils.h"
#include "../../src/oflike/utils/ofLog.h"
#include "../../src/addons/core/ofxNetwork/ofxTcpClient.h"
#include "../../src/core/Context.h"

using namespace oflike;

class TcpClientApp : public ofBaseApp {
public:
    ofxTcpClient client;
    std::string serverHost = "localhost";
    uint16_t serverPort = 8080;
    std::string messageToSend = "Hello from TCP Client!";
    std::string receivedData;
    bool connected = false;
    float lastSendTime = 0.0f;
    float sendInterval = 2.0f;  // Send message every 2 seconds
    int messageCount = 0;

    void setup() override {
        ofSetFrameRate(60);

        // Attempt to connect to server
        ofLog() << "Attempting to connect to " << serverHost << ":" << serverPort;
        connected = client.setup(serverHost, serverPort);

        if (connected) {
            ofLog() << "Connected to server!";
            ofLog() << "Remote IP: " << client.getRemoteIP();
            ofLog() << "Remote Port: " << client.getRemotePort();
        } else {
            ofLogError() << "Failed to connect: " << client.getError();
        }
    }

    void update() override {
        // Check connection status
        connected = client.isConnected();

        if (connected) {
            // Send periodic messages
            float currentTime = ofGetElapsedTimef();
            if (currentTime - lastSendTime > sendInterval) {
                std::string message = messageToSend + " #" + std::to_string(messageCount++);
                int sent = client.sendMessage(message, "\n");
                if (sent > 0) {
                    ofLog() << "Sent: " << message << " (" << sent << " bytes)";
                } else {
                    ofLogError() << "Send failed: " << client.getError();
                }
                lastSendTime = currentTime;
            }

            // Check for received data
            std::string response = client.receiveMessage("\n");
            if (!response.empty()) {
                receivedData = response;
                ofLog() << "Received: " << receivedData;
            }
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

        // Server info box
        ofSetColor(60);
        ofDrawRectangle(20, boxY, 360, boxHeight);
        ofSetColor(255, 200, 0);
        ofDrawRectangle(20, boxY, 360, 25);

        // Connection details box
        boxY += boxSpacing;
        ofSetColor(60);
        ofDrawRectangle(20, boxY, 360, boxHeight);
        ofSetColor(0, 150, 255);
        ofDrawRectangle(20, boxY, 360, 25);

        // Message counter box
        boxY += boxSpacing;
        ofSetColor(60);
        ofDrawRectangle(20, boxY, 360, boxHeight);
        ofSetColor(150, 0, 255);
        ofDrawRectangle(20, boxY, 360, 25);

        // Received data box
        boxY += boxSpacing;
        ofSetColor(60);
        ofDrawRectangle(20, boxY, 360, 80);
        ofSetColor(0, 255, 150);
        ofDrawRectangle(20, boxY, 360, 25);

        // Instructions box
        boxY += 100;
        ofSetColor(50);
        ofDrawRectangle(20, boxY, 360, 80);
        ofSetColor(200, 200, 200);
        ofDrawRectangle(20, boxY, 360, 25);

        // Visual representation: data flow
        if (connected && messageCount > 0) {
            // Animate data packets
            float time = ofGetElapsedTimef();
            float progress = fmod(time, sendInterval) / sendInterval;

            // Client -> Server (outgoing)
            ofSetColor(0, 150, 255, 200);
            float outX = 400 + progress * 200;
            ofDrawCircle(outX, 100, 10);
            ofSetColor(0, 150, 255, 100);
            ofDrawLine(400, 100, 600, 100);

            // Server -> Client (incoming)
            if (!receivedData.empty()) {
                float inProgress = fmod(time + sendInterval/2, sendInterval) / sendInterval;
                ofSetColor(0, 255, 150, 200);
                float inX = 600 - inProgress * 200;
                ofDrawCircle(inX, 150, 10);
                ofSetColor(0, 255, 150, 100);
                ofDrawLine(400, 150, 600, 150);
            }

            // Client box
            ofSetColor(100);
            ofDrawRectangle(370, 70, 60, 100);
            ofSetColor(255);
            ofDrawCircle(400, 120, 5);

            // Server box
            ofSetColor(80);
            ofDrawRectangle(570, 70, 60, 100);
            ofSetColor(255);
            ofDrawCircle(600, 120, 5);
        }

        // FPS indicator
        ofSetColor(255, 255, 255, 100);
        float fps = ofGetFrameRate();
        float barWidth = (fps / 60.0f) * 100.0f;
        ofDrawRectangle(ofGetWidth() - 110, 10, barWidth, 5);
    }

    void keyPressed(int key) override {
        if (key == ' ') {
            // Send message on spacebar
            if (connected) {
                std::string message = "Manual message #" + std::to_string(messageCount++);
                int sent = client.sendMessage(message, "\n");
                if (sent > 0) {
                    ofLog() << "Manually sent: " << message;
                }
            }
        } else if (key == 'r' || key == 'R') {
            // Reconnect
            ofLog() << "Reconnecting...";
            client.close();
            connected = client.setup(serverHost, serverPort);
            messageCount = 0;
            receivedData.clear();
        } else if (key == 'd' || key == 'D') {
            // Disconnect
            ofLog() << "Disconnecting...";
            client.close();
            connected = false;
        }
    }

    void exit() override {
        if (connected) {
            client.close();
            ofLog() << "Connection closed";
        }
    }
};

// Usage Instructions:
//
// 1. Start a TCP server on localhost:8080 before running this example
//    You can use netcat: nc -l 8080
//    Or use the tcp_server example
//
// 2. Replace TestApp in src/platform/bridge/SwiftBridge.mm:
//    #include "../../examples/17_tcp_client/main.cpp"
//    testApp_ = std::make_unique<TcpClientApp>();
//
// 3. Build and run
//
// Controls:
// - Space: Send manual message
// - R: Reconnect to server
// - D: Disconnect from server
