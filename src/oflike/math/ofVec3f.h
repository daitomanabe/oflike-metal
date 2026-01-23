#pragma once

// oflike-metal ofVec3f - openFrameworks API compatible 3D vector
// Uses Apple simd internally for performance

#include "../../math/Types.h"
#include <cmath>

namespace oflike {

/// \brief A 3D vector class for points, directions, and normals.
/// \details ofVec3f provides openFrameworks-compatible 3D vector operations
/// backed by Apple's simd library for optimal performance on macOS.
class ofVec3f {
public:
    // ========================================================================
    // Member Variables
    // ========================================================================

    float x;
    float y;
    float z;

    static const int DIM = 3;

    // ========================================================================
    // Constructors
    // ========================================================================

    /// \brief Default constructor, initializes to (0, 0, 0)
    ofVec3f() : x(0.0f), y(0.0f), z(0.0f) {}

    /// \brief Construct from x, y, z components
    ofVec3f(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    /// \brief Construct from a scalar (sets all components to the same value)
    explicit ofVec3f(float scalar) : x(scalar), y(scalar), z(scalar) {}

    /// \brief Construct from simd_float3
    explicit ofVec3f(const simd_float3& v) : x(v.x), y(v.y), z(v.z) {}

    // ========================================================================
    // simd Conversion
    // ========================================================================

    /// \brief Convert to simd_float3
    simd_float3 toSimd() const {
        return simd_make_float3(x, y, z);
    }

    /// \brief Create from simd_float3
    static ofVec3f fromSimd(const simd_float3& v) {
        return ofVec3f(v.x, v.y, v.z);
    }

    /// \brief Implicit conversion to simd_float3
    operator simd_float3() const {
        return toSimd();
    }

    // ========================================================================
    // Setters
    // ========================================================================

    /// \brief Set all components
    void set(float _x, float _y, float _z) {
        x = _x;
        y = _y;
        z = _z;
    }

    /// \brief Set all components to the same scalar value
    void set(float scalar) {
        x = y = z = scalar;
    }

    /// \brief Copy from another vector
    void set(const ofVec3f& vec) {
        x = vec.x;
        y = vec.y;
        z = vec.z;
    }

    // ========================================================================
    // Arithmetic Operators
    // ========================================================================

    /// \brief Vector addition
    ofVec3f operator+(const ofVec3f& vec) const {
        return ofVec3f(x + vec.x, y + vec.y, z + vec.z);
    }

    /// \brief Add scalar to all components
    ofVec3f operator+(float f) const {
        return ofVec3f(x + f, y + f, z + f);
    }

    /// \brief In-place addition
    ofVec3f& operator+=(const ofVec3f& vec) {
        x += vec.x;
        y += vec.y;
        z += vec.z;
        return *this;
    }

    /// \brief In-place scalar addition
    ofVec3f& operator+=(float f) {
        x += f;
        y += f;
        z += f;
        return *this;
    }

    /// \brief Vector subtraction
    ofVec3f operator-(const ofVec3f& vec) const {
        return ofVec3f(x - vec.x, y - vec.y, z - vec.z);
    }

    /// \brief Subtract scalar from all components
    ofVec3f operator-(float f) const {
        return ofVec3f(x - f, y - f, z - f);
    }

    /// \brief Unary negation
    ofVec3f operator-() const {
        return ofVec3f(-x, -y, -z);
    }

    /// \brief In-place subtraction
    ofVec3f& operator-=(const ofVec3f& vec) {
        x -= vec.x;
        y -= vec.y;
        z -= vec.z;
        return *this;
    }

    /// \brief In-place scalar subtraction
    ofVec3f& operator-=(float f) {
        x -= f;
        y -= f;
        z -= f;
        return *this;
    }

    /// \brief Scalar multiplication
    ofVec3f operator*(float f) const {
        return ofVec3f(x * f, y * f, z * f);
    }

    /// \brief In-place scalar multiplication
    ofVec3f& operator*=(float f) {
        x *= f;
        y *= f;
        z *= f;
        return *this;
    }

    /// \brief Scalar division
    ofVec3f operator/(float f) const {
        return ofVec3f(x / f, y / f, z / f);
    }

    /// \brief In-place scalar division
    ofVec3f& operator/=(float f) {
        x /= f;
        y /= f;
        z /= f;
        return *this;
    }

    // ========================================================================
    // Comparison Operators
    // ========================================================================

    /// \brief Equality test
    bool operator==(const ofVec3f& vec) const {
        return (x == vec.x) && (y == vec.y) && (z == vec.z);
    }

    /// \brief Inequality test
    bool operator!=(const ofVec3f& vec) const {
        return (x != vec.x) || (y != vec.y) || (z != vec.z);
    }

    // ========================================================================
    // Array Access
    // ========================================================================

    /// \brief Array-style access (0=x, 1=y, 2=z)
    float& operator[](int n) {
        if (n == 0) return x;
        if (n == 1) return y;
        return z;
    }

    /// \brief Const array-style access
    const float& operator[](int n) const {
        if (n == 0) return x;
        if (n == 1) return y;
        return z;
    }

    // ========================================================================
    // Vector Operations
    // ========================================================================

    /// \brief Returns the length (magnitude) of the vector
    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    /// \brief Returns the squared length (avoids sqrt for performance)
    float lengthSquared() const {
        return x * x + y * y + z * z;
    }

    /// \brief Normalizes the vector to length 1 (modifies this vector)
    ofVec3f& normalize() {
        float len = length();
        if (len > 0.0f) {
            x /= len;
            y /= len;
            z /= len;
        }
        return *this;
    }

    /// \brief Returns a normalized copy of this vector
    ofVec3f getNormalized() const {
        ofVec3f result(*this);
        result.normalize();
        return result;
    }

    /// \brief Scales the vector to a specific length
    ofVec3f& scale(float len) {
        float currentLen = length();
        if (currentLen > 0.0f) {
            float factor = len / currentLen;
            x *= factor;
            y *= factor;
            z *= factor;
        }
        return *this;
    }

    /// \brief Returns a scaled copy
    ofVec3f getScaled(float len) const {
        ofVec3f result(*this);
        result.scale(len);
        return result;
    }

    /// \brief Limits the length to a maximum
    ofVec3f& limit(float max) {
        float len = length();
        if (len > max && len > 0.0f) {
            float factor = max / len;
            x *= factor;
            y *= factor;
            z *= factor;
        }
        return *this;
    }

    /// \brief Returns a length-limited copy
    ofVec3f getLimited(float max) const {
        ofVec3f result(*this);
        result.limit(max);
        return result;
    }

    // ========================================================================
    // Dot Product
    // ========================================================================

    /// \brief Dot product with another vector
    /// \details Measures how parallel two vectors are.
    /// Result: 1 = parallel, 0 = perpendicular, -1 = opposite
    float dot(const ofVec3f& vec) const {
        return x * vec.x + y * vec.y + z * vec.z;
    }

    // ========================================================================
    // Cross Product (3D specific)
    // ========================================================================

    /// \brief Cross product with another vector
    /// \details Returns a vector perpendicular to both this and vec.
    /// The magnitude equals the area of the parallelogram formed by the vectors.
    /// Direction follows the right-hand rule.
    ofVec3f cross(const ofVec3f& vec) const {
        return ofVec3f(
            y * vec.z - z * vec.y,
            z * vec.x - x * vec.z,
            x * vec.y - y * vec.x
        );
    }

    /// \brief Returns the cross product
    ofVec3f getCrossed(const ofVec3f& vec) const {
        return cross(vec);
    }

    // ========================================================================
    // Distance
    // ========================================================================

    /// \brief Euclidean distance to another point
    float distance(const ofVec3f& pnt) const {
        float dx = x - pnt.x;
        float dy = y - pnt.y;
        float dz = z - pnt.z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    /// \brief Squared distance (faster, avoids sqrt)
    float squareDistance(const ofVec3f& pnt) const {
        float dx = x - pnt.x;
        float dy = y - pnt.y;
        float dz = z - pnt.z;
        return dx * dx + dy * dy + dz * dz;
    }

    // ========================================================================
    // Interpolation
    // ========================================================================

    /// \brief Linear interpolation between this and another vector
    /// \param pnt Target vector
    /// \param p Interpolation factor (0.0 = this, 1.0 = pnt)
    ofVec3f& interpolate(const ofVec3f& pnt, float p) {
        x += (pnt.x - x) * p;
        y += (pnt.y - y) * p;
        z += (pnt.z - z) * p;
        return *this;
    }

    /// \brief Returns interpolated copy
    ofVec3f getInterpolated(const ofVec3f& pnt, float p) const {
        ofVec3f result(*this);
        result.interpolate(pnt, p);
        return result;
    }

    // ========================================================================
    // Midpoint
    // ========================================================================

    /// \brief Sets this vector to the midpoint between this and pnt
    ofVec3f& middle(const ofVec3f& pnt) {
        x = (x + pnt.x) * 0.5f;
        y = (y + pnt.y) * 0.5f;
        z = (z + pnt.z) * 0.5f;
        return *this;
    }

    /// \brief Returns the midpoint
    ofVec3f getMiddle(const ofVec3f& pnt) const {
        return ofVec3f(
            (x + pnt.x) * 0.5f,
            (y + pnt.y) * 0.5f,
            (z + pnt.z) * 0.5f
        );
    }

    // ========================================================================
    // Rotation
    // ========================================================================

    /// \brief Rotates vector around an arbitrary axis
    /// \param angle Rotation angle in degrees
    /// \param axis Axis of rotation (should be normalized)
    ofVec3f& rotate(float angle, const ofVec3f& axis) {
        return rotateRad(angle * 0.017453292519943295f, axis); // deg to rad
    }

    /// \brief Rotates vector around an arbitrary axis (radians)
    ofVec3f& rotateRad(float angleRad, const ofVec3f& axis) {
        float c = std::cos(angleRad);
        float s = std::sin(angleRad);
        float t = 1.0f - c;

        ofVec3f ax = axis.getNormalized();

        float newX = (t * ax.x * ax.x + c)       * x +
                     (t * ax.x * ax.y - s * ax.z) * y +
                     (t * ax.x * ax.z + s * ax.y) * z;
        float newY = (t * ax.x * ax.y + s * ax.z) * x +
                     (t * ax.y * ax.y + c)       * y +
                     (t * ax.y * ax.z - s * ax.x) * z;
        float newZ = (t * ax.x * ax.z - s * ax.y) * x +
                     (t * ax.y * ax.z + s * ax.x) * y +
                     (t * ax.z * ax.z + c)       * z;

        x = newX;
        y = newY;
        z = newZ;
        return *this;
    }

    /// \brief Returns rotated copy (degrees)
    ofVec3f getRotated(float angle, const ofVec3f& axis) const {
        ofVec3f result(*this);
        result.rotate(angle, axis);
        return result;
    }

    /// \brief Returns rotated copy (radians)
    ofVec3f getRotatedRad(float angleRad, const ofVec3f& axis) const {
        ofVec3f result(*this);
        result.rotateRad(angleRad, axis);
        return result;
    }

    // ========================================================================
    // Perpendicular
    // ========================================================================

    /// \brief Returns a perpendicular vector
    /// \details Uses cross product with a non-parallel vector
    ofVec3f getPerpendicular() const {
        // Choose axis that's least parallel to this vector
        if (std::abs(x) < std::abs(y)) {
            if (std::abs(x) < std::abs(z)) {
                return cross(ofVec3f(1, 0, 0)).getNormalized();
            } else {
                return cross(ofVec3f(0, 0, 1)).getNormalized();
            }
        } else {
            if (std::abs(y) < std::abs(z)) {
                return cross(ofVec3f(0, 1, 0)).getNormalized();
            } else {
                return cross(ofVec3f(0, 0, 1)).getNormalized();
            }
        }
    }

    // ========================================================================
    // Angle
    // ========================================================================

    /// \brief Returns angle in degrees to another vector
    float angle(const ofVec3f& vec) const {
        return angleRad(vec) * 57.29577951308232f; // rad to deg
    }

    /// \brief Returns angle in radians to another vector
    float angleRad(const ofVec3f& vec) const {
        float d = dot(vec);
        float len1 = length();
        float len2 = vec.length();

        if (len1 == 0.0f || len2 == 0.0f) return 0.0f;

        float cosAngle = d / (len1 * len2);
        // Clamp to avoid NaN from acos
        if (cosAngle > 1.0f) cosAngle = 1.0f;
        if (cosAngle < -1.0f) cosAngle = -1.0f;

        return std::acos(cosAngle);
    }

    // ========================================================================
    // Pointer Access
    // ========================================================================

    /// \brief Get pointer to x component (y and z follow in memory)
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
    bool match(const ofVec3f& vec, float tolerance = 0.0001f) const {
        return (std::abs(x - vec.x) < tolerance) &&
               (std::abs(y - vec.y) < tolerance) &&
               (std::abs(z - vec.z) < tolerance);
    }

    // ========================================================================
    // Static Helpers
    // ========================================================================

    /// \brief Returns a vector with all components set to 0
    static ofVec3f zero() {
        return ofVec3f(0.0f, 0.0f, 0.0f);
    }

    /// \brief Returns a vector with all components set to 1
    static ofVec3f one() {
        return ofVec3f(1.0f, 1.0f, 1.0f);
    }
};

// ============================================================================
// Global Operators (scalar on left side)
// ============================================================================

/// \brief Scalar multiplication (scalar * vector)
inline ofVec3f operator*(float f, const ofVec3f& vec) {
    return vec * f;
}

/// \brief Scalar addition (scalar + vector)
inline ofVec3f operator+(float f, const ofVec3f& vec) {
    return vec + f;
}

} // namespace oflike
