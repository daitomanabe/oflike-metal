/// Example 01: Basic Drawing
/// Demonstrates basic oflike-metal drawing capabilities

#include "core/AppBase.h"
#include "oflike/graphics/ofGraphics.h"
#include "oflike/types/ofColor.h"
#include "oflike/utils/ofUtils.h"

class ofApp : public ofBaseApp {
public:
    float circleX = 0.0f;
    float circleY = 0.0f;
    float angle = 0.0f;

    void setup() override {
        circleX = ofGetWidth() / 2.0f;
        circleY = ofGetHeight() / 2.0f;
        ofSetFrameRate(60);
    }

    void update() override {
        angle += 0.02f;
        float orbitRadius = 100.0f;
        circleX = ofGetWidth() / 2.0f + cos(angle) * orbitRadius;
        circleY = ofGetHeight() / 2.0f + sin(angle) * orbitRadius;
    }

    void draw() override {
        ofBackground(40);

        // Center point
        ofSetColor(255);
        ofDrawCircle(ofGetWidth() / 2.0f, ofGetHeight() / 2.0f, 5);

        // Orbit path
        ofNoFill();
        ofSetColor(80);
        ofDrawCircle(ofGetWidth() / 2.0f, ofGetHeight() / 2.0f, 100);

        // Animated circle
        ofFill();
        ofSetColor(255, 100, 100);
        ofDrawCircle(circleX, circleY, 50);

        // Corner shapes
        ofSetColor(255, 0, 0);
        ofDrawRectangle(50, 50, 80, 60);

        ofSetColor(0, 255, 0);
        ofDrawTriangle(ofGetWidth() - 130, 50, ofGetWidth() - 50, 50, ofGetWidth() - 90, 110);

        ofSetColor(0, 100, 255);
        ofDrawCircle(90, ofGetHeight() - 90, 40);

        ofNoFill();
        ofSetColor(255, 255, 0);
        ofDrawRectangle(ofGetWidth() - 130, ofGetHeight() - 110, 80, 60);

        // Lines to animated circle
        ofSetColor(100, 100, 100, 100);
        ofDrawLine(50, 50, circleX, circleY);
        ofDrawLine(ofGetWidth() - 90, 50, circleX, circleY);

        // FPS bar
        ofFill();
        ofSetColor(255, 200);
        float barWidth = (ofGetFrameRate() / 60.0f) * 100.0f;
        ofDrawRectangle(ofGetWidth() / 2.0f - 50, 10, barWidth, 5);
    }

    void windowResized(int w, int h) override {
        circleX = w / 2.0f;
        circleY = h / 2.0f;
    }
};

// Entry point
#define OF_APP ofApp
#include "oflike/app/ofMain.h"
