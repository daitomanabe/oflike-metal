// linesExample - Using ofMesh for line drawing and ofTrueTypeFont for text outlines
// Demonstrates ofMesh with LINE_STRIP and LINE_LOOP modes
// Font rendering uses Core Text (NOT FreeType) - see ARCHITECTURE.md

#include "../../src/oflike/ofMain.h"

class LinesExampleApp : public AppBase {
public:
  void setup() override {
    ofBackground(240, 240, 240);
    ofSetCircleResolution(100);
    lineWidth = 2.0f;
    bUseLineColors = false;

    // Load font for text outlines (using Core Text, NOT FreeType)
    // Load with makeContours=true to enable getStringAsPoints()
    font.load("Helvetica", 48, true, true, true);  // makeContours=true
    textString = "oflike";
  }

  ofTrueTypeFont font;
  std::string textString;

  void update() override {
    ofSetLineWidth(lineWidth);
  }

  void draw() override {
    // Draw gradient background
    ofBackgroundGradient(ofFloatColor(1.0), ofFloatColor(0.8));

    float elapsedTime = ofGetElapsedTimef();

    // Info text
    ofSetColor(60);
    std::stringstream ss;
    ss << "Line width (+/-): " << ofToString(lineWidth, 1);
    ss << std::endl << "Line colors (c): " << (bUseLineColors ? "yes" : "no");
    ofDrawBitmapString(ss.str(), 24, 36);

    // Draw outline rectangle using ofPath
    ofPushStyle();
    ofSetLineWidth(std::max(1.f, lineWidth / 4.f));
    float rectRadius = 10.f + (1.f - std::abs(std::sin(elapsedTime))) * 80.f;
    ofRectangle outlineRect(200, 150, ofGetWidth() - 400, ofGetHeight() - 300);

    ofPath testRectPath;
    testRectPath.setCurveResolution(16);
    testRectPath.setCircleResolution(120);
    testRectPath.rectRounded(outlineRect, rectRadius);
    testRectPath.setFilled(false);
    testRectPath.setFillColor(ofColor::lightBlue());
    testRectPath.setStrokeWidth(2.0f);
    testRectPath.setStrokeColor(ofColor(60));
    testRectPath.draw();
    ofPopStyle();

    // Main mesh drawing in center
    ofPushMatrix();
    ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);

    int numPts = 150;
    float height = 200.0f;

    // Create line mesh using LINE_STRIP mode
    ofMesh lineMesh;
    lineMesh.setMode(OF_PRIMITIVE_LINE_STRIP);

    ofFloatColor lineColor;
    for (int i = 0; i < numPts; i++) {
      float fi = static_cast<float>(i);
      float fpct = fi / static_cast<float>(numPts);

      // Calculate 3D position (z will be ignored in 2D rendering)
      glm::vec3 pt(
        std::cos(fi * 0.15f + elapsedTime) * (fpct + 0.1f) * height * 0.5f,
        std::sin(fi * 0.08f + elapsedTime) * height * 0.5f + height * (fpct * 2.0f - 1.0f) * 0.5f,
        std::cos(fi * -0.05f + elapsedTime) * height * 0.25f
      );
      lineMesh.addVertex(pt);

      if (bUseLineColors) {
        lineColor.setHsb(fpct, 0.8f, 0.85f);
        lineMesh.addColor(lineColor);
      }
    }

    // Set color if not using per-vertex colors
    if (!bUseLineColors) {
      lineMesh.disableColors();
      ofSetColor(60);
    }

    lineMesh.draw();

    // Create box mesh using LINE_LOOP mode
    ofMesh boxMesh;
    boxMesh.setMode(OF_PRIMITIVE_LINE_LOOP);

    float bsize = height * 0.8f;
    boxMesh.addVertex(glm::vec3(-bsize, -bsize, 0.f));
    boxMesh.addVertex(glm::vec3(bsize, -bsize, 0.f));
    boxMesh.addVertex(glm::vec3(bsize, bsize, 0.f));
    boxMesh.addVertex(glm::vec3(-bsize, bsize, 0.f));

    if (!bUseLineColors) {
      // Add per-vertex colors to box mesh
      boxMesh.addColor(ofFloatColor(ofColor::magenta()));
      boxMesh.addColor(ofFloatColor(ofColor::yellow()));
      boxMesh.addColor(ofFloatColor(ofColor::green()));
      boxMesh.addColor(ofFloatColor(ofColor::blueSteel()));
    } else {
      boxMesh.disableColors();
    }

    boxMesh.addIndex(0);
    boxMesh.addIndex(1);
    boxMesh.addIndex(2);
    boxMesh.addIndex(3);

    ofPushMatrix();
    ofTranslate(0.0f, height * 0.5f * std::sin(elapsedTime));
    // Note: X rotation would need 3D support
    // ofRotateXRad(glm::half_pi<float>());
    boxMesh.draw();
    ofPopMatrix();

    ofPopMatrix();

    // Draw decorative radial lines on the sides
    ofPushMatrix();
    ofTranslate(100, ofGetHeight() / 2);

    for (int i = 0; i < 20; i++) {
      float angle = i * TWO_PI / 20.0f + elapsedTime * 0.5f;
      float len = 60 + 20 * std::sin(elapsedTime + i * 0.3f);
      ofSetColor(ofColor::fromHsb(i * 12, 200, 200));
      ofDrawLine(0, 0, std::cos(angle) * len, std::sin(angle) * len);
    }

    ofPopMatrix();

    ofPushMatrix();
    ofTranslate(ofGetWidth() - 100, ofGetHeight() / 2);

    for (int i = 0; i < 20; i++) {
      float angle = i * TWO_PI / 20.0f - elapsedTime * 0.5f;
      float len = 60 + 20 * std::sin(elapsedTime + i * 0.3f);
      ofSetColor(ofColor::fromHsb(255 - i * 12, 200, 200));
      ofDrawLine(0, 0, std::cos(angle) * len, std::sin(angle) * len);
    }

    ofPopMatrix();

    // Draw circles at the bottom with outlines
    ofPushStyle();
    ofNoFill();
    ofSetColor(ofColor::seaGreen());

    ofPushMatrix();
    ofTranslate(ofGetWidth() / 2, ofGetHeight() - 80);
    ofDrawCircle(-120, 0, 40);
    ofDrawCircle(0, 0, 50);
    ofDrawCircle(120, 0, 40);
    ofPopMatrix();

    ofPopStyle();

    // Animated grid lines at top
    ofSetColor(200, 200, 255);
    for (int i = 0; i < 10; i++) {
      float offset = std::sin(elapsedTime + i * 0.5f) * 20;
      float y = 100 + i * 15 + offset;
      ofDrawLine(50, y, 200, y);
      ofDrawLine(ofGetWidth() - 200, y, ofGetWidth() - 50, y);
    }

    // Draw text as vector outlines using ofTrueTypeFont (Core Text based)
    if (font.isLoaded()) {
      ofPushMatrix();
      ofRectangle fontRect = font.getStringBoundingBox(textString, 0.0f, 0.0f);
      float textX = (ofGetWidth() - fontRect.getWidth()) / 2;
      float textY = ofGetHeight() - 150;
      ofTranslate(textX, textY);

      // Get text as vector paths
      std::vector<ofPath> textPaths = font.getStringAsPoints(textString, true, false);

      // Draw each character path with animated colors
      for (size_t i = 0; i < textPaths.size(); i++) {
        ofPath& fp = textPaths[i];

        // Animated fill color
        float hue = std::fmod(elapsedTime * 0.1f + i * 0.1f, 1.0f);
        ofFloatColor fillColor;
        fillColor.setHsb(hue, 0.6f, 0.9f);

        fp.setFilled(true);
        fp.setFillColor(fillColor);
        fp.setStrokeWidth(lineWidth);
        fp.setStrokeColor(ofColor(60));
        fp.draw();
      }

      ofPopMatrix();
    }
  }

  void keyPressed(int key) override {
    if (key == '+' || key == '=') {
      lineWidth += 1.f;
    }
    if (key == '-' || key == '_') {
      lineWidth -= 1.f;
      if (lineWidth < 0.5f) lineWidth = 0.5f;
    }
    if (key == 'c') {
      bUseLineColors = !bUseLineColors;
    }
  }

private:
  float lineWidth;
  bool bUseLineColors;
};

OF_MAIN(LinesExampleApp);
