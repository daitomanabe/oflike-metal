#include "AutoXcodeGenTest4.h"

void AutoXcodeGenTest4::setup() {
    // Initialization
    ofSetWindowTitle("AutoXcodeGenTest4");
}

void AutoXcodeGenTest4::update() {
    // Update logic
}

void AutoXcodeGenTest4::draw() {
    ofClear(50, 50, 50);

    // Draw example
    ofSetColor(255);
    ofDrawCircle(ofGetWidth() / 2, ofGetHeight() / 2, 100);
}

void AutoXcodeGenTest4::keyPressed(int key) {
    // Handle key press
}

void AutoXcodeGenTest4::keyReleased(int key) {
    // Handle key release
}

void AutoXcodeGenTest4::mouseMoved(int x, int y) {
    // Handle mouse move
}

void AutoXcodeGenTest4::mouseDragged(int x, int y, int button) {
    // Handle mouse drag
}

void AutoXcodeGenTest4::mousePressed(int x, int y, int button) {
    // Handle mouse press
}

void AutoXcodeGenTest4::mouseReleased(int x, int y, int button) {
    // Handle mouse release
}

void AutoXcodeGenTest4::windowResized(int w, int h) {
    // Handle window resize
}

extern "C" ofBaseApp* ofCreateApp() {
    return new AutoXcodeGenTest4();
}
