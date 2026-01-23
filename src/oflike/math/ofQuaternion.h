#pragma once

// oflike-metal ofQuaternion - openFrameworks API compatible quaternion
// Uses Apple simd internally for performance

#include "../../math/Types.h"
#include "ofVec3f.h"
#include "ofVec4f.h"
#include <cmath>

namespace oflike {

// Forward declaration for matrix conversion
class ofMatrix4x4;

/// \brief A quaternion class for representing 3D rotations.
/// \details ofQuaternion provides openFrameworks-compatible quaternion operations
/// backed by Apple's simd library for optimal performance on macOS.
/// Quaternions are stored as (x, y, z, w) where w is the scalar component.
class ofQuaternion {
public:
    // ========================================================================
    // Member Variables
    // ========================================================================

    /// Internal storage: simd_quatf (x, y, z, w)
    simd_quatf quat;

    // ========================================================================
    // Constructors
    // ========================================================================

    /// \brief Default constructor, initializes to identity quaternion (0, 0, 0, 1)
    ofQuaternion() : quat(simd_quaternion(0.0f, 0.0f, 0.0f, 1.0f)) {}

    /// \brief Construct from x, y, z, w components (quaternion components)
    ofQuaternion(float x, float y, float z, float w)
        : quat(simd_quaternion(x, y, z, w)) {}

    /// \brief Construct from simd_quatf
    explicit ofQuaternion(const simd_quatf& q) : quat(q) {}

    /// \brief Construct from axis-angle (angle in degrees)
    ofQuaternion(float angle, const ofVec3f& axis) {
        float radians = angle * M_PI / 180.0f;
        simd_float3 ax = simd_normalize(simd_make_float3(axis.x, axis.y, axis.z));
        quat = simd_quaternion(radians, ax);
    }

    // ========================================================================
    // simd Conversion
    // ========================================================================

    /// \brief Convert to simd_quatf
    simd_quatf toSimd() const {
        return quat;
    }

    /// \brief Create from simd_quatf
    static ofQuaternion fromSimd(const simd_quatf& q) {
        return ofQuaternion(q);
    }

    /// \brief Implicit conversion to simd_quatf
    operator simd_quatf() const {
        return quat;
    }

    // ========================================================================
    // Component Access
    // ========================================================================

    /// \brief Get x component (vector part)
    float x() const { return quat.vector.x; }

    /// \brief Get y component (vector part)
    float y() const { return quat.vector.y; }

    /// \brief Get z component (vector part)
    float z() const { return quat.vector.z; }

    /// \brief Get w component (scalar part)
    float w() const { return quat.vector.w; }

    /// \brief Set components
    void set(float x, float y, float z, float w) {
        quat = simd_quaternion(x, y, z, w);
    }

    /// \brief Get vector part as ofVec3f
    ofVec3f getVector() const {
        return ofVec3f(quat.vector.x, quat.vector.y, quat.vector.z);
    }

    // ========================================================================
    // Static Factory Methods
    // ========================================================================

    /// \brief Returns identity quaternion (0, 0, 0, 1)
    static ofQuaternion identity() {
        return ofQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
    }

    /// \brief Create from axis and angle (angle in degrees)
    static ofQuaternion makeRotate(float angle, const ofVec3f& axis) {
        return ofQuaternion(angle, axis);
    }

    /// \brief Create from axis and angle with separate components
    static ofQuaternion makeRotate(float angle, float x, float y, float z) {
        return ofQuaternion(angle, ofVec3f(x, y, z));
    }

    /// \brief Create from Euler angles (degrees) - ZYX order
    /// \param yaw Rotation around Z axis (degrees)
    /// \param pitch Rotation around Y axis (degrees)
    /// \param roll Rotation around X axis (degrees)
    static ofQuaternion makeFromEuler(float yaw, float pitch, float roll) {
        // Convert to radians
        float yawRad = yaw * M_PI / 180.0f;
        float pitchRad = pitch * M_PI / 180.0f;
        float rollRad = roll * M_PI / 180.0f;

        // Create quaternions for each axis
        simd_quatf qx = simd_quaternion(rollRad, simd_make_float3(1.0f, 0.0f, 0.0f));
        simd_quatf qy = simd_quaternion(pitchRad, simd_make_float3(0.0f, 1.0f, 0.0f));
        simd_quatf qz = simd_quaternion(yawRad, simd_make_float3(0.0f, 0.0f, 1.0f));

        // Combine: Z * Y * X (applied in reverse order: X, Y, Z)
        return ofQuaternion(simd_mul(simd_mul(qz, qy), qx));
    }

    /// \brief Create from Euler angles as ofVec3f (degrees)
    static ofQuaternion makeFromEuler(const ofVec3f& euler) {
        return makeFromEuler(euler.z, euler.y, euler.x);
    }

    /// \brief Create quaternion that rotates from v1 to v2
    static ofQuaternion makeRotate(const ofVec3f& from, const ofVec3f& to) {
        simd_float3 v1 = simd_normalize(simd_make_float3(from.x, from.y, from.z));
        simd_float3 v2 = simd_normalize(simd_make_float3(to.x, to.y, to.z));

        float dot = simd_dot(v1, v2);

        // Check if vectors are parallel
        if (dot >= 1.0f - 1e-6f) {
            return identity(); // Same direction
        }
        if (dot <= -1.0f + 1e-6f) {
            // Opposite directions - find perpendicular axis
            simd_float3 axis = simd_cross(simd_make_float3(1, 0, 0), v1);
            if (simd_length(axis) < 1e-6f) {
                axis = simd_cross(simd_make_float3(0, 1, 0), v1);
            }
            axis = simd_normalize(axis);
            return ofQuaternion(simd_quaternion(M_PI, axis));
        }

        // Standard case
        simd_float3 axis = simd_cross(v1, v2);
        float angle = acosf(dot);
        return ofQuaternion(simd_quaternion(angle, simd_normalize(axis)));
    }

    // ========================================================================
    // Quaternion Operations
    // ========================================================================

    /// \brief Quaternion multiplication
    ofQuaternion operator*(const ofQuaternion& other) const {
        return ofQuaternion(simd_mul(quat, other.quat));
    }

    /// \brief In-place quaternion multiplication
    ofQuaternion& operator*=(const ofQuaternion& other) {
        quat = simd_mul(quat, other.quat);
        return *this;
    }

    /// \brief Rotate a vector by this quaternion
    ofVec3f operator*(const ofVec3f& v) const {
        simd_float3 vec = simd_make_float3(v.x, v.y, v.z);
        simd_float3 result = simd_act(quat, vec);
        return ofVec3f(result.x, result.y, result.z);
    }

    // ========================================================================
    // Quaternion Math
    // ========================================================================

    /// \brief Get length (magnitude) of quaternion
    float length() const {
        return simd_length(quat);
    }

    /// \brief Normalize this quaternion
    ofQuaternion& normalize() {
        quat = simd_normalize(quat);
        return *this;
    }

    /// \brief Get normalized copy
    ofQuaternion getNormalized() const {
        return ofQuaternion(simd_normalize(quat));
    }

    /// \brief Get conjugate (inverse rotation for unit quaternions)
    ofQuaternion getConjugate() const {
        return ofQuaternion(simd_conjugate(quat));
    }

    /// \brief Conjugate this quaternion
    ofQuaternion& conjugate() {
        quat = simd_conjugate(quat);
        return *this;
    }

    /// \brief Get inverse
    ofQuaternion getInverse() const {
        return ofQuaternion(simd_inverse(quat));
    }

    /// \brief Invert this quaternion
    ofQuaternion& invert() {
        quat = simd_inverse(quat);
        return *this;
    }

    // ========================================================================
    // Interpolation
    // ========================================================================

    /// \brief Spherical linear interpolation (slerp)
    /// \param target Target quaternion
    /// \param t Interpolation parameter (0.0 = this, 1.0 = target)
    static ofQuaternion slerp(const ofQuaternion& from, const ofQuaternion& to, float t) {
        return ofQuaternion(simd_slerp(from.quat, to.quat, t));
    }

    /// \brief In-place slerp towards target
    ofQuaternion& slerp(const ofQuaternion& target, float t) {
        quat = simd_slerp(quat, target.quat, t);
        return *this;
    }

    /// \brief Get slerp result
    ofQuaternion getSlerp(const ofQuaternion& target, float t) const {
        return slerp(*this, target, t);
    }

    // ========================================================================
    // Conversion to Rotation Representations
    // ========================================================================

    /// \brief Extract axis and angle from quaternion
    /// \param angle Output angle in degrees
    /// \param axis Output normalized axis
    void getRotate(float& angle, ofVec3f& axis) const {
        simd_float3 ax;
        float rad = simd_angle(quat);
        ax = simd_axis(quat);

        angle = rad * 180.0f / M_PI;
        axis = ofVec3f(ax.x, ax.y, ax.z);
    }

    /// \brief Convert to Euler angles (degrees) - ZYX order
    /// \return ofVec3f(roll, pitch, yaw) in degrees
    ofVec3f getEuler() const {
        // Extract roll (X), pitch (Y), yaw (Z) from quaternion
        float x = quat.vector.x;
        float y = quat.vector.y;
        float z = quat.vector.z;
        float w = quat.vector.w;

        // Roll (X-axis rotation)
        float sinr_cosp = 2.0f * (w * x + y * z);
        float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
        float roll = atan2f(sinr_cosp, cosr_cosp);

        // Pitch (Y-axis rotation)
        float sinp = 2.0f * (w * y - z * x);
        float pitch;
        if (fabsf(sinp) >= 1.0f) {
            pitch = copysignf(M_PI / 2.0f, sinp); // Use 90 degrees if out of range
        } else {
            pitch = asinf(sinp);
        }

        // Yaw (Z-axis rotation)
        float siny_cosp = 2.0f * (w * z + x * y);
        float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
        float yaw = atan2f(siny_cosp, cosy_cosp);

        // Convert to degrees
        return ofVec3f(
            roll * 180.0f / M_PI,
            pitch * 180.0f / M_PI,
            yaw * 180.0f / M_PI
        );
    }

    /// \brief Convert to rotation matrix
    /// Implementation is at the end of this file after ofMatrix4x4 is included
    ofMatrix4x4 getMatrix() const;

    // ========================================================================
    // Comparison
    // ========================================================================

    /// \brief Equality test
    bool operator==(const ofQuaternion& other) const {
        return quat.vector.x == other.quat.vector.x &&
               quat.vector.y == other.quat.vector.y &&
               quat.vector.z == other.quat.vector.z &&
               quat.vector.w == other.quat.vector.w;
    }

    /// \brief Inequality test
    bool operator!=(const ofQuaternion& other) const {
        return !(*this == other);
    }

    // ========================================================================
    // Misc
    // ========================================================================

    /// \brief Dot product with another quaternion
    float dot(const ofQuaternion& other) const {
        return simd_dot(quat, other.quat);
    }
};

} // namespace oflike

// ============================================================================
// Implementation of getMatrix() - requires ofMatrix4x4.h
// ============================================================================

#include "ofMatrix4x4.h"

namespace oflike {
inline ofMatrix4x4 ofQuaternion::getMatrix() const {
    simd_float4x4 mat = simd_matrix4x4(quat);
    return ofMatrix4x4(mat);
}
} // namespace oflike
