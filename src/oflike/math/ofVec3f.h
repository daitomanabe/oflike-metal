#pragma once

#include <cmath>

struct ofVec3f {
  float x{0.f}, y{0.f}, z{0.f};

  ofVec3f() = default;
  ofVec3f(float x_, float y_, float z_ = 0.f) : x(x_), y(y_), z(z_) {}

  ofVec3f operator+(const ofVec3f& v) const { return ofVec3f(x + v.x, y + v.y, z + v.z); }
  ofVec3f operator-(const ofVec3f& v) const { return ofVec3f(x - v.x, y - v.y, z - v.z); }
  ofVec3f operator*(float s) const { return ofVec3f(x * s, y * s, z * s); }
  ofVec3f operator/(float s) const { return ofVec3f(x / s, y / s, z / s); }

  ofVec3f& operator+=(const ofVec3f& v) { x += v.x; y += v.y; z += v.z; return *this; }
  ofVec3f& operator-=(const ofVec3f& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
  ofVec3f& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
  ofVec3f& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }

  float length() const { return std::sqrt(x * x + y * y + z * z); }
  float lengthSquared() const { return x * x + y * y + z * z; }

  ofVec3f normalized() const {
    float len = length();
    if (len > 0) return *this / len;
    return *this;
  }

  void normalize() {
    float len = length();
    if (len > 0) { x /= len; y /= len; z /= len; }
  }

  float dot(const ofVec3f& v) const { return x * v.x + y * v.y + z * v.z; }
  float distance(const ofVec3f& v) const { return (*this - v).length(); }

  ofVec3f cross(const ofVec3f& v) const {
    return ofVec3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
  }
};
