// OSC Parameter Sync Example
// Demonstrates bidirectional parameter synchronization via OSC

#include "ofMain.h"
#include "ofxOscParameterSync.h"

class ofApp : public ofBaseApp {
public:
    void setup() override {
        ofSetFrameRate(60);
        ofSetWindowTitle("OSC Parameter Sync Example");

        // Setup parameters
        volume.set("volume", 0.5f, 0.0f, 1.0f);
        frequency.set("frequency", 440, 100, 1000);
        enabled.set("enabled", true);

        // Create parameter group
        audioParams.setName("audio");
        audioParams.add(std::make_shared<ofParameter<float>>(volume));
        audioParams.add(std::make_shared<ofParameter<int>>(frequency));
        audioParams.add(std::make_shared<ofParameter<bool>>(enabled));

        // Setup OSC parameter sync
        // Sends to localhost:9001, receives on 9000
        oscSync.setup("localhost", 9001, 9000);

        // Add individual parameters
        oscSync.add(volume, "/audio/volume");
        oscSync.add(frequency, "/audio/frequency");
        oscSync.add(enabled, "/audio/enabled");

        // Or add entire group
        // oscSync.add(audioParams, "/audio");

        ofLogNotice() << "OSC Parameter Sync started";
        ofLogNotice() << "Sending to: localhost:9001";
        ofLogNotice() << "Receiving on: 9000";
        ofLogNotice() << "";
        ofLogNotice() << "Use another OSC app to send/receive:";
        ofLogNotice() << "  /audio/volume [0.0-1.0]";
        ofLogNotice() << "  /audio/frequency [100-1000]";
        ofLogNotice() << "  /audio/enabled [true/false]";
    }

    void update() override {
        // Update OSC sync (processes incoming messages)
        oscSync.update();
    }

    void draw() override {
        ofBackground(30);
        ofSetColor(255);

        // Draw parameter values
        int y = 100;
        int lineHeight = 40;

        ofDrawBitmapString("OSC Parameter Sync Example", 50, 50);
        ofDrawBitmapString("Press keys to change values:", 50, 70);

        // Volume
        ofDrawBitmapString("Volume (Q/A): " + std::to_string(volume.get()), 50, y);
        ofSetColor(100, 200, 100);
        ofDrawRectangle(250, y - 15, volume.get() * 300, 20);
        ofSetColor(255);
        y += lineHeight;

        // Frequency
        ofDrawBitmapString("Frequency (W/S): " + std::to_string(frequency.get()) + " Hz", 50, y);
        float freqNorm = (frequency.get() - 100.0f) / (1000.0f - 100.0f);
        ofSetColor(100, 100, 200);
        ofDrawRectangle(250, y - 15, freqNorm * 300, 20);
        ofSetColor(255);
        y += lineHeight;

        // Enabled
        std::string enabledStr = enabled.get() ? "ON" : "OFF";
        ofDrawBitmapString("Enabled (E): " + enabledStr, 50, y);
        if (enabled.get()) {
            ofSetColor(0, 255, 0);
        } else {
            ofSetColor(255, 0, 0);
        }
        ofDrawRectangle(250, y - 15, 60, 20);
        ofSetColor(255);
        y += lineHeight;

        // Instructions
        y += 40;
        ofDrawBitmapString("OSC Addresses:", 50, y);
        y += 25;
        ofSetColor(150);
        ofDrawBitmapString("/audio/volume - Float [0.0-1.0]", 70, y);
        y += 20;
        ofDrawBitmapString("/audio/frequency - Int [100-1000]", 70, y);
        y += 20;
        ofDrawBitmapString("/audio/enabled - Bool [0/1]", 70, y);
        ofSetColor(255);

        y += 40;
        ofDrawBitmapString("Send OSC to localhost:9001 to control parameters", 50, y);
        y += 20;
        ofDrawBitmapString("Receive OSC on localhost:9000 to monitor changes", 50, y);
    }

    void keyPressed(int key) override {
        // Volume control
        if (key == 'q' || key == 'Q') {
            volume.set(std::min(1.0f, volume.get() + 0.1f));
        }
        if (key == 'a' || key == 'A') {
            volume.set(std::max(0.0f, volume.get() - 0.1f));
        }

        // Frequency control
        if (key == 'w' || key == 'W') {
            frequency.set(std::min(1000, frequency.get() + 50));
        }
        if (key == 's' || key == 'S') {
            frequency.set(std::max(100, frequency.get() - 50));
        }

        // Toggle enabled
        if (key == 'e' || key == 'E') {
            enabled.set(!enabled.get());
        }

        // Send all parameters
        if (key == ' ') {
            oscSync.sendAll();
            ofLogNotice() << "Sent all parameters via OSC";
        }
    }

private:
    ofxOscParameterSync oscSync;

    // Parameters
    ofParameter<float> volume;
    ofParameter<int> frequency;
    ofParameter<bool> enabled;

    ofParameterGroup audioParams;
};

int main() {
    ofSetupOpenGL(800, 400, OF_WINDOW);
    ofRunApp(new ofApp());
}
