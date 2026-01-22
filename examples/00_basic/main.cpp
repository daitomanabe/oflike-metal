#include "../../src/oflike/ofMain.h"

class BasicApp : public AppBase {
public:
  void setup() override {
    ofBackground(20, 20, 30);
    ofSetFrameRate(60);
  }

  void update() override {
    angle += 0.5f;
  }

  void draw() override {
    float time = ofGetElapsedTimef();

    // ============== Filled Shapes ==============
    ofFill();

    // Rectangle
    ofSetColor(255, 80, 80);
    ofDrawRectangle(50, 80, 80, 60);

    // Circle with HSB color
    ofColor hsbColor = ofColor::fromHsb(fmod(time * 50, 255), 200, 255);
    ofSetColor(hsbColor);
    ofDrawCircle(200, 110, 35);

    // Ellipse
    ofSetColor(80, 200, 80);
    ofDrawEllipse(300, 110, 90, 50);

    // Triangle
    ofSetColor(255, 200, 80);
    ofDrawTriangle(380, 140, 460, 140, 420, 80);

    // ============== Custom Shape (Star) ==============
    ofSetColor(200, 100, 255);
    drawStar(550, 110, 40, 20, 5);

    // ============== Outline Shapes ==============
    ofNoFill();
    ofSetLineWidth(2);

    ofSetColor(255, 120, 120);
    ofDrawRectangle(50, 180, 80, 60);

    ofSetColor(120, 255, 120);
    ofDrawCircle(200, 210, 35);

    ofSetColor(120, 120, 255);
    ofDrawEllipse(300, 210, 90, 50);

    // ============== Bezier Curve ==============
    ofFill();
    ofSetColor(255, 200, 100);
    ofBeginShape();
    ofVertex(380, 250);
    ofBezierVertex(400, 180, 460, 180, 480, 250);
    ofEndShape();

    // ============== Catmull-Rom Curve ==============
    ofNoFill();
    ofSetLineWidth(3);
    ofSetColor(100, 200, 255);
    ofBeginShape();
    float curveOffset = sin(time * 2) * 20;
    ofCurveVertex(500, 200);  // Control point
    ofCurveVertex(520, 200 + curveOffset);
    ofCurveVertex(560, 240 - curveOffset);
    ofCurveVertex(600, 200 + curveOffset);
    ofCurveVertex(640, 240);  // Control point
    ofEndShape();

    // ============== Lines ==============
    ofSetLineWidth(2);
    ofSetColor(200, 200, 200);
    for (int i = 0; i < 5; i++) {
      float y = 280 + i * 12;
      ofDrawLine(50, y, 180, y);
    }

    // ============== Matrix Transformations ==============
    ofFill();

    // Rotating square
    ofPushMatrix();
    ofTranslate(120, 400);
    ofRotate(angle);
    ofSetColor(200, 100, 200);
    ofDrawRectangle(-30, -30, 60, 60);
    ofPopMatrix();

    // Nested transforms
    ofPushMatrix();
    ofTranslate(280, 400);
    ofRotate(angle * 0.5f);
    ofSetColor(100, 200, 200);
    ofDrawRectangle(-25, -25, 50, 50);

    // Child object orbiting around parent
    ofPushMatrix();
    ofTranslate(60, 0);
    ofRotate(angle * 2);
    ofSetColor(200, 200, 100);
    ofDrawRectangle(-12, -12, 24, 24);
    ofPopMatrix();

    ofPopMatrix();

    // Scaling circle
    ofPushMatrix();
    ofTranslate(420, 400);
    float scaleAmt = ofMap(sin(time * 2), -1, 1, 0.6f, 1.4f);
    ofScale(scaleAmt);
    ofSetColor(150, 150, 255);
    ofDrawCircle(0, 0, 25);
    ofPopMatrix();

    // ============== Vector Animation ==============
    ofVec2f center(560, 400);
    ofVec2f dir(40, 0);
    for (int i = 0; i < 8; i++) {
      ofVec2f rotated = dir.rotated(i * TWO_PI / 8.f + time);
      ofVec2f end = center + rotated;

      ofSetColor(255, 255, 255, 150);
      ofSetLineWidth(1);
      ofDrawLine(center.x, center.y, end.x, end.y);

      ofSetColor(255, 100, 100);
      ofDrawCircle(end.x, end.y, 4);
    }

    // ============== HSB Color Gradient ==============
    ofFill();
    for (int i = 0; i < 100; i++) {
      ofColor c = ofColor::fromHsb(i * 2.55f, 200, 255);
      ofSetColor(c);
      ofDrawRectangle(50 + i * 6, 480, 6, 20);
    }

    // ============== Keyboard & Mouse Info ==============
    ofSetColor(255);
    std::string info;
    info += "FPS: " + ofToString(ofGetFrameRate(), 1) + "\n";
    info += "Frame: " + ofToString(ofGetFrameNum()) + "\n";
    info += "Time: " + ofToString(ofGetElapsedTimef(), 2) + "s\n";
    info += "Window: " + ofToString(ofGetWidth()) + " x " + ofToString(ofGetHeight()) + "\n";
    info += "Mouse: " + ofToString(ofGetMouseX()) + ", " + ofToString(ofGetMouseY()) + "\n";

    // Show modifier keys
    std::string mods = "Modifiers: ";
    if (ofGetShiftPressed()) mods += "SHIFT ";
    if (ofGetCtrlPressed()) mods += "CTRL ";
    if (ofGetAltPressed()) mods += "ALT ";
    if (ofGetCmdPressed()) mods += "CMD ";
    if (mods == "Modifiers: ") mods += "none";
    info += mods + "\n";

    ofDrawBitmapString(info, 10, 20);

    // Section labels
    ofSetColor(180, 180, 180);
    ofDrawBitmapString("Filled Shapes", 50, 65);
    ofDrawBitmapString("Outline Shapes", 50, 165);
    ofDrawBitmapString("Curves", 380, 165);
    ofDrawBitmapString("Transforms", 100, 340);
    ofDrawBitmapString("Vectors", 540, 340);
    ofDrawBitmapString("HSB Gradient", 50, 520);

    // Instructions
    ofSetColor(200, 200, 200);
    ofDrawBitmapString("Press 'f' for fullscreen | ESC to quit", 10, ofGetHeight() - 20);
  }

  void keyPressed(int key) override {
    if (key == 'f' || key == 'F') {
      ofToggleFullscreen();
    }
  }

private:
  float angle{0.f};

  // Draw a star shape using ofBeginShape/ofVertex
  void drawStar(float x, float y, float outerRadius, float innerRadius, int points) {
    ofBeginShape();
    for (int i = 0; i < points * 2; i++) {
      float angle = i * PI / points - HALF_PI;
      float radius = (i % 2 == 0) ? outerRadius : innerRadius;
      ofVertex(x + cos(angle) * radius, y + sin(angle) * radius);
    }
    ofEndShape(true);
  }
};

OF_MAIN(BasicApp);
