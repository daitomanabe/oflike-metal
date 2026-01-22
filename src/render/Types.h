#pragma once

#include <cmath>

namespace oflike {

// RGBA color with float components (0.0 - 1.0)
struct Color4f {
  float r{1.f}, g{1.f}, b{1.f}, a{1.f};
};

// 2D vertex with position, UV, and color
struct Vertex2D {
  float x{0.f}, y{0.f};
  float u{0.f}, v{0.f};
  float r{1.f}, g{1.f}, b{1.f}, a{1.f};
};

// Simple 2D transformation matrix (3x3, but we only need 2x3 for 2D)
// Layout: | a  b  tx |
//         | c  d  ty |
struct Matrix2D {
  float m[6] = {1, 0, 0, 1, 0, 0};  // [a, b, c, d, tx, ty]

  static Matrix2D identity() {
    return Matrix2D{1, 0, 0, 1, 0, 0};
  }

  static Matrix2D translation(float tx, float ty) {
    return Matrix2D{1, 0, 0, 1, tx, ty};
  }

  static Matrix2D rotation(float radians) {
    float c = std::cos(radians);
    float s = std::sin(radians);
    return Matrix2D{c, -s, s, c, 0, 0};
  }

  static Matrix2D scaling(float sx, float sy) {
    return Matrix2D{sx, 0, 0, sy, 0, 0};
  }

  Matrix2D operator*(const Matrix2D& other) const {
    Matrix2D result;
    result.m[0] = m[0] * other.m[0] + m[1] * other.m[2];
    result.m[1] = m[0] * other.m[1] + m[1] * other.m[3];
    result.m[2] = m[2] * other.m[0] + m[3] * other.m[2];
    result.m[3] = m[2] * other.m[1] + m[3] * other.m[3];
    result.m[4] = m[0] * other.m[4] + m[1] * other.m[5] + m[4];
    result.m[5] = m[2] * other.m[4] + m[3] * other.m[5] + m[5];
    return result;
  }

  void transform(float& x, float& y) const {
    float nx = m[0] * x + m[1] * y + m[4];
    float ny = m[2] * x + m[3] * y + m[5];
    x = nx;
    y = ny;
  }
};

} // namespace oflike
