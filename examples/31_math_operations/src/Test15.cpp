#include "Test15.h"
#include <oflike/graphics/ofGraphics.h>
#include <oflike/math/ofVec2f.h>
#include <oflike/math/ofVec3f.h>
#include <oflike/math/ofVec4f.h>
#include <oflike/math/ofMatrix4x4.h>
#include <oflike/math/ofQuaternion.h>
#include <oflike/math/ofMath.h>
#include <cmath>
#include <cstdio>

void Test15::setup() {
    ofSetWindowTitle("test15 - Math API Test (1-4: modes)");
    font.load("Helvetica", 16.0f);
    cam.setDistance(400);

    // Run some quick math tests
    printf("=== Math Module Tests ===\n");

    // Vector tests
    oflike::ofVec3f v1(1, 0, 0);
    oflike::ofVec3f v2(0, 1, 0);
    oflike::ofVec3f cross = v1.cross(v2);
    printf("Cross product (1,0,0) x (0,1,0) = (%.1f, %.1f, %.1f)\n", cross.x, cross.y, cross.z);

    float dot = v1.dot(v2);
    printf("Dot product: %.1f\n", dot);

    // Matrix test
    oflike::ofMatrix4x4 m = oflike::ofMatrix4x4::newRotationMatrix(90, 0, 0, 1);
    oflike::ofVec3f rotated = m * oflike::ofVec3f(1, 0, 0);
    printf("Rotate (1,0,0) by 90deg around Z = (%.2f, %.2f, %.2f)\n", rotated.x, rotated.y, rotated.z);

    // Quaternion test
    oflike::ofQuaternion q(90, oflike::ofVec3f(0, 0, 1));
    oflike::ofVec3f qRotated = q * oflike::ofVec3f(1, 0, 0);
    printf("Quaternion rotate (1,0,0) by 90deg around Z = (%.2f, %.2f, %.2f)\n", qRotated.x, qRotated.y, qRotated.z);

    // Math functions
    printf("ofRandom(10): %.2f\n", ofRandom(10));
    printf("ofNoise(0.5): %.2f\n", ofNoise(0.5f));
    printf("ofMap(5, 0, 10, 0, 100): %.1f\n", ofMap(5, 0, 10, 0, 100));
    printf("ofClamp(15, 0, 10): %.1f\n", ofClamp(15.0f, 0.0f, 10.0f));
    printf("ofLerp(0, 100, 0.5): %.1f\n", ofLerp(0, 100, 0.5f));
    printf("=========================\n");
}

void Test15::update() {
    cam.update();
}

void Test15::draw() {
    ofBackground(30, 35, 45);

    float time = ofGetElapsedTimef();

    // Draw title
    ofSetColor(255);
    if (font.isLoaded()) {
        font.drawString("Math Module Test", 20, 30);
    }

    std::string modeStr;
    switch (testMode) {
        case 0: modeStr = "Mode 1: Vectors (ofVec2f, ofVec3f, ofVec4f)"; break;
        case 1: modeStr = "Mode 2: Matrix (ofMatrix4x4)"; break;
        case 2: modeStr = "Mode 3: Quaternion (ofQuaternion)"; break;
        case 3: modeStr = "Mode 4: Math Functions"; break;
    }

    ofSetColor(200, 200, 100);
    if (font.isLoaded()) {
        font.drawString(modeStr, 20, 55);
    }

    if (testMode == 0) {
        // === Vector visualization ===
        float cx = 200, cy = 250;

        // Draw coordinate axes
        ofSetColor(100);
        ofDrawLine(cx - 150, cy, cx + 150, cy);
        ofDrawLine(cx, cy - 150, cx, cy + 150);

        // ofVec2f operations
        oflike::ofVec2f v2a(100, 0);
        oflike::ofVec2f v2b(0, 80);

        // Rotate v2a
        float angle = time * 60;
        float rad = angle * DEG_TO_RAD;
        oflike::ofVec2f v2Rotated(v2a.x * std::cos(rad) - v2a.y * std::sin(rad),
                                   v2a.x * std::sin(rad) + v2a.y * std::cos(rad));

        // Draw vectors
        ofSetLineWidth(2);
        ofSetColor(255, 100, 100);
        ofDrawLine(cx, cy, cx + v2Rotated.x, cy - v2Rotated.y);
        ofSetColor(100, 255, 100);
        ofDrawLine(cx, cy, cx + v2b.x, cy - v2b.y);

        // Addition
        oflike::ofVec2f sum = v2Rotated + v2b;
        ofSetColor(100, 100, 255);
        ofDrawLine(cx, cy, cx + sum.x, cy - sum.y);

        // Labels
        ofSetColor(180);
        if (font.isLoaded()) {
            font.drawString("ofVec2f: Red=rotating, Green=static, Blue=sum", 50, 420);

            char info[256];
            snprintf(info, sizeof(info), "Rotating: (%.1f, %.1f) len=%.1f",
                     v2Rotated.x, v2Rotated.y, v2Rotated.length());
            font.drawString(info, 50, 450);
        }

        // === 3D Vector demo ===
        ofSetColor(255);
        if (font.isLoaded()) {
            font.drawString("ofVec3f operations:", 450, 100);
        }

        oflike::ofVec3f a(1, 2, 3);
        oflike::ofVec3f b(4, 5, 6);

        char buf[256];
        int y = 130;

        ofSetColor(200);
        snprintf(buf, sizeof(buf), "a = (%.0f, %.0f, %.0f)", a.x, a.y, a.z);
        if (font.isLoaded()) font.drawString(buf, 450, y); y += 25;

        snprintf(buf, sizeof(buf), "b = (%.0f, %.0f, %.0f)", b.x, b.y, b.z);
        if (font.isLoaded()) font.drawString(buf, 450, y); y += 25;

        snprintf(buf, sizeof(buf), "a + b = (%.0f, %.0f, %.0f)", (a+b).x, (a+b).y, (a+b).z);
        if (font.isLoaded()) font.drawString(buf, 450, y); y += 25;

        snprintf(buf, sizeof(buf), "a.dot(b) = %.0f", a.dot(b));
        if (font.isLoaded()) font.drawString(buf, 450, y); y += 25;

        oflike::ofVec3f c = a.cross(b);
        snprintf(buf, sizeof(buf), "a.cross(b) = (%.0f, %.0f, %.0f)", c.x, c.y, c.z);
        if (font.isLoaded()) font.drawString(buf, 450, y); y += 25;

        snprintf(buf, sizeof(buf), "a.length() = %.2f", a.length());
        if (font.isLoaded()) font.drawString(buf, 450, y); y += 25;

        oflike::ofVec3f n = a.getNormalized();
        snprintf(buf, sizeof(buf), "a.normalized() = (%.2f, %.2f, %.2f)", n.x, n.y, n.z);
        if (font.isLoaded()) font.drawString(buf, 450, y); y += 25;

        snprintf(buf, sizeof(buf), "a.distance(b) = %.2f", a.distance(b));
        if (font.isLoaded()) font.drawString(buf, 450, y); y += 25;

    } else if (testMode == 1) {
        // === Matrix visualization - 3D ===
        cam.begin();
        ofEnableDepthTest();

        // Draw coordinate axes
        ofSetLineWidth(2);
        ofSetColor(255, 0, 0); ofDrawLine(0, 0, 0, 100, 0, 0);
        ofSetColor(0, 255, 0); ofDrawLine(0, 0, 0, 0, 100, 0);
        ofSetColor(0, 0, 255); ofDrawLine(0, 0, 0, 0, 0, 100);
        ofSetLineWidth(1);

        // Original box
        ofSetColor(100, 100, 100);
        ofNoFill();
        ofDrawBox(0, 0, 0, 50);

        // Transformed boxes
        ofPushMatrix();

        // Translation (demonstrating ofMatrix4x4 creation)
        // Note: Direct matrix application requires shader integration
        ofTranslate(80, 0, 0);
        ofSetColor(255, 100, 100);
        ofDrawBox(0, 0, 0, 40);
        ofPopMatrix();

        // Rotation
        ofPushMatrix();
        ofTranslate(-80, 0, 0);
        ofRotateY(time * 45);
        ofSetColor(100, 255, 100);
        ofDrawBox(0, 0, 0, 40);
        ofPopMatrix();

        // Scale
        ofPushMatrix();
        ofTranslate(0, 80, 0);
        float s = 1.0f + 0.3f * std::sin(time * 2);
        ofScale(s, s, s);
        ofSetColor(100, 100, 255);
        ofDrawBox(0, 0, 0, 40);
        ofPopMatrix();

        ofFill();
        ofDisableDepthTest();
        cam.end();

        // Matrix info
        ofSetColor(180);
        if (font.isLoaded()) {
            font.drawString("ofMatrix4x4: Translation (red), Rotation (green), Scale (blue)", 20, ofGetHeight() - 60);
            font.drawString("Use mouse to rotate camera", 20, ofGetHeight() - 35);
        }

    } else if (testMode == 2) {
        // === Quaternion visualization ===
        cam.begin();
        ofEnableDepthTest();

        // Draw axes
        ofSetLineWidth(2);
        ofSetColor(255, 0, 0); ofDrawLine(0, 0, 0, 100, 0, 0);
        ofSetColor(0, 255, 0); ofDrawLine(0, 0, 0, 0, 100, 0);
        ofSetColor(0, 0, 255); ofDrawLine(0, 0, 0, 0, 0, 100);
        ofSetLineWidth(1);

        // Quaternion rotation
        oflike::ofQuaternion q1 = oflike::ofQuaternion::makeRotate(time * 30, oflike::ofVec3f(0, 1, 0));
        oflike::ofQuaternion q2 = oflike::ofQuaternion::makeRotate(45, oflike::ofVec3f(1, 0, 0));
        oflike::ofQuaternion combined = q1 * q2;

        // Get Euler angles
        oflike::ofVec3f euler = combined.getEuler();

        // Apply rotation via Euler (simplified)
        ofPushMatrix();
        ofRotateX(euler.x);
        ofRotateY(euler.y);
        ofRotateZ(euler.z);

        ofSetColor(255, 200, 100);
        ofDrawBox(0, 0, 0, 60);

        // Draw local axes
        ofSetLineWidth(3);
        ofSetColor(255, 50, 50); ofDrawLine(0, 0, 0, 50, 0, 0);
        ofSetColor(50, 255, 50); ofDrawLine(0, 0, 0, 0, 50, 0);
        ofSetColor(50, 50, 255); ofDrawLine(0, 0, 0, 0, 0, 50);
        ofSetLineWidth(1);

        ofPopMatrix();

        // SLERP demo
        oflike::ofQuaternion qA = oflike::ofQuaternion::makeRotate(0, oflike::ofVec3f(0, 1, 0));
        oflike::ofQuaternion qB = oflike::ofQuaternion::makeRotate(90, oflike::ofVec3f(0, 1, 0));
        float t = (std::sin(time) + 1.0f) * 0.5f;
        oflike::ofQuaternion qSlerp = oflike::ofQuaternion::slerp(qA, qB, t);

        ofPushMatrix();
        ofTranslate(100, 0, 0);
        oflike::ofVec3f slerpEuler = qSlerp.getEuler();
        ofRotateY(slerpEuler.y);
        ofSetColor(100, 200, 255);
        ofDrawCone(0, 0, 0, 15, 40);
        ofPopMatrix();

        ofDisableDepthTest();
        cam.end();

        // Info
        ofSetColor(180);
        if (font.isLoaded()) {
            char buf[128];
            snprintf(buf, sizeof(buf), "Euler: (%.1f, %.1f, %.1f)", euler.x, euler.y, euler.z);
            font.drawString(buf, 20, ofGetHeight() - 60);
            snprintf(buf, sizeof(buf), "SLERP t=%.2f (cone on right)", t);
            font.drawString(buf, 20, ofGetHeight() - 35);
        }

    } else if (testMode == 3) {
        // === Math functions visualization ===
        float graphY = 150;
        float graphH = 100;
        float graphW = 300;

        // ofNoise visualization
        ofSetColor(255);
        if (font.isLoaded()) font.drawString("ofNoise(x + time):", 50, graphY - 20);

        ofSetColor(100, 200, 255);
        ofNoFill();
        ofBeginShape();
        for (int i = 0; i < (int)graphW; i++) {
            float x = i / graphW * 5.0f;
            float n = ofNoise(x + time * 0.5f);
            ofVertex(50 + i, graphY + graphH - n * graphH);
        }
        ofEndShape(false);
        ofFill();

        // ofRandom visualization
        graphY += 150;
        ofSetColor(255);
        if (font.isLoaded()) font.drawString("ofRandom() samples:", 50, graphY - 20);

        static float randomValues[50];
        static int frameCount = 0;
        if (frameCount++ % 10 == 0) {
            for (int i = 49; i > 0; i--) randomValues[i] = randomValues[i-1];
            randomValues[0] = ofRandom(1.0f);
        }

        ofSetColor(255, 150, 100);
        for (int i = 0; i < 50; i++) {
            float h = randomValues[i] * graphH;
            ofDrawRectangle(50 + i * 6, graphY + graphH - h, 4, h);
        }

        // ofMap visualization
        graphY += 150;
        ofSetColor(255);
        if (font.isLoaded()) font.drawString("ofMap(-1..1 -> 0..100):", 50, graphY - 20);

        float input = std::sin(time);
        float mapped = ofMap(input, -1, 1, 0, 100);

        ofSetColor(100, 255, 150);
        ofDrawRectangle(50, graphY, mapped * 3, 30);
        ofSetColor(180);
        char buf[64];
        snprintf(buf, sizeof(buf), "in=%.2f out=%.1f", input, mapped);
        if (font.isLoaded()) font.drawString(buf, 50, graphY + 55);

        // ofLerp visualization
        graphY += 100;
        ofSetColor(255);
        if (font.isLoaded()) font.drawString("ofLerp(0, 300, t):", 50, graphY - 20);

        float lerpT = (std::sin(time * 2) + 1.0f) * 0.5f;
        float lerpVal = ofLerp(0, 300, lerpT);

        ofSetColor(255, 200, 100);
        ofDrawCircle(50 + lerpVal, graphY + 15, 10);
        ofSetColor(100);
        ofDrawLine(50, graphY + 15, 350, graphY + 15);

        // Right side - more functions
        float rx = 450;
        float ry = 120;

        ofSetColor(255);
        if (font.isLoaded()) font.drawString("Other functions:", rx, ry);

        ofSetColor(180);
        ry += 30;
        snprintf(buf, sizeof(buf), "ofClamp(%.1f, 0, 10) = %.1f", time, ofClamp(time, 0.0f, 10.0f));
        if (font.isLoaded()) font.drawString(buf, rx, ry); ry += 25;

        snprintf(buf, sizeof(buf), "ofDist(0,0, 3,4) = %.1f", ofDist(0, 0, 3, 4));
        if (font.isLoaded()) font.drawString(buf, rx, ry); ry += 25;

        snprintf(buf, sizeof(buf), "ofDegToRad(180) = %.4f", ofDegToRad(180));
        if (font.isLoaded()) font.drawString(buf, rx, ry); ry += 25;

        snprintf(buf, sizeof(buf), "ofRadToDeg(PI) = %.1f", ofRadToDeg(PI));
        if (font.isLoaded()) font.drawString(buf, rx, ry); ry += 25;

        snprintf(buf, sizeof(buf), "ofWrapDegrees(450) = %.1f", ofWrapDegrees(450));
        if (font.isLoaded()) font.drawString(buf, rx, ry); ry += 25;

        snprintf(buf, sizeof(buf), "ofSign(-5) = %d", ofSign(-5));
        if (font.isLoaded()) font.drawString(buf, rx, ry); ry += 25;

        snprintf(buf, sizeof(buf), "ofSignedNoise(time) = %.2f", ofSignedNoise(time));
        if (font.isLoaded()) font.drawString(buf, rx, ry); ry += 25;
    }

    // Key hints
    ofSetColor(150);
    if (font.isLoaded()) {
        font.drawString("Keys: 1-4 = switch mode", 20, ofGetHeight() - 10);
    }
}

void Test15::keyPressed(int key) {
    printf("Key pressed: %d\n", key);
    if (key == 18) testMode = 0;      // 1
    else if (key == 19) testMode = 1; // 2
    else if (key == 20) testMode = 2; // 3
    else if (key == 21) testMode = 3; // 4
}

void Test15::keyReleased(int key) { (void)key; }
void Test15::mouseMoved(int x, int y) { (void)x; (void)y; }
void Test15::mouseDragged(int x, int y, int button) {
    if (testMode == 1 || testMode == 2) {
        cam.onMouseDragged(x, y, button);
    }
}
void Test15::mousePressed(int x, int y, int button) {
    if (testMode == 1 || testMode == 2) {
        cam.onMousePressed(x, y, button);
    }
}
void Test15::mouseReleased(int x, int y, int button) {
    if (testMode == 1 || testMode == 2) {
        cam.onMouseReleased(x, y, button);
    }
}
void Test15::mouseScrolled(int x, int y, float scrollX, float scrollY) {
    (void)x; (void)y; (void)scrollX;
    if (testMode == 1 || testMode == 2) {
        cam.onMouseScrolled(0, scrollY);
    }
}
void Test15::windowResized(int w, int h) { (void)w; (void)h; }

extern "C" ofBaseApp* ofCreateApp() {
    return new Test15();
}
