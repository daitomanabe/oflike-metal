#pragma once

// oflike-metal ofCamera - openFrameworks API compatible camera class
// Provides view and projection matrices for 3D rendering

#include "../math/ofVec3f.h"
#include "../math/ofMatrix4x4.h"
#include "../math/ofQuaternion.h"

namespace oflike {

/// \brief 3D camera class for rendering
/// \details ofCamera provides an openFrameworks-compatible camera interface
/// with position, orientation, and projection control. Supports both perspective
/// and orthographic projections.
class ofCamera {
public:
    // ========================================================================
    // Construction / Destruction
    // ========================================================================

    /// \brief Default constructor
    /// Creates a camera with default settings:
    /// - Position at origin (0, 0, 0)
    /// - Looking down negative Z axis
    /// - 60° field of view
    /// - Near clip: 0.1, Far clip: 10000
    ofCamera();

    /// \brief Destructor
    ~ofCamera();

    // ========================================================================
    // Begin / End Rendering
    // ========================================================================

    /// \brief Begin rendering with this camera
    /// \details Applies the camera's view and projection matrices to the render context.
    /// Must be paired with end() call.
    void begin();

    /// \brief End rendering with this camera
    /// \details Restores the previous view and projection matrices.
    void end();

    // ========================================================================
    // Position & Orientation
    // ========================================================================

    /// \brief Set the camera position
    /// \param x X coordinate
    /// \param y Y coordinate
    /// \param z Z coordinate
    void setPosition(float x, float y, float z);

    /// \brief Set the camera position
    /// \param p Position vector
    void setPosition(const ofVec3f& p);

    /// \brief Get the camera position
    /// \return Current position
    ofVec3f getPosition() const;

    /// \brief Set the camera orientation as a quaternion
    /// \param q Orientation quaternion
    void setOrientation(const ofQuaternion& q);

    /// \brief Get the camera orientation
    /// \return Current orientation quaternion
    ofQuaternion getOrientation() const;

    /// \brief Point the camera at a target position
    /// \param target The position to look at
    /// \param up The up direction (default: Y-up)
    void lookAt(const ofVec3f& target, const ofVec3f& up = ofVec3f(0, 1, 0));

    /// \brief Point the camera at a target position
    /// \param x Target X coordinate
    /// \param y Target Y coordinate
    /// \param z Target Z coordinate
    /// \param upX Up direction X component (default: 0)
    /// \param upY Up direction Y component (default: 1)
    /// \param upZ Up direction Z component (default: 0)
    void lookAt(float x, float y, float z,
                float upX = 0, float upY = 1, float upZ = 0);

    // ========================================================================
    // Projection Parameters
    // ========================================================================

    /// \brief Set the near clipping plane distance
    /// \param near Distance to near plane (must be > 0)
    void setNearClip(float near);

    /// \brief Get the near clipping plane distance
    /// \return Distance to near plane
    float getNearClip() const;

    /// \brief Set the far clipping plane distance
    /// \param far Distance to far plane (must be > near)
    void setFarClip(float far);

    /// \brief Get the far clipping plane distance
    /// \return Distance to far plane
    float getFarClip() const;

    /// \brief Set the field of view (perspective projection)
    /// \param fov Field of view in degrees (typically 30-90)
    void setFov(float fov);

    /// \brief Get the field of view
    /// \return Field of view in degrees
    float getFov() const;

    /// \brief Set the aspect ratio (width / height)
    /// \param ratio Aspect ratio
    void setAspectRatio(float ratio);

    /// \brief Get the aspect ratio
    /// \return Current aspect ratio
    float getAspectRatio() const;

    // ========================================================================
    // Matrix Access
    // ========================================================================

    /// \brief Get the model-view matrix
    /// \return The view transformation matrix
    ofMatrix4x4 getModelViewMatrix() const;

    /// \brief Get the projection matrix
    /// \return The projection matrix
    ofMatrix4x4 getProjectionMatrix() const;

    /// \brief Get the combined model-view-projection matrix
    /// \return MVP matrix
    ofMatrix4x4 getModelViewProjectionMatrix() const;

    // ========================================================================
    // Screen-World Coordinate Conversion
    // ========================================================================

    /// \brief Convert world coordinates to screen coordinates
    /// \param world World position
    /// \return Screen position (x, y in pixels, z = depth)
    ofVec3f worldToScreen(const ofVec3f& world) const;

    /// \brief Convert screen coordinates to world coordinates
    /// \param screen Screen position (x, y in pixels, z = depth)
    /// \return World position
    ofVec3f screenToWorld(const ofVec3f& screen) const;

    // ========================================================================
    // Additional Getters
    // ========================================================================

    /// \brief Get the forward direction vector (-Z in camera space)
    /// \return Forward direction (normalized)
    ofVec3f getForwardDir() const;

    /// \brief Get the up direction vector (Y in camera space)
    /// \return Up direction (normalized)
    ofVec3f getUpDir() const;

    /// \brief Get the right direction vector (X in camera space)
    /// \return Right direction (normalized)
    ofVec3f getRightDir() const;

private:
    // ========================================================================
    // Internal State
    // ========================================================================

    /// Position in world space
    ofVec3f position_;

    /// Orientation quaternion
    ofQuaternion orientation_;

    /// Field of view in degrees (perspective mode)
    float fov_;

    /// Near clipping plane distance
    float nearClip_;

    /// Far clipping plane distance
    float farClip_;

    /// Aspect ratio (width / height)
    float aspectRatio_;

    /// Whether camera is currently active (between begin/end)
    bool isActive_;

    /// Cached projection matrix (updated when parameters change)
    mutable ofMatrix4x4 projectionMatrix_;

    /// Whether projection matrix needs recalculation
    mutable bool projectionDirty_;

    // ========================================================================
    // Internal Methods
    // ========================================================================

    /// \brief Recalculate projection matrix if dirty
    void updateProjectionMatrix() const;

    /// \brief Calculate view matrix from position and orientation
    ofMatrix4x4 calculateViewMatrix() const;
};

} // namespace oflike
