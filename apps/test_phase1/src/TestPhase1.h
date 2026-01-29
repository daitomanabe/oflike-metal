#pragma once

// Test app for Phase 1 features: ofNode, ofCamera, ofFbo, ofShader

#include <oflike/ofApp.h>
#include <oflike/3d/ofNode.h>
#include <oflike/3d/ofCamera.h>
#include <oflike/graphics/ofFbo.h>
#include <oflike/graphics/ofCoreText.h>

class TestPhase1 : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseScrolled(int x, int y, float scrollX, float scrollY);
    void windowResized(int w, int h);

private:
    // Phase 1 Feature: ofNode - Scene graph hierarchy
    oflike::ofNode parentNode;
    oflike::ofNode childNode1;
    oflike::ofNode childNode2;
    oflike::ofNode grandchildNode;

    // Phase 1 Feature: ofCamera
    oflike::ofCamera camera;

    // Phase 1 Feature: ofFbo
    oflike::ofFbo fbo;
    bool fboAllocated = false;

    // UI
    ofCoreText font;
    int testMode = 0;  // 0: ofNode, 1: ofCamera, 2: ofFbo
    static constexpr int NUM_TEST_MODES = 3;

    // Animation
    float time = 0.0f;

    // Camera control
    bool isDragging = false;
    float lastMouseX = 0.0f;
    float lastMouseY = 0.0f;
    float cameraDistance = 300.0f;
    float cameraLongitude = 45.0f;
    float cameraLatitude = 30.0f;

    // Helper methods
    void drawNodeTest();
    void drawCameraTest();
    void drawFboTest();
    void drawNodeHierarchy(oflike::ofNode& node, int depth);
    void updateCameraPosition();
};
