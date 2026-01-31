#pragma once

// Test app for Phase 2 features: ofVbo, ofTexture extensions, ofLight/ofMaterial

#include <oflike/ofApp.h>
#include <oflike/3d/ofCamera.h>
#include <oflike/graphics/ofVbo.h>
#include <oflike/image/ofTexture.h>
#include <oflike/lighting/ofLight.h>
#include <oflike/lighting/ofMaterial.h>
#include <oflike/graphics/ofCoreText.h>

class TestPhase2 : public ofBaseApp {
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
    // Phase 2 Feature: ofVbo
    oflike::ofVbo vbo;
    std::vector<oflike::ofVec3f> vboVertices;
    std::vector<oflike::ofFloatColor> vboColors;
    std::vector<unsigned int> vboIndices;

    // Phase 2 Feature: ofTexture with mipmap
    oflike::ofTexture texture;
    bool mipmapEnabled = false;

    // Phase 2 Feature: ofLight/ofMaterial
    oflike::ofLight light;
    oflike::ofMaterial material;

    // Camera
    oflike::ofCamera camera;

    // UI
    ofCoreText font;
    int testMode = 0;  // 0: ofVbo, 1: ofTexture, 2: ofLight/ofMaterial
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
    void drawVboTest();
    void drawTextureTest();
    void drawLightingTest();
    void setupVbo();
    void setupTexture();
    void updateCameraPosition();
};
