// Ported from openFrameworks polygonExample
// Demonstrates various polygon drawing techniques

#include "../../src/oflike/ofMain.h"

class PolygonExampleApp : public AppBase {
public:
  void setup() override {
    ofBackground(255, 255, 255);

    nCurveVertices = 7;

    curveVertices[0] = {326, 209};
    curveVertices[1] = {306, 279};
    curveVertices[2] = {265, 331};
    curveVertices[3] = {304, 383};
    curveVertices[4] = {374, 383};
    curveVertices[5] = {418, 309};
    curveVertices[6] = {345, 279};

    for (int i = 0; i < nCurveVertices; i++) {
      curveVerticesOver[i] = false;
      curveVerticesDragged[i] = false;
    }
  }

  void update() override {
  }

  void draw() override {
    int mouseX = ofGetMouseX();
    int mouseY = ofGetMouseY();

    ofFill();
    ofSetHexColor(0xe0be21);

    //------(a) Star with winding rule odd
    ofSetPolyMode(OF_POLY_WINDING_ODD);
    ofBeginShape();
    ofVertex(200, 135);
    ofVertex(15, 135);
    ofVertex(165, 25);
    ofVertex(105, 200);
    ofVertex(50, 25);
    ofEndShape();

    //------(b) Star with winding rule nonzero
    ofSetHexColor(0xb5de10);
    ofSetPolyMode(OF_POLY_WINDING_NONZERO);
    ofBeginShape();
    ofVertex(400, 135);
    ofVertex(215, 135);
    ofVertex(365, 25);
    ofVertex(305, 200);
    ofVertex(250, 25);
    ofEndShape();

    //------(c) Dynamic star based on mouse position
    float xPct = (float)(mouseX) / (float)(ofGetWidth());
    float yPct = (float)(mouseY) / (float)(ofGetHeight());
    int nTips = 5 + xPct * 60;
    int nStarPts = nTips * 2;
    float angleChangePerPt = TWO_PI / (float)nStarPts;
    float innerRadius = 0 + yPct * 80;
    float outerRadius = 80;
    float origx = 525;
    float origy = 100;
    float angle = 0;

    ofSetHexColor(0xa16bca);
    ofBeginShape();
    for (int i = 0; i < nStarPts; i++) {
      if (i % 2 == 0) {
        float x = origx + innerRadius * std::cos(angle);
        float y = origy + innerRadius * std::sin(angle);
        ofVertex(x, y);
      } else {
        float x = origx + outerRadius * std::cos(angle);
        float y = origy + outerRadius * std::sin(angle);
        ofVertex(x, y);
      }
      angle += angleChangePerPt;
    }
    ofEndShape();

    //------(d) Random polygon
    ofSetHexColor(0x0cb0b6);
    ofSetPolyMode(OF_POLY_WINDING_ODD);
    ofBeginShape();
    for (int i = 0; i < 10; i++) {
      ofVertex(ofRandom(650, 850), ofRandom(20, 200));
    }
    ofEndShape();

    //------(e) Spiral shape using sin/cos
    ofPushMatrix();
    ofTranslate(100, 300);
    ofSetHexColor(0xff2220);
    ofFill();
    ofSetPolyMode(OF_POLY_WINDING_ODD);
    ofBeginShape();
    float angleStep = TWO_PI / (100.0f + sin(ofGetElapsedTimef() / 5.0f) * 60);
    float radiusAdder = 0.5f;
    float radius = 0;
    for (int i = 0; i < 200; i++) {
      float anglef = (i)*angleStep;
      float x = radius * std::cos(anglef);
      float y = radius * std::sin(anglef);
      ofVertex(x, y);
      radius += radiusAdder;
    }
    ofEndShape(true);
    ofPopMatrix();

    //------(f) Curve vertices (Catmull-Rom spline)
    ofSetHexColor(0x2bdbe6);
    ofBeginShape();
    for (int i = 0; i < nCurveVertices; i++) {
      if (i == 0) {
        ofCurveVertex(curveVertices[0].x, curveVertices[0].y);
        ofCurveVertex(curveVertices[0].x, curveVertices[0].y);
      } else if (i == nCurveVertices - 1) {
        ofCurveVertex(curveVertices[i].x, curveVertices[i].y);
        ofCurveVertex(curveVertices[0].x, curveVertices[0].y);
        ofCurveVertex(curveVertices[0].x, curveVertices[0].y);
      } else {
        ofCurveVertex(curveVertices[i].x, curveVertices[i].y);
      }
    }
    ofEndShape();

    // Show non-curve version faintly
    ofEnableAlphaBlending();
    ofNoFill();
    ofSetColor(0, 0, 0, 40);
    ofBeginShape();
    for (int i = 0; i < nCurveVertices; i++) {
      ofVertex(curveVertices[i].x, curveVertices[i].y);
    }
    ofEndShape(true);

    // Draw control points
    ofSetColor(0, 0, 0, 80);
    for (int i = 0; i < nCurveVertices; i++) {
      if (curveVerticesOver[i]) ofFill();
      else ofNoFill();
      ofDrawCircle(curveVertices[i].x, curveVertices[i].y, 4);
    }
    ofDisableAlphaBlending();

    //------(g) Bezier curve
    float x0 = 500;
    float y0 = 300;
    float x1 = 550 + 50 * cos(ofGetElapsedTimef() * 1.0f);
    float y1 = 300 + 100 * sin(ofGetElapsedTimef() / 3.5f);
    float x2 = 600 + 30 * cos(ofGetElapsedTimef() * 2.0f);
    float y2 = 300 + 100 * sin(ofGetElapsedTimef());
    float x3 = 650;
    float y3 = 300;

    ofFill();
    ofSetHexColor(0xFF9933);
    ofBeginShape();
    ofVertex(x0, y0);
    ofBezierVertex(x1, y1, x2, y2, x3, y3);
    ofEndShape();

    ofEnableAlphaBlending();
    ofFill();
    ofSetColor(0, 0, 0, 40);
    ofDrawCircle(x0, y0, 4);
    ofDrawCircle(x1, y1, 4);
    ofDrawCircle(x2, y2, 4);
    ofDrawCircle(x3, y3, 4);
    ofDisableAlphaBlending();

    //------(h) Holes using ofNextContour
    ofFill();
    ofSetHexColor(0xd3ffd3);
    ofDrawRectangle(80, 480, 140, 70);
    ofSetHexColor(0xff00ff);

    ofBeginShape();
    ofVertex(100, 500);
    ofVertex(180, 550);
    ofVertex(100, 600);

    ofNextContour(true);

    ofVertex(120, 520);
    ofVertex(160, 550);
    ofVertex(120, 580);

    ofEndShape(true);

    // Labels
    ofSetHexColor(0x000000);
    ofDrawBitmapString("(a) star\nwinding rule odd", 20, 210);
    ofDrawBitmapString("(b) star\nwinding rule nonzero", 220, 210);
    ofDrawBitmapString("(c) dynamically\ncreated shape", 420, 210);
    ofDrawBitmapString("(d) random points\npoly", 670, 210);
    ofDrawBitmapString("(e) fun with sin/cos", 20, 410);
    ofDrawBitmapString("(f) ofCurveVertex\nuses catmull rom", 220, 410);
    ofDrawBitmapString("(g) ofBezierVertex\nuses bezier to draw curves", 460, 410);
    ofDrawBitmapString("(h) ofNextContour\nallows for holes", 20, 610);

    // FPS
    ofDrawBitmapString("FPS: " + ofToString(ofGetFrameRate(), 1), 10, 20);
    ofDrawBitmapString("Drag the curve points (f) to modify the shape", 10, 35);
  }

  void mouseMoved(float x, float y) override {
    for (int i = 0; i < nCurveVertices; i++) {
      float diffx = x - curveVertices[i].x;
      float diffy = y - curveVertices[i].y;
      float dist = sqrt(diffx * diffx + diffy * diffy);
      curveVerticesOver[i] = (dist < 8);
    }
  }

  void mouseDragged(float x, float y, int button) override {
    for (int i = 0; i < nCurveVertices; i++) {
      if (curveVerticesDragged[i]) {
        curveVertices[i].x = x;
        curveVertices[i].y = y;
      }
    }
  }

  void mousePressed(float x, float y, int button) override {
    for (int i = 0; i < nCurveVertices; i++) {
      float diffx = x - curveVertices[i].x;
      float diffy = y - curveVertices[i].y;
      float dist = sqrt(diffx * diffx + diffy * diffy);
      curveVerticesDragged[i] = (dist < 8);
    }
  }

  void mouseReleased(float x, float y, int button) override {
    for (int i = 0; i < nCurveVertices; i++) {
      curveVerticesDragged[i] = false;
    }
  }

private:
  static constexpr int MAX_CURVE_VERTICES = 10;
  int nCurveVertices;
  ofVec2f curveVertices[MAX_CURVE_VERTICES];
  bool curveVerticesOver[MAX_CURVE_VERTICES];
  bool curveVerticesDragged[MAX_CURVE_VERTICES];
};

OF_MAIN(PolygonExampleApp);
