#include "Test5.h"

void Test5::setup() {
    // Initialization
    ofSetWindowTitle("Test5");
}

void Test5::update() {
    // Update logic
}

void Test5::draw() {
    ofClear(50, 50, 50);

    // Draw example
    ofSetColor(255);
    ofDrawCircle(ofGetWidth() / 2, ofGetHeight() / 2, 100);
}

void Test5::keyPressed(int key) {
    // Handle key press
}

void Test5::keyReleased(int key) {
    // Handle key release
}

void Test5::mouseMoved(int x, int y) {
    // Handle mouse move
}

void Test5::mouseDragged(int x, int y, int button) {
    // Handle mouse drag
}

void Test5::mousePressed(int x, int y, int button) {
    // Handle mouse press
}

void Test5::mouseReleased(int x, int y, int button) {
    // Handle mouse release
}

void Test5::windowResized(int w, int h) {
    // Handle window resize
}

extern "C" ofBaseApp* ofCreateApp() {
    return new Test5();
}
