/// \file main.cpp
/// \brief Example 15: GUI Basic
///
/// Demonstrates basic ofxGui usage with SwiftUI:
/// - ofxPanel setup
/// - Float slider (radius control)
/// - Int slider (resolution control)
/// - Boolean toggle (wireframe mode)
/// - Color picker (shape color)
/// - Button (reset values)
/// - Label (display text)
///
/// This example shows how to create a simple GUI panel
/// and use it to control drawing parameters in real-time.

#include "../../src/core/AppBase.h"
#include "../../src/oflike/graphics/ofGraphics.h"
#include "../../src/oflike/types/ofColor.h"
#include "../../src/oflike/utils/ofUtils.h"
#include "../../src/core/Context.h"
#include "../../src/addons/core/ofxGui/ofxGui.h"

class GuiBasicApp : public ofBaseApp {
public:
    // GUI panel
    ofxPanel gui;

    // Parameters controlled by GUI
    float radius = 100.0f;
    int resolution = 32;
    bool wireframe = false;
    ofColor shapeColor = ofColor(255, 100, 100);
    std::string infoText = "GUI Basic Example";

    // Animation
    float angle = 0.0f;
    float centerX = 0.0f;
    float centerY = 0.0f;

    void setup() override {
        // Set frame rate
        ofSetFrameRate(60);

        // Setup GUI panel
        gui.setup("Controls");

        // Add parameters to GUI
        gui.addSlider("Radius", radius, 10.0f, 300.0f);
        gui.addSlider("Resolution", resolution, 3, 128);
        gui.addToggle("Wireframe", wireframe);
        gui.addColor("Shape Color", shapeColor);
        gui.addButton("Reset", [this]() {
            // Reset all values to defaults
            radius = 100.0f;
            resolution = 32;
            wireframe = false;
            shapeColor = ofColor(255, 100, 100);
            infoText = "Values Reset!";
        });
        gui.addLabel("Info", infoText);

        // Initialize positions
        centerX = ofGetWidth() / 2.0f;
        centerY = ofGetHeight() / 2.0f;
    }

    void update() override {
        // Animate rotation
        angle += 0.02f;

        // Update info text
        infoText = "FPS: " + ofToString(static_cast<int>(ofGetFrameRate()));
    }

    void draw() override {
        // Dark background
        ofBackground(30);

        // Set drawing style based on GUI
        if (wireframe) {
            ofNoFill();
            ofSetLineWidth(2.0f);
        } else {
            ofFill();
        }

        // Set color from GUI
        ofSetColor(shapeColor);

        // Set circle resolution from GUI
        ofSetCircleResolution(resolution);

        // Draw circle at center
        ofDrawCircle(centerX, centerY, radius);

        // Draw rotating smaller circles around the main circle
        ofSetCircleResolution(32);
        for (int i = 0; i < 8; i++) {
            float a = angle + (i * TWO_PI / 8.0f);
            float x = centerX + cos(a) * (radius + 50.0f);
            float y = centerY + sin(a) * (radius + 50.0f);

            // Alternate colors
            if (i % 2 == 0) {
                ofSetColor(shapeColor.r, shapeColor.g, shapeColor.b, 150);
            } else {
                ofSetColor(shapeColor.r / 2, shapeColor.g / 2, shapeColor.b / 2, 150);
            }

            ofDrawCircle(x, y, 20);
        }

        // Draw crosshair at center
        ofSetColor(255, 255, 255, 100);
        ofSetLineWidth(1.0f);
        ofDrawLine(centerX - 10, centerY, centerX + 10, centerY);
        ofDrawLine(centerX, centerY - 10, centerX, centerY + 10);

        // Draw corner indicators
        ofNoFill();
        ofSetColor(100, 100, 100, 150);
        ofSetLineWidth(1.0f);

        // Corner rectangles
        ofDrawRectangle(10, 10, 30, 30);
        ofDrawRectangle(ofGetWidth() - 40, 10, 30, 30);
        ofDrawRectangle(10, ofGetHeight() - 40, 30, 30);
        ofDrawRectangle(ofGetWidth() - 40, ofGetHeight() - 40, 30, 30);

        // Note: The GUI panel is rendered by SwiftUI overlay
        // It appears automatically on the right side of the window
    }

    void keyPressed(int key) override {
        if (key == ' ') {
            // Space: toggle wireframe
            wireframe = !wireframe;
        } else if (key == 'r' || key == 'R') {
            // R: randomize color
            shapeColor = ofColor(
                ofRandom(255),
                ofRandom(255),
                ofRandom(255)
            );
            infoText = "Color Randomized!";
        } else if (key == '+' || key == '=') {
            // +: increase radius
            radius = ofClamp(radius + 10.0f, 10.0f, 300.0f);
        } else if (key == '-' || key == '_') {
            // -: decrease radius
            radius = ofClamp(radius - 10.0f, 10.0f, 300.0f);
        }
    }

    void windowResized(int w, int h) override {
        // Recenter on window resize
        centerX = w / 2.0f;
        centerY = h / 2.0f;
    }
};

// To use this example:
// 1. Replace TestApp in src/platform/bridge/SwiftBridge.mm with GuiBasicApp
// 2. Include this file in SwiftBridge.mm:
//    #include "../../examples/15_gui_basic/main.cpp"
// 3. Change app instantiation:
//    testApp_ = std::make_unique<GuiBasicApp>();
// 4. Build and run the Xcode project
//
// The GUI panel will appear on the right side of the window with SwiftUI styling:
// - .ultraThinMaterial translucent background
// - Dark mode support
// - macOS-native controls (sliders, toggles, color picker, buttons)
//
// Controls:
// - Space: Toggle wireframe mode
// - R: Randomize shape color
// - +/-: Increase/decrease radius
// - Mouse: Interact with GUI panel controls
//
// Note: This demonstrates ofxGui integration with SwiftUI.
// The GUI parameters are automatically bound to the C++ variables.
