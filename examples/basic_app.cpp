#include "../src/oflike/ofMain.h"

class ofApp : public AppBase {
public:
  void setup() override {
    ofBackground(10, 10, 10);
  }

  void update() override {
    x += 1.5f;
    if (x > 800.f) x = -w;
  }

  void draw() override {
    // Draw animated rectangle
    ofSetColor(255, 60, 60, 255);
    ofDrawRectangle(x, y, w, h);

    // Draw debug info
    ofSetColor(255, 255, 255, 255);

    std::string info;
    info += "FPS: " + ofToString(ofGetFrameRate(), 1) + "\n";
    info += "Frame: " + ofToString(ofGetFrameNum()) + "\n";
    info += "Time: " + ofToString(ofGetElapsedTimef(), 2) + "s\n";
    info += "Window: " + ofToString(ofGetWidth()) + " x " + ofToString(ofGetHeight()) + "\n";
    info += "Mouse: " + ofToString((int)mouseX) + ", " + ofToString((int)mouseY) + "\n";

    ofDrawBitmapString(info, 10, 20);

    // Draw instructions
    ofSetColor(200, 200, 200, 255);
    ofDrawBitmapString("Drag to move rectangle", 10, ofGetHeight() - 20);
  }

  void mouseDragged(float mx, float my, int /*button*/) override {
    x = mx - w * 0.5f;
    y = my - h * 0.5f;
    mouseX = mx;
    mouseY = my;
  }

  void mouseMoved(float mx, float my) override {
    mouseX = mx;
    mouseY = my;
  }

  void mousePressed(float mx, float my, int /*button*/) override {
    mouseX = mx;
    mouseY = my;
  }

private:
  float x{100.f};
  float y{120.f};
  float w{160.f};
  float h{120.f};
  float mouseX{0.f};
  float mouseY{0.f};
};

OF_MAIN(ofApp);
