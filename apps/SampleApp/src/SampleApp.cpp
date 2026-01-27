#include "SampleApp.h"

void SampleApp::setup() {
    // Initialization
    ofSetWindowTitle("SampleApp");
}

void SampleApp::update() {
    // Update logic
}

void SampleApp::draw() {
    ofClear(50, 50, 50);

    // Draw example
    ofSetColor(255);
    ofDrawCircle(ofGetWidth() / 2, ofGetHeight() / 2, 100);
}

void SampleApp::keyPressed(int key) {
    // Handle key press
}

void SampleApp::keyReleased(int key) {
    // Handle key release
}

void SampleApp::mouseMoved(int x, int y) {
    // Handle mouse move
}

void SampleApp::mouseDragged(int x, int y, int button) {
    // Handle mouse drag
}

void SampleApp::mousePressed(int x, int y, int button) {
    // Handle mouse press
}

void SampleApp::mouseReleased(int x, int y, int button) {
    // Handle mouse release
}

void SampleApp::windowResized(int w, int h) {
    // Handle window resize
}
