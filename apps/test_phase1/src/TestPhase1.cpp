#include "TestPhase1.h"
#include <cmath>
#include <string>

void TestPhase1::setup() {
    ofSetWindowTitle("test_phase1 - Phase 1 Feature Test (1-4: switch test)");
    ofSetCircleResolution(64);

    // Load font
    font.load("Helvetica", 18.0f);

    // =========================================
    // Setup ofNode hierarchy test
    // =========================================
    // Create a parent-child hierarchy:
    //   parentNode
    //   ├── childNode1
    //   │   └── grandchildNode
    //   └── childNode2

    parentNode.setPosition(0, 0, 0);

    childNode1.setParent(parentNode);
    childNode1.setPosition(50, 0, 0);  // Local offset from parent

    childNode2.setParent(parentNode);
    childNode2.setPosition(-50, 0, 0);  // Local offset from parent

    grandchildNode.setParent(childNode1);
    grandchildNode.setPosition(30, 30, 0);  // Local offset from childNode1

    // =========================================
    // Setup ofCamera test
    // =========================================
    camera.setPosition(0, 0, cameraDistance);
    camera.lookAt(oflike::ofVec3f(0, 0, 0));
    camera.setFov(60.0f);
    camera.setNearClip(0.1f);
    camera.setFarClip(10000.0f);

    // =========================================
    // Setup ofFbo test
    // =========================================
    fbo.allocate(512, 512, oflike::OF_IMAGE_COLOR_ALPHA, 0);
    fboAllocated = fbo.isAllocated();

    // =========================================
    // Setup ofShader test
    // =========================================
    // Load different shader variants from the same source file
    bool loaded1 = shaderGradient.load("TestShader", "vertex_test", "fragment_gradient");
    bool loaded2 = shaderWave.load("TestShader", "vertex_test", "fragment_wave");
    bool loaded3 = shaderCircles.load("TestShader", "vertex_test", "fragment_circles");
    bool loaded4 = shaderNoise.load("TestShader", "vertex_test", "fragment_noise");
    shadersLoaded = loaded1 && loaded2 && loaded3 && loaded4;
}

void TestPhase1::update() {
    time = ofGetElapsedTimef();

    // Animate the node hierarchy
    parentNode.setOrientation(oflike::ofQuaternion::makeRotate(time * 20, oflike::ofVec3f(0, 1, 0)));
    childNode1.rotateY(0.5f);
    childNode2.rotateX(0.3f);
}

void TestPhase1::draw() {
    ofBackground(30, 35, 45);

    switch (testMode) {
        case 0:
            drawNodeTest();
            break;
        case 1:
            drawCameraTest();
            break;
        case 2:
            drawFboTest();
            break;
        case 3:
            drawShaderTest();
            break;
    }

    // Draw test mode indicator
    ofSetColor(255);
    float y = ofGetHeight() - 30;
    if (font.isLoaded()) {
        std::string modeNames[] = {"ofNode", "ofCamera", "ofFbo", "ofShader"};
        std::string text = "Test " + std::to_string(testMode + 1) + "/" + std::to_string(NUM_TEST_MODES) + ": " + modeNames[testMode] + " (Press 1-4 to switch)";
        font.drawString(text, 20, y);
    }
}

void TestPhase1::drawNodeTest() {
    // Draw title
    ofSetColor(255, 255, 100);
    if (font.isLoaded()) {
        font.drawString("ofNode Test - Parent/Child Hierarchy & Transforms", 20, 40);
    }

    // Draw the node hierarchy visualization
    ofPushMatrix();
    ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);

    // Draw connections and nodes
    ofSetColor(100, 100, 100);

    // Get global positions
    oflike::ofVec3f parentPos = parentNode.getGlobalPosition();
    oflike::ofVec3f child1Pos = childNode1.getGlobalPosition();
    oflike::ofVec3f child2Pos = childNode2.getGlobalPosition();
    oflike::ofVec3f grandchildPos = grandchildNode.getGlobalPosition();

    // Draw connections
    ofSetLineWidth(2.0f);
    ofSetColor(100, 150, 200);
    ofDrawLine(parentPos.x, parentPos.y, child1Pos.x, child1Pos.y);
    ofDrawLine(parentPos.x, parentPos.y, child2Pos.x, child2Pos.y);
    ofDrawLine(child1Pos.x, child1Pos.y, grandchildPos.x, grandchildPos.y);

    // Draw parent node (large, red)
    ofFill();
    ofSetColor(255, 100, 100);
    ofDrawCircle(parentPos.x, parentPos.y, 25);
    ofSetColor(255);
    if (font.isLoaded()) {
        font.drawString("Parent", parentPos.x - 25, parentPos.y - 35);
    }

    // Draw child node 1 (medium, green)
    ofSetColor(100, 255, 100);
    ofDrawCircle(child1Pos.x, child1Pos.y, 20);
    ofSetColor(255);
    if (font.isLoaded()) {
        font.drawString("Child1", child1Pos.x - 20, child1Pos.y - 30);
    }

    // Draw child node 2 (medium, blue)
    ofSetColor(100, 100, 255);
    ofDrawCircle(child2Pos.x, child2Pos.y, 20);
    ofSetColor(255);
    if (font.isLoaded()) {
        font.drawString("Child2", child2Pos.x - 20, child2Pos.y - 30);
    }

    // Draw grandchild node (small, yellow)
    ofSetColor(255, 255, 100);
    ofDrawCircle(grandchildPos.x, grandchildPos.y, 15);
    ofSetColor(255);
    if (font.isLoaded()) {
        font.drawString("Grandchild", grandchildPos.x - 35, grandchildPos.y - 25);
    }

    ofPopMatrix();

    // Draw info
    ofSetColor(200, 200, 200);
    float infoY = 80;
    if (font.isLoaded()) {
        font.drawString("Parent local: " + std::to_string((int)parentNode.getPosition().x) + ", " + std::to_string((int)parentNode.getPosition().y), 20, infoY);
        infoY += 25;
        font.drawString("Child1 local: " + std::to_string((int)childNode1.getPosition().x) + ", global: " + std::to_string((int)child1Pos.x) + ", " + std::to_string((int)child1Pos.y), 20, infoY);
        infoY += 25;
        font.drawString("Grandchild local: " + std::to_string((int)grandchildNode.getPosition().x) + ", global: " + std::to_string((int)grandchildPos.x) + ", " + std::to_string((int)grandchildPos.y), 20, infoY);
        infoY += 25;
        font.drawString("Children count: " + std::to_string(parentNode.getNumChildren()), 20, infoY);
    }
}

void TestPhase1::drawCameraTest() {
    // Draw title
    ofSetColor(255, 255, 100);
    if (font.isLoaded()) {
        font.drawString("ofCamera Test - Perspective Projection & View Matrix", 20, 40);
    }

    // Update camera position from spherical coordinates
    updateCameraPosition();

    // Begin camera
    camera.begin();
    ofEnableDepthTest();

    // Draw origin axes
    ofSetLineWidth(2.0f);
    ofSetColor(255, 0, 0);
    ofDrawLine(0, 0, 0, 100, 0, 0);  // X axis (red)
    ofSetColor(0, 255, 0);
    ofDrawLine(0, 0, 0, 0, 100, 0);  // Y axis (green)
    ofSetColor(0, 0, 255);
    ofDrawLine(0, 0, 0, 0, 0, 100);  // Z axis (blue)

    // Draw a grid on XZ plane
    ofSetColor(80, 80, 80);
    ofSetLineWidth(1.0f);
    for (int i = -5; i <= 5; i++) {
        ofDrawLine(i * 20, 0, -100, i * 20, 0, 100);
        ofDrawLine(-100, 0, i * 20, 100, 0, i * 20);
    }

    // Draw rotating cube
    ofFill();
    ofSetColor(100, 150, 255);
    ofPushMatrix();
    ofRotateY(time * 30);
    ofDrawBox(0, 25, 0, 40);
    ofPopMatrix();

    // Draw sphere
    ofSetColor(255, 150, 100);
    ofDrawSphere(-60, 30, 0, 20);

    // Draw wireframe cone
    ofNoFill();
    ofSetColor(100, 255, 150);
    ofDrawCone(60, 25, 0, 15, 40);

    ofDisableDepthTest();
    camera.end();

    // Draw camera info
    ofSetColor(200, 200, 200);
    float infoY = 80;
    if (font.isLoaded()) {
        oflike::ofVec3f camPos = camera.getPosition();
        font.drawString("Camera pos: " + std::to_string((int)camPos.x) + ", " + std::to_string((int)camPos.y) + ", " + std::to_string((int)camPos.z), 20, infoY);
        infoY += 25;
        font.drawString("FOV: " + std::to_string((int)camera.getFov()) + " deg", 20, infoY);
        infoY += 25;
        font.drawString("Near/Far: " + std::to_string(camera.getNearClip()) + " / " + std::to_string((int)camera.getFarClip()), 20, infoY);
        infoY += 25;
        font.drawString("Drag to orbit camera", 20, infoY);
    }
}

void TestPhase1::drawFboTest() {
    // Draw title
    ofSetColor(255, 255, 100);
    if (font.isLoaded()) {
        font.drawString("ofFbo Test - Off-screen Rendering", 20, 40);
    }

    if (!fboAllocated) {
        ofSetColor(255, 100, 100);
        if (font.isLoaded()) {
            font.drawString("FBO allocation failed!", 20, 80);
        }
        return;
    }

    // Render to FBO
    fbo.begin();

    // Clear with dark blue
    ofClear(20, 30, 60, 255);

    // Draw animated content in FBO
    ofSetColor(255, 200, 100);
    float cx = 256 + std::cos(time * 2) * 100;
    float cy = 256 + std::sin(time * 2) * 100;
    ofDrawCircle(cx, cy, 40);

    // Draw rectangle
    ofSetColor(100, 200, 255);
    ofPushMatrix();
    ofTranslate(256, 256);
    ofRotate(time * 45);
    ofDrawRectangle(-30, -30, 60, 60);
    ofPopMatrix();

    // Draw some text
    ofSetColor(255);
    if (font.isLoaded()) {
        font.drawString("FBO Content", 180, 50);
    }

    fbo.end();

    // Draw FBO to screen
    ofSetColor(255);
    fbo.draw(50, 100);

    // Draw FBO at different size
    fbo.draw(600, 100, 256, 256);

    // Draw FBO info
    ofSetColor(200, 200, 200);
    float infoY = ofGetHeight() - 100;
    if (font.isLoaded()) {
        font.drawString("FBO Size: " + std::to_string(fbo.getWidth()) + " x " + std::to_string(fbo.getHeight()), 20, infoY);
        infoY += 25;
        font.drawString("Allocated: " + std::string(fboAllocated ? "Yes" : "No"), 20, infoY);
        infoY += 25;
        font.drawString("Depth buffer: " + std::string(fbo.hasDepthBuffer() ? "Yes" : "No"), 20, infoY);
    }

    // Labels
    ofSetColor(150, 150, 150);
    if (font.isLoaded()) {
        font.drawString("Original size (512x512)", 50, 90);
        font.drawString("Scaled (256x256)", 600, 90);
    }
}

void TestPhase1::updateCameraPosition() {
    // Convert spherical to Cartesian coordinates
    float latRad = cameraLatitude * M_PI / 180.0f;
    float lonRad = cameraLongitude * M_PI / 180.0f;

    float x = cameraDistance * std::cos(latRad) * std::sin(lonRad);
    float y = cameraDistance * std::sin(latRad);
    float z = cameraDistance * std::cos(latRad) * std::cos(lonRad);

    camera.setPosition(x, y, z);
    camera.lookAt(oflike::ofVec3f(0, 0, 0));
}

void TestPhase1::drawShaderTest() {
    // Draw title
    ofSetColor(255, 255, 100);
    if (font.isLoaded()) {
        font.drawString("ofShader Test - Custom Metal Shaders", 20, 40);
    }

    if (!shadersLoaded) {
        ofSetColor(255, 100, 100);
        if (font.isLoaded()) {
            font.drawString("Shader loading failed! Check console for errors.", 20, 80);
        }
        return;
    }

    // Get current shader
    oflike::ofShader* shader = nullptr;
    std::string shaderName;
    switch (currentShader) {
        case 0:
            shader = &shaderGradient;
            shaderName = "Gradient";
            break;
        case 1:
            shader = &shaderWave;
            shaderName = "Wave";
            break;
        case 2:
            shader = &shaderCircles;
            shaderName = "Circles";
            break;
        case 3:
            shader = &shaderNoise;
            shaderName = "Noise";
            break;
    }

    // Draw shader preview rectangle
    float rectX = 100;
    float rectY = 100;
    float rectW = 400;
    float rectH = 400;

    if (shader && shader->isLoaded()) {
        shader->begin();

        // Set time uniform (shader reads it as first float in buffer)
        shader->setUniform1f("time", time);

        // Draw rectangle with shader
        ofSetColor(255);
        ofDrawRectangle(rectX, rectY, rectW, rectH);

        shader->end();
    }

    // Draw non-shaded elements for comparison
    ofSetColor(255);
    ofDrawRectangle(550, 100, 200, 200);
    ofSetColor(200, 200, 200);
    if (font.isLoaded()) {
        font.drawString("No shader", 600, 320);
    }

    // Draw shader info
    ofSetColor(200, 200, 200);
    float infoY = ofGetHeight() - 150;
    if (font.isLoaded()) {
        font.drawString("Current Shader: " + shaderName + " (" + std::to_string(currentShader + 1) + "/4)", 20, infoY);
        infoY += 25;
        font.drawString("Press Q/W to change shader", 20, infoY);
        infoY += 25;
        font.drawString("Loaded: " + std::string(shadersLoaded ? "Yes" : "No"), 20, infoY);
        infoY += 25;
        if (shader) {
            font.drawString("Vertex: " + shader->getVertexFunctionName(), 20, infoY);
            infoY += 25;
            font.drawString("Fragment: " + shader->getFragmentFunctionName(), 20, infoY);
        }
    }
}

void TestPhase1::keyPressed(int key) {
    // Number keys 1-4 to switch test modes
    if (key >= '1' && key <= '4') {
        testMode = key - '1';
    }
    // Also handle keycode 18-21 for 1-4 on some keyboards
    if (key >= 18 && key <= 21) {
        testMode = key - 18;
    }

    // Shader switching with Q/W in shader test mode
    if (testMode == 3) {
        if (key == 'q' || key == 'Q') {
            currentShader = (currentShader + 3) % 4;  // Previous
        }
        if (key == 'w' || key == 'W') {
            currentShader = (currentShader + 1) % 4;  // Next
        }
    }
}

void TestPhase1::keyReleased(int key) {
    (void)key;
}

void TestPhase1::mouseMoved(int x, int y) {
    (void)x; (void)y;
}

void TestPhase1::mouseDragged(int x, int y, int button) {
    (void)button;
    if (testMode == 1 && isDragging) {
        float dx = x - lastMouseX;
        float dy = y - lastMouseY;

        cameraLongitude += dx * 0.5f;
        cameraLatitude += dy * 0.5f;

        // Clamp latitude
        cameraLatitude = std::max(-89.0f, std::min(89.0f, cameraLatitude));

        lastMouseX = x;
        lastMouseY = y;
    }
}

void TestPhase1::mousePressed(int x, int y, int button) {
    (void)button;
    isDragging = true;
    lastMouseX = x;
    lastMouseY = y;
}

void TestPhase1::mouseReleased(int x, int y, int button) {
    (void)x; (void)y; (void)button;
    isDragging = false;
}

void TestPhase1::mouseScrolled(int x, int y, float scrollX, float scrollY) {
    (void)x; (void)y; (void)scrollX;
    if (testMode == 1) {
        cameraDistance -= scrollY * 10.0f;
        cameraDistance = std::max(50.0f, std::min(1000.0f, cameraDistance));
    }
}

void TestPhase1::windowResized(int w, int h) {
    if (w > 0 && h > 0) {
        camera.setAspectRatio((float)w / (float)h);
    }
}

extern "C" ofBaseApp* ofCreateApp() {
    return new TestPhase1();
}
