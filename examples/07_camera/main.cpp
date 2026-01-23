/// \file main.cpp
/// \brief Example 07: Camera - ofCamera and ofEasyCam demonstrations
///
/// Demonstrates camera systems with oflike-metal:
/// - ofCamera: Manual camera control with position, orientation, lookAt
/// - ofEasyCam: Interactive camera with automatic mouse orbit/zoom
/// - Camera switching (toggle between manual and EasyCam)
/// - Multiple 3D objects for reference
/// - Camera position display
/// - Smooth transitions between cameras
///
/// Controls:
/// - TAB: Switch between manual camera and EasyCam
/// - W/S: Move camera forward/backward (manual mode only)
/// - A/D: Move camera left/right (manual mode only)
/// - Q/E: Move camera up/down (manual mode only)
/// - Arrow keys: Rotate camera (manual mode only)
/// - Mouse drag: Orbit camera (EasyCam mode only)
/// - Mouse wheel: Zoom in/out (EasyCam mode only)
/// - SPACE: Toggle animation
/// - R: Reset camera to default position

#include "../../src/core/AppBase.h"
#include "../../src/oflike/graphics/ofGraphics.h"
#include "../../src/oflike/3d/ofCamera.h"
#include "../../src/oflike/3d/ofEasyCam.h"
#include "../../src/oflike/lighting/ofLight.h"
#include "../../src/oflike/lighting/ofMaterial.h"
#include "../../src/oflike/types/ofColor.h"
#include "../../src/oflike/utils/ofUtils.h"
#include "../../src/oflike/utils/ofLog.h"
#include <cmath>

using namespace oflike;

class CameraApp : public ofBaseApp {
public:
    // Cameras
    ofCamera manualCamera;
    ofEasyCam easyCamera;
    bool useEasyCam = false;

    // Manual camera control state
    float cameraSpeed = 5.0f;
    float rotationSpeed = 2.0f;
    ofVec3f cameraVelocity;
    float cameraPitch = 0.0f;   // X rotation
    float cameraYaw = 0.0f;     // Y rotation

    // Animation
    bool animating = true;
    float animationTime = 0.0f;

    // Lighting
    ofLight light1;
    ofLight light2;
    ofMaterial material1;
    ofMaterial material2;
    ofMaterial material3;

    void setup() override {
        ofSetFrameRate(60);

        // Setup manual camera
        manualCamera.setPosition(0, 200, 500);
        manualCamera.lookAt(ofVec3f(0, 0, 0));
        manualCamera.setNearClip(1.0f);
        manualCamera.setFarClip(10000.0f);
        manualCamera.setFov(60.0f);

        // Setup EasyCam
        easyCamera.setDistance(500);
        easyCamera.setTarget(ofVec3f(0, 0, 0));
        easyCamera.setNearClip(1.0f);
        easyCamera.setFarClip(10000.0f);
        easyCamera.setFov(60.0f);
        easyCamera.enableMouseInput();
        easyCamera.enableInertia();

        // Setup lights
        light1.setPointLight();
        light1.setPosition(300, 400, 200);
        light1.setAmbientColor(ofColor(40, 40, 40));
        light1.setDiffuseColor(ofColor(255, 200, 150));
        light1.setSpecularColor(ofColor(255, 255, 255));

        light2.setPointLight();
        light2.setPosition(-300, 200, -200);
        light2.setAmbientColor(ofColor(20, 20, 20));
        light2.setDiffuseColor(ofColor(150, 150, 255));
        light2.setSpecularColor(ofColor(255, 255, 255));

        // Setup materials
        material1.setAmbientColor(ofColor(100, 50, 50));
        material1.setDiffuseColor(ofColor(255, 100, 100));
        material1.setSpecularColor(ofColor(255, 255, 255));
        material1.setShininess(64.0f);

        material2.setAmbientColor(ofColor(50, 100, 50));
        material2.setDiffuseColor(ofColor(100, 255, 100));
        material2.setSpecularColor(ofColor(255, 255, 255));
        material2.setShininess(128.0f);

        material3.setAmbientColor(ofColor(50, 50, 100));
        material3.setDiffuseColor(ofColor(100, 100, 255));
        material3.setSpecularColor(ofColor(255, 255, 255));
        material3.setShininess(32.0f);

        ofLogNotice() << "07_camera: Setup complete";
        ofLogNotice() << "Controls:";
        ofLogNotice() << "  TAB: Switch camera mode (Manual/EasyCam)";
        ofLogNotice() << "  W/A/S/D/Q/E: Move camera (Manual mode)";
        ofLogNotice() << "  Arrow keys: Rotate camera (Manual mode)";
        ofLogNotice() << "  Mouse drag: Orbit (EasyCam mode)";
        ofLogNotice() << "  Mouse wheel: Zoom (EasyCam mode)";
        ofLogNotice() << "  SPACE: Toggle animation";
        ofLogNotice() << "  R: Reset camera";
        ofLogNotice() << "Current mode: Manual Camera";
    }

    void update() override {
        // Update animation time
        if (animating) {
            animationTime += 0.01f;
        }

        // Update EasyCam (for inertia)
        if (useEasyCam) {
            easyCamera.update();
        }

        // Update manual camera position
        if (!useEasyCam) {
            // Apply velocity to camera position
            ofVec3f forward = manualCamera.getOrientation() * ofVec3f(0, 0, -1);
            ofVec3f right = manualCamera.getOrientation() * ofVec3f(1, 0, 0);
            ofVec3f up = ofVec3f(0, 1, 0);

            ofVec3f movement = forward * cameraVelocity.z +
                              right * cameraVelocity.x +
                              up * cameraVelocity.y;

            ofVec3f newPos = manualCamera.getPosition() + movement;
            manualCamera.setPosition(newPos);

            // Apply rotation
            ofQuaternion pitch(cameraPitch, ofVec3f(1, 0, 0));
            ofQuaternion yaw(cameraYaw, ofVec3f(0, 1, 0));
            ofQuaternion rotation = yaw * pitch;
            manualCamera.setOrientation(rotation);
        }
    }

    void draw() override {
        // Background
        ofBackground(20, 20, 30);

        // Enable depth test and lighting
        ofEnableDepthTest();
        ofEnableLighting();
        light1.enable();
        light2.enable();

        // Begin camera (switch between manual and EasyCam)
        if (useEasyCam) {
            easyCamera.begin();
        } else {
            manualCamera.begin();
        }

        // Draw reference grid
        drawGrid();

        // Draw coordinate axes at origin
        drawAxes(ofVec3f(0, 0, 0), 150.0f);

        // Draw scene objects
        drawScene();

        // End camera
        if (useEasyCam) {
            easyCamera.end();
        } else {
            manualCamera.end();
        }

        // Disable lighting and depth test for UI
        ofDisableLighting();
        ofDisableDepthTest();

        // Draw UI overlay (camera info)
        drawUI();
    }

    void drawGrid() {
        // Draw ground plane grid
        ofPushMatrix();
        ofRotateZ(90);
        ofSetColor(40, 40, 50);
        ofDrawGridPlane(50, 20);
        ofPopMatrix();
    }

    void drawAxes(const ofVec3f& position, float length) {
        float lineWidth = 3.0f;
        ofSetLineWidth(lineWidth);

        ofPushMatrix();
        ofTranslate(position.x, position.y, position.z);

        // X axis (red)
        ofSetColor(255, 0, 0);
        ofDrawLine(0, 0, 0, length, 0, 0);

        // Y axis (green)
        ofSetColor(0, 255, 0);
        ofDrawLine(0, 0, 0, 0, length, 0);

        // Z axis (blue)
        ofSetColor(0, 0, 255);
        ofDrawLine(0, 0, 0, 0, 0, length);

        ofPopMatrix();
    }

    void drawScene() {
        // Draw multiple objects to demonstrate camera viewing

        // Center sphere (animated)
        ofPushMatrix();
        float centerY = sin(animationTime * 2.0f) * 50.0f;
        ofTranslate(0, centerY, 0);
        material1.begin();
        ofDrawSphere(0, 0, 0, 80);
        material1.end();
        ofPopMatrix();

        // Orbiting boxes (4 boxes in a circle)
        float orbitRadius = 250.0f;
        float orbitSpeed = animationTime * 0.5f;

        for (int i = 0; i < 4; i++) {
            float angle = orbitSpeed + (i * M_PI * 0.5f);
            float x = cos(angle) * orbitRadius;
            float z = sin(angle) * orbitRadius;

            ofPushMatrix();
            ofTranslate(x, 0, z);
            ofRotateY(angle * 180.0f / M_PI);
            ofRotateX(animationTime * 30.0f);
            material2.begin();
            ofDrawBox(0, 0, 0, 60);
            material2.end();
            ofPopMatrix();
        }

        // Vertical cylinders at corners
        float cornerDist = 400.0f;
        ofVec3f corners[4] = {
            ofVec3f(-cornerDist, 0, -cornerDist),
            ofVec3f(cornerDist, 0, -cornerDist),
            ofVec3f(cornerDist, 0, cornerDist),
            ofVec3f(-cornerDist, 0, cornerDist)
        };

        for (int i = 0; i < 4; i++) {
            ofPushMatrix();
            ofTranslate(corners[i].x, 0, corners[i].z);
            ofRotateX(90);
            material3.begin();
            ofDrawCylinder(0, 0, 0, 30, 150);
            material3.end();
            ofPopMatrix();

            // Draw small axes at each corner
            drawAxes(corners[i], 50.0f);
        }

        // Small spheres along a path
        int pathCount = 12;
        for (int i = 0; i < pathCount; i++) {
            float t = (float)i / (float)pathCount;
            float pathAngle = t * M_PI * 2.0f + animationTime;
            float pathRadius = 150.0f;
            float x = cos(pathAngle) * pathRadius;
            float z = sin(pathAngle) * pathRadius;
            float y = sin(t * M_PI * 4.0f + animationTime * 2.0f) * 30.0f + 100.0f;

            ofPushMatrix();
            ofTranslate(x, y, z);

            // Rainbow colors based on position
            ofColor c = ofColor::fromHsb(t * 255.0f, 200, 255);
            ofSetColor(c.r, c.g, c.b);
            ofDrawSphere(0, 0, 0, 15);
            ofPopMatrix();
        }
    }

    void drawUI() {
        // Draw camera mode and position info
        // Note: This would be better with ofTrueTypeFont, but for now we'll use console output

        ofSetColor(255, 255, 255);

        // Simple visual indicator of camera mode
        std::string modeText = useEasyCam ? "[EasyCam Mode]" : "[Manual Camera Mode]";

        // Draw a simple colored box as mode indicator
        if (useEasyCam) {
            ofSetColor(100, 255, 100, 200);
        } else {
            ofSetColor(255, 100, 100, 200);
        }
        ofDrawRectangle(10, 10, 200, 30);

        // Camera position info (would be drawn as text with font)
        // For now, this information is logged to console
    }

    void keyPressed(int key) override {
        // Camera mode switching
        if (key == '\t') {  // TAB key
            useEasyCam = !useEasyCam;
            std::string mode = useEasyCam ? "EasyCam" : "Manual Camera";
            ofLogNotice() << "Switched to: " << mode;

            // Sync camera positions when switching
            if (useEasyCam) {
                // When switching to EasyCam, set target to where manual camera was looking
                easyCamera.setTarget(ofVec3f(0, 0, 0));
                float dist = manualCamera.getPosition().length();
                easyCamera.setDistance(dist);
            } else {
                // When switching to manual, copy EasyCam position
                manualCamera.setPosition(easyCamera.getPosition());
                ofVec3f target = easyCamera.getTarget();
                manualCamera.lookAt(target);
            }
        }

        // Animation toggle
        if (key == ' ') {
            animating = !animating;
            ofLogNotice() << "Animation: " << (animating ? "ON" : "OFF");
        }

        // Reset camera
        if (key == 'r' || key == 'R') {
            if (useEasyCam) {
                easyCamera.setDistance(500);
                easyCamera.setTarget(ofVec3f(0, 0, 0));
            } else {
                manualCamera.setPosition(0, 200, 500);
                manualCamera.lookAt(ofVec3f(0, 0, 0));
                cameraPitch = 0.0f;
                cameraYaw = 0.0f;
            }
            ofLogNotice() << "Camera reset";
        }

        // Manual camera movement (only when not using EasyCam)
        if (!useEasyCam) {
            // Forward/backward
            if (key == 'w' || key == 'W') {
                cameraVelocity.z = cameraSpeed;
            }
            if (key == 's' || key == 'S') {
                cameraVelocity.z = -cameraSpeed;
            }

            // Left/right
            if (key == 'a' || key == 'A') {
                cameraVelocity.x = -cameraSpeed;
            }
            if (key == 'd' || key == 'D') {
                cameraVelocity.x = cameraSpeed;
            }

            // Up/down
            if (key == 'q' || key == 'Q') {
                cameraVelocity.y = cameraSpeed;
            }
            if (key == 'e' || key == 'E') {
                cameraVelocity.y = -cameraSpeed;
            }

            // Rotation with arrow keys (macOS key codes)
            // Left: 123, Right: 124, Down: 125, Up: 126
            if (key == 123) {  // Left arrow
                cameraYaw -= rotationSpeed;
            }
            if (key == 124) {  // Right arrow
                cameraYaw += rotationSpeed;
            }
            if (key == 126) {  // Up arrow
                cameraPitch -= rotationSpeed;
            }
            if (key == 125) {  // Down arrow
                cameraPitch += rotationSpeed;
            }

            // Clamp pitch
            if (cameraPitch > 89.0f) cameraPitch = 89.0f;
            if (cameraPitch < -89.0f) cameraPitch = -89.0f;

            // Log camera position
            ofVec3f pos = manualCamera.getPosition();
            ofLogVerbose() << "Camera position: (" << pos.x << ", " << pos.y << ", " << pos.z << ")";
        }
    }

    void keyReleased(int key) override {
        // Stop manual camera movement
        if (!useEasyCam) {
            if (key == 'w' || key == 'W' || key == 's' || key == 'S') {
                cameraVelocity.z = 0.0f;
            }
            if (key == 'a' || key == 'A' || key == 'd' || key == 'D') {
                cameraVelocity.x = 0.0f;
            }
            if (key == 'q' || key == 'Q' || key == 'e' || key == 'E') {
                cameraVelocity.y = 0.0f;
            }
        }
    }

    void mouseDragged(int x, int y, int button) override {
        // EasyCam handles mouse events automatically
        if (useEasyCam) {
            easyCamera.onMouseDragged(x, y, button);
        }
    }

    void mousePressed(int x, int y, int button) override {
        if (useEasyCam) {
            easyCamera.onMousePressed(x, y, button);
        }
    }

    void mouseReleased(int x, int y, int button) override {
        if (useEasyCam) {
            easyCamera.onMouseReleased(x, y, button);
        }
    }

    void mouseScrolled(int x, int y, float scrollX, float scrollY) override {
        if (useEasyCam) {
            easyCamera.onMouseScrolled(scrollX, scrollY);
        }
    }

    void windowResized(int w, int h) override {
        // Update camera aspect ratios
        float aspect = static_cast<float>(w) / static_cast<float>(h);
        manualCamera.setAspectRatio(aspect);
        easyCamera.setAspectRatio(aspect);
    }
};

// Main function
int main() {
    // In a real application, this would be integrated with the SwiftUI app
    // For now, this demonstrates the API structure
    ofLogNotice() << "07_camera example ready";
    ofLogNotice() << "Note: Integrate CameraApp with SwiftBridge.mm to run";
    return 0;
}
