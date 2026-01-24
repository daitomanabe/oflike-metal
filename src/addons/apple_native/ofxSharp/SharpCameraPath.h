#pragma once

// SharpCameraPath - Camera path animation for Gaussian Splatting scenes
//
// This class provides camera path animation with multiple interpolation modes for
// creating cinematic camera movements through 3D Gaussian Splatting scenes. Features:
// - Orbit: Circular motion around a target point
// - Dolly: Linear motion along a path
// - Spiral: Helical motion combining orbit and vertical movement
// - Keyframe: Custom animation with spline interpolation
// - Smooth interpolation (Catmull-Rom, Bezier)
// - Time-based or frame-based playback
// - Camera orientation control (lookAt target)
//
// Usage:
//   Sharp::CameraPath path;
//   path.setOrbitPath(center, radius, duration);
//   path.update(deltaTime);
//   path.applyToCamera(camera);
//
// or with keyframes:
//   path.addKeyframe(0.0f, position1, target1);
//   path.addKeyframe(2.0f, position2, target2);
//   path.addKeyframe(4.0f, position3, target3);
//   path.setInterpolationMode(Sharp::InterpolationMode::CatmullRom);
//   path.play();

#include "math/Types.h"
#include <vector>
#include <memory>

// Forward declarations
namespace oflike {
    class ofCamera;
    class ofVec3f;
}

namespace Sharp {

// ============================================================================
// Enums
// ============================================================================

/// Path type
enum class PathType {
    Orbit,      ///< Circular orbit around a point
    Dolly,      ///< Linear dolly along control points
    Spiral,     ///< Helical spiral path
    Keyframe    ///< Custom keyframe animation
};

/// Interpolation mode for keyframe animation
enum class InterpolationMode {
    Linear,      ///< Linear interpolation
    CatmullRom,  ///< Smooth Catmull-Rom spline
    Bezier       ///< Cubic Bezier curves
};

/// Playback mode
enum class PlaybackMode {
    Once,        ///< Play once and stop
    Loop,        ///< Loop continuously
    PingPong     ///< Play forward then backward
};

// ============================================================================
// Keyframe Structure
// ============================================================================

/// Camera keyframe
struct CameraKeyframe {
    float time;                    ///< Time in seconds
    oflike::float3 position;       ///< Camera position
    oflike::float3 target;         ///< LookAt target position
    float fov;                     ///< Field of view (degrees)

    CameraKeyframe()
        : time(0.0f)
        , position{0.0f, 0.0f, 0.0f}
        , target{0.0f, 0.0f, -1.0f}
        , fov(60.0f)
    {}

    CameraKeyframe(float t, const oflike::float3& pos, const oflike::float3& tgt, float f = 60.0f)
        : time(t)
        , position(pos)
        , target(tgt)
        , fov(f)
    {}
};

// ============================================================================
// CameraPath Class
// ============================================================================

class CameraPath {
public:
    // ========================================================================
    // Construction / Destruction
    // ========================================================================

    CameraPath();
    ~CameraPath();

    // Move semantics
    CameraPath(CameraPath&& other) noexcept;
    CameraPath& operator=(CameraPath&& other) noexcept;

    // Disable copy
    CameraPath(const CameraPath&) = delete;
    CameraPath& operator=(const CameraPath&) = delete;

    // ========================================================================
    // Path Configuration
    // ========================================================================

    /**
     * Configure an orbit path.
     * @param center Center point to orbit around
     * @param radius Orbit radius
     * @param duration Duration in seconds for one complete orbit
     * @param axis Rotation axis (default: Y-up)
     * @param startAngle Starting angle in radians (default: 0)
     */
    void setOrbitPath(const oflike::float3& center,
                      float radius,
                      float duration,
                      const oflike::float3& axis = {0.0f, 1.0f, 0.0f},
                      float startAngle = 0.0f);

    /**
     * Configure a dolly path (linear motion through control points).
     * @param controlPoints Path control points
     * @param duration Duration in seconds to traverse the entire path
     * @param closed Whether path is closed (loops back to start)
     */
    void setDollyPath(const std::vector<oflike::float3>& controlPoints,
                      float duration,
                      bool closed = false);

    /**
     * Configure a spiral path.
     * @param center Base center point
     * @param radius Horizontal radius
     * @param height Vertical height change
     * @param revolutions Number of complete revolutions
     * @param duration Duration in seconds
     * @param axis Rotation axis (default: Y-up)
     */
    void setSpiralPath(const oflike::float3& center,
                       float radius,
                       float height,
                       float revolutions,
                       float duration,
                       const oflike::float3& axis = {0.0f, 1.0f, 0.0f});

    // ========================================================================
    // Keyframe Animation
    // ========================================================================

    /**
     * Add a camera keyframe.
     * Automatically switches to Keyframe path type.
     * @param time Time in seconds
     * @param position Camera position
     * @param target LookAt target
     * @param fov Field of view (default: 60)
     */
    void addKeyframe(float time,
                     const oflike::float3& position,
                     const oflike::float3& target,
                     float fov = 60.0f);

    /**
     * Add a camera keyframe (full struct).
     */
    void addKeyframe(const CameraKeyframe& keyframe);

    /**
     * Clear all keyframes.
     */
    void clearKeyframes();

    /**
     * Get number of keyframes.
     */
    size_t getKeyframeCount() const;

    /**
     * Get keyframe at index.
     */
    const CameraKeyframe& getKeyframe(size_t index) const;

    /**
     * Set keyframe at index.
     */
    void setKeyframe(size_t index, const CameraKeyframe& keyframe);

    /**
     * Remove keyframe at index.
     */
    void removeKeyframe(size_t index);

    /**
     * Sort keyframes by time (automatically called when needed).
     */
    void sortKeyframes();

    // ========================================================================
    // Interpolation
    // ========================================================================

    /**
     * Set interpolation mode (for keyframe animation).
     */
    void setInterpolationMode(InterpolationMode mode);

    /**
     * Get interpolation mode.
     */
    InterpolationMode getInterpolationMode() const;

    // ========================================================================
    // LookAt Target
    // ========================================================================

    /**
     * Set fixed lookAt target (applies to orbit, dolly, spiral).
     * For keyframe animation, use per-keyframe targets instead.
     */
    void setLookAtTarget(const oflike::float3& target);

    /**
     * Get lookAt target.
     */
    oflike::float3 getLookAtTarget() const;

    /**
     * Enable/disable fixed lookAt (if disabled, camera maintains orientation).
     */
    void setLookAtEnabled(bool enabled);

    /**
     * Check if lookAt is enabled.
     */
    bool isLookAtEnabled() const;

    // ========================================================================
    // Playback Control
    // ========================================================================

    /**
     * Start playback from the beginning.
     */
    void play();

    /**
     * Pause playback.
     */
    void pause();

    /**
     * Stop playback and reset to beginning.
     */
    void stop();

    /**
     * Check if currently playing.
     */
    bool isPlaying() const;

    /**
     * Set playback mode.
     */
    void setPlaybackMode(PlaybackMode mode);

    /**
     * Get playback mode.
     */
    PlaybackMode getPlaybackMode() const;

    /**
     * Set playback speed multiplier (1.0 = normal speed).
     */
    void setSpeed(float speed);

    /**
     * Get playback speed.
     */
    float getSpeed() const;

    // ========================================================================
    // Time Control
    // ========================================================================

    /**
     * Update path animation.
     * @param deltaTime Time elapsed since last update (seconds)
     */
    void update(float deltaTime);

    /**
     * Set current time directly.
     * @param time Time in seconds
     */
    void setTime(float time);

    /**
     * Get current time.
     */
    float getTime() const;

    /**
     * Get total duration.
     */
    float getDuration() const;

    /**
     * Get normalized time (0.0 - 1.0).
     */
    float getNormalizedTime() const;

    /**
     * Set normalized time (0.0 - 1.0).
     */
    void setNormalizedTime(float t);

    // ========================================================================
    // Camera Application
    // ========================================================================

    /**
     * Apply current path state to a camera.
     * Updates camera position, orientation, and FOV.
     */
    void applyToCamera(oflike::ofCamera& camera) const;

    /**
     * Get current camera position on the path.
     */
    oflike::float3 getPosition() const;

    /**
     * Get current camera target (lookAt point).
     */
    oflike::float3 getTarget() const;

    /**
     * Get current field of view.
     */
    float getFov() const;

    // ========================================================================
    // Path Info
    // ========================================================================

    /**
     * Get current path type.
     */
    PathType getPathType() const;

    /**
     * Check if path is configured.
     */
    bool isConfigured() const;

private:
    // pImpl pattern to hide implementation details
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace Sharp
