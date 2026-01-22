#pragma once

#include <cmath>

struct ofVec2f {
  float x{0.f}, y{0.f};

  ofVec2f() = default;
  ofVec2f(float x_, float y_) : x(x_), y(y_) {}

  ofVec2f operator+(const ofVec2f& v) const { return ofVec2f(x + v.x, y + v.y); }
  ofVec2f operator-(const ofVec2f& v) const { return ofVec2f(x - v.x, y - v.y); }
  ofVec2f operator*(float s) const { return ofVec2f(x * s, y * s); }
  ofVec2f operator/(float s) const { return ofVec2f(x / s, y / s); }

  ofVec2f& operator+=(const ofVec2f& v) { x += v.x; y += v.y; return *this; }
  ofVec2f& operator-=(const ofVec2f& v) { x -= v.x; y -= v.y; return *this; }
  ofVec2f& operator*=(float s) { x *= s; y *= s; return *this; }
  ofVec2f& operator/=(float s) { x /= s; y /= s; return *this; }

  float length() const { return std::sqrt(x * x + y * y); }
  float lengthSquared() const { return x * x + y * y; }

  ofVec2f normalized() const {
    float len = length();
    if (len > 0) return *this / len;
    return *this;
  }

  void normalize() {
    float len = length();
    if (len > 0) { x /= len; y /= len; }
  }

  float dot(const ofVec2f& v) const { return x * v.x + y * v.y; }
  float distance(const ofVec2f& v) const { return (*this - v).length(); }

  ofVec2f rotated(float radians) const {
    float c = std::cos(radians);
    float s = std::sin(radians);
    return ofVec2f(x * c - y * s, x * s + y * c);
  }
};
