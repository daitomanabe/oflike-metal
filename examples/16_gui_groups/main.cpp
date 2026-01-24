/// \file main.cpp
/// \brief Example 16: GUI Groups
///
/// Demonstrates advanced ofxGui usage with parameter groups:
/// - ofxGuiGroup for organizing parameters
/// - Multiple groups (Shape, Animation, Rendering, Colors)
/// - Nested parameter organization
/// - Group tabs for navigation
/// - Complex parameter relationships
///
/// This example shows how to organize a complex GUI
/// with multiple parameter groups for better usability.

#include "../../src/core/AppBase.h"
#include "../../src/oflike/graphics/ofGraphics.h"
#include "../../src/oflike/types/ofColor.h"
#include "../../src/oflike/utils/ofUtils.h"
#include "../../src/core/Context.h"
#include "../../src/addons/core/ofxGui/ofxGui.h"

class GuiGroupsApp : public ofBaseApp {
public:
    // GUI
    ofxPanel gui;
    ofxGuiGroup shapeGroup;
    ofxGuiGroup animationGroup;
    ofxGuiGroup renderingGroup;
    ofxGuiGroup colorsGroup;

    // Shape parameters
    float shapeRadius = 80.0f;
    int shapeResolution = 32;
    int shapeCount = 8;

    // Animation parameters
    float rotationSpeed = 1.0f;
    float orbitRadius = 120.0f;
    bool autoRotate = true;
    bool oscillate = false;

    // Rendering parameters
    bool wireframe = false;
    float lineWidth = 2.0f;
    int alphaValue = 255;
    bool showTrails = false;

    // Color parameters
    ofColor backgroundColor = ofColor(20, 20, 30);
    ofColor mainColor = ofColor(255, 100, 100);
    ofColor accentColor = ofColor(100, 200, 255);

    // Animation state
    float angle = 0.0f;
    float oscillation = 0.0f;
    float centerX = 0.0f;
    float centerY = 0.0f;

    void setup() override {
        // Set frame rate
        ofSetFrameRate(60);

        // Setup main GUI panel
        gui.setup("Advanced Controls");

        // Setup groups
        shapeGroup.setName("Shape");
        animationGroup.setName("Animation");
        renderingGroup.setName("Rendering");
        colorsGroup.setName("Colors");

        // Add parameters to Shape group
        shapeGroup.addSlider("Radius", shapeRadius, 20.0f, 200.0f);
        shapeGroup.addSlider("Resolution", shapeResolution, 3, 128);
        shapeGroup.addSlider("Count", shapeCount, 1, 16);
        shapeGroup.addButton("Randomize", [this]() {
            shapeRadius = ofRandom(20.0f, 200.0f);
            shapeResolution = static_cast<int>(ofRandom(8, 64));
            shapeCount = static_cast<int>(ofRandom(3, 12));
        });

        // Add parameters to Animation group
        animationGroup.addSlider("Rotation Speed", rotationSpeed, 0.0f, 5.0f);
        animationGroup.addSlider("Orbit Radius", orbitRadius, 50.0f, 300.0f);
        animationGroup.addToggle("Auto Rotate", autoRotate);
        animationGroup.addToggle("Oscillate", oscillate);
        animationGroup.addButton("Reset Animation", [this]() {
            angle = 0.0f;
            oscillation = 0.0f;
            rotationSpeed = 1.0f;
        });

        // Add parameters to Rendering group
        renderingGroup.addToggle("Wireframe", wireframe);
        renderingGroup.addSlider("Line Width", lineWidth, 1.0f, 10.0f);
        renderingGroup.addSlider("Alpha", alphaValue, 0, 255);
        renderingGroup.addToggle("Show Trails", showTrails);

        // Add parameters to Colors group
        colorsGroup.addColor("Background", backgroundColor);
        colorsGroup.addColor("Main Color", mainColor);
        colorsGroup.addColor("Accent Color", accentColor);
        colorsGroup.addButton("Randomize Colors", [this]() {
            backgroundColor = ofColor(ofRandom(50), ofRandom(50), ofRandom(50));
            mainColor = ofColor(ofRandom(255), ofRandom(255), ofRandom(255));
            accentColor = ofColor(ofRandom(255), ofRandom(255), ofRandom(255));
        });

        // Add groups to panel
        gui.add(shapeGroup);
        gui.add(animationGroup);
        gui.add(renderingGroup);
        gui.add(colorsGroup);

        // Initialize positions
        centerX = ofGetWidth() / 2.0f;
        centerY = ofGetHeight() / 2.0f;
    }

    void update() override {
        // Update animation
        if (autoRotate) {
            angle += 0.01f * rotationSpeed;
        }

        if (oscillate) {
            oscillation += 0.05f * rotationSpeed;
        }
    }

    void draw() override {
        // Set background color from GUI
        ofBackground(backgroundColor);

        // Apply rendering settings
        if (wireframe) {
            ofNoFill();
            ofSetLineWidth(lineWidth);
        } else {
            ofFill();
        }

        // Calculate oscillation offset
        float oscOffset = oscillate ? sin(oscillation) * 30.0f : 0.0f;

        // Set circle resolution
        ofSetCircleResolution(shapeResolution);

        // Draw center shape
        ofColor centerColor = mainColor;
        centerColor.a = alphaValue;
        ofSetColor(centerColor);
        ofDrawCircle(centerX, centerY, shapeRadius + oscOffset);

        // Draw orbiting shapes
        for (int i = 0; i < shapeCount; i++) {
            float a = angle + (i * TWO_PI / shapeCount);
            float x = centerX + cos(a) * orbitRadius;
            float y = centerY + sin(a) * orbitRadius;

            // Alternate between main and accent colors
            ofColor shapeColor = (i % 2 == 0) ? mainColor : accentColor;
            shapeColor.a = alphaValue;
            ofSetColor(shapeColor);

            float size = shapeRadius * 0.3f + oscOffset * 0.5f;
            ofDrawCircle(x, y, size);

            // Draw connecting lines if not in wireframe mode
            if (!wireframe && showTrails) {
                ofSetColor(shapeColor.r, shapeColor.g, shapeColor.b, alphaValue / 3);
                ofSetLineWidth(1.0f);
                ofDrawLine(centerX, centerY, x, y);
            }
        }

        // Draw orbit circle
        if (showTrails) {
            ofNoFill();
            ofSetColor(mainColor.r / 2, mainColor.g / 2, mainColor.b / 2, 100);
            ofSetLineWidth(1.0f);
            ofDrawCircle(centerX, centerY, orbitRadius);
        }

        // Draw center crosshair
        ofSetColor(255, 255, 255, 50);
        ofSetLineWidth(1.0f);
        ofDrawLine(centerX - 15, centerY, centerX + 15, centerY);
        ofDrawLine(centerX, centerY - 15, centerX, centerY + 15);

        // Draw corner frame
        ofNoFill();
        ofSetColor(accentColor.r, accentColor.g, accentColor.b, 80);
        ofSetLineWidth(2.0f);
        float frameSize = 40.0f;
        float margin = 20.0f;

        // Top-left corner
        ofDrawLine(margin, margin, margin + frameSize, margin);
        ofDrawLine(margin, margin, margin, margin + frameSize);

        // Top-right corner
        ofDrawLine(ofGetWidth() - margin - frameSize, margin, ofGetWidth() - margin, margin);
        ofDrawLine(ofGetWidth() - margin, margin, ofGetWidth() - margin, margin + frameSize);

        // Bottom-left corner
        ofDrawLine(margin, ofGetHeight() - margin - frameSize, margin, ofGetHeight() - margin);
        ofDrawLine(margin, ofGetHeight() - margin, margin + frameSize, ofGetHeight() - margin);

        // Bottom-right corner
        ofDrawLine(ofGetWidth() - margin - frameSize, ofGetHeight() - margin, ofGetWidth() - margin, ofGetHeight() - margin);
        ofDrawLine(ofGetWidth() - margin, ofGetHeight() - margin - frameSize, ofGetWidth() - margin, ofGetHeight() - margin);

        // Note: The GUI panel is rendered by SwiftUI overlay
        // Group tabs appear at the top of the panel for easy navigation
    }

    void keyPressed(int key) override {
        if (key == ' ') {
            // Space: toggle auto-rotate
            autoRotate = !autoRotate;
        } else if (key == 'w' || key == 'W') {
            // W: toggle wireframe
            wireframe = !wireframe;
        } else if (key == 't' || key == 'T') {
            // T: toggle trails
            showTrails = !showTrails;
        } else if (key == 'o' || key == 'O') {
            // O: toggle oscillation
            oscillate = !oscillate;
        } else if (key == 'r' || key == 'R') {
            // R: randomize all
            shapeRadius = ofRandom(20.0f, 200.0f);
            shapeResolution = static_cast<int>(ofRandom(8, 64));
            shapeCount = static_cast<int>(ofRandom(3, 12));
            backgroundColor = ofColor(ofRandom(50), ofRandom(50), ofRandom(50));
            mainColor = ofColor(ofRandom(255), ofRandom(255), ofRandom(255));
            accentColor = ofColor(ofRandom(255), ofRandom(255), ofRandom(255));
        } else if (key == '+' || key == '=') {
            // +: increase rotation speed
            rotationSpeed = ofClamp(rotationSpeed + 0.2f, 0.0f, 5.0f);
        } else if (key == '-' || key == '_') {
            // -: decrease rotation speed
            rotationSpeed = ofClamp(rotationSpeed - 0.2f, 0.0f, 5.0f);
        }
    }

    void windowResized(int w, int h) override {
        // Recenter on window resize
        centerX = w / 2.0f;
        centerY = h / 2.0f;
    }
};

// To use this example:
// 1. Replace TestApp in src/platform/bridge/SwiftBridge.mm with GuiGroupsApp
// 2. Include this file in SwiftBridge.mm:
//    #include "../../examples/16_gui_groups/main.cpp"
// 3. Change app instantiation:
//    testApp_ = std::make_unique<GuiGroupsApp>();
// 4. Build and run the Xcode project
//
// The GUI panel will appear with multiple groups:
// - "Shape" group: controls for shape appearance
// - "Animation" group: controls for animation parameters
// - "Rendering" group: controls for drawing style
// - "Colors" group: color pickers for all colors
//
// Group tabs at the top of the panel allow switching between parameter sets.
// The "All" tab shows all parameters from all groups.
//
// Controls:
// - Space: Toggle auto-rotation
// - W: Toggle wireframe mode
// - T: Toggle trails/orbit circle
// - O: Toggle oscillation effect
// - R: Randomize all parameters
// - +/-: Increase/decrease rotation speed
// - Mouse: Interact with GUI panel controls
//
// This example demonstrates:
// - Parameter organization with ofxGuiGroup
// - Group-based navigation with tabs
// - Complex parameter relationships
// - Button callbacks for actions
// - Real-time parameter updates
//
// Note: All GUI parameters are automatically synchronized
// with C++ variables through SwiftUI binding.
