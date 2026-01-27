#include "Fff.h"

void Fff::setup() {
    // Initialization
    ofSetWindowTitle("fff");
}

void Fff::update() {
    // Update logic
}

void Fff::draw() {
    ofClear(50, 50, 50);

    // Draw example
    ofSetColor(255);
    ofDrawCircle(ofGetWidth() / 2, ofGetHeight() / 2, 100);
}

void Fff::keyPressed(int key) {
    // Handle key press
}

void Fff::keyReleased(int key) {
    // Handle key release
}

void Fff::mouseMoved(int x, int y) {
    // Handle mouse move
}

void Fff::mouseDragged(int x, int y, int button) {
    // Handle mouse drag
}

void Fff::mousePressed(int x, int y, int button) {
    // Handle mouse press
}

void Fff::mouseReleased(int x, int y, int button) {
    // Handle mouse release
}

void Fff::windowResized(int w, int h) {
    // Handle window resize
}

extern "C" ofBaseApp* ofCreateApp() {
    return new Fff();
}
