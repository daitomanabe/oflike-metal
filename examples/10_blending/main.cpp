// Simplified blendingExample
// Demonstrates different blend modes with shapes

#include "../../src/oflike/ofMain.h"

class BlendingExampleApp : public AppBase {
public:
  void setup() override {
    ofBackground(50, 50, 50);
    blendMode = OF_BLENDMODE_ALPHA;
    modeString = "Alpha";
  }

  void update() override {
  }

  void draw() override {
    int mouseX = ofGetMouseX();
    int mouseY = ofGetMouseY();

    // Instructions
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    ofSetColor(255);
    ofDrawBitmapString("Press 1-5 to switch blend modes", 20, 30);
    ofDrawBitmapString("Current mode: " + modeString, 20, 50);
    ofDrawBitmapString("1: Alpha, 2: Add, 3: Multiply, 4: Subtract, 5: Screen", 20, 70);

    // Draw background shapes
    ofFill();

    // Red circle
    ofSetColor(255, 0, 0);
    ofDrawCircle(200, 300, 100);

    // Green circle
    ofSetColor(0, 255, 0);
    ofDrawCircle(350, 300, 100);

    // Blue circle
    ofSetColor(0, 0, 255);
    ofDrawCircle(275, 450, 100);

    // Now draw overlapping shapes with the selected blend mode
    ofEnableBlendMode(blendMode);

    // Draw a large semi-transparent white shape that follows the mouse
    ofSetColor(255, 255, 255, 180);
    ofDrawCircle(mouseX, mouseY, 120);

    // Draw some overlapping colored shapes
    ofSetColor(255, 200, 0, 150);
    ofDrawRectangle(mouseX - 80, mouseY - 80, 80, 80);

    ofSetColor(0, 200, 255, 150);
    ofDrawRectangle(mouseX, mouseY, 80, 80);

    ofSetColor(255, 0, 200, 150);
    ofDrawTriangle(mouseX - 40, mouseY + 100, mouseX + 40, mouseY + 100, mouseX, mouseY + 40);

    ofDisableBlendMode();

    // Draw blend mode visualization
    ofSetColor(255);
    ofDrawBitmapString("Move mouse to see blending effects", 20, ofGetHeight() - 20);

    // Draw a small legend
    int legendY = 100;
    int legendX = ofGetWidth() - 180;

    ofEnableAlphaBlending();
    ofSetColor(255);
    ofDrawBitmapString("Background:", legendX, legendY);

    ofSetColor(255, 0, 0);
    ofDrawCircle(legendX + 30, legendY + 25, 15);
    ofSetColor(255);
    ofDrawBitmapString("Red", legendX + 50, legendY + 30);

    ofSetColor(0, 255, 0);
    ofDrawCircle(legendX + 30, legendY + 55, 15);
    ofSetColor(255);
    ofDrawBitmapString("Green", legendX + 50, legendY + 60);

    ofSetColor(0, 0, 255);
    ofDrawCircle(legendX + 30, legendY + 85, 15);
    ofSetColor(255);
    ofDrawBitmapString("Blue", legendX + 50, legendY + 90);
  }

  void keyPressed(int key) override {
    switch (key) {
      case '1':
        blendMode = OF_BLENDMODE_ALPHA;
        modeString = "Alpha";
        break;
      case '2':
        blendMode = OF_BLENDMODE_ADD;
        modeString = "Add";
        break;
      case '3':
        blendMode = OF_BLENDMODE_MULTIPLY;
        modeString = "Multiply";
        break;
      case '4':
        blendMode = OF_BLENDMODE_SUBTRACT;
        modeString = "Subtract";
        break;
      case '5':
        blendMode = OF_BLENDMODE_SCREEN;
        modeString = "Screen";
        break;
      default:
        break;
    }
  }

private:
  ofBlendMode blendMode;
  std::string modeString;
};

OF_MAIN(BlendingExampleApp);
