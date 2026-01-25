#include "PerformanceTestApp.h"

void PerformanceTestApp::setup() {
    ofSetFrameRate(60);
    ofLogNotice("PerformanceTestApp") << "Setup complete - rendering " << numCircles << " circles per frame";
}

void PerformanceTestApp::update() {
    angle += 0.02f;
}

void PerformanceTestApp::draw() {
    // Clear background
    ofBackground(20, 20, 30);

    // Draw multiple circles to generate draw calls
    // Each ofDrawCircle() generates a draw call with vertices
    float centerX = ofGetWidth() / 2.0f;
    float centerY = ofGetHeight() / 2.0f;

    for (int i = 0; i < numCircles; i++) {
        float t = (angle + i * 0.3f);
        float x = centerX + cos(t) * 200.0f;
        float y = centerY + sin(t) * 200.0f;

        // Color varies by index
        float hue = (float)i / numCircles * 255.0f;
        ofSetColor(hue, 200, 255 - hue);

        // Each circle generates a separate draw call
        ofDrawCircle(x, y, 30.0f);
    }

    // Draw center marker
    ofSetColor(255, 255, 255);
    ofDrawCircle(centerX, centerY, 10.0f);

    // Draw some rectangles to add more draw calls
    for (int i = 0; i < 5; i++) {
        float x = 50 + i * 150;
        float y = 50;
        ofSetColor(100 + i * 30, 100, 200);
        ofDrawRectangle(x, y, 80, 80);
    }

    // Info text (if you have ofTrueTypeFont working, you can add stats display here)
    ofSetColor(255, 255, 100);
    // Note: Text rendering would add more draw calls
    // ofDrawBitmapString("Check performance overlay (top-left)", 10, ofGetHeight() - 20);
}
