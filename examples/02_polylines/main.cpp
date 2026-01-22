// Polylines Example - ported from openFrameworks
// Demonstrates: ofPolyline, curves, resampling, smoothing

#include "../../src/oflike/ofMain.h"
#include <vector>

class PolylinesApp : public AppBase {
public:
  ofPolyline drawnLine;
  ofPolyline smoothedLine;
  std::vector<ofPolyline> savedLines;
  bool isDrawing = false;
  int smoothingAmount = 4;

  void setup() override {
    ofBackground(30);
  }

  void update() override {
    // Continuously smooth the drawn line
    if (drawnLine.size() > 2) {
      smoothedLine = drawnLine.getSmoothed(smoothingAmount);
    }
  }

  void draw() override {
    // Draw saved lines
    for (size_t i = 0; i < savedLines.size(); i++) {
      ofColor c = ofColor::fromHsb((i * 40) % 255, 180, 255);
      ofSetColor(c);
      ofSetLineWidth(2);
      ofNoFill();
      savedLines[i].draw();
    }

    // Draw current line being drawn
    if (drawnLine.size() > 1) {
      // Original line (faint)
      ofSetColor(100, 100, 100);
      ofSetLineWidth(1);
      drawnLine.draw();

      // Smoothed line
      ofSetColor(255, 200, 100);
      ofSetLineWidth(3);
      smoothedLine.draw();

      // Draw points
      ofFill();
      ofSetColor(255, 100, 100);
      for (size_t i = 0; i < drawnLine.size(); i++) {
        ofDrawCircle(drawnLine[i].x, drawnLine[i].y, 3);
      }
    }

    // Draw animated point along saved lines
    float t = fmod(ofGetElapsedTimef() * 0.2f, 1.0f);
    for (const auto& line : savedLines) {
      if (line.size() > 1) {
        ofVec2f pos = line.getPointAtPercent(t);
        ofVec2f normal = line.getNormalAtPercent(t);

        // Draw point
        ofSetColor(255);
        ofDrawCircle(pos.x, pos.y, 6);

        // Draw normal
        ofSetColor(100, 255, 100);
        ofDrawLine(pos.x, pos.y, pos.x + normal.x * 20, pos.y + normal.y * 20);
      }
    }

    // UI
    ofSetColor(255);
    ofDrawBitmapString("Draw with mouse to create lines", 10, 20);
    ofDrawBitmapString("Smoothing: " + ofToString(smoothingAmount) + " (UP/DOWN to change)", 10, 35);
    ofDrawBitmapString("Press 'c' to clear, SPACE to save current line", 10, 50);
    ofDrawBitmapString("Lines saved: " + ofToString(savedLines.size()), 10, 65);

    // Draw a demo bezier curve
    ofSetColor(100, 200, 255);
    ofNoFill();
    ofSetLineWidth(2);

    ofPolyline bezier;
    float bx = 600, by = 400;
    bezier.addVertex(bx, by);
    bezier.bezierTo(bx + 100, by - 100, bx + 200, by + 100, bx + 300, by);

    bezier.draw();

    // Draw control points
    ofFill();
    ofSetColor(255, 100, 100);
    ofDrawCircle(bx, by, 5);
    ofDrawCircle(bx + 300, by, 5);
    ofSetColor(100, 255, 100);
    ofDrawCircle(bx + 100, by - 100, 5);
    ofDrawCircle(bx + 200, by + 100, 5);

    ofSetColor(150);
    ofDrawBitmapString("Bezier Curve Demo", bx, by - 20);
  }

  void mousePressed(float x, float y, int button) override {
    isDrawing = true;
    drawnLine.clear();
    drawnLine.addVertex(x, y);
  }

  void mouseDragged(float x, float y, int button) override {
    if (isDrawing) {
      // Only add if moved enough distance
      if (drawnLine.size() == 0 ||
          drawnLine[drawnLine.size() - 1].distance(ofVec2f(x, y)) > 5) {
        drawnLine.addVertex(x, y);
      }
    }
  }

  void mouseReleased(float x, float y, int button) override {
    isDrawing = false;
  }

  void keyPressed(int key) override {
    if (key == 'c' || key == 'C') {
      drawnLine.clear();
      smoothedLine.clear();
      savedLines.clear();
    }
    if (key == ' ') {
      if (smoothedLine.size() > 1) {
        savedLines.push_back(smoothedLine);
        drawnLine.clear();
        smoothedLine.clear();
      }
    }
    if (key == OF_KEY_UP) {
      smoothingAmount = std::min(20, smoothingAmount + 1);
    }
    if (key == OF_KEY_DOWN) {
      smoothingAmount = std::max(0, smoothingAmount - 1);
    }
  }
};

OF_MAIN(PolylinesApp);
