#include "TestPhase2.h"
#include <cmath>
#include <string>

void TestPhase2::setup() {
    ofSetWindowTitle("test_phase2 - Phase 2 Feature Test (1-3: switch, M: mipmap)");
    ofSetCircleResolution(64);

    // Load font
    font.load("Helvetica", 18.0f);

    // Setup VBO
    setupVbo();

    // Setup texture
    setupTexture();

    // Setup camera
    camera.setPosition(0, 0, cameraDistance);
    camera.lookAt(oflike::ofVec3f(0, 0, 0));
    camera.setFov(60.0f);
    camera.setNearClip(0.1f);
    camera.setFarClip(10000.0f);

    // Setup light
    light.setPointLight();
    light.setPosition(100, 100, 100);
    light.setDiffuseColor(oflike::ofFloatColor(1.0f, 1.0f, 1.0f, 1.0f));
    light.setSpecularColor(oflike::ofFloatColor(1.0f, 1.0f, 1.0f, 1.0f));
    light.setAmbientColor(oflike::ofFloatColor(0.2f, 0.2f, 0.2f, 1.0f));
    light.enable();

    // Setup material
    material.setDiffuseColor(oflike::ofFloatColor(0.8f, 0.3f, 0.3f, 1.0f));
    material.setSpecularColor(oflike::ofFloatColor(1.0f, 1.0f, 1.0f, 1.0f));
    material.setAmbientColor(oflike::ofFloatColor(0.2f, 0.1f, 0.1f, 1.0f));
    material.setShininess(64.0f);
}

void TestPhase2::setupVbo() {
    // Create a grid of points
    const int gridSize = 20;
    const float spacing = 10.0f;
    const float halfSize = gridSize * spacing * 0.5f;

    vboVertices.clear();
    vboColors.clear();
    vboIndices.clear();

    // Generate grid vertices
    for (int z = 0; z <= gridSize; ++z) {
        for (int x = 0; x <= gridSize; ++x) {
            float px = x * spacing - halfSize;
            float pz = z * spacing - halfSize;
            float py = std::sin(px * 0.1f) * std::cos(pz * 0.1f) * 20.0f;

            vboVertices.push_back(oflike::ofVec3f(px, py, pz));

            // Color based on height
            float t = (py + 20.0f) / 40.0f;
            vboColors.push_back(oflike::ofFloatColor(
                0.2f + t * 0.8f,
                0.5f - t * 0.3f,
                1.0f - t * 0.5f,
                1.0f
            ));
        }
    }

    // Generate indices for triangle strip
    for (int z = 0; z < gridSize; ++z) {
        for (int x = 0; x < gridSize; ++x) {
            unsigned int i0 = z * (gridSize + 1) + x;
            unsigned int i1 = i0 + 1;
            unsigned int i2 = i0 + (gridSize + 1);
            unsigned int i3 = i2 + 1;

            // First triangle
            vboIndices.push_back(i0);
            vboIndices.push_back(i2);
            vboIndices.push_back(i1);

            // Second triangle
            vboIndices.push_back(i1);
            vboIndices.push_back(i2);
            vboIndices.push_back(i3);
        }
    }

    // Upload to VBO
    vbo.setVertexData(vboVertices);
    vbo.setColorData(vboColors);
    vbo.setIndexData(vboIndices);
}

void TestPhase2::setupTexture() {
    // Create a checkerboard texture
    const int texSize = 256;
    const int checkSize = 32;

    oflike::ofPixels pixels;
    pixels.allocate(texSize, texSize, 4);  // RGBA

    unsigned char* data = pixels.getData();
    for (int y = 0; y < texSize; ++y) {
        for (int x = 0; x < texSize; ++x) {
            int checkX = x / checkSize;
            int checkY = y / checkSize;
            bool white = (checkX + checkY) % 2 == 0;

            int idx = (y * texSize + x) * 4;
            data[idx + 0] = white ? 255 : 50;   // R
            data[idx + 1] = white ? 255 : 100;  // G
            data[idx + 2] = white ? 255 : 150;  // B
            data[idx + 3] = 255;                 // A
        }
    }

    // Allocate and load texture
    texture.allocate(texSize, texSize, oflike::OF_IMAGE_COLOR_ALPHA);
    texture.loadData(pixels);
}

void TestPhase2::update() {
    time = ofGetElapsedTimef();

    // Animate VBO vertices (wave animation)
    for (size_t i = 0; i < vboVertices.size(); ++i) {
        float px = vboVertices[i].x;
        float pz = vboVertices[i].z;
        vboVertices[i].y = std::sin(px * 0.1f + time) * std::cos(pz * 0.1f + time * 0.7f) * 20.0f;

        // Update color based on height
        float t = (vboVertices[i].y + 20.0f) / 40.0f;
        vboColors[i] = oflike::ofFloatColor(
            0.2f + t * 0.8f,
            0.5f - t * 0.3f,
            1.0f - t * 0.5f,
            1.0f
        );
    }

    // Update VBO data
    vbo.updateVertexData(0, vboVertices.data(), static_cast<int>(vboVertices.size()));
    vbo.updateColorData(0, vboColors.data(), static_cast<int>(vboColors.size()));

    // Animate light position
    light.setPosition(
        std::cos(time * 0.5f) * 150.0f,
        100.0f,
        std::sin(time * 0.5f) * 150.0f
    );
}

void TestPhase2::draw() {
    ofBackground(30, 35, 45);

    switch (testMode) {
        case 0:
            drawVboTest();
            break;
        case 1:
            drawTextureTest();
            break;
        case 2:
            drawLightingTest();
            break;
    }

    // Draw test mode indicator
    ofSetColor(255);
    float y = ofGetHeight() - 30;
    if (font.isLoaded()) {
        std::string modeNames[] = {"ofVbo", "ofTexture+Mipmap", "ofLight+ofMaterial"};
        std::string text = "Test " + std::to_string(testMode + 1) + "/" + std::to_string(NUM_TEST_MODES) + ": " + modeNames[testMode] + " (Press 1-3)";
        font.drawString(text, 20, y);
    }
}

void TestPhase2::drawVboTest() {
    // Draw title
    ofSetColor(255, 255, 100);
    if (font.isLoaded()) {
        font.drawString("ofVbo Test - GPU Vertex Buffer with Dynamic Updates", 20, 40);
    }

    // Update camera position
    updateCameraPosition();

    // Begin camera
    camera.begin();
    ofEnableDepthTest();

    // Draw VBO
    vbo.drawElements(OF_MESH_FILL, 0, vbo.getNumIndices());

    // Draw wireframe overlay
    ofSetColor(255, 255, 255, 50);
    vbo.drawElements(OF_MESH_WIREFRAME, 0, vbo.getNumIndices());

    ofDisableDepthTest();
    camera.end();

    // Draw info
    ofSetColor(200, 200, 200);
    float infoY = 80;
    if (font.isLoaded()) {
        font.drawString("Vertices: " + std::to_string(vbo.getNumVertices()), 20, infoY);
        infoY += 25;
        font.drawString("Indices: " + std::to_string(vbo.getNumIndices()), 20, infoY);
        infoY += 25;
        font.drawString("Has colors: " + std::string(vbo.hasColors() ? "Yes" : "No"), 20, infoY);
        infoY += 25;
        font.drawString("Drag to orbit, scroll to zoom", 20, infoY);
    }
}

void TestPhase2::drawTextureTest() {
    // Draw title
    ofSetColor(255, 255, 100);
    if (font.isLoaded()) {
        font.drawString("ofTexture Test - Mipmap Generation & Filtering", 20, 40);
    }

    // Draw texture at different sizes to show mipmap effect
    ofSetColor(255);

    // Large
    texture.draw(50, 100, 256, 256);
    if (font.isLoaded()) {
        font.drawString("256x256", 50, 380);
    }

    // Medium
    texture.draw(350, 100, 128, 128);
    if (font.isLoaded()) {
        font.drawString("128x128", 350, 250);
    }

    // Small
    texture.draw(520, 100, 64, 64);
    if (font.isLoaded()) {
        font.drawString("64x64", 520, 190);

        // Even smaller
        texture.draw(620, 100, 32, 32);
        font.drawString("32x32", 620, 160);

        texture.draw(690, 100, 16, 16);
        font.drawString("16x16", 690, 145);
    }

    // Draw info
    ofSetColor(200, 200, 200);
    float infoY = 450;
    if (font.isLoaded()) {
        font.drawString("Texture size: " + std::to_string(texture.getWidth()) + "x" + std::to_string(texture.getHeight()), 20, infoY);
        infoY += 25;
        font.drawString("Mipmap enabled: " + std::string(texture.hasMipmap() ? "Yes" : "No"), 20, infoY);
        infoY += 25;
        font.drawString("Mipmap levels: " + std::to_string(texture.getNumMipmapLevels()), 20, infoY);
        infoY += 25;
        font.drawString("Press M to toggle mipmap", 20, infoY);
    }
}

void TestPhase2::drawLightingTest() {
    // Draw title
    ofSetColor(255, 255, 100);
    if (font.isLoaded()) {
        font.drawString("ofLight + ofMaterial Test - Lighting System", 20, 40);
    }

    // Update camera position
    updateCameraPosition();

    // Begin camera
    camera.begin();
    ofEnableDepthTest();

    // Begin material
    material.begin();

    // Draw lit sphere
    ofSetColor(255);
    ofDrawSphere(0, 0, 0, 60);

    // Draw some boxes around
    for (int i = 0; i < 6; ++i) {
        float angle = i * 60.0f * 3.14159f / 180.0f;
        float x = std::cos(angle) * 120;
        float z = std::sin(angle) * 120;

        ofPushMatrix();
        ofTranslate(x, 0, z);
        ofRotateY(time * 30 + i * 60);
        ofDrawBox(30);
        ofPopMatrix();
    }

    // End material
    material.end();

    // Draw light position indicator
    oflike::ofVec3f lightPos = light.getPosition();
    ofSetColor(255, 255, 100);
    ofDrawSphere(lightPos.x, lightPos.y, lightPos.z, 5);

    ofDisableDepthTest();
    camera.end();

    // Draw info
    ofSetColor(200, 200, 200);
    float infoY = 80;
    if (font.isLoaded()) {
        font.drawString("Light type: Point", 20, infoY);
        infoY += 25;
        oflike::ofVec3f lp = light.getPosition();
        font.drawString("Light pos: " + std::to_string((int)lp.x) + ", " + std::to_string((int)lp.y) + ", " + std::to_string((int)lp.z), 20, infoY);
        infoY += 25;
        font.drawString("Material shininess: " + std::to_string((int)material.getShininess()), 20, infoY);
        infoY += 25;
        font.drawString("Light enabled: " + std::string(light.isEnabled() ? "Yes" : "No"), 20, infoY);
    }
}

void TestPhase2::updateCameraPosition() {
    float latRad = cameraLatitude * M_PI / 180.0f;
    float lonRad = cameraLongitude * M_PI / 180.0f;

    float x = cameraDistance * std::cos(latRad) * std::sin(lonRad);
    float y = cameraDistance * std::sin(latRad);
    float z = cameraDistance * std::cos(latRad) * std::cos(lonRad);

    camera.setPosition(x, y, z);
    camera.lookAt(oflike::ofVec3f(0, 0, 0));
}

void TestPhase2::keyPressed(int key) {
    // Number keys 1-3 to switch test modes
    if (key >= '1' && key <= '3') {
        testMode = key - '1';
    }
    if (key >= 18 && key <= 20) {
        testMode = key - 18;
    }

    // M key to toggle mipmap (keycode 46 on macOS)
    if (key == 46 || key == 'm' || key == 'M') {
        if (texture.hasMipmap()) {
            texture.disableMipmap();
        } else {
            texture.enableMipmap();
            texture.generateMipmap();
        }
    }
}

void TestPhase2::keyReleased(int key) {
    (void)key;
}

void TestPhase2::mouseMoved(int x, int y) {
    (void)x; (void)y;
}

void TestPhase2::mouseDragged(int x, int y, int button) {
    (void)button;
    if (isDragging && (testMode == 0 || testMode == 2)) {
        float dx = x - lastMouseX;
        float dy = y - lastMouseY;

        cameraLongitude += dx * 0.5f;
        cameraLatitude += dy * 0.5f;
        cameraLatitude = std::max(-89.0f, std::min(89.0f, cameraLatitude));

        lastMouseX = x;
        lastMouseY = y;
    }
}

void TestPhase2::mousePressed(int x, int y, int button) {
    (void)button;
    isDragging = true;
    lastMouseX = x;
    lastMouseY = y;
}

void TestPhase2::mouseReleased(int x, int y, int button) {
    (void)x; (void)y; (void)button;
    isDragging = false;
}

void TestPhase2::mouseScrolled(int x, int y, float scrollX, float scrollY) {
    (void)x; (void)y; (void)scrollX;
    if (testMode == 0 || testMode == 2) {
        cameraDistance -= scrollY * 10.0f;
        cameraDistance = std::max(50.0f, std::min(1000.0f, cameraDistance));
    }
}

void TestPhase2::windowResized(int w, int h) {
    if (w > 0 && h > 0) {
        camera.setAspectRatio((float)w / (float)h);
    }
}

extern "C" ofBaseApp* ofCreateApp() {
    return new TestPhase2();
}
