#include "Test10.h"

void Test10::setup() {
    // Initialization
    ofSetWindowTitle("test10");
}

void Test10::update() {
    // Update logic
}

void Test10::draw() {
    ofClear(50, 50, 50);

    // Draw example
    ofSetColor(255);
    ofDrawCircle(ofGetWidth() / 2, ofGetHeight() / 2, 100);
}

void Test10::keyPressed(int key) {
    // Handle key press
}

void Test10::keyReleased(int key) {
    // Handle key release
}

void Test10::mouseMoved(int x, int y) {
    // Handle mouse move
}

void Test10::mouseDragged(int x, int y, int button) {
    // Handle mouse drag
}

void Test10::mousePressed(int x, int y, int button) {
    // Handle mouse press
}

void Test10::mouseReleased(int x, int y, int button) {
    // Handle mouse release
}

void Test10::windowResized(int w, int h) {
    // Handle window resize
}

extern "C" ofBaseApp* ofCreateApp() {
    return new Test10();
}
