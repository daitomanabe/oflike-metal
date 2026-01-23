/// \file main.cpp
/// \brief Example 01: Basic Drawing
///
/// Demonstrates basic oflike-metal drawing capabilities:
/// - ofApp structure (setup/update/draw)
/// - Background color
/// - Basic 2D shapes (circle, rectangle, line, triangle)
/// - Color setting
/// - Simple animation
///
/// This example shows the core openFrameworks API pattern.
/// In a production setup, replace TestApp in SwiftBridge.mm with this app class.

#include "../../src/core/AppBase.h"
#include "../../src/oflike/graphics/ofGraphics.h"
#include "../../src/oflike/types/ofColor.h"
#include "../../src/oflike/utils/ofUtils.h"
#include "../../src/core/Context.h"

class BasicDrawingApp : public ofBaseApp {
public:
    float circleX = 0.0f;
    float circleY = 0.0f;
    float circleRadius = 50.0f;
    float angle = 0.0f;

    void setup() override {
        // Set initial circle position to center
        circleX = ofGetWidth() / 2.0f;
        circleY = ofGetHeight() / 2.0f;

        // Set frame rate
        ofSetFrameRate(60);
    }

    void update() override {
        // Animate circle position in a circular path
        angle += 0.02f;
        float orbitRadius = 100.0f;
        circleX = ofGetWidth() / 2.0f + cos(angle) * orbitRadius;
        circleY = ofGetHeight() / 2.0f + sin(angle) * orbitRadius;
    }

    void draw() override {
        // Set background to dark gray
        ofBackground(40);

        // Draw center point
        ofSetColor(255, 255, 255);
        ofDrawCircle(ofGetWidth() / 2.0f, ofGetHeight() / 2.0f, 5);

        // Draw orbit path (circle outline)
        ofNoFill();
        ofSetColor(80, 80, 80);
        ofSetLineWidth(1.0f);
        ofDrawCircle(ofGetWidth() / 2.0f, ofGetHeight() / 2.0f, 100);

        // Draw animated circle
        ofFill();
        ofSetColor(255, 100, 100);
        ofDrawCircle(circleX, circleY, circleRadius);

        // Draw static shapes in corners

        // Top-left: Red rectangle
        ofSetColor(255, 0, 0);
        ofDrawRectangle(50, 50, 80, 60);

        // Top-right: Green triangle
        ofSetColor(0, 255, 0);
        ofDrawTriangle(
            ofGetWidth() - 130, 50,
            ofGetWidth() - 50, 50,
            ofGetWidth() - 90, 110
        );

        // Bottom-left: Blue circle
        ofSetColor(0, 100, 255);
        ofDrawCircle(90, ofGetHeight() - 90, 40);

        // Bottom-right: Yellow rectangle (outlined)
        ofNoFill();
        ofSetColor(255, 255, 0);
        ofSetLineWidth(3.0f);
        ofDrawRectangle(ofGetWidth() - 130, ofGetHeight() - 110, 80, 60);

        // Draw connecting lines
        ofSetColor(100, 100, 100, 100);
        ofSetLineWidth(1.0f);
        ofDrawLine(50, 50, circleX, circleY);
        ofDrawLine(ofGetWidth() - 90, 50, circleX, circleY);
        ofDrawLine(90, ofGetHeight() - 90, circleX, circleY);
        ofDrawLine(ofGetWidth() - 90, ofGetHeight() - 80, circleX, circleY);

        // Draw text information (simple, using rectangles as placeholders)
        // Note: Text rendering would use ofTrueTypeFont when integrated
        ofFill();
        ofSetColor(255, 255, 255, 200);

        // FPS counter placeholder (top-center)
        // In a full app, this would use ofDrawBitmapString or ofTrueTypeFont
        float fps = ofGetFrameRate();
        // For now, just draw a small indicator that updates
        int fpsInt = static_cast<int>(fps);
        float barWidth = (fpsInt / 60.0f) * 100.0f;
        ofDrawRectangle(ofGetWidth() / 2.0f - 50, 10, barWidth, 5);
    }

    void keyPressed(int key) override {
        if (key == ' ') {
            // Toggle fill on space key
            static bool filled = true;
            filled = !filled;
            if (filled) {
                ofFill();
            } else {
                ofNoFill();
            }
        }
    }

    void windowResized(int w, int h) override {
        // Recenter on window resize
        circleX = w / 2.0f;
        circleY = h / 2.0f;
    }
};

// To use this example:
// 1. Replace TestApp in src/platform/bridge/SwiftBridge.mm with BasicDrawingApp
// 2. Include this file's header in SwiftBridge.mm
// 3. Build and run the Xcode project
//
// Example modification in SwiftBridge.mm:
//   #include "../../examples/01_basics/main.cpp"  // Include BasicDrawingApp
//   // In setup():
//   testApp_ = std::make_unique<BasicDrawingApp>();  // Replace TestApp

// Note: This example demonstrates API usage.
// Integration with the main app will be improved in Phase 18.
