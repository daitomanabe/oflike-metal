#pragma once

#include <string>
#include <sstream>
#include "ofContext.hpp"

// openFrameworks-like facade functions.

inline void ofBackground(int r, int g, int b) {
  oflike::engine().setClearColor(oflike::Color4f{r / 255.f, g / 255.f, b / 255.f, 1.f});
}

inline void ofSetColor(int r, int g, int b, int a = 255) {
  oflike::engine().drawList().setColor(oflike::Color4f{r / 255.f, g / 255.f, b / 255.f, a / 255.f});
}

inline void ofDrawRectangle(float x, float y, float w, float h) {
  oflike::engine().drawList().addRect(x, y, w, h);
}

// Draw bitmap string at position (x, y)
inline void ofDrawBitmapString(const std::string& text, float x, float y) {
  oflike::engine().drawList().addText(text, x, y);
}

// Overload for different types (like oF)
template<typename T>
inline void ofDrawBitmapString(const T& value, float x, float y) {
  std::ostringstream oss;
  oss << value;
  ofDrawBitmapString(oss.str(), x, y);
}

// Get window width in points
inline int ofGetWidth() {
  return oflike::engine().getWindowWidth();
}

// Get window height in points
inline int ofGetHeight() {
  return oflike::engine().getWindowHeight();
}

// Get current frame rate
inline float ofGetFrameRate() {
  return oflike::engine().getFrameRate();
}

// Get elapsed time since app start
inline float ofGetElapsedTimef() {
  return static_cast<float>(oflike::engine().elapsedTimeSeconds());
}

// Get frame number
inline int ofGetFrameNum() {
  return oflike::engine().getFrameNum();
}

// Convert to string (oF-like utility)
template<typename T>
inline std::string ofToString(const T& value) {
  std::ostringstream oss;
  oss << value;
  return oss.str();
}

template<typename T>
inline std::string ofToString(const T& value, int precision) {
  std::ostringstream oss;
  oss.precision(precision);
  oss << std::fixed << value;
  return oss.str();
}
