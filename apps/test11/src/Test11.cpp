#include "Test11.h"

void Test11::setup() {
    // Initialization
    ofSetWindowTitle("test11");
}

void Test11::update() {
    // Update logic
}

void Test11::draw() {
    ofClear(50, 50, 50);

    // Draw example
    ofSetColor(255);
    ofDrawCircle(ofGetWidth() / 2, ofGetHeight() / 2, 100);
}

void Test11::keyPressed(int key) {
    // Handle key press
}

void Test11::keyReleased(int key) {
    // Handle key release
}

void Test11::mouseMoved(int x, int y) {
    // Handle mouse move
}

void Test11::mouseDragged(int x, int y, int button) {
    // Handle mouse drag
}

void Test11::mousePressed(int x, int y, int button) {
    // Handle mouse press
}

void Test11::mouseReleased(int x, int y, int button) {
    // Handle mouse release
}

void Test11::windowResized(int w, int h) {
    // Handle window resize
}

extern "C" ofBaseApp* ofCreateApp() {
    return new Test11();
}
