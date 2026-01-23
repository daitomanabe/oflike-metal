#pragma once

// oflike-metal ofVec2f - openFrameworks API compatible 2D vector
// Uses Apple simd internally for performance

#include "../../math/Types.h"
#include <cmath>

namespace oflike {

/// \brief A 2D vector class for points and directions.
/// \details ofVec2f provides openFrameworks-compatible 2D vector operations
/// backed by Apple's simd library for optimal performance on macOS.
class ofVec2f {
public:
    // ========================================================================
    // Member Variables
    // ========================================================================

    float x;
    float y;

    static const int DIM = 2;

    // ========================================================================
    // Constructors
    // ========================================================================

    /// \brief Default constructor, initializes to (0, 0)
    ofVec2f() : x(0.0f), y(0.0f) {}

    /// \brief Construct from x and y components
    ofVec2f(float _x, float _y) : x(_x), y(_y) {}

    /// \brief Construct from a scalar (sets both x and y to the same value)
    explicit ofVec2f(float scalar) : x(scalar), y(scalar) {}

    /// \brief Construct from simd_float2
    explicit ofVec2f(const simd_float2& v) : x(v.x), y(v.y) {}

    // ========================================================================
    // simd Conversion
    // ========================================================================

    /// \brief Convert to simd_float2
    simd_float2 toSimd() const {
        return simd_make_float2(x, y);
    }

    /// \brief Create from simd_float2
    static ofVec2f fromSimd(const simd_float2& v) {
        return ofVec2f(v.x, v.y);
    }

    /// \brief Implicit conversion to simd_float2
    operator simd_float2() const {
        return toSimd();
    }

    // ========================================================================
    // Setters
    // ========================================================================

    /// \brief Set both components
    void set(float _x, float _y) {
        x = _x;
        y = _y;
    }

    /// \brief Set both components to the same scalar value
    void set(float scalar) {
        x = y = scalar;
    }

    /// \brief Copy from another vector
    void set(const ofVec2f& vec) {
        x = vec.x;
        y = vec.y;
    }

    // ========================================================================
    // Arithmetic Operators
    // ========================================================================

    /// \brief Vector addition
    ofVec2f operator+(const ofVec2f& vec) const {
        return ofVec2f(x + vec.x, y + vec.y);
    }

    /// \brief Add scalar to both components
    ofVec2f operator+(float f) const {
        return ofVec2f(x + f, y + f);
    }

    /// \brief In-place addition
    ofVec2f& operator+=(const ofVec2f& vec) {
        x += vec.x;
        y += vec.y;
        return *this;
    }

    /// \brief In-place scalar addition
    ofVec2f& operator+=(float f) {
        x += f;
        y += f;
        return *this;
    }

    /// \brief Vector subtraction
    ofVec2f operator-(const ofVec2f& vec) const {
        return ofVec2f(x - vec.x, y - vec.y);
    }

    /// \brief Subtract scalar from both components
    ofVec2f operator-(float f) const {
        return ofVec2f(x - f, y - f);
    }

    /// \brief Unary negation
    ofVec2f operator-() const {
        return ofVec2f(-x, -y);
    }

    /// \brief In-place subtraction
    ofVec2f& operator-=(const ofVec2f& vec) {
        x -= vec.x;
        y -= vec.y;
        return *this;
    }

    /// \brief In-place scalar subtraction
    ofVec2f& operator-=(float f) {
        x -= f;
        y -= f;
        return *this;
    }

    /// \brief Scalar multiplication
    ofVec2f operator*(float f) const {
        return ofVec2f(x * f, y * f);
    }

    /// \brief In-place scalar multiplication
    ofVec2f& operator*=(float f) {
        x *= f;
        y *= f;
        return *this;
    }

    /// \brief Scalar division
    ofVec2f operator/(float f) const {
        return ofVec2f(x / f, y / f);
    }

    /// \brief In-place scalar division
    ofVec2f& operator/=(float f) {
        x /= f;
        y /= f;
        return *this;
    }

    // ========================================================================
    // Comparison Operators
    // ========================================================================

    /// \brief Equality test
    bool operator==(const ofVec2f& vec) const {
        return (x == vec.x) && (y == vec.y);
    }

    /// \brief Inequality test
    bool operator!=(const ofVec2f& vec) const {
        return (x != vec.x) || (y != vec.y);
    }

    // ========================================================================
    // Array Access
    // ========================================================================

    /// \brief Array-style access (0=x, 1=y)
    float& operator[](int n) {
        return (n == 0) ? x : y;
    }

    /// \brief Const array-style access
    const float& operator[](int n) const {
        return (n == 0) ? x : y;
    }

    // ========================================================================
    // Vector Operations
    // ========================================================================

    /// \brief Returns the length (magnitude) of the vector
    float length() const {
        return std::sqrt(x * x + y * y);
    }

    /// \brief Returns the squared length (avoids sqrt for performance)
    float lengthSquared() const {
        return x * x + y * y;
    }

    /// \brief Normalizes the vector to length 1 (modifies this vector)
    ofVec2f& normalize() {
        float len = length();
        if (len > 0.0f) {
            x /= len;
            y /= len;
        }
        return *this;
    }

    /// \brief Returns a normalized copy of this vector
    ofVec2f getNormalized() const {
        ofVec2f result(*this);
        result.normalize();
        return result;
    }

    /// \brief Scales the vector to a specific length
    ofVec2f& scale(float len) {
        float currentLen = length();
        if (currentLen > 0.0f) {
            x *= len / currentLen;
            y *= len / currentLen;
        }
        return *this;
    }

    /// \brief Returns a scaled copy
    ofVec2f getScaled(float len) const {
        ofVec2f result(*this);
        result.scale(len);
        return result;
    }

    /// \brief Limits the length to a maximum
    ofVec2f& limit(float max) {
        float len = length();
        if (len > max && len > 0.0f) {
            x *= max / len;
            y *= max / len;
        }
        return *this;
    }

    /// \brief Returns a length-limited copy
    ofVec2f getLimited(float max) const {
        ofVec2f result(*this);
        result.limit(max);
        return result;
    }

    // ========================================================================
    // Dot Product
    // ========================================================================

    /// \brief Dot product with another vector
    float dot(const ofVec2f& vec) const {
        return x * vec.x + y * vec.y;
    }

    // ========================================================================
    // Distance
    // ========================================================================

    /// \brief Euclidean distance to another point
    float distance(const ofVec2f& pnt) const {
        float dx = x - pnt.x;
        float dy = y - pnt.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    /// \brief Squared distance (faster, avoids sqrt)
    float squareDistance(const ofVec2f& pnt) const {
        float dx = x - pnt.x;
        float dy = y - pnt.y;
        return dx * dx + dy * dy;
    }

    // ========================================================================
    // Pointer Access
    // ========================================================================

    /// \brief Get pointer to x component (y follows in memory)
    float* getPtr() {
        return &x;
    }

    /// \brief Get const pointer to x component
    const float* getPtr() const {
        return &x;
    }

    // ========================================================================
    // Static Helpers
    // ========================================================================

    /// \brief Returns a vector with both components set to 0
    static ofVec2f zero() {
        return ofVec2f(0.0f, 0.0f);
    }

    /// \brief Returns a vector with both components set to 1
    static ofVec2f one() {
        return ofVec2f(1.0f, 1.0f);
    }
};

// ============================================================================
// Global Operators (scalar on left side)
// ============================================================================

/// \brief Scalar multiplication (scalar * vector)
inline ofVec2f operator*(float f, const ofVec2f& vec) {
    return vec * f;
}

/// \brief Scalar addition (scalar + vector)
inline ofVec2f operator+(float f, const ofVec2f& vec) {
    return vec + f;
}

} // namespace oflike
