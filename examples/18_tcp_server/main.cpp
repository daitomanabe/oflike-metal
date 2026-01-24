/// \file main.cpp
/// \brief Example 18: TCP Server
///
/// Demonstrates TCP server functionality using ofxTcpServer:
/// - Starting a TCP server
/// - Accepting client connections
/// - Receiving messages from clients
/// - Sending responses to clients
/// - Broadcasting to all clients
/// - Client connection management
///
/// This example creates a TCP server that echoes received messages.

#include "../../src/core/AppBase.h"
#include "../../src/oflike/graphics/ofGraphics.h"
#include "../../src/oflike/types/ofColor.h"
#include "../../src/oflike/math/ofMath.h"
#include "../../src/oflike/utils/ofUtils.h"
#include "../../src/oflike/utils/ofLog.h"
#include "../../src/addons/core/ofxNetwork/ofxTcpServer.h"
#include "../../src/core/Context.h"

using namespace oflike;

class TcpServerApp : public ofBaseApp {
public:
    ofxTcpServer server;
    uint16_t serverPort = 8080;
    int maxClients = 10;
    std::vector<std::string> lastMessages;
    int totalMessagesReceived = 0;
    float lastBroadcastTime = 0.0f;
    float broadcastInterval = 5.0f;  // Broadcast to all clients every 5 seconds

    void setup() override {
        ofSetFrameRate(60);

        // Setup server
        server.setMaxClients(maxClients);
        bool success = server.setup(serverPort);

        if (success) {
            ofLog() << "TCP Server started on port " << serverPort;
            ofLog() << "Waiting for clients...";
            ofLog() << "Max clients: " << maxClients;
        } else {
            ofLogError() << "Failed to start server: " << server.getError();
        }

        lastMessages.resize(maxClients, "");
    }

    void update() override {
        if (!server.isListening()) {
            return;
        }

        // Accept new clients (non-blocking)
        server.waitForClient(0);  // 0 = non-blocking

        int numClients = server.getNumClients();

        // Process messages from each connected client
        for (int i = 0; i < numClients; i++) {
            if (!server.isClientConnected(i)) {
                continue;
            }

            // Receive message
            std::string message = server.receive(i);
            if (!message.empty()) {
                totalMessagesReceived++;
                lastMessages[i] = message;

                ofLog() << "Client " << i << " (" << server.getClientIP(i) << "): " << message;

                // Echo message back to sender
                std::string response = "Echo: " + message;
                int sent = server.send(i, response + "\n");
                if (sent > 0) {
                    ofLog() << "Sent echo to client " << i;
                } else {
                    ofLogError() << "Failed to send to client " << i;
                }
            }
        }

        // Periodic broadcast to all clients
        float currentTime = ofGetElapsedTimef();
        if (currentTime - lastBroadcastTime > broadcastInterval && numClients > 0) {
            std::string broadcast = "Broadcast: " + std::to_string(numClients) + " clients connected";
            server.sendToAll(broadcast + "\n");
            ofLog() << "Broadcast: " << broadcast;
            lastBroadcastTime = currentTime;
        }
    }

    void draw() override {
        ofBackground(30);

        int numClients = server.getNumClients();

        // Title
        ofSetColor(255);
        ofDrawRectangle(20, 20, 250, 30);

        // Server status indicator
        float indicatorX = 300;
        float indicatorY = 35;
        float indicatorSize = 20;

        if (server.isListening()) {
            ofSetColor(0, 255, 0);  // Green = listening
        } else {
            ofSetColor(255, 0, 0);  // Red = not listening
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

        // Clients info box
        boxY += boxSpacing;
        ofSetColor(60);
        ofDrawRectangle(20, boxY, 360, boxHeight);
        ofSetColor(0, 150, 255);
        ofDrawRectangle(20, boxY, 360, 25);

        // Messages counter box
        boxY += boxSpacing;
        ofSetColor(60);
        ofDrawRectangle(20, boxY, 360, boxHeight);
        ofSetColor(150, 0, 255);
        ofDrawRectangle(20, boxY, 360, 25);

        // Connected clients list
        boxY += boxSpacing;
        float clientBoxHeight = 150;
        ofSetColor(60);
        ofDrawRectangle(20, boxY, 360, clientBoxHeight);
        ofSetColor(0, 255, 150);
        ofDrawRectangle(20, boxY, 360, 25);

        // Draw client connections visually
        if (numClients > 0) {
            // Server box (center)
            float serverX = 450;
            float serverY = 150;
            ofSetColor(80);
            ofDrawRectangle(serverX - 30, serverY - 40, 60, 80);
            ofSetColor(0, 255, 0);
            ofDrawCircle(serverX, serverY, 10);

            // Client boxes (arranged in a circle around server)
            for (int i = 0; i < numClients && i < maxClients; i++) {
                if (server.isClientConnected(i)) {
                    float angle = (i / (float)maxClients) * TWO_PI;
                    float radius = 120;
                    float clientX = serverX + cos(angle) * radius;
                    float clientY = serverY + sin(angle) * radius;

                    // Draw connection line
                    if (!lastMessages[i].empty()) {
                        ofSetColor(0, 255, 150, 150);
                    } else {
                        ofSetColor(100, 100, 100, 100);
                    }
                    ofDrawLine(serverX, serverY, clientX, clientY);

                    // Draw client box
                    ofSetColor(100);
                    ofDrawRectangle(clientX - 20, clientY - 20, 40, 40);

                    // Client indicator
                    if (server.isClientConnected(i)) {
                        ofSetColor(0, 200, 255);
                    } else {
                        ofSetColor(100);
                    }
                    ofDrawCircle(clientX, clientY, 8);

                    // Animate data packets
                    if (!lastMessages[i].empty()) {
                        float time = ofGetElapsedTimef();
                        float progress = fmod(time * 2.0f, 1.0f);
                        float packetX = clientX + (serverX - clientX) * progress;
                        float packetY = clientY + (serverY - clientY) * progress;
                        ofSetColor(0, 255, 150, 200);
                        ofDrawCircle(packetX, packetY, 5);
                    }
                }
            }
        }

        // Instructions box
        boxY += clientBoxHeight + 20;
        ofSetColor(50);
        ofDrawRectangle(20, boxY, 360, 100);
        ofSetColor(200, 200, 200);
        ofDrawRectangle(20, boxY, 360, 25);

        // FPS indicator
        ofSetColor(255, 255, 255, 100);
        float fps = ofGetFrameRate();
        float barWidth = (fps / 60.0f) * 100.0f;
        ofDrawRectangle(ofGetWidth() - 110, 10, barWidth, 5);
    }

    void keyPressed(int key) override {
        if (key == ' ') {
            // Broadcast message on spacebar
            int numClients = server.getNumClients();
            if (numClients > 0) {
                std::string broadcast = "Manual broadcast message";
                server.sendToAll(broadcast + "\n");
                ofLog() << "Broadcast to " << numClients << " clients: " << broadcast;
            } else {
                ofLog() << "No clients connected to broadcast to";
            }
        } else if (key == 'd' || key == 'D') {
            // Disconnect first client
            if (server.getNumClients() > 0) {
                ofLog() << "Disconnecting client 0";
                server.disconnectClient(0);
            }
        } else if (key == 'x' || key == 'X') {
            // Clear all messages
            for (size_t i = 0; i < lastMessages.size(); i++) {
                lastMessages[i] = "";
            }
            totalMessagesReceived = 0;
            ofLog() << "Cleared message history";
        }
    }

    void exit() override {
        if (server.isListening()) {
            server.close();
            ofLog() << "Server closed";
        }
    }
};

// Usage Instructions:
//
// 1. Build and run this example first
//
// 2. Connect clients using netcat:
//    nc localhost 8080
//    Then type messages and press Enter
//
// 3. Or run the tcp_client example (17_tcp_client) in another instance
//
// 4. Replace TestApp in src/platform/bridge/SwiftBridge.mm:
//    #include "../../examples/18_tcp_server/main.cpp"
//    testApp_ = std::make_unique<TcpServerApp>();
//
// Controls:
// - Space: Broadcast message to all connected clients
// - D: Disconnect first client
// - X: Clear message history
