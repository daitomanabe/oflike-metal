#pragma once

#include <string>
#include <sstream>
#include <cmath>
#include "../../core/Context.h"
#include "ofColor.h"

// ============== Background & Color ==============

inline void ofBackground(int r, int g, int b) {
  oflike::engine().setClearColor(oflike::Color4f{r / 255.f, g / 255.f, b / 255.f, 1.f});
}

inline void ofBackground(int gray) {
  ofBackground(gray, gray, gray);
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

// ============== Circle & Ellipse ==============

inline void ofDrawCircle(float x, float y, float radius) {
  oflike::engine().drawList().addCircle(x, y, radius);
}

inline void ofCircle(float x, float y, float radius) {
  ofDrawCircle(x, y, radius);
}

inline void ofDrawEllipse(float x, float y, float width, float height) {
  oflike::engine().drawList().addEllipse(x, y, width * 0.5f, height * 0.5f);
}

inline void ofEllipse(float x, float y, float width, float height) {
  ofDrawEllipse(x, y, width, height);
}

// ============== Triangle ==============

inline void ofDrawTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
  oflike::engine().drawList().addTriangle(x1, y1, x2, y2, x3, y3);
}

inline void ofTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
  ofDrawTriangle(x1, y1, x2, y2, x3, y3);
}

// ============== Line ==============

inline void ofDrawLine(float x1, float y1, float x2, float y2) {
  oflike::engine().drawList().addLine(x1, y1, x2, y2);
}

inline void ofLine(float x1, float y1, float x2, float y2) {
  ofDrawLine(x1, y1, x2, y2);
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

inline void ofVertex(float x, float y, float u, float v) {
  oflike::engine().drawList().vertex(x, y, u, v);
}

inline void ofCurveVertex(float x, float y) {
  oflike::engine().drawList().curveVertex(x, y);
}

inline void ofBezierVertex(float cx1, float cy1, float cx2, float cy2, float x, float y) {
  oflike::engine().drawList().bezierVertex(cx1, cy1, cx2, cy2, x, y);
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

inline void ofRotate(float degrees) {
  oflike::engine().drawList().rotate(degrees * 3.14159265358979f / 180.f);
}

inline void ofRotateRad(float radians) {
  oflike::engine().drawList().rotate(radians);
}

inline void ofRotateDeg(float degrees) {
  ofRotate(degrees);
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
