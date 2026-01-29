#include "Test12.h"

void Test12::setup() {
    // Initialization
    ofSetWindowTitle("test12");
}

void Test12::update() {
    // Update logic
}

void Test12::draw() {
    ofClear(50, 50, 50);

    // Draw example
    ofSetColor(255);
    ofDrawCircle(ofGetWidth() / 2, ofGetHeight() / 2, 100);
}

void Test12::keyPressed(int key) {
    // Handle key press
}

void Test12::keyReleased(int key) {
    // Handle key release
}

void Test12::mouseMoved(int x, int y) {
    // Handle mouse move
}

void Test12::mouseDragged(int x, int y, int button) {
    // Handle mouse drag
}

void Test12::mousePressed(int x, int y, int button) {
    // Handle mouse press
}

void Test12::mouseReleased(int x, int y, int button) {
    // Handle mouse release
}

void Test12::windowResized(int w, int h) {
    // Handle window resize
}

extern "C" ofBaseApp* ofCreateApp() {
    return new Test12();
}
