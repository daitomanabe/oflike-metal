#include "ValidationApp.h"

void ValidationApp::setup() {
    ofSetFrameRate(60);

    // Load test image (optional - app should run without it)
    testImage.load("test.jpg");

    // Load test font (optional - app should run without it)
    testFont.load("Arial.ttf", 32);
}

void ValidationApp::update() {
    angle += 0.02f;
}

void ValidationApp::draw() {
    ofBackground(50);

    // Test 1: ofDrawCircle
    ofSetColor(255, 100, 100);
    float centerX = ofGetWidth() / 2.0f;
    float centerY = ofGetHeight() / 2.0f;
    ofDrawCircle(centerX, centerY, 100);

    // Animated circle
    float orbitX = centerX + cos(angle) * 150.0f;
    float orbitY = centerY + sin(angle) * 150.0f;
    ofSetColor(100, 255, 100);
    ofDrawCircle(orbitX, orbitY, 30);

    // Test 2: ofImage (if loaded)
    if (testImage.isAllocated()) {
        ofSetColor(255);
        testImage.draw(50, 50, 200, 150);
    }

    // Test 3: ofTrueTypeFont
    if (testFont.isLoaded()) {
        ofSetColor(255, 255, 100);
        testFont.drawString("Hello, oflike-metal!", 50, ofGetHeight() - 50);

        // Info text
        ofSetColor(255);
        testFont.drawString("FPS: " + ofToString(ofGetFrameRate(), 1),
                           ofGetWidth() - 150, 50);
    }
}

extern "C" ofBaseApp* ofCreateApp() {
    return new ValidationApp();
}
