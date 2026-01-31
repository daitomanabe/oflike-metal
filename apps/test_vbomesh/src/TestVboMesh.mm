#include "TestVboMesh.h"
#include <oflike/graphics/ofGraphics.h>
#include <oflike/utils/ofUtils.h>
#include <cmath>

void TestVboMesh::setup() {
    // Setup font
    font.load("Helvetica", 14);

    // Setup camera
    camera.setDistance(500);
    camera.setNearClip(1);
    camera.setFarClip(5000);

    // Setup light
    light.setPosition(200, 300, 400);
    light.setDiffuseColor(255, 255, 255);
    light.setAmbientColor(50, 50, 50);
    light.enable();

    // Create static VboMesh primitives
    staticSphere = oflike::VboMesh::createSphere(100, 32);
    staticBox = oflike::VboMesh::createBox(150, 150, 150);
    staticCone = oflike::VboMesh::createCone(80, 200, 32);

    // Create dynamic mesh (updated every frame)
    sourceMesh = oflike::ofMesh::sphere(80, 16);
    dynamicMesh.allocate(sourceMesh.getNumVertices(),
                         sourceMesh.getNumIndices(),
                         oflike::VboUsageHint::Stream);

    // Create instance data for instanced rendering
    instances.resize(100);
    for (int i = 0; i < 100; i++) {
        float angle = (float)i / 100.0f * 2.0f * M_PI;
        float radius = 200.0f + (i % 3) * 50.0f;
        float x = cosf(angle) * radius;
        float z = sinf(angle) * radius;
        float y = ((i % 5) - 2) * 30.0f;

        simd_float4x4 transform = matrix_identity_float4x4;
        transform.columns[3] = simd_make_float4(x, y, z, 1.0f);

        // Random color tint
        float r = 0.5f + 0.5f * sinf(i * 0.3f);
        float g = 0.5f + 0.5f * sinf(i * 0.3f + 2.0f);
        float b = 0.5f + 0.5f * sinf(i * 0.3f + 4.0f);

        instances[i] = oflike::VboInstanceData(transform, simd_make_float4(r, g, b, 1.0f));
    }
}

void TestVboMesh::update() {
    rotationAngle += 1.0f;
    if (rotationAngle > 360.0f) rotationAngle -= 360.0f;

    // Update dynamic mesh vertices (wave effect)
    if (testMode == 1) {
        float time = ofGetElapsedTimef();
        auto vertices = sourceMesh.getVertices();

        for (size_t i = 0; i < vertices.size(); i++) {
            oflike::ofVec3f& v = vertices[i];
            float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
            if (len > 0) {
                // Pulse effect
                float pulse = 1.0f + 0.2f * sinf(time * 3.0f + len * 0.1f);
                float scale = 80.0f * pulse / len;
                v.x *= scale / 80.0f;
                v.y *= scale / 80.0f;
                v.z *= scale / 80.0f;
            }
        }

        dynamicMesh.updateVertices(vertices.data(), vertices.size());
        dynamicMesh.updateIndices(sourceMesh.getIndices().data(), sourceMesh.getNumIndices());
        dynamicMesh.setVertexCount(vertices.size());
        dynamicMesh.setIndexCount(sourceMesh.getNumIndices());
    }

    // Update frame time
    static float lastTime = 0;
    float currentTime = ofGetElapsedTimef();
    lastFrameTime = (currentTime - lastTime) * 1000.0f;
    lastTime = currentTime;
    avgFrameTime = avgFrameTime * 0.95f + lastFrameTime * 0.05f;
}

void TestVboMesh::draw() {
    ofBackground(40, 44, 52);

    switch (testMode) {
        case 0: drawStaticMeshTest(); break;
        case 1: drawDynamicMeshTest(); break;
        case 2: drawInstancedTest(); break;
        case 3: drawComparisonTest(); break;
    }

    // UI
    ofSetColor(255, 255, 255);
    font.drawString("VboMesh Test - Build " __DATE__ " " __TIME__, 20, 30);

    // Mode indicator
    ofSetColor(255, 255, 0);
    font.drawString("MODE " + std::to_string(testMode + 1) + ": " + std::string(modeNames[testMode]), 20, 55);

    // Status
    ofSetColor(200, 200, 200);
    font.drawString("Wireframe: " + std::string(wireframe ? "ON" : "OFF"), 20, 80);

    // Instructions
    ofSetColor(150, 150, 150);
    font.drawString("Keys: 1-4 switch mode, W toggle wireframe", 20, 105);
}

void TestVboMesh::drawStaticMeshTest() {
    camera.begin();
    ofEnableDepthTest();
    ofEnableLighting();

    if (wireframe) ofNoFill();
    else ofFill();

    // Draw rotating primitives
    ofPushMatrix();
    ofRotateY(rotationAngle);

    // Sphere on left
    ofPushMatrix();
    ofTranslate(-200, 0, 0);
    ofSetColor(255, 100, 100);
    staticSphere.draw();
    ofPopMatrix();

    // Box in center
    ofPushMatrix();
    ofSetColor(100, 255, 100);
    staticBox.draw();
    ofPopMatrix();

    // Cone on right
    ofPushMatrix();
    ofTranslate(200, 0, 0);
    ofSetColor(100, 100, 255);
    staticCone.draw();
    ofPopMatrix();

    ofPopMatrix();

    ofDisableLighting();
    ofDisableDepthTest();
    camera.end();
}

void TestVboMesh::drawDynamicMeshTest() {
    camera.begin();
    ofEnableDepthTest();
    ofEnableLighting();

    if (wireframe) ofNoFill();
    else ofFill();

    ofPushMatrix();
    ofRotateY(rotationAngle);
    ofSetColor(255, 200, 100);
    dynamicMesh.draw();
    ofPopMatrix();

    ofDisableLighting();
    ofDisableDepthTest();
    camera.end();
}

void TestVboMesh::drawInstancedTest() {
    camera.begin();
    ofEnableDepthTest();
    ofEnableLighting();

    if (wireframe) ofNoFill();
    else ofFill();

    // Simplified: just draw 3 spheres at different positions
    ofPushMatrix();
    ofRotateY(rotationAngle * 0.5f);

    ofSetColor(255, 100, 100);
    ofPushMatrix();
    ofTranslate(-150, 0, 0);
    staticSphere.draw();
    ofPopMatrix();

    ofSetColor(100, 255, 100);
    ofPushMatrix();
    ofTranslate(0, 0, 0);
    staticSphere.draw();
    ofPopMatrix();

    ofSetColor(100, 100, 255);
    ofPushMatrix();
    ofTranslate(150, 0, 0);
    staticSphere.draw();
    ofPopMatrix();

    ofPopMatrix();

    ofDisableLighting();
    ofDisableDepthTest();
    camera.end();
}

void TestVboMesh::drawComparisonTest() {
    camera.begin();
    ofEnableDepthTest();
    ofEnableLighting();

    if (wireframe) ofNoFill();
    else ofFill();

    ofPushMatrix();
    ofRotateY(rotationAngle);

    // VboMesh on left
    ofPushMatrix();
    ofTranslate(-150, 0, 0);
    ofSetColor(100, 255, 100);
    staticSphere.draw();
    ofPopMatrix();

    // ofMesh on right
    ofPushMatrix();
    ofTranslate(150, 0, 0);
    ofSetColor(255, 100, 100);
    sourceMesh.draw();
    ofPopMatrix();

    ofPopMatrix();

    ofDisableLighting();
    ofDisableDepthTest();
    camera.end();

    // Labels
    ofSetColor(255);
    font.drawString("VboMesh (left) vs ofMesh (right)", 20, 150);
}

// Track last key for debugging
static int lastKeyPressed = 0;
static int keyPressCount = 0;

// macOS virtual key codes
enum {
    kVK_ANSI_1 = 18,
    kVK_ANSI_2 = 19,
    kVK_ANSI_3 = 20,
    kVK_ANSI_4 = 21,
    kVK_ANSI_W = 13
};

void TestVboMesh::keyPressed(int key) {
    lastKeyPressed = key;
    keyPressCount++;

    // Use macOS key codes
    if (key == kVK_ANSI_1) testMode = 0;
    else if (key == kVK_ANSI_2) testMode = 1;
    else if (key == kVK_ANSI_3) testMode = 2;
    else if (key == kVK_ANSI_4) testMode = 3;
    else if (key == kVK_ANSI_W) wireframe = !wireframe;
}

int getLastKeyPressed() { return lastKeyPressed; }
int getKeyPressCount() { return keyPressCount; }

void TestVboMesh::keyReleased(int key) {
    (void)key;
}

void TestVboMesh::mouseMoved(int x, int y) {
    (void)x; (void)y;
}

void TestVboMesh::mouseDragged(int x, int y, int button) {
    camera.onMouseDragged(x, y, button);
}

void TestVboMesh::mousePressed(int x, int y, int button) {
    camera.onMousePressed(x, y, button);
}

void TestVboMesh::mouseReleased(int x, int y, int button) {
    camera.onMouseReleased(x, y, button);
}

void TestVboMesh::mouseScrolled(int x, int y, float scrollX, float scrollY) {
    (void)x; (void)y;
    camera.onMouseScrolled(scrollX, scrollY);
}

void TestVboMesh::windowResized(int w, int h) {
    (void)w; (void)h;
}

extern "C" ofBaseApp* ofCreateApp() {
    return new TestVboMesh();
}
