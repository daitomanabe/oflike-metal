#pragma once

#include <string>
#include <sstream>
#include <cmath>
#include "../../core/Context.h"
#include "ofColor.h"
#include "../math/ofVec2f.h"
#include "../math/ofVec3f.h"
#include "../types/ofRectangle.h"

// ============== Primitive Mode ==============

enum ofPrimitiveMode {
  OF_PRIMITIVE_TRIANGLES,
  OF_PRIMITIVE_TRIANGLE_STRIP,
  OF_PRIMITIVE_TRIANGLE_FAN,
  OF_PRIMITIVE_LINES,
  OF_PRIMITIVE_LINE_STRIP,
  OF_PRIMITIVE_LINE_LOOP,
  OF_PRIMITIVE_POINTS
};

// ============== Gradient Mode ==============

enum ofGradientMode {
  OF_GRADIENT_LINEAR,
  OF_GRADIENT_CIRCULAR,
  OF_GRADIENT_BAR
};

// ============== Background & Color ==============

inline void ofBackground(int r, int g, int b, int a = 255) {
  oflike::engine().setClearColor(oflike::Color4f{r / 255.f, g / 255.f, b / 255.f, a / 255.f});
}

inline void ofBackground(int gray) {
  ofBackground(gray, gray, gray);
}

inline void ofBackground(int gray, int a) {
  ofBackground(gray, gray, gray, a);
}

inline void ofBackground(const ofColor& c) {
  oflike::engine().setClearColor(oflike::Color4f{c.r / 255.f, c.g / 255.f, c.b / 255.f, c.a / 255.f});
}

inline void ofBackground(const ofFloatColor& c) {
  oflike::engine().setClearColor(oflike::Color4f{c.r, c.g, c.b, c.a});
}

inline void ofBackgroundHex(int hexColor, int a = 255) {
  int r = (hexColor >> 16) & 0xFF;
  int g = (hexColor >> 8) & 0xFF;
  int b = hexColor & 0xFF;
  ofBackground(r, g, b, a);
}

// Gradient background
// For CIRCULAR mode: draws a radial gradient from center
// For LINEAR mode: draws a vertical gradient
// For BAR mode: draws horizontal bars
inline void ofBackgroundGradient(const ofColor& start, const ofColor& end,
                                  ofGradientMode mode = OF_GRADIENT_CIRCULAR) {
  // Save current state
  auto& dl = oflike::engine().drawList();
  bool wasFilled = dl.isFilled();
  auto prevColor = oflike::engine().getStyle().color;

  dl.setFilled(true);

  float w = static_cast<float>(oflike::engine().getWindowWidth());
  float h = static_cast<float>(oflike::engine().getWindowHeight());

  if (mode == OF_GRADIENT_LINEAR || mode == OF_GRADIENT_CIRCULAR) {
    // Draw a full-screen quad with vertex colors for gradient
    // For now, use a simple approach: draw multiple horizontal bands
    int numBands = 64;
    float bandHeight = h / numBands;

    for (int i = 0; i < numBands; i++) {
      float t = static_cast<float>(i) / (numBands - 1);
      if (mode == OF_GRADIENT_CIRCULAR) {
        // Radial gradient approximation
        float dist = std::abs(t - 0.5f) * 2.0f;
        t = dist * dist;
      }
      ofColor c = start.getLerped(end, t);
      dl.setColor(oflike::Color4f{c.r / 255.f, c.g / 255.f, c.b / 255.f, c.a / 255.f});
      dl.addRect(0, i * bandHeight, w, bandHeight + 1);
    }
  } else {
    // BAR mode: horizontal gradient
    int numBands = 64;
    float bandWidth = w / numBands;

    for (int i = 0; i < numBands; i++) {
      float t = static_cast<float>(i) / (numBands - 1);
      ofColor c = start.getLerped(end, t);
      dl.setColor(oflike::Color4f{c.r / 255.f, c.g / 255.f, c.b / 255.f, c.a / 255.f});
      dl.addRect(i * bandWidth, 0, bandWidth + 1, h);
    }
  }

  // Restore state
  dl.setFilled(wasFilled);
  dl.setColor(prevColor);
}

inline void ofBackgroundGradient(const ofFloatColor& start, const ofFloatColor& end,
                                  ofGradientMode mode = OF_GRADIENT_CIRCULAR) {
  ofBackgroundGradient(
    ofColor(start.r * 255, start.g * 255, start.b * 255, start.a * 255),
    ofColor(end.r * 255, end.g * 255, end.b * 255, end.a * 255),
    mode
  );
}

inline void ofSetColor(int r, int g, int b, int a = 255) {
  oflike::engine().drawList().setColor(oflike::Color4f{r / 255.f, g / 255.f, b / 255.f, a / 255.f});
}

inline void ofSetColor(int gray, int a = 255) {
  ofSetColor(gray, gray, gray, a);
}

inline void ofSetColor(const oflike::Color4f& c) {
  oflike::engine().drawList().setColor(c);
}

inline void ofSetColor(const ofColor& c) {
  oflike::engine().drawList().setColor(oflike::Color4f{c.r / 255.f, c.g / 255.f, c.b / 255.f, c.a / 255.f});
}

inline void ofSetColor(const ofFloatColor& c) {
  oflike::engine().drawList().setColor(oflike::Color4f{c.r, c.g, c.b, c.a});
}

// ============== Fill Mode ==============

inline void ofFill() {
  oflike::engine().drawList().setFilled(true);
}

inline void ofNoFill() {
  oflike::engine().drawList().setFilled(false);
}

// ============== Line Width ==============

inline void ofSetLineWidth(float width) {
  oflike::engine().drawList().setLineWidth(width);
}

inline float ofGetLineWidth() {
  return oflike::engine().drawList().getLineWidth();
}

// ============== Rectangle ==============

inline void ofDrawRectangle(float x, float y, float w, float h) {
  oflike::engine().drawList().addRect(x, y, w, h);
}

inline void ofRect(float x, float y, float w, float h) {
  ofDrawRectangle(x, y, w, h);
}

inline void ofDrawRectangle(const ofVec2f& p, float w, float h) {
  ofDrawRectangle(p.x, p.y, w, h);
}

inline void ofDrawRectangle(float x, float y, float z, float w, float h) {
  (void)z;  // 2D rendering ignores z
  ofDrawRectangle(x, y, w, h);
}

inline void ofDrawRectangle(const ofRectangle& r) {
  ofDrawRectangle(r.x, r.y, r.width, r.height);
}

// ============== Rounded Rectangle ==============

inline void ofDrawRectRounded(float x, float y, float w, float h, float r) {
  oflike::engine().drawList().addRectRounded(x, y, w, h, r);
}

inline void ofDrawRectRounded(float x, float y, float w, float h,
                              float topLeftRadius, float topRightRadius,
                              float bottomRightRadius, float bottomLeftRadius) {
  oflike::engine().drawList().addRectRounded(x, y, w, h,
                                              topLeftRadius, topRightRadius,
                                              bottomRightRadius, bottomLeftRadius);
}

inline void ofRectRounded(float x, float y, float w, float h, float r) {
  ofDrawRectRounded(x, y, w, h, r);
}

inline void ofRectRounded(float x, float y, float w, float h,
                          float topLeftRadius, float topRightRadius,
                          float bottomRightRadius, float bottomLeftRadius) {
  ofDrawRectRounded(x, y, w, h, topLeftRadius, topRightRadius,
                    bottomRightRadius, bottomLeftRadius);
}

inline void ofDrawRectRounded(const ofVec2f& p, float w, float h, float r) {
  ofDrawRectRounded(p.x, p.y, w, h, r);
}

inline void ofDrawRectRounded(const ofVec2f& p, float w, float h,
                              float topLeftRadius, float topRightRadius,
                              float bottomRightRadius, float bottomLeftRadius) {
  ofDrawRectRounded(p.x, p.y, w, h, topLeftRadius, topRightRadius,
                    bottomRightRadius, bottomLeftRadius);
}

// ============== Circle & Ellipse ==============

inline void ofDrawCircle(float x, float y, float radius) {
  int res = oflike::engine().getCircleResolution();
  oflike::engine().drawList().addCircle(x, y, radius, res);
}

inline void ofCircle(float x, float y, float radius) {
  ofDrawCircle(x, y, radius);
}

inline void ofDrawCircle(float x, float y, float z, float radius) {
  (void)z;  // 2D rendering ignores z
  ofDrawCircle(x, y, radius);
}

inline void ofDrawCircle(const ofVec2f& p, float radius) {
  ofDrawCircle(p.x, p.y, radius);
}

inline void ofDrawEllipse(float x, float y, float width, float height) {
  int res = oflike::engine().getCircleResolution();
  oflike::engine().drawList().addEllipse(x, y, width * 0.5f, height * 0.5f, res);
}

inline void ofEllipse(float x, float y, float width, float height) {
  ofDrawEllipse(x, y, width, height);
}

inline void ofDrawEllipse(float x, float y, float z, float width, float height) {
  (void)z;  // 2D rendering ignores z
  ofDrawEllipse(x, y, width, height);
}

inline void ofDrawEllipse(const ofVec2f& p, float width, float height) {
  ofDrawEllipse(p.x, p.y, width, height);
}

// ============== Triangle ==============

inline void ofDrawTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
  oflike::engine().drawList().addTriangle(x1, y1, x2, y2, x3, y3);
}

inline void ofTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
  ofDrawTriangle(x1, y1, x2, y2, x3, y3);
}

inline void ofDrawTriangle(const ofVec2f& p1, const ofVec2f& p2, const ofVec2f& p3) {
  ofDrawTriangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
}

// ============== Line ==============

inline void ofDrawLine(float x1, float y1, float x2, float y2) {
  oflike::engine().drawList().addLine(x1, y1, x2, y2);
}

inline void ofLine(float x1, float y1, float x2, float y2) {
  ofDrawLine(x1, y1, x2, y2);
}

inline void ofDrawLine(const ofVec2f& p1, const ofVec2f& p2) {
  ofDrawLine(p1.x, p1.y, p2.x, p2.y);
}

// ============== Custom Shapes ==============

inline void ofBeginShape() {
  oflike::engine().drawList().beginShape();
}

inline void ofEndShape(bool close = false) {
  oflike::engine().drawList().endShape(close);
}

inline void ofVertex(float x, float y) {
  oflike::engine().drawList().vertex(x, y);
}

inline void ofVertex(float x, float y, float z) {
  (void)z;  // 2D rendering ignores z
  oflike::engine().drawList().vertex(x, y);
}

inline void ofVertex(const ofVec2f& p) {
  oflike::engine().drawList().vertex(p.x, p.y);
}

inline void ofVertex(const ofVec3f& p) {
  oflike::engine().drawList().vertex(p.x, p.y);
}

inline void ofVertex(float x, float y, float u, float v) {
  oflike::engine().drawList().vertex(x, y, u, v);
}

inline void ofCurveVertex(float x, float y) {
  oflike::engine().drawList().curveVertex(x, y);
}

inline void ofCurveVertex(const ofVec2f& p) {
  oflike::engine().drawList().curveVertex(p.x, p.y);
}

inline void ofBezierVertex(float cx1, float cy1, float cx2, float cy2, float x, float y) {
  oflike::engine().drawList().bezierVertex(cx1, cy1, cx2, cy2, x, y);
}

inline void ofBezierVertex(const ofVec2f& cp1, const ofVec2f& cp2, const ofVec2f& p) {
  oflike::engine().drawList().bezierVertex(cp1.x, cp1.y, cp2.x, cp2.y, p.x, p.y);
}

inline void ofNextContour(bool close = true) {
  oflike::engine().drawList().nextContour(close);
}

// Standalone Bezier curve drawing (from p0 through p1, p2 as control points, to p3)
inline void ofDrawBezier(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3) {
  ofBeginShape();
  ofVertex(x0, y0);
  ofBezierVertex(x1, y1, x2, y2, x3, y3);
  ofEndShape();
}

// Standalone Catmull-Rom curve drawing
inline void ofDrawCurve(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3) {
  ofBeginShape();
  ofCurveVertex(x0, y0);
  ofCurveVertex(x1, y1);
  ofCurveVertex(x2, y2);
  ofCurveVertex(x3, y3);
  ofEndShape();
}

// ============== Matrix Transformations ==============

inline void ofPushMatrix() {
  oflike::engine().drawList().pushMatrix();
}

inline void ofPopMatrix() {
  oflike::engine().drawList().popMatrix();
}

inline void ofTranslate(float x, float y) {
  oflike::engine().drawList().translate(x, y);
}

inline void ofTranslate(float x, float y, float z) {
  oflike::engine().drawList().translate(x, y, z);
}

inline void ofTranslate(const ofVec2f& p) {
  oflike::engine().drawList().translate(p.x, p.y);
}

inline void ofTranslate(const ofVec3f& p) {
  oflike::engine().drawList().translate(p.x, p.y, p.z);
}

inline void ofRotate(float degrees) {
  oflike::engine().drawList().rotate(degrees * 3.14159265358979f / 180.f);
}

inline void ofRotateRad(float radians) {
  oflike::engine().drawList().rotate(radians);
}

inline void ofRotateDeg(float degrees) {
  ofRotate(degrees);
}

inline void ofRotateZDeg(float degrees) {
  ofRotate(degrees);
}

inline void ofRotateZRad(float radians) {
  ofRotateRad(radians);
}

// Note: X/Y axis rotations are 3D operations.
// In 2D mode, these have limited effect.
// Full 3D support will be added with ofMesh implementation.
inline void ofRotateXDeg(float degrees) {
  // TODO: Implement 3D X-axis rotation
  (void)degrees;
}

inline void ofRotateXRad(float radians) {
  // TODO: Implement 3D X-axis rotation
  (void)radians;
}

inline void ofRotateYDeg(float degrees) {
  // TODO: Implement 3D Y-axis rotation
  (void)degrees;
}

inline void ofRotateYRad(float radians) {
  // TODO: Implement 3D Y-axis rotation
  (void)radians;
}

inline void ofRotateDeg(float degrees, float vx, float vy, float vz) {
  // For 2D: only Z rotation is meaningful
  if (vz != 0.0f) {
    ofRotate(degrees * (vz > 0 ? 1.0f : -1.0f));
  }
  (void)vx; (void)vy;
}

inline void ofRotateRad(float radians, float vx, float vy, float vz) {
  // For 2D: only Z rotation is meaningful
  if (vz != 0.0f) {
    ofRotateRad(radians * (vz > 0 ? 1.0f : -1.0f));
  }
  (void)vx; (void)vy;
}

inline void ofLoadIdentityMatrix() {
  oflike::engine().drawList().resetMatrix();
}

inline void ofScale(float s) {
  oflike::engine().drawList().scale(s, s);
}

inline void ofScale(float sx, float sy) {
  oflike::engine().drawList().scale(sx, sy);
}

// ============== Text ==============

inline void ofDrawBitmapString(const std::string& text, float x, float y) {
  oflike::engine().drawList().addText(text, x, y);
}

template<typename T>
inline void ofDrawBitmapString(const T& value, float x, float y) {
  std::ostringstream oss;
  oss << value;
  ofDrawBitmapString(oss.str(), x, y);
}

// ============== Window Info ==============

inline int ofGetWidth() {
  return oflike::engine().getWindowWidth();
}

inline int ofGetHeight() {
  return oflike::engine().getWindowHeight();
}

inline float ofGetFrameRate() {
  return oflike::engine().getFrameRate();
}

inline float ofGetElapsedTimef() {
  return static_cast<float>(oflike::engine().elapsedTimeSeconds());
}

inline int ofGetFrameNum() {
  return oflike::engine().getFrameNum();
}

// ============== Mouse ==============

inline int ofGetMouseX() {
  return oflike::engine().getMouseX();
}

inline int ofGetMouseY() {
  return oflike::engine().getMouseY();
}

inline bool ofGetMousePressed(int button = 0) {
  return oflike::engine().getMousePressed(button);
}

// ============== Blending ==============

inline void ofEnableAlphaBlending() {
  oflike::engine().setBlendMode(oflike::BlendMode::Alpha);
}

inline void ofDisableAlphaBlending() {
  oflike::engine().setBlendMode(oflike::BlendMode::Disabled);
}

enum ofBlendMode {
  OF_BLENDMODE_DISABLED = 0,
  OF_BLENDMODE_ALPHA = 1,
  OF_BLENDMODE_ADD = 2,
  OF_BLENDMODE_SUBTRACT = 3,
  OF_BLENDMODE_MULTIPLY = 4,
  OF_BLENDMODE_SCREEN = 5
};

inline void ofEnableBlendMode(ofBlendMode mode) {
  oflike::engine().setBlendMode(static_cast<oflike::BlendMode>(mode));
}

inline void ofDisableBlendMode() {
  oflike::engine().setBlendMode(oflike::BlendMode::Disabled);
}

// ============== Depth Test ==============

inline void ofEnableDepthTest() {
  oflike::engine().setDepthTest(true);
}

inline void ofDisableDepthTest() {
  oflike::engine().setDepthTest(false);
}

inline void ofSetDepthTest(bool enabled) {
  oflike::engine().setDepthTest(enabled);
}

// ============== Smoothing ==============

inline void ofEnableSmoothing() {
  oflike::engine().setSmoothing(true);
}

inline void ofDisableSmoothing() {
  oflike::engine().setSmoothing(false);
}

inline void ofEnableAntiAliasing() {
  oflike::engine().setSmoothing(true);
}

inline void ofDisableAntiAliasing() {
  oflike::engine().setSmoothing(false);
}

// ============== Style Management ==============

inline void ofPushStyle() {
  oflike::engine().pushStyle();
}

inline void ofPopStyle() {
  oflike::engine().popStyle();
}

inline oflike::Style ofGetStyle() {
  return oflike::engine().getStyle();
}

inline void ofSetStyle(const oflike::Style& style) {
  oflike::engine().setStyle(style);
}

// ============== Rectangle Mode ==============

enum ofRectMode {
  OF_RECTMODE_CORNER = 0,
  OF_RECTMODE_CENTER = 1
};

inline void ofSetRectMode(ofRectMode mode) {
  oflike::engine().setRectMode(static_cast<oflike::RectMode>(mode));
}

inline ofRectMode ofGetRectMode() {
  return static_cast<ofRectMode>(oflike::engine().getRectMode());
}

// ============== Resolution Settings ==============

inline void ofSetCircleResolution(int resolution) {
  oflike::engine().setCircleResolution(resolution);
}

inline void ofSetCurveResolution(int resolution) {
  oflike::engine().setCurveResolution(resolution);
}

// ============== Point Size ==============

inline void ofSetPointSize(float size) {
  oflike::engine().setPointSize(size);
}

// ============== Polygon Winding Mode ==============

enum ofPolyWindingMode {
  OF_POLY_WINDING_ODD = 0,
  OF_POLY_WINDING_NONZERO = 1,
  OF_POLY_WINDING_POSITIVE = 2,
  OF_POLY_WINDING_NEGATIVE = 3,
  OF_POLY_WINDING_ABS_GEQ_TWO = 4
};

inline void ofSetPolyMode(ofPolyWindingMode mode) {
  oflike::engine().setPolyMode(static_cast<oflike::PolyWindingMode>(mode));
}

// ============== Clear ==============

inline void ofClear(float r, float g, float b, float a = 255.f) {
  oflike::engine().setBackgroundColor(oflike::Color4f{r / 255.f, g / 255.f, b / 255.f, a / 255.f});
}

inline void ofClear(float gray, float a = 255.f) {
  ofClear(gray, gray, gray, a);
}

inline void ofClear(const ofColor& c) {
  oflike::engine().setBackgroundColor(oflike::Color4f{c.r / 255.f, c.g / 255.f, c.b / 255.f, c.a / 255.f});
}

inline void ofClear(const ofFloatColor& c) {
  oflike::engine().setBackgroundColor(oflike::Color4f{c.r, c.g, c.b, c.a});
}

// ============== Background Color Getters/Setters ==============

inline ofFloatColor ofGetBackgroundColor() {
  auto c = oflike::engine().getBackgroundColor();
  return ofFloatColor(c.r, c.g, c.b, c.a);
}

inline void ofSetBackgroundColor(int r, int g, int b, int a = 255) {
  oflike::engine().setBackgroundColor(oflike::Color4f{r / 255.f, g / 255.f, b / 255.f, a / 255.f});
}

inline void ofSetBackgroundColor(int gray, int a = 255) {
  ofSetBackgroundColor(gray, gray, gray, a);
}

inline void ofSetBackgroundColor(const ofColor& c) {
  oflike::engine().setBackgroundColor(oflike::Color4f{c.r / 255.f, c.g / 255.f, c.b / 255.f, c.a / 255.f});
}

inline void ofSetBackgroundColor(const ofFloatColor& c) {
  oflike::engine().setBackgroundColor(oflike::Color4f{c.r, c.g, c.b, c.a});
}

inline void ofSetBackgroundColorHex(int hexColor, int a = 255) {
  int r = (hexColor >> 16) & 0xFF;
  int g = (hexColor >> 8) & 0xFF;
  int b = hexColor & 0xFF;
  ofSetBackgroundColor(r, g, b, a);
}

// ============== Fill State Getter ==============

enum ofFillFlag {
  OF_OUTLINE = 0,
  OF_FILLED = 1
};

inline ofFillFlag ofGetFill() {
  return oflike::engine().drawList().isFilled() ? OF_FILLED : OF_OUTLINE;
}

// ============== Hex Color ==============

inline void ofSetHexColor(int hexColor) {
  int r = (hexColor >> 16) & 0xFF;
  int g = (hexColor >> 8) & 0xFF;
  int b = hexColor & 0xFF;
  ofSetColor(r, g, b);
}

// ============== RAII Wrappers ==============

// Automatically manages push/pop matrix scope
struct ofScopedMatrix {
  ofScopedMatrix() { ofPushMatrix(); }
  ~ofScopedMatrix() { ofPopMatrix(); }
  ofScopedMatrix(const ofScopedMatrix&) = delete;
  ofScopedMatrix& operator=(const ofScopedMatrix&) = delete;
};

// Automatically manages push/pop style scope
struct ofScopedStyle {
  ofScopedStyle() { ofPushStyle(); }
  ~ofScopedStyle() { ofPopStyle(); }
  ofScopedStyle(const ofScopedStyle&) = delete;
  ofScopedStyle& operator=(const ofScopedStyle&) = delete;
};

// Combines both matrix and style scope
struct ofScopedMatrixStyle : public ofScopedMatrix, public ofScopedStyle {
  ofScopedMatrixStyle() : ofScopedMatrix(), ofScopedStyle() {}
};

// ============== Time Functions ==============

inline uint64_t ofGetElapsedTimeMillis() {
  return static_cast<uint64_t>(oflike::engine().elapsedTimeSeconds() * 1000.0);
}

inline uint64_t ofGetElapsedTimeMicros() {
  return static_cast<uint64_t>(oflike::engine().elapsedTimeSeconds() * 1000000.0);
}

// ============== Scale with Vec ==============

inline void ofScale(const ofVec2f& s) {
  oflike::engine().drawList().scale(s.x, s.y);
}

inline void ofScale(const ofVec3f& s) {
  oflike::engine().drawList().scale(s.x, s.y);
}

inline void ofScale(float x, float y, float z) {
  (void)z;  // 2D rendering ignores z
  oflike::engine().drawList().scale(x, y);
}
