#pragma once

// oflike-metal ofVec4f - openFrameworks API compatible 4D vector
// Uses Apple simd internally for performance

#include "../../math/Types.h"
#include <cmath>

namespace oflike {

/// \brief A 4D vector class for homogeneous coordinates and RGBA colors.
/// \details ofVec4f provides openFrameworks-compatible 4D vector operations
/// backed by Apple's simd library for optimal performance on macOS.
class ofVec4f {
public:
    // ========================================================================
    // Member Variables
    // ========================================================================

    float x;
    float y;
    float z;
    float w;

    static const int DIM = 4;

    // ========================================================================
    // Constructors
    // ========================================================================

    /// \brief Default constructor, initializes to (0, 0, 0, 0)
    ofVec4f() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}

    /// \brief Construct from x, y, z, w components
    ofVec4f(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}

    /// \brief Construct from a scalar (sets all components to the same value)
    explicit ofVec4f(float scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}

    /// \brief Construct from simd_float4
    explicit ofVec4f(const simd_float4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}

    // ========================================================================
    // simd Conversion
    // ========================================================================

    /// \brief Convert to simd_float4
    simd_float4 toSimd() const {
        return simd_make_float4(x, y, z, w);
    }

    /// \brief Create from simd_float4
    static ofVec4f fromSimd(const simd_float4& v) {
        return ofVec4f(v.x, v.y, v.z, v.w);
    }

    /// \brief Implicit conversion to simd_float4
    operator simd_float4() const {
        return toSimd();
    }

    // ========================================================================
    // Setters
    // ========================================================================

    /// \brief Set all components
    void set(float _x, float _y, float _z, float _w) {
        x = _x;
        y = _y;
        z = _z;
        w = _w;
    }

    /// \brief Set all components to the same scalar value
    void set(float scalar) {
        x = y = z = w = scalar;
    }

    /// \brief Copy from another vector
    void set(const ofVec4f& vec) {
        x = vec.x;
        y = vec.y;
        z = vec.z;
        w = vec.w;
    }

    // ========================================================================
    // Arithmetic Operators
    // ========================================================================

    /// \brief Vector addition
    ofVec4f operator+(const ofVec4f& vec) const {
        return ofVec4f(x + vec.x, y + vec.y, z + vec.z, w + vec.w);
    }

    /// \brief Add scalar to all components
    ofVec4f operator+(float f) const {
        return ofVec4f(x + f, y + f, z + f, w + f);
    }

    /// \brief In-place addition
    ofVec4f& operator+=(const ofVec4f& vec) {
        x += vec.x;
        y += vec.y;
        z += vec.z;
        w += vec.w;
        return *this;
    }

    /// \brief In-place scalar addition
    ofVec4f& operator+=(float f) {
        x += f;
        y += f;
        z += f;
        w += f;
        return *this;
    }

    /// \brief Vector subtraction
    ofVec4f operator-(const ofVec4f& vec) const {
        return ofVec4f(x - vec.x, y - vec.y, z - vec.z, w - vec.w);
    }

    /// \brief Subtract scalar from all components
    ofVec4f operator-(float f) const {
        return ofVec4f(x - f, y - f, z - f, w - f);
    }

    /// \brief Unary negation
    ofVec4f operator-() const {
        return ofVec4f(-x, -y, -z, -w);
    }

    /// \brief In-place subtraction
    ofVec4f& operator-=(const ofVec4f& vec) {
        x -= vec.x;
        y -= vec.y;
        z -= vec.z;
        w -= vec.w;
        return *this;
    }

    /// \brief In-place scalar subtraction
    ofVec4f& operator-=(float f) {
        x -= f;
        y -= f;
        z -= f;
        w -= f;
        return *this;
    }

    /// \brief Scalar multiplication
    ofVec4f operator*(float f) const {
        return ofVec4f(x * f, y * f, z * f, w * f);
    }

    /// \brief In-place scalar multiplication
    ofVec4f& operator*=(float f) {
        x *= f;
        y *= f;
        z *= f;
        w *= f;
        return *this;
    }

    /// \brief Scalar division
    ofVec4f operator/(float f) const {
        return ofVec4f(x / f, y / f, z / f, w / f);
    }

    /// \brief In-place scalar division
    ofVec4f& operator/=(float f) {
        x /= f;
        y /= f;
        z /= f;
        w /= f;
        return *this;
    }

    // ========================================================================
    // Comparison Operators
    // ========================================================================

    /// \brief Equality test
    bool operator==(const ofVec4f& vec) const {
        return (x == vec.x) && (y == vec.y) && (z == vec.z) && (w == vec.w);
    }

    /// \brief Inequality test
    bool operator!=(const ofVec4f& vec) const {
        return (x != vec.x) || (y != vec.y) || (z != vec.z) || (w != vec.w);
    }

    // ========================================================================
    // Array Access
    // ========================================================================

    /// \brief Array-style access (0=x, 1=y, 2=z, 3=w)
    float& operator[](int n) {
        if (n == 0) return x;
        if (n == 1) return y;
        if (n == 2) return z;
        return w;
    }

    /// \brief Const array-style access
    const float& operator[](int n) const {
        if (n == 0) return x;
        if (n == 1) return y;
        if (n == 2) return z;
        return w;
    }

    // ========================================================================
    // Vector Operations
    // ========================================================================

    /// \brief Returns the length (magnitude) of the vector
    float length() const {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }

    /// \brief Returns the squared length (avoids sqrt for performance)
    float lengthSquared() const {
        return x * x + y * y + z * z + w * w;
    }

    /// \brief Normalizes the vector to length 1 (modifies this vector)
    ofVec4f& normalize() {
        float len = length();
        if (len > 0.0f) {
            x /= len;
            y /= len;
            z /= len;
            w /= len;
        }
        return *this;
    }

    /// \brief Returns a normalized copy of this vector
    ofVec4f getNormalized() const {
        ofVec4f result(*this);
        result.normalize();
        return result;
    }

    /// \brief Scales the vector to a specific length
    ofVec4f& scale(float len) {
        float currentLen = length();
        if (currentLen > 0.0f) {
            float factor = len / currentLen;
            x *= factor;
            y *= factor;
            z *= factor;
            w *= factor;
        }
        return *this;
    }

    /// \brief Returns a scaled copy
    ofVec4f getScaled(float len) const {
        ofVec4f result(*this);
        result.scale(len);
        return result;
    }

    /// \brief Limits the length to a maximum
    ofVec4f& limit(float max) {
        float len = length();
        if (len > max && len > 0.0f) {
            float factor = max / len;
            x *= factor;
            y *= factor;
            z *= factor;
            w *= factor;
        }
        return *this;
    }

    /// \brief Returns a length-limited copy
    ofVec4f getLimited(float max) const {
        ofVec4f result(*this);
        result.limit(max);
        return result;
    }

    // ========================================================================
    // Dot Product
    // ========================================================================

    /// \brief Dot product with another vector
    float dot(const ofVec4f& vec) const {
        return x * vec.x + y * vec.y + z * vec.z + w * vec.w;
    }

    // ========================================================================
    // Distance
    // ========================================================================

    /// \brief Euclidean distance to another point
    float distance(const ofVec4f& pnt) const {
        float dx = x - pnt.x;
        float dy = y - pnt.y;
        float dz = z - pnt.z;
        float dw = w - pnt.w;
        return std::sqrt(dx * dx + dy * dy + dz * dz + dw * dw);
    }

    /// \brief Squared distance (faster, avoids sqrt)
    float squareDistance(const ofVec4f& pnt) const {
        float dx = x - pnt.x;
        float dy = y - pnt.y;
        float dz = z - pnt.z;
        float dw = w - pnt.w;
        return dx * dx + dy * dy + dz * dz + dw * dw;
    }

    // ========================================================================
    // Interpolation
    // ========================================================================

    /// \brief Linear interpolation between this and another vector
    /// \param pnt Target vector
    /// \param p Interpolation factor (0.0 = this, 1.0 = pnt)
    ofVec4f& interpolate(const ofVec4f& pnt, float p) {
        x += (pnt.x - x) * p;
        y += (pnt.y - y) * p;
        z += (pnt.z - z) * p;
        w += (pnt.w - w) * p;
        return *this;
    }

    /// \brief Returns interpolated copy
    ofVec4f getInterpolated(const ofVec4f& pnt, float p) const {
        ofVec4f result(*this);
        result.interpolate(pnt, p);
        return result;
    }

    // ========================================================================
    // Midpoint
    // ========================================================================

    /// \brief Sets this vector to the midpoint between this and pnt
    ofVec4f& middle(const ofVec4f& pnt) {
        x = (x + pnt.x) * 0.5f;
        y = (y + pnt.y) * 0.5f;
        z = (z + pnt.z) * 0.5f;
        w = (w + pnt.w) * 0.5f;
        return *this;
    }

    /// \brief Returns the midpoint
    ofVec4f getMiddle(const ofVec4f& pnt) const {
        return ofVec4f(
            (x + pnt.x) * 0.5f,
            (y + pnt.y) * 0.5f,
            (z + pnt.z) * 0.5f,
            (w + pnt.w) * 0.5f
        );
    }

    // ========================================================================
    // Pointer Access
    // ========================================================================

    /// \brief Get pointer to x component (y, z, w follow in memory)
    float* getPtr() {
        return &x;
    }

    /// \brief Get const pointer to x component
    const float* getPtr() const {
        return &x;
    }

    // ========================================================================
    // Comparison with Tolerance
    // ========================================================================

    /// \brief Tests if components match within tolerance
    bool match(const ofVec4f& vec, float tolerance = 0.0001f) const {
        return (std::abs(x - vec.x) < tolerance) &&
               (std::abs(y - vec.y) < tolerance) &&
               (std::abs(z - vec.z) < tolerance) &&
               (std::abs(w - vec.w) < tolerance);
    }

    // ========================================================================
    // Static Helpers
    // ========================================================================

    /// \brief Returns a vector with all components set to 0
    static ofVec4f zero() {
        return ofVec4f(0.0f, 0.0f, 0.0f, 0.0f);
    }

    /// \brief Returns a vector with all components set to 1
    static ofVec4f one() {
        return ofVec4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
};

// ============================================================================
// Global Operators (scalar on left side)
// ============================================================================

/// \brief Scalar multiplication (scalar * vector)
inline ofVec4f operator*(float f, const ofVec4f& vec) {
    return vec * f;
}

/// \brief Scalar addition (scalar + vector)
inline ofVec4f operator+(float f, const ofVec4f& vec) {
    return vec + f;
}

} // namespace oflike
