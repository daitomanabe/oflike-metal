#include "Test13.h"
#include <cmath>

void Test13::setup() {
    ofSetWindowTitle("test13 - ofGraphics API Test (SPACE: toggle 2D/3D)");
    ofSetCircleResolution(64);
    ofSetSphereResolution(24);

    // Load system font
    font.load("Helvetica", 24.0f);

    // Setup camera
    cam.setDistance(300);
    cam.setTarget(0, 0, 0);
    cam.enableMouseInput();
}

void Test13::update() {
    cam.update();
}

void Test13::draw() {
    ofBackground(30, 30, 40);

    float time = ofGetElapsedTimef();

    if (show3D) {
        // =====================================================
        // 3D MODE - Using camera with proper 3D coordinates
        // =====================================================

        // Begin camera view
        cam.begin();

        // Enable depth testing for proper 3D rendering
        ofEnableDepthTest();

        // Test 1: Central filled box (rotating)
        ofFill();
        ofSetColor(100, 100, 255);
        ofPushMatrix();
        ofRotateY(time * 30);
        ofDrawBox(50);  // Blue box at origin
        ofPopMatrix();

        // Test 2: Wireframe box to the left
        ofNoFill();
        ofSetColor(255, 100, 100);
        ofSetLineWidth(2.0f);
        ofPushMatrix();
        ofTranslate(-100, 0, 0);
        ofRotateX(time * 25);
        ofDrawBox(40);  // Red wireframe box
        ofPopMatrix();

        // Test 3: Filled box to the right
        ofFill();
        ofSetColor(100, 255, 100);
        ofPushMatrix();
        ofTranslate(100, 0, 0);
        ofRotateZ(time * 20);
        ofDrawBox(35);  // Green box
        ofPopMatrix();

        // Test 4: Small boxes in a circle
        for (int i = 0; i < 8; i++) {
            float angle = i * 45.0f * 3.14159f / 180.0f + time * 0.5f;
            float radius = 120.0f;
            float x = std::cos(angle) * radius;
            float z = std::sin(angle) * radius;

            ofSetColor(255 - i * 30, 100 + i * 20, 150);
            ofDrawBox(x, 0, z, 15);
        }

        // Test 5: Floor grid (wireframe)
        ofNoFill();
        ofSetColor(80, 80, 100);
        ofSetLineWidth(1.0f);
        for (int i = -5; i <= 5; i++) {
            ofDrawLine(i * 30, -50, -150, i * 30, -50, 150);
            ofDrawLine(-150, -50, i * 30, 150, -50, i * 30);
        }

        ofSetLineWidth(1.0f);
        ofDisableDepthTest();

        // End camera view
        cam.end();

    } else {
        // =====================================================
        // 2D MODE
        // =====================================================

        // === 1. Basic Lines ===
        ofSetColor(255, 100, 100);
        ofSetLineWidth(2.0f);
        ofDrawLine(50, 50, 150, 100);
        ofDrawLine(150, 50, 50, 100);

        // === 2. Rectangles (Fill vs NoFill) ===
        ofFill();
        ofSetColor(100, 200, 100);
        ofDrawRectangle(200, 50, 80, 60);

        ofNoFill();
        ofSetColor(200, 200, 100);
        ofSetLineWidth(3.0f);
        ofDrawRectangle(300, 50, 80, 60);

        // === 3. Rounded Rectangle ===
        ofFill();
        ofSetColor(100, 150, 255);
        ofDrawRectRounded(400, 50, 100, 60, 15);

        // === 4. Circles ===
        ofFill();
        ofSetColor(255, 150, 50);
        ofDrawCircle(100, 200, 40);

        ofNoFill();
        ofSetLineWidth(2.0f);
        ofSetColor(50, 200, 255);
        ofDrawCircle(200, 200, 40);

        // === 5. Ellipse ===
        ofFill();
        ofSetColor(200, 100, 255);
        ofDrawEllipse(320, 200, 100, 50);

        // === 6. Triangle ===
        ofFill();
        ofSetColor(255, 200, 100);
        ofDrawTriangle(450, 160, 500, 240, 400, 240);

        // === 7. Custom Shape (Star) ===
        ofFill();
        ofSetColor(255, 255, 100);
        ofPushMatrix();
        ofTranslate(600, 200);
        ofRotate(time * 30);
        ofBeginShape();
        for (int i = 0; i < 10; i++) {
            float angle = i * 36.0f * 3.14159f / 180.0f;
            float radius = (i % 2 == 0) ? 40.0f : 20.0f;
            ofVertex(std::cos(angle) * radius, std::sin(angle) * radius);
        }
        ofEndShape(true);
        ofPopMatrix();

        // === 8. Bezier Curve ===
        ofNoFill();
        ofSetColor(100, 255, 200);
        ofSetLineWidth(3.0f);
        ofDrawBezier(50, 320, 100, 280, 200, 400, 250, 320);

        // === 9. Matrix Transform Demo ===
        ofPushMatrix();
        ofTranslate(400, 350);
        ofRotate(time * 45);
        ofScale(1.0f + 0.3f * std::sin(time * 2));
        ofFill();
        ofSetColor(150, 200, 255);
        ofDrawRectangle(-30, -30, 60, 60);
        ofPopMatrix();

        // === 10. Alpha Blending Demo ===
        ofEnableAlphaBlending();
        ofFill();
        ofSetColor(255, 0, 0, 150);
        ofDrawCircle(550, 380, 50);
        ofSetColor(0, 255, 0, 150);
        ofDrawCircle(580, 380, 50);
        ofSetColor(0, 0, 255, 150);
        ofDrawCircle(565, 350, 50);
    }

    // Reset
    ofSetLineWidth(1.0f);
    ofFill();

    // Draw mode indicator with text
    float indicatorX = 20;
    float indicatorY = ofGetHeight() - 50;

    // Background box
    ofSetColor(50, 50, 60);
    ofDrawRectRounded(indicatorX, indicatorY, 280, 35, 5);

    // Text label
    ofSetColor(255);
    if (font.isLoaded()) {
        std::string modeText = show3D ? "3D MODE - Press SPACE" : "2D MODE - Press SPACE";
        font.drawString(modeText, indicatorX + 15, indicatorY + 25);
    }
}

void Test13::keyPressed(int key) {
    printf("Key pressed: %d\n", key);
    // SPACE key (keycode 49 on macOS)
    if (key == 49) {
        show3D = !show3D;
        printf("Toggled to %s mode\n", show3D ? "3D" : "2D");
    }
}

void Test13::keyReleased(int key) {
    (void)key;
}

void Test13::mouseMoved(int x, int y) {
    (void)x; (void)y;
}

void Test13::mouseDragged(int x, int y, int button) {
    if (show3D) {
        cam.onMouseDragged(x, y, button);
    }
}

void Test13::mousePressed(int x, int y, int button) {
    if (show3D) {
        cam.onMousePressed(x, y, button);
    }
}

void Test13::mouseReleased(int x, int y, int button) {
    if (show3D) {
        cam.onMouseReleased(x, y, button);
    }
}

void Test13::mouseScrolled(int x, int y, float scrollX, float scrollY) {
    (void)x; (void)y; (void)scrollX;
    if (show3D) {
        cam.onMouseScrolled(0, scrollY);
    }
}

void Test13::windowResized(int w, int h) {
    (void)w; (void)h;
}

extern "C" ofBaseApp* ofCreateApp() {
    return new Test13();
}
