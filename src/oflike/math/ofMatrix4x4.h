#pragma once

// oflike-metal ofMatrix4x4 - openFrameworks API compatible 4x4 matrix
// Uses Apple simd internally for performance

#include "../../math/Types.h"
#include "ofVec3f.h"
#include "ofVec4f.h"
#include <cmath>

namespace oflike {

/// \brief A 4x4 matrix class for 3D transformations.
/// \details ofMatrix4x4 provides openFrameworks-compatible matrix operations
/// backed by Apple's simd library for optimal performance on macOS.
/// Stores data in column-major order (matching OpenGL/Metal conventions).
class ofMatrix4x4 {
public:
    // ========================================================================
    // Member Variables
    // ========================================================================

    /// Internal storage: column-major simd_float4x4
    simd_float4x4 mat;

    // ========================================================================
    // Constructors
    // ========================================================================

    /// \brief Default constructor, initializes to identity matrix
    ofMatrix4x4() : mat(matrix_identity_float4x4) {}

    /// \brief Construct from simd_float4x4
    explicit ofMatrix4x4(const simd_float4x4& m) : mat(m) {}

    /// \brief Construct from 16 floats (row-major order for user convenience)
    ofMatrix4x4(float m00, float m01, float m02, float m03,
                float m10, float m11, float m12, float m13,
                float m20, float m21, float m22, float m23,
                float m30, float m31, float m32, float m33) {
        // User provides row-major, we store column-major
        mat.columns[0] = simd_make_float4(m00, m10, m20, m30);
        mat.columns[1] = simd_make_float4(m01, m11, m21, m31);
        mat.columns[2] = simd_make_float4(m02, m12, m22, m32);
        mat.columns[3] = simd_make_float4(m03, m13, m23, m33);
    }

    // ========================================================================
    // simd Conversion
    // ========================================================================

    /// \brief Convert to simd_float4x4
    simd_float4x4 toSimd() const {
        return mat;
    }

    /// \brief Create from simd_float4x4
    static ofMatrix4x4 fromSimd(const simd_float4x4& m) {
        return ofMatrix4x4(m);
    }

    /// \brief Implicit conversion to simd_float4x4
    operator simd_float4x4() const {
        return mat;
    }

    // ========================================================================
    // Static Factory Methods
    // ========================================================================

    /// \brief Returns identity matrix
    static ofMatrix4x4 identity() {
        return ofMatrix4x4(matrix_identity_float4x4);
    }

    /// \brief Returns zero matrix
    static ofMatrix4x4 zero() {
        simd_float4x4 m = {};
        return ofMatrix4x4(m);
    }

    /// \brief Create translation matrix
    static ofMatrix4x4 newTranslationMatrix(float x, float y, float z) {
        simd_float4x4 m = matrix_identity_float4x4;
        m.columns[3] = simd_make_float4(x, y, z, 1.0f);
        return ofMatrix4x4(m);
    }

    /// \brief Create translation matrix from vector
    static ofMatrix4x4 newTranslationMatrix(const ofVec3f& v) {
        return newTranslationMatrix(v.x, v.y, v.z);
    }

    /// \brief Create rotation matrix around X axis (angle in degrees)
    static ofMatrix4x4 newRotationMatrix(float angle, float x, float y, float z) {
        float radians = angle * M_PI / 180.0f;
        simd_float3 axis = simd_normalize(simd_make_float3(x, y, z));
        simd_quatf q = simd_quaternion(radians, axis);
        return ofMatrix4x4(simd_matrix4x4(q));
    }

    /// \brief Create rotation matrix from axis and angle
    static ofMatrix4x4 newRotationMatrix(float angle, const ofVec3f& axis) {
        return newRotationMatrix(angle, axis.x, axis.y, axis.z);
    }

    /// \brief Create scale matrix
    static ofMatrix4x4 newScaleMatrix(float x, float y, float z) {
        simd_float4x4 m = matrix_identity_float4x4;
        m.columns[0].x = x;
        m.columns[1].y = y;
        m.columns[2].z = z;
        return ofMatrix4x4(m);
    }

    /// \brief Create uniform scale matrix
    static ofMatrix4x4 newScaleMatrix(float scale) {
        return newScaleMatrix(scale, scale, scale);
    }

    /// \brief Create scale matrix from vector
    static ofMatrix4x4 newScaleMatrix(const ofVec3f& v) {
        return newScaleMatrix(v.x, v.y, v.z);
    }

    /// \brief Create lookAt view matrix (right-handed)
    static ofMatrix4x4 newLookAtMatrix(const ofVec3f& eye, const ofVec3f& center, const ofVec3f& up) {
        // Compute forward (z-axis), right (x-axis), and up (y-axis) vectors
        simd_float3 f = simd_normalize(simd_make_float3(
            center.x - eye.x,
            center.y - eye.y,
            center.z - eye.z
        ));

        simd_float3 up_simd = simd_make_float3(up.x, up.y, up.z);
        simd_float3 r = simd_normalize(simd_cross(f, up_simd));
        simd_float3 u = simd_cross(r, f);

        // Build view matrix (right-handed, looking down -Z)
        simd_float4x4 m;
        m.columns[0] = simd_make_float4(r.x, u.x, -f.x, 0.0f);
        m.columns[1] = simd_make_float4(r.y, u.y, -f.y, 0.0f);
        m.columns[2] = simd_make_float4(r.z, u.z, -f.z, 0.0f);
        m.columns[3] = simd_make_float4(
            -simd_dot(r, simd_make_float3(eye.x, eye.y, eye.z)),
            -simd_dot(u, simd_make_float3(eye.x, eye.y, eye.z)),
            simd_dot(f, simd_make_float3(eye.x, eye.y, eye.z)),
            1.0f
        );

        return ofMatrix4x4(m);
    }

    /// \brief Create perspective projection matrix (right-handed, FOV in degrees)
    static ofMatrix4x4 newPerspectiveMatrix(float fovY, float aspect, float zNear, float zFar) {
        float fovYRadians = fovY * M_PI / 180.0f;
        float f = 1.0f / tanf(fovYRadians * 0.5f);

        simd_float4x4 m = {};
        m.columns[0] = simd_make_float4(f / aspect, 0.0f, 0.0f, 0.0f);
        m.columns[1] = simd_make_float4(0.0f, f, 0.0f, 0.0f);
        m.columns[2] = simd_make_float4(0.0f, 0.0f, (zFar + zNear) / (zNear - zFar), -1.0f);
        m.columns[3] = simd_make_float4(0.0f, 0.0f, (2.0f * zFar * zNear) / (zNear - zFar), 0.0f);

        return ofMatrix4x4(m);
    }

    /// \brief Create orthographic projection matrix (right-handed)
    static ofMatrix4x4 newOrthoMatrix(float left, float right, float bottom, float top, float zNear, float zFar) {
        float rl = right - left;
        float tb = top - bottom;
        float fn = zFar - zNear;

        simd_float4x4 m = {};
        m.columns[0] = simd_make_float4(2.0f / rl, 0.0f, 0.0f, 0.0f);
        m.columns[1] = simd_make_float4(0.0f, 2.0f / tb, 0.0f, 0.0f);
        m.columns[2] = simd_make_float4(0.0f, 0.0f, -2.0f / fn, 0.0f);
        m.columns[3] = simd_make_float4(
            -(right + left) / rl,
            -(top + bottom) / tb,
            -(zFar + zNear) / fn,
            1.0f
        );

        return ofMatrix4x4(m);
    }

    /// \brief Create 2D orthographic projection matrix (for 2D rendering)
    static ofMatrix4x4 newOrtho2DMatrix(float left, float right, float bottom, float top) {
        return newOrthoMatrix(left, right, bottom, top, -1.0f, 1.0f);
    }

    // ========================================================================
    // Element Access
    // ========================================================================

    /// \brief Get element at (row, col) - row-major indexing for user convenience
    float operator()(int row, int col) const {
        return mat.columns[col][row];
    }

    /// \brief Set element at (row, col) using pointer access
    void set(int row, int col, float value) {
        float* data = (float*)&mat;
        data[col * 4 + row] = value;
    }

    /// \brief Get pointer to matrix data (column-major)
    float* getPtr() {
        return (float*)&mat;
    }

    /// \brief Get const pointer to matrix data (column-major)
    const float* getPtr() const {
        return (const float*)&mat;
    }

    // ========================================================================
    // Matrix Operations
    // ========================================================================

    /// \brief Matrix multiplication
    ofMatrix4x4 operator*(const ofMatrix4x4& other) const {
        return ofMatrix4x4(simd_mul(mat, other.mat));
    }

    /// \brief In-place matrix multiplication
    ofMatrix4x4& operator*=(const ofMatrix4x4& other) {
        mat = simd_mul(mat, other.mat);
        return *this;
    }

    /// \brief Transform a 3D vector (treats as point with w=1)
    ofVec3f operator*(const ofVec3f& v) const {
        simd_float4 v4 = simd_make_float4(v.x, v.y, v.z, 1.0f);
        simd_float4 result = simd_mul(mat, v4);
        // Divide by w for perspective correct
        if (result.w != 0.0f && result.w != 1.0f) {
            return ofVec3f(result.x / result.w, result.y / result.w, result.z / result.w);
        }
        return ofVec3f(result.x, result.y, result.z);
    }

    /// \brief Transform a 4D vector
    ofVec4f operator*(const ofVec4f& v) const {
        simd_float4 v4 = simd_make_float4(v.x, v.y, v.z, v.w);
        simd_float4 result = simd_mul(mat, v4);
        return ofVec4f(result.x, result.y, result.z, result.w);
    }

    // ========================================================================
    // Transformation Methods (modify this matrix)
    // ========================================================================

    /// \brief Apply translation (post-multiply)
    void translate(float x, float y, float z) {
        *this *= newTranslationMatrix(x, y, z);
    }

    /// \brief Apply translation from vector
    void translate(const ofVec3f& v) {
        translate(v.x, v.y, v.z);
    }

    /// \brief Apply rotation (post-multiply, angle in degrees)
    void rotate(float angle, float x, float y, float z) {
        *this *= newRotationMatrix(angle, x, y, z);
    }

    /// \brief Apply rotation from axis and angle
    void rotate(float angle, const ofVec3f& axis) {
        rotate(angle, axis.x, axis.y, axis.z);
    }

    /// \brief Apply rotation around X axis (angle in degrees)
    void rotateX(float angle) {
        rotate(angle, 1.0f, 0.0f, 0.0f);
    }

    /// \brief Apply rotation around Y axis (angle in degrees)
    void rotateY(float angle) {
        rotate(angle, 0.0f, 1.0f, 0.0f);
    }

    /// \brief Apply rotation around Z axis (angle in degrees)
    void rotateZ(float angle) {
        rotate(angle, 0.0f, 0.0f, 1.0f);
    }

    /// \brief Apply scale (post-multiply)
    void scale(float x, float y, float z) {
        *this *= newScaleMatrix(x, y, z);
    }

    /// \brief Apply uniform scale
    void scale(float scale) {
        this->scale(scale, scale, scale);
    }

    /// \brief Apply scale from vector
    void scale(const ofVec3f& v) {
        scale(v.x, v.y, v.z);
    }

    // ========================================================================
    // Matrix Decomposition
    // ========================================================================

    /// \brief Get translation component
    ofVec3f getTranslation() const {
        return ofVec3f(mat.columns[3].x, mat.columns[3].y, mat.columns[3].z);
    }

    /// \brief Get scale component (length of each axis)
    ofVec3f getScale() const {
        float sx = simd_length(simd_make_float3(mat.columns[0].x, mat.columns[0].y, mat.columns[0].z));
        float sy = simd_length(simd_make_float3(mat.columns[1].x, mat.columns[1].y, mat.columns[1].z));
        float sz = simd_length(simd_make_float3(mat.columns[2].x, mat.columns[2].y, mat.columns[2].z));
        return ofVec3f(sx, sy, sz);
    }

    // ========================================================================
    // Advanced Operations
    // ========================================================================

    /// \brief Transpose this matrix
    void transpose() {
        mat = simd_transpose(mat);
    }

    /// \brief Get transposed copy
    ofMatrix4x4 getTransposed() const {
        return ofMatrix4x4(simd_transpose(mat));
    }

    /// \brief Invert this matrix
    void invert() {
        mat = simd_inverse(mat);
    }

    /// \brief Get inverted copy
    ofMatrix4x4 getInverse() const {
        return ofMatrix4x4(simd_inverse(mat));
    }

    /// \brief Get determinant
    float determinant() const {
        return simd_determinant(mat);
    }

    // ========================================================================
    // Comparison
    // ========================================================================

    /// \brief Equality test
    bool operator==(const ofMatrix4x4& other) const {
        return simd_equal(mat.columns[0], other.mat.columns[0]) &&
               simd_equal(mat.columns[1], other.mat.columns[1]) &&
               simd_equal(mat.columns[2], other.mat.columns[2]) &&
               simd_equal(mat.columns[3], other.mat.columns[3]);
    }

    /// \brief Inequality test
    bool operator!=(const ofMatrix4x4& other) const {
        return !(*this == other);
    }

    // ========================================================================
    // Setters
    // ========================================================================

    /// \brief Set to identity matrix
    void makeIdentity() {
        mat = matrix_identity_float4x4;
    }

    /// \brief Set translation component (preserves rotation/scale)
    void setTranslation(float x, float y, float z) {
        mat.columns[3].x = x;
        mat.columns[3].y = y;
        mat.columns[3].z = z;
    }

    /// \brief Set translation from vector
    void setTranslation(const ofVec3f& v) {
        setTranslation(v.x, v.y, v.z);
    }

    // ========================================================================
    // Pre-multiplication Methods (for camera-style transforms)
    // ========================================================================

    /// \brief Pre-multiply translation (left side)
    void preMult(const ofMatrix4x4& other) {
        mat = simd_mul(other.mat, mat);
    }

    /// \brief Post-multiply translation (right side, same as operator*)
    void postMult(const ofMatrix4x4& other) {
        mat = simd_mul(mat, other.mat);
    }
};

// ============================================================================
// Global Helper Functions (oF-style API)
// ============================================================================

/// \brief Create identity matrix
inline ofMatrix4x4 ofMatrix4x4Identity() {
    return ofMatrix4x4::identity();
}

/// \brief Create translation matrix
inline ofMatrix4x4 ofMatrix4x4Translation(float x, float y, float z) {
    return ofMatrix4x4::newTranslationMatrix(x, y, z);
}

/// \brief Create rotation matrix
inline ofMatrix4x4 ofMatrix4x4Rotation(float angle, float x, float y, float z) {
    return ofMatrix4x4::newRotationMatrix(angle, x, y, z);
}

/// \brief Create scale matrix
inline ofMatrix4x4 ofMatrix4x4Scale(float x, float y, float z) {
    return ofMatrix4x4::newScaleMatrix(x, y, z);
}

} // namespace oflike
