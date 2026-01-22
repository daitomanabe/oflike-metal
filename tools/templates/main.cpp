#include "oflike/ofMain.h"

class App : public AppBase {
public:
  void setup() override {
    ofBackground(40, 40, 50);
    ofSetFrameRate(60);
  }

  void update() override {
    // Update logic here
  }

  void draw() override {
    // Draw a simple scene
    ofSetColor(255);
    ofDrawBitmapString("Hello, oflike!", 20, 30);
    ofDrawBitmapString("FPS: " + ofToString(ofGetFrameRate(), 1), 20, 50);

    // Draw a rotating rectangle
    float time = ofGetElapsedTimef();
    ofPushMatrix();
    ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
    ofRotate(time * 45);
    ofSetColor(100, 200, 255);
    ofDrawRectangle(-50, -50, 100, 100);
    ofPopMatrix();

    // Instructions
    ofSetColor(180);
    ofDrawBitmapString("Press 'f' for fullscreen | ESC to quit", 20, ofGetHeight() - 20);
  }

  void keyPressed(int key) override {
    if (key == 'f' || key == 'F') {
      ofToggleFullscreen();
    }
  }

  void mousePressed(float x, float y, int button) override {
    // Handle mouse press
  }
};

OF_MAIN(App);
