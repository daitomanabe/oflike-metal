#pragma once

// Test app for Phase 4 features: ofSoundPlayer, of3dPrimitive

#include <oflike/ofApp.h>
#include <oflike/graphics/ofCoreText.h>
#include <oflike/3d/of3dPrimitive.h>
#include <oflike/3d/ofEasyCam.h>
#include <oflike/sound/ofSoundPlayer.h>
#include <oflike/lighting/ofLight.h>

class TestPhase4 : public ofBaseApp {
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
    void drawSoundTest();
    void draw3DPrimitiveTest();

    ofCoreText font;
    oflike::ofEasyCam camera;
    oflike::ofLight light;

    // Sound
    oflike::ofSoundPlayer soundPlayer;
    bool soundLoaded = false;

    // 3D Primitives
    oflike::ofBoxPrimitive box;
    oflike::ofSpherePrimitive sphere;
    oflike::ofCylinderPrimitive cylinder;
    oflike::ofConePrimitive cone;
    oflike::ofPlanePrimitive plane;
    oflike::ofIcoSpherePrimitive icoSphere;

    int currentPrimitive = 0;
    static const int NUM_PRIMITIVES = 6;
    const char* primitiveNames[NUM_PRIMITIVES] = {
        "Box", "Sphere", "Cylinder", "Cone", "Plane", "IcoSphere"
    };

    int testMode = 0;
    static const int NUM_TEST_MODES = 2;

    float rotationAngle = 0.0f;
    bool wireframe = false;
};
