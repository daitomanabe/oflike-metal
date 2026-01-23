/*
 * 12_easycam - ofEasyCam Focused Demonstration
 *
 * Demonstrates ofEasyCam features with:
 * - Mouse orbit control (left drag to rotate)
 * - Mouse zoom control (right drag or scroll wheel)
 * - Target setting and manipulation
 * - Distance control
 * - Inertia toggle
 * - Auto distance
 * - Reset functionality
 * - Interactive scene with multiple objects
 * - Visual feedback for camera state
 * - Mouse input enable/disable
 *
 * Controls:
 * - Mouse Left Drag: Orbit around target
 * - Mouse Right Drag: Zoom in/out
 * - Mouse Wheel: Zoom in/out
 * - 1-5: Set different preset targets
 * - +/-: Adjust distance
 * - I: Toggle inertia
 * - M: Toggle mouse input
 * - A: Toggle auto distance
 * - R: Reset to default view
 * - T: Animate target position
 * - SPACE: Reset camera orientation
 *
 * Integration:
 * - Copy this file to your project
 * - Link against oflike-metal library
 * - Build and run
 */

#include "../../src/core/AppBase.h"
#include "../../src/oflike/graphics/ofGraphics.h"
#include "../../src/oflike/3d/ofEasyCam.h"
#include "../../src/oflike/lighting/ofLight.h"
#include "../../src/oflike/lighting/ofMaterial.h"
#include "../../src/oflike/graphics/ofTrueTypeFont.h"
#include "../../src/oflike/utils/ofUtils.h"
#include "../../src/oflike/math/ofMath.h"

using namespace oflike;

class EasyCamApp : public ofBaseApp {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void keyPressed(int key) override;

private:
    void drawScene();
    void drawGrid();
    void drawTarget();
    void drawCameraInfo();
    void drawControls();
    void setPresetTarget(int preset);

    // Camera
    ofEasyCam camera_;

    // Scene objects
    ofLight light1_;
    ofLight light2_;
    ofMaterial material_;

    // Target animation
    bool animateTarget_ = false;
    float animationTime_ = 0.0f;
    ofVec3f targetCenter_;

    // Preset targets
    ofVec3f presetTargets_[5];

    // Camera state tracking
    bool inertiaEnabled_ = true;
    bool mouseInputEnabled_ = true;
    bool autoDistance_ = false;

    // Font
    ofTrueTypeFont font_;

    // Colors
    ofColor targetColor_;
    ofColor gridColor_;
};

void EasyCamApp::setup() {
    ofSetFrameRate(60);
    ofSetWindowTitle("12_easycam - ofEasyCam Control");
    ofBackground(20, 20, 25);

    // Setup camera
    camera_.setDistance(400);
    camera_.setTarget(ofVec3f(0, 0, 0));
    camera_.setNearClip(1.0f);
    camera_.setFarClip(10000.0f);
    camera_.setFov(60.0f);
    camera_.enableMouseInput();
    camera_.enableInertia();

    // Setup lights
    light1_.setPointLight();
    light1_.setPosition(300, 400, 200);
    light1_.setAmbientColor(ofColor(40, 40, 50));
    light1_.setDiffuseColor(ofColor(255, 220, 180));
    light1_.setSpecularColor(ofColor(255, 255, 255));

    light2_.setPointLight();
    light2_.setPosition(-300, 200, -200);
    light2_.setAmbientColor(ofColor(20, 20, 30));
    light2_.setDiffuseColor(ofColor(180, 200, 255));
    light2_.setSpecularColor(ofColor(255, 255, 255));

    // Setup material
    material_.setAmbientColor(ofColor(60, 60, 80));
    material_.setDiffuseColor(ofColor(150, 180, 255));
    material_.setSpecularColor(ofColor(255, 255, 255));
    material_.setShininess(128.0f);

    // Initialize preset targets
    presetTargets_[0] = ofVec3f(0, 0, 0);       // Center
    presetTargets_[1] = ofVec3f(100, 50, 0);    // Right
    presetTargets_[2] = ofVec3f(-100, 50, 0);   // Left
    presetTargets_[3] = ofVec3f(0, 100, 100);   // Top-front
    presetTargets_[4] = ofVec3f(0, -50, -100);  // Bottom-back

    targetCenter_ = ofVec3f(0, 0, 0);

    // Colors
    targetColor_ = ofColor(255, 100, 100);
    gridColor_ = ofColor(60, 60, 70);

    // Load font
    if (!font_.load("Helvetica", 12)) {
        ofLogWarning("EasyCamApp") << "Font loading failed (OK for headless)";
    }

    ofLogNotice("EasyCamApp") << "Setup complete - use mouse to orbit and zoom";
}

void EasyCamApp::update() {
    animationTime_ += ofGetLastFrameTime();

    // Animate target if enabled
    if (animateTarget_) {
        float radius = 80.0f;
        float speed = 1.0f;
        ofVec3f offset(
            cos(animationTime_ * speed) * radius,
            sin(animationTime_ * speed * 0.7f) * radius * 0.5f,
            sin(animationTime_ * speed) * radius
        );
        camera_.setTarget(targetCenter_ + offset);
    }
}

void EasyCamApp::draw() {
    ofBackground(20, 20, 25);

    // Begin camera
    camera_.begin();

    // Enable lighting
    ofEnableLighting();
    light1_.enable();
    light2_.enable();

    // Draw grid for reference
    drawGrid();

    // Draw target marker
    drawTarget();

    // Draw scene
    drawScene();

    // Disable lighting
    light1_.disable();
    light2_.disable();
    ofDisableLighting();

    // End camera
    camera_.end();

    // Draw 2D overlay
    drawCameraInfo();
    drawControls();
}

void EasyCamApp::drawScene() {
    material_.begin();

    // Central large sphere at origin
    ofPushMatrix();
    ofSetColor(255);
    ofDrawSphere(0, 0, 0, 50);
    ofPopMatrix();

    // Orbiting cubes
    float time = animationTime_;
    for (int i = 0; i < 8; i++) {
        float angle = (i / 8.0f) * TWO_PI + time * 0.5f;
        float radius = 150.0f;
        float height = sin(time + i) * 30.0f;

        ofPushMatrix();
        ofTranslate(
            cos(angle) * radius,
            height,
            sin(angle) * radius
        );
        ofRotateYDeg(time * 30.0f + i * 45.0f);
        ofSetColor(255);
        ofDrawBox(30, 30, 30);
        ofPopMatrix();
    }

    // Corner spheres at preset positions
    for (int i = 1; i < 5; i++) {
        ofPushMatrix();
        ofTranslate(presetTargets_[i]);
        ofSetColor(200, 150 + i * 20, 100);
        ofDrawSphere(20);
        ofPopMatrix();
    }

    material_.end();
}

void EasyCamApp::drawGrid() {
    ofPushMatrix();
    ofPushStyle();

    ofSetColor(gridColor_);
    ofSetLineWidth(1);

    int gridSize = 500;
    int step = 50;

    // Draw grid lines
    for (int x = -gridSize; x <= gridSize; x += step) {
        ofDrawLine(x, 0, -gridSize, x, 0, gridSize);
        ofDrawLine(-gridSize, 0, x, gridSize, 0, x);
    }

    // Draw axes
    ofSetLineWidth(2);

    // X axis (red)
    ofSetColor(255, 50, 50);
    ofDrawLine(0, 0, 0, 200, 0, 0);
    ofDrawCone(200, 0, 0, 10, 20);

    // Y axis (green)
    ofSetColor(50, 255, 50);
    ofDrawLine(0, 0, 0, 0, 200, 0);
    ofPushMatrix();
    ofTranslate(0, 200, 0);
    ofRotateXDeg(-90);
    ofDrawCone(10, 20);
    ofPopMatrix();

    // Z axis (blue)
    ofSetColor(50, 50, 255);
    ofDrawLine(0, 0, 0, 0, 0, 200);
    ofPushMatrix();
    ofTranslate(0, 0, 200);
    ofRotateXDeg(90);
    ofDrawCone(10, 20);
    ofPopMatrix();

    ofPopStyle();
    ofPopMatrix();
}

void EasyCamApp::drawTarget() {
    ofVec3f target = camera_.getTarget();

    ofPushMatrix();
    ofPushStyle();

    ofTranslate(target);

    // Draw crosshair
    ofSetColor(targetColor_, 200);
    ofSetLineWidth(2);
    ofNoFill();

    // Rotating rings
    float time = animationTime_;
    ofPushMatrix();
    ofRotateYDeg(time * 30.0f);
    ofDrawCircle(0, 0, 0, 30);
    ofPopMatrix();

    ofPushMatrix();
    ofRotateXDeg(90);
    ofRotateZDeg(time * -45.0f);
    ofDrawCircle(0, 0, 0, 25);
    ofPopMatrix();

    // Lines
    ofDrawLine(-40, 0, 0, -15, 0, 0);
    ofDrawLine(15, 0, 0, 40, 0, 0);
    ofDrawLine(0, -40, 0, 0, -15, 0);
    ofDrawLine(0, 15, 0, 0, 40, 0);
    ofDrawLine(0, 0, -40, 0, 0, -15);
    ofDrawLine(0, 0, 15, 0, 0, 40);

    // Center sphere
    ofFill();
    ofDrawSphere(0, 0, 0, 5);

    ofPopStyle();
    ofPopMatrix();
}

void EasyCamApp::drawCameraInfo() {
    if (!font_.isLoaded()) return;

    float x = 20;
    float y = 30;
    float lineHeight = 18;

    ofSetColor(255, 240);
    font_.drawString("EASYCAM INFO", x, y);
    y += lineHeight * 1.5f;

    ofVec3f target = camera_.getTarget();
    float distance = camera_.getDistance();

    ofSetColor(200);
    font_.drawString("Target: (" + ofToString(target.x, 1) + ", " +
                    ofToString(target.y, 1) + ", " +
                    ofToString(target.z, 1) + ")", x, y);
    y += lineHeight;

    font_.drawString("Distance: " + ofToString(distance, 1), x, y);
    y += lineHeight;

    y += lineHeight * 0.5f;

    // State indicators
    ofSetColor(mouseInputEnabled_ ? ofColor(100, 255, 100) : ofColor(100, 100, 100));
    font_.drawString("Mouse Input: " + std::string(mouseInputEnabled_ ? "ON" : "OFF"), x, y);
    y += lineHeight;

    ofSetColor(inertiaEnabled_ ? ofColor(100, 255, 100) : ofColor(100, 100, 100));
    font_.drawString("Inertia: " + std::string(inertiaEnabled_ ? "ON" : "OFF"), x, y);
    y += lineHeight;

    ofSetColor(autoDistance_ ? ofColor(100, 255, 100) : ofColor(100, 100, 100));
    font_.drawString("Auto Distance: " + std::string(autoDistance_ ? "ON" : "OFF"), x, y);
    y += lineHeight;

    ofSetColor(animateTarget_ ? ofColor(255, 200, 100) : ofColor(100, 100, 100));
    font_.drawString("Target Animation: " + std::string(animateTarget_ ? "ON" : "OFF"), x, y);
}

void EasyCamApp::drawControls() {
    if (!font_.isLoaded()) return;

    float x = ofGetWidth() - 280;
    float y = 30;
    float lineHeight = 18;

    ofSetColor(255, 240);
    font_.drawString("CONTROLS", x, y);
    y += lineHeight * 1.5f;

    ofSetColor(150);
    font_.drawString("Left Drag: Orbit camera", x, y);
    y += lineHeight;
    font_.drawString("Right Drag: Zoom", x, y);
    y += lineHeight;
    font_.drawString("Mouse Wheel: Zoom", x, y);
    y += lineHeight;

    y += lineHeight * 0.5f;

    font_.drawString("1-5: Preset targets", x, y);
    y += lineHeight;
    font_.drawString("+/-: Adjust distance", x, y);
    y += lineHeight;
    font_.drawString("I: Toggle inertia", x, y);
    y += lineHeight;
    font_.drawString("M: Toggle mouse input", x, y);
    y += lineHeight;
    font_.drawString("A: Toggle auto distance", x, y);
    y += lineHeight;
    font_.drawString("T: Animate target", x, y);
    y += lineHeight;
    font_.drawString("R: Reset view", x, y);
    y += lineHeight;
    font_.drawString("SPACE: Reset orientation", x, y);
}

void EasyCamApp::keyPressed(int key) {
    switch (key) {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
            setPresetTarget(key - '1');
            animateTarget_ = false;
            ofLogNotice("EasyCamApp") << "Set preset target " << (key - '0');
            break;

        case '+':
        case '=':
            {
                float dist = camera_.getDistance();
                camera_.setDistance(dist - 20.0f);
                ofLogNotice("EasyCamApp") << "Distance: " << camera_.getDistance();
            }
            break;

        case '-':
        case '_':
            {
                float dist = camera_.getDistance();
                camera_.setDistance(dist + 20.0f);
                ofLogNotice("EasyCamApp") << "Distance: " << camera_.getDistance();
            }
            break;

        case 'i':
        case 'I':
            inertiaEnabled_ = !inertiaEnabled_;
            if (inertiaEnabled_) {
                camera_.enableInertia();
                ofLogNotice("EasyCamApp") << "Inertia enabled";
            } else {
                camera_.disableInertia();
                ofLogNotice("EasyCamApp") << "Inertia disabled";
            }
            break;

        case 'm':
        case 'M':
            mouseInputEnabled_ = !mouseInputEnabled_;
            if (mouseInputEnabled_) {
                camera_.enableMouseInput();
                ofLogNotice("EasyCamApp") << "Mouse input enabled";
            } else {
                camera_.disableMouseInput();
                ofLogNotice("EasyCamApp") << "Mouse input disabled";
            }
            break;

        case 'a':
        case 'A':
            autoDistance_ = !autoDistance_;
            camera_.setAutoDistance(autoDistance_);
            ofLogNotice("EasyCamApp") << "Auto distance: " << (autoDistance_ ? "ON" : "OFF");
            break;

        case 't':
        case 'T':
            animateTarget_ = !animateTarget_;
            if (animateTarget_) {
                targetCenter_ = camera_.getTarget();
                ofLogNotice("EasyCamApp") << "Target animation started";
            } else {
                ofLogNotice("EasyCamApp") << "Target animation stopped";
            }
            break;

        case 'r':
        case 'R':
            camera_.setDistance(400);
            camera_.setTarget(ofVec3f(0, 0, 0));
            animateTarget_ = false;
            ofLogNotice("EasyCamApp") << "Camera reset to default";
            break;

        case ' ':
            // Reset camera orientation while keeping target and distance
            camera_.setPosition(0, 0, camera_.getDistance());
            camera_.lookAt(camera_.getTarget());
            ofLogNotice("EasyCamApp") << "Camera orientation reset";
            break;
    }
}

void EasyCamApp::setPresetTarget(int preset) {
    if (preset >= 0 && preset < 5) {
        camera_.setTarget(presetTargets_[preset]);
        ofLogNotice("EasyCamApp") << "Target set to preset " << preset
                                   << ": (" << presetTargets_[preset].x
                                   << ", " << presetTargets_[preset].y
                                   << ", " << presetTargets_[preset].z << ")";
    }
}

int main() {
    ofSetupOpenGL(1280, 720, OF_WINDOW);
    ofRunApp(new EasyCamApp());
    return 0;
}
