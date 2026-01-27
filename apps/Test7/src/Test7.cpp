#include "Test7.h"

void Test7::setup() {
    // Initialization
    ofSetWindowTitle("Test7");
}

void Test7::update() {
    // Update logic
}

void Test7::draw() {
    ofClear(50, 50, 50);

    // Draw example
    ofSetColor(255);
    ofDrawCircle(ofGetWidth() / 2, ofGetHeight() / 2, 100);
}

void Test7::keyPressed(int key) {
    // Handle key press
}

void Test7::keyReleased(int key) {
    // Handle key release
}

void Test7::mouseMoved(int x, int y) {
    // Handle mouse move
}

void Test7::mouseDragged(int x, int y, int button) {
    // Handle mouse drag
}

void Test7::mousePressed(int x, int y, int button) {
    // Handle mouse press
}

void Test7::mouseReleased(int x, int y, int button) {
    // Handle mouse release
}

void Test7::windowResized(int w, int h) {
    // Handle window resize
}

extern "C" ofBaseApp* ofCreateApp() {
    return new Test7();
}
