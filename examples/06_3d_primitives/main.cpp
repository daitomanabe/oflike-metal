/// \file main.cpp
/// \brief Example 06: 3D Primitives
///
/// Demonstrates 3D primitive rendering with oflike-metal:
/// - 3D primitive drawing (box, sphere, cylinder, cone, plane, icosphere)
/// - 3D camera (ofCamera)
/// - Depth testing
/// - Lighting system (ofLight, ofMaterial)
/// - Rotation and animation
/// - Multiple primitives with different materials
///
/// Controls:
/// - SPACE: Toggle rotation animation
/// - L: Toggle lighting
/// - W: Toggle wireframe mode
/// - Mouse drag: Rotate camera (manual control)

#include "../../src/core/AppBase.h"
#include "../../src/oflike/graphics/ofGraphics.h"
#include "../../src/oflike/3d/ofCamera.h"
#include "../../src/oflike/lighting/ofLight.h"
#include "../../src/oflike/lighting/ofMaterial.h"
#include "../../src/oflike/types/ofColor.h"
#include "../../src/oflike/utils/ofUtils.h"
#include "../../src/oflike/utils/ofLog.h"
#include "../../src/core/Context.h"
#include <cmath>

using namespace oflike;

class Primitives3DApp : public ofBaseApp {
public:
    // Camera
    ofCamera camera;
    float cameraDistance = 500.0f;
    float cameraRotationY = 0.0f;
    float cameraRotationX = 20.0f;

    // Animation
    bool rotating = true;
    float rotationAngle = 0.0f;

    // Lighting
    bool lightingEnabled = true;
    ofLight light;

    // Materials
    ofMaterial redMaterial;
    ofMaterial greenMaterial;
    ofMaterial blueMaterial;
    ofMaterial yellowMaterial;
    ofMaterial cyanMaterial;
    ofMaterial magentaMaterial;

    // Wireframe mode
    bool wireframeMode = false;

    // Mouse interaction
    bool isDragging = false;
    float lastMouseX = 0.0f;
    float lastMouseY = 0.0f;

    void setup() override {
        ofSetFrameRate(60);

        // Setup camera
        camera.setPosition(0, 0, cameraDistance);
        camera.lookAt(ofVec3f(0, 0, 0));
        camera.setNearClip(1.0f);
        camera.setFarClip(10000.0f);
        camera.setFov(60.0f);

        // Setup light
        light.setPointLight();
        light.setPosition(200, 200, 200);
        light.setAmbientColor(ofColor(50, 50, 50));
        light.setDiffuseColor(ofColor(200, 200, 200));
        light.setSpecularColor(ofColor(255, 255, 255));

        // Setup materials
        // Red material (for box)
        redMaterial.setAmbientColor(ofColor(100, 20, 20));
        redMaterial.setDiffuseColor(ofColor(255, 50, 50));
        redMaterial.setSpecularColor(ofColor(255, 255, 255));
        redMaterial.setShininess(64.0f);

        // Green material (for sphere)
        greenMaterial.setAmbientColor(ofColor(20, 100, 20));
        greenMaterial.setDiffuseColor(ofColor(50, 255, 50));
        greenMaterial.setSpecularColor(ofColor(255, 255, 255));
        greenMaterial.setShininess(128.0f);

        // Blue material (for cylinder)
        blueMaterial.setAmbientColor(ofColor(20, 20, 100));
        blueMaterial.setDiffuseColor(ofColor(50, 50, 255));
        blueMaterial.setSpecularColor(ofColor(255, 255, 255));
        blueMaterial.setShininess(32.0f);

        // Yellow material (for cone)
        yellowMaterial.setAmbientColor(ofColor(100, 100, 20));
        yellowMaterial.setDiffuseColor(ofColor(255, 255, 50));
        yellowMaterial.setSpecularColor(ofColor(255, 255, 255));
        yellowMaterial.setShininess(16.0f);

        // Cyan material (for plane)
        cyanMaterial.setAmbientColor(ofColor(20, 100, 100));
        cyanMaterial.setDiffuseColor(ofColor(50, 255, 255));
        cyanMaterial.setSpecularColor(ofColor(255, 255, 255));
        cyanMaterial.setShininess(8.0f);

        // Magenta material (for icosphere)
        magentaMaterial.setAmbientColor(ofColor(100, 20, 100));
        magentaMaterial.setDiffuseColor(ofColor(255, 50, 255));
        magentaMaterial.setSpecularColor(ofColor(255, 255, 255));
        magentaMaterial.setShininess(96.0f);

        ofLogNotice() << "06_3d_primitives: Setup complete";
        ofLogNotice() << "Controls:";
        ofLogNotice() << "  SPACE: Toggle rotation";
        ofLogNotice() << "  L: Toggle lighting";
        ofLogNotice() << "  W: Toggle wireframe";
        ofLogNotice() << "  Mouse drag: Rotate camera";
    }

    void update() override {
        // Update rotation animation
        if (rotating) {
            rotationAngle += 0.5f;
            if (rotationAngle >= 360.0f) {
                rotationAngle -= 360.0f;
            }
        }

        // Update camera position based on rotation
        float radY = cameraRotationY * M_PI / 180.0f;
        float radX = cameraRotationX * M_PI / 180.0f;

        float x = cameraDistance * sin(radY) * cos(radX);
        float y = cameraDistance * sin(radX);
        float z = cameraDistance * cos(radY) * cos(radX);

        camera.setPosition(x, y, z);
        camera.lookAt(ofVec3f(0, 0, 0));
    }

    void draw() override {
        // Background
        ofBackground(30);

        // Enable depth test
        ofEnableDepthTest();

        // Setup lighting
        if (lightingEnabled) {
            ofEnableLighting();
            light.enable();
        } else {
            ofDisableLighting();
        }

        // Begin camera
        camera.begin();

        // Draw coordinate axes
        drawAxes();

        // Draw grid
        ofPushMatrix();
        ofRotateZ(90);
        ofSetColor(60, 60, 60);
        ofDrawGridPlane(10, 10);
        ofPopMatrix();

        // Draw primitives in a 3x2 grid layout

        // Row 1: Box, Sphere, Cylinder

        // Box (top-left)
        ofPushMatrix();
        ofTranslate(-150, -100, 0);
        ofRotateY(rotationAngle);
        ofRotateX(rotationAngle * 0.5f);
        if (lightingEnabled) {
            redMaterial.begin();
        } else {
            ofSetColor(255, 50, 50);
        }
        if (wireframeMode) {
            ofNoFill();
            ofSetLineWidth(2.0f);
        } else {
            ofFill();
        }
        ofDrawBox(0, 0, 0, 80);
        if (lightingEnabled) {
            redMaterial.end();
        }
        ofPopMatrix();

        // Sphere (top-center)
        ofPushMatrix();
        ofTranslate(0, -100, 0);
        ofRotateY(rotationAngle * 1.5f);
        if (lightingEnabled) {
            greenMaterial.begin();
        } else {
            ofSetColor(50, 255, 50);
        }
        if (wireframeMode) {
            ofNoFill();
            ofSetLineWidth(2.0f);
        } else {
            ofFill();
        }
        ofDrawSphere(0, 0, 0, 50);
        if (lightingEnabled) {
            greenMaterial.end();
        }
        ofPopMatrix();

        // Cylinder (top-right)
        ofPushMatrix();
        ofTranslate(150, -100, 0);
        ofRotateY(rotationAngle);
        ofRotateX(90);
        if (lightingEnabled) {
            blueMaterial.begin();
        } else {
            ofSetColor(50, 50, 255);
        }
        if (wireframeMode) {
            ofNoFill();
            ofSetLineWidth(2.0f);
        } else {
            ofFill();
        }
        ofDrawCylinder(0, 0, 0, 40, 100);
        if (lightingEnabled) {
            blueMaterial.end();
        }
        ofPopMatrix();

        // Row 2: Cone, Plane, IcoSphere

        // Cone (bottom-left)
        ofPushMatrix();
        ofTranslate(-150, 100, 0);
        ofRotateY(rotationAngle * 2.0f);
        ofRotateX(180);
        if (lightingEnabled) {
            yellowMaterial.begin();
        } else {
            ofSetColor(255, 255, 50);
        }
        if (wireframeMode) {
            ofNoFill();
            ofSetLineWidth(2.0f);
        } else {
            ofFill();
        }
        ofDrawCone(0, 0, 0, 50, 100);
        if (lightingEnabled) {
            yellowMaterial.end();
        }
        ofPopMatrix();

        // Plane (bottom-center)
        ofPushMatrix();
        ofTranslate(0, 100, 0);
        ofRotateY(rotationAngle);
        ofRotateX(rotationAngle * 0.3f);
        if (lightingEnabled) {
            cyanMaterial.begin();
        } else {
            ofSetColor(50, 255, 255);
        }
        if (wireframeMode) {
            ofNoFill();
            ofSetLineWidth(2.0f);
        } else {
            ofFill();
        }
        ofDrawPlane(0, 0, 0, 100, 100);
        if (lightingEnabled) {
            cyanMaterial.end();
        }
        ofPopMatrix();

        // IcoSphere (bottom-right)
        ofPushMatrix();
        ofTranslate(150, 100, 0);
        ofRotateY(rotationAngle * 0.7f);
        ofRotateX(rotationAngle * 0.4f);
        if (lightingEnabled) {
            magentaMaterial.begin();
        } else {
            ofSetColor(255, 50, 255);
        }
        if (wireframeMode) {
            ofNoFill();
            ofSetLineWidth(2.0f);
        } else {
            ofFill();
        }
        ofDrawIcoSphere(0, 0, 0, 50);
        if (lightingEnabled) {
            magentaMaterial.end();
        }
        ofPopMatrix();

        // End camera
        camera.end();

        // Disable depth test
        ofDisableDepthTest();

        // Note: UI overlay with text rendering would require ofTrueTypeFont
        // For console output of state, see keyPressed() method
    }

    void drawAxes() {
        float axisLength = 100.0f;
        float axisWidth = 3.0f;

        ofSetLineWidth(axisWidth);

        // X axis (red)
        ofSetColor(255, 0, 0);
        ofDrawLine(0, 0, 0, axisLength, 0, 0);

        // Y axis (green)
        ofSetColor(0, 255, 0);
        ofDrawLine(0, 0, 0, 0, axisLength, 0);

        // Z axis (blue)
        ofSetColor(0, 0, 255);
        ofDrawLine(0, 0, 0, 0, 0, axisLength);
    }

    void keyPressed(int key) override {
        if (key == ' ') {
            rotating = !rotating;
            ofLogNotice() << "Rotation: " << (rotating ? "ON" : "OFF");
        } else if (key == 'l' || key == 'L') {
            lightingEnabled = !lightingEnabled;
            ofLogNotice() << "Lighting: " << (lightingEnabled ? "ON" : "OFF");
        } else if (key == 'w' || key == 'W') {
            wireframeMode = !wireframeMode;
            ofLogNotice() << "Wireframe: " << (wireframeMode ? "ON" : "OFF");
        }
    }

    void mouseDragged(int x, int y, int button) override {
        if (isDragging) {
            float deltaX = x - lastMouseX;
            float deltaY = y - lastMouseY;

            cameraRotationY += deltaX * 0.5f;
            cameraRotationX += deltaY * 0.5f;

            // Clamp X rotation
            if (cameraRotationX > 89.0f) cameraRotationX = 89.0f;
            if (cameraRotationX < -89.0f) cameraRotationX = -89.0f;

            lastMouseX = x;
            lastMouseY = y;
        }
    }

    void mousePressed(int x, int y, int button) override {
        isDragging = true;
        lastMouseX = x;
        lastMouseY = y;
    }

    void mouseReleased(int x, int y, int button) override {
        isDragging = false;
    }

    void mouseScrolled(int x, int y, float scrollX, float scrollY) override {
        // Zoom with mouse wheel
        cameraDistance -= scrollY * 10.0f;
        if (cameraDistance < 100.0f) cameraDistance = 100.0f;
        if (cameraDistance > 1000.0f) cameraDistance = 1000.0f;
    }

    void windowResized(int w, int h) override {
        // Update camera aspect ratio
        camera.setAspectRatio(static_cast<float>(w) / static_cast<float>(h));
    }
};

// Main function
int main() {
    // In a real application, this would be integrated with the SwiftUI app
    // For now, this demonstrates the API structure
    ofLogNotice() << "06_3d_primitives example ready";
    ofLogNotice() << "Note: Integrate Primitives3DApp with SwiftBridge.mm to run";
    return 0;
}
