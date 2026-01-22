// Ported from openFrameworks graphicsExample
// Demonstrates basic drawing functions

#include "../../src/oflike/ofMain.h"

class GraphicsExampleApp : public AppBase {
public:
  void setup() override {
    counter = 0;
    ofSetCircleResolution(50);
    ofBackground(255, 255, 255);
    bSmooth = false;
  }

  void update() override {
    // update event slowly increments the counter variable
    counter = counter + 0.033f;
  }

  void draw() override {
    //--------------------------- circles
    // let's draw a circle:
    ofSetColor(255, 130, 0);
    float radius = 50 + 10 * sin(counter);
    ofFill();  // draw "filled shapes"
    ofDrawCircle(100, 400, radius);

    // now just an outline
    ofNoFill();
    ofSetHexColor(0xCCCCCC);
    ofDrawCircle(100, 400, 80);

    // use the bitMap type
    ofSetHexColor(0x000000);
    ofDrawBitmapString("circle", 75, 500);

    //--------------------------- rectangles
    ofFill();
    for (int i = 0; i < 200; i++) {
      ofSetColor((int)ofRandom(0, 255), (int)ofRandom(0, 255), (int)ofRandom(0, 255));
      ofDrawRectangle(ofRandom(250, 350), ofRandom(350, 450), ofRandom(10, 20), ofRandom(10, 20));
    }
    ofSetHexColor(0x000000);
    ofDrawBitmapString("rectangles", 275, 500);

    //--------------------------- transparency
    ofSetHexColor(0x00FF33);
    ofDrawRectangle(400, 350, 100, 100);
    // alpha is usually turned off - for speed purposes. let's turn it on!
    ofEnableAlphaBlending();
    ofSetColor(255, 0, 0, 127);  // red, 50% transparent
    ofDrawRectangle(450, 430, 100, 33);
    ofSetColor(255, 0, 0, (int)(counter * 10.0f) % 255);  // red, variable transparent
    ofDrawRectangle(450, 370, 100, 33);
    ofDisableAlphaBlending();

    ofSetHexColor(0x000000);
    ofDrawBitmapString("transparency", 410, 500);

    //--------------------------- lines
    // a bunch of red lines, make them smooth if the flag is set
    ofSetHexColor(0xFF0000);
    for (int i = 0; i < 20; i++) {
      ofDrawLine(600, 300 + (i * 5), 800, 250 + (i * 10));
    }

    ofSetHexColor(0x000000);
    ofDrawBitmapString("lines\npress 's' to toggle smoothness", 600, 500);

    // Info display
    ofSetColor(0);
    ofDrawBitmapString("FPS: " + ofToString(ofGetFrameRate(), 1), 10, 20);
    ofDrawBitmapString("Smoothing: " + std::string(bSmooth ? "ON" : "OFF"), 10, 35);
  }

  void keyPressed(int key) override {
    // the key of s toggles antialiasing
    if (key == 's' || key == 'S') {
      bSmooth = !bSmooth;
      if (bSmooth) {
        ofEnableAntiAliasing();
      } else {
        ofDisableAntiAliasing();
      }
    }
  }

private:
  float counter;
  bool bSmooth;
};

OF_MAIN(GraphicsExampleApp);
