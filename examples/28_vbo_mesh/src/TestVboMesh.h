#pragma once

// Test app for VboMesh - Modern Metal GPU Mesh
// Tests storage modes, dynamic updates, and instanced rendering

#include <oflike/ofApp.h>
#include <oflike/graphics/ofCoreText.h>
#include <oflike/3d/VboMesh.h>
#include <oflike/3d/ofMesh.h>
#include <oflike/3d/ofEasyCam.h>
#include <oflike/lighting/ofLight.h>

class TestVboMesh : public ofBaseApp {
public:
    void setup() override;
    void update() override;
    void draw() override;

    void keyPressed(int key) override;
    void keyReleased(int key) override;
    void mouseMoved(int x, int y) override;
    void mouseDragged(int x, int y, int button) override;
    void mousePressed(int x, int y, int button) override;
    void mouseReleased(int x, int y, int button) override;
    void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
    void windowResized(int w, int h) override;

private:
    void drawStaticMeshTest();
    void drawDynamicMeshTest();
    void drawInstancedTest();
    void drawComparisonTest();

    ofCoreText font;
    oflike::ofEasyCam camera;
    oflike::ofLight light;

    // Static VboMesh (GPU-only storage)
    oflike::VboMesh staticSphere;
    oflike::VboMesh staticBox;
    oflike::VboMesh staticCone;

    // Dynamic VboMesh (CPU/GPU shared)
    oflike::VboMesh dynamicMesh;
    oflike::ofMesh sourceMesh;  // For comparison

    // Instance data
    std::vector<oflike::VboInstanceData> instances;

    // Test mode
    int testMode = 0;
    static constexpr int NUM_TEST_MODES = 4;
    const char* modeNames[NUM_TEST_MODES] = {
        "Static VboMesh",
        "Dynamic VboMesh",
        "Instanced Rendering",
        "VboMesh vs ofMesh"
    };

    float rotationAngle = 0.0f;
    bool wireframe = false;
    int currentPrimitive = 0;

    // Performance tracking
    float lastFrameTime = 0.0f;
    float avgFrameTime = 0.0f;
};
