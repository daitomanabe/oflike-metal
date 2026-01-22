#pragma once

#include <cmath>
#include <cstdlib>
#include <random>

#include "ofVectorMath.h"

// Constants
constexpr float PI = 3.14159265358979f;
constexpr float TWO_PI = 6.28318530717958f;
constexpr float HALF_PI = 1.57079632679489f;

// Random number in range [0, max)
inline float ofRandom(float max) {
  static std::mt19937 gen(std::random_device{}());
  std::uniform_real_distribution<float> dist(0.f, max);
  return dist(gen);
}

// Random number in range [min, max)
inline float ofRandom(float min, float max) {
  static std::mt19937 gen(std::random_device{}());
  std::uniform_real_distribution<float> dist(min, max);
  return dist(gen);
}

// Random integer in range [min, max]
inline int ofRandom(int min, int max) {
  static std::mt19937 gen(std::random_device{}());
  std::uniform_int_distribution<int> dist(min, max);
  return dist(gen);
}

// Map value from one range to another
inline float ofMap(float value, float inputMin, float inputMax, float outputMin, float outputMax, bool clamp = false) {
  float outVal = ((value - inputMin) / (inputMax - inputMin)) * (outputMax - outputMin) + outputMin;
  if (clamp) {
    if (outputMin < outputMax) {
      if (outVal < outputMin) outVal = outputMin;
      else if (outVal > outputMax) outVal = outputMax;
    } else {
      if (outVal > outputMin) outVal = outputMin;
      else if (outVal < outputMax) outVal = outputMax;
    }
  }
  return outVal;
}

// Linear interpolation
inline float ofLerp(float start, float end, float t) {
  return start + t * (end - start);
}

// Clamp value to range
inline float ofClamp(float value, float min, float max) {
  return value < min ? min : (value > max ? max : value);
}

// Normalize value to range [0, 1]
inline float ofNormalize(float value, float min, float max) {
  return (value - min) / (max - min);
}

// Distance between two points
inline float ofDist(float x1, float y1, float x2, float y2) {
  float dx = x2 - x1;
  float dy = y2 - y1;
  return std::sqrt(dx * dx + dy * dy);
}

inline float ofDist(float x1, float y1, float z1, float x2, float y2, float z2) {
  float dx = x2 - x1;
  float dy = y2 - y1;
  float dz = z2 - z1;
  return std::sqrt(dx * dx + dy * dy + dz * dz);
}

// Angle from point (x1,y1) to point (x2,y2) in radians
inline float ofAngleRad(float x1, float y1, float x2, float y2) {
  return std::atan2(y2 - y1, x2 - x1);
}

// Angle in degrees
inline float ofAngleDeg(float x1, float y1, float x2, float y2) {
  return ofAngleRad(x1, y1, x2, y2) * 180.f / PI;
}

// Degrees to radians
inline float ofDegToRad(float degrees) {
  return degrees * PI / 180.f;
}

// Radians to degrees
inline float ofRadToDeg(float radians) {
  return radians * 180.f / PI;
}

// Sign of a number (-1, 0, or 1)
inline int ofSign(float n) {
  if (n > 0) return 1;
  else if (n < 0) return -1;
  else return 0;
}

// Wrap value to range [min, max)
inline float ofWrap(float value, float min, float max) {
  float range = max - min;
  while (value >= max) value -= range;
  while (value < min) value += range;
  return value;
}

// Simple 1D Perlin-like noise
inline float ofNoise(float x) {
  int xi = static_cast<int>(std::floor(x)) & 255;
  float xf = x - std::floor(x);

  auto hash = [](int n) -> float {
    n = (n << 13) ^ n;
    n = n * (n * n * 15731 + 789221) + 1376312589;
    return (1.0f - ((n & 0x7fffffff) / 1073741824.0f));
  };

  float u = xf * xf * (3.f - 2.f * xf);
  return ofLerp(hash(xi), hash(xi + 1), u) * 0.5f + 0.5f;
}

// 2D noise
inline float ofNoise(float x, float y) {
  int xi = static_cast<int>(std::floor(x)) & 255;
  int yi = static_cast<int>(std::floor(y)) & 255;
  float xf = x - std::floor(x);
  float yf = y - std::floor(y);

  auto hash = [](int x, int y) -> float {
    int n = x + y * 57;
    n = (n << 13) ^ n;
    n = n * (n * n * 15731 + 789221) + 1376312589;
    return (1.0f - ((n & 0x7fffffff) / 1073741824.0f));
  };

  float u = xf * xf * (3.f - 2.f * xf);
  float v = yf * yf * (3.f - 2.f * yf);

  float n00 = hash(xi, yi);
  float n10 = hash(xi + 1, yi);
  float n01 = hash(xi, yi + 1);
  float n11 = hash(xi + 1, yi + 1);

  float nx0 = ofLerp(n00, n10, u);
  float nx1 = ofLerp(n01, n11, u);

  return ofLerp(nx0, nx1, v) * 0.5f + 0.5f;
}

// Signed noise (-1 to 1)
inline float ofSignedNoise(float x) {
  return ofNoise(x) * 2.f - 1.f;
}

inline float ofSignedNoise(float x, float y) {
  return ofNoise(x, y) * 2.f - 1.f;
}
