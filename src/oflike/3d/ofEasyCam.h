#pragma once

// oflike-metal ofEasyCam - Easy-to-use camera with mouse controls
// Extends ofCamera with automatic mouse interaction for orbit/zoom

#include "ofCamera.h"

namespace oflike {

/// \brief Easy-to-use camera with mouse interaction
/// \details ofEasyCam extends ofCamera with automatic mouse controls for
/// orbiting around a target point, zooming with mouse wheel, and optional inertia.
/// This matches the openFrameworks ofEasyCam API.
class ofEasyCam : public ofCamera {
public:
    // ========================================================================
    // Construction / Destruction
    // ========================================================================

    /// \brief Default constructor
    /// Creates an ofEasyCam with default settings:
    /// - Target at origin (0, 0, 0)
    /// - Distance: 100 units
    /// - Mouse input enabled
    /// - Inertia disabled
    ofEasyCam();

    /// \brief Destructor
    ~ofEasyCam();

    // ========================================================================
    // Target & Distance
    // ========================================================================

    /// \brief Set the distance from target
    /// \param distance Distance in world units
    void setDistance(float distance);

    /// \brief Get the current distance from target
    /// \return Distance in world units
    float getDistance() const;

    /// \brief Set the target position to orbit around
    /// \param target Target position
    void setTarget(const ofVec3f& target);

    /// \brief Set the target position to orbit around
    /// \param x Target X coordinate
    /// \param y Target Y coordinate
    /// \param z Target Z coordinate
    void setTarget(float x, float y, float z);

    /// \brief Get the current target position
    /// \return Target position
    ofVec3f getTarget() const;

    // ========================================================================
    // Auto Distance
    // ========================================================================

    /// \brief Enable/disable auto distance calculation
    /// \details When enabled, automatically adjusts distance based on scene bounds
    /// \param autoDistance True to enable, false to disable
    void setAutoDistance(bool autoDistance);

    /// \brief Check if auto distance is enabled
    /// \return True if auto distance is enabled
    bool getAutoDistance() const;

    // ========================================================================
    // Mouse Input
    // ========================================================================

    /// \brief Enable mouse input for camera control
    /// \details Enables:
    /// - Left drag: Rotate around target
    /// - Right drag: Zoom in/out
    /// - Mouse wheel: Zoom in/out
    void enableMouseInput();

    /// \brief Disable mouse input
    void disableMouseInput();

    /// \brief Check if mouse input is enabled
    /// \return True if mouse input is enabled
    bool getMouseInputEnabled() const;

    // ========================================================================
    // Inertia
    // ========================================================================

    /// \brief Enable inertia for smooth camera motion
    /// \details When enabled, camera continues moving after mouse release
    /// with gradual deceleration
    void enableInertia();

    /// \brief Disable inertia
    void disableInertia();

    /// \brief Check if inertia is enabled
    /// \return True if inertia is enabled
    bool getInertiaEnabled() const;

    /// \brief Set inertia damping factor
    /// \param damping Damping factor (0.0 = no inertia, 1.0 = no damping)
    void setInertiaDamping(float damping);

    /// \brief Get inertia damping factor
    /// \return Damping factor
    float getInertiaDamping() const;

    // ========================================================================
    // Mouse Event Handlers (called by event system)
    // ========================================================================

    /// \brief Handle mouse pressed event
    /// \param x Mouse X coordinate
    /// \param y Mouse Y coordinate
    /// \param button Mouse button (0=left, 1=middle, 2=right)
    void onMousePressed(float x, float y, int button);

    /// \brief Handle mouse dragged event
    /// \param x Mouse X coordinate
    /// \param y Mouse Y coordinate
    /// \param button Mouse button
    void onMouseDragged(float x, float y, int button);

    /// \brief Handle mouse released event
    /// \param x Mouse X coordinate
    /// \param y Mouse Y coordinate
    /// \param button Mouse button
    void onMouseReleased(float x, float y, int button);

    /// \brief Handle mouse scrolled event
    /// \param x Scroll X amount
    /// \param y Scroll Y amount
    void onMouseScrolled(float x, float y);

    // ========================================================================
    // Update
    // ========================================================================

    /// \brief Update camera (call once per frame for inertia)
    void update();

private:
    // ========================================================================
    // Internal State
    // ========================================================================

    /// Target position to orbit around
    ofVec3f target_;

    /// Distance from target
    float distance_;

    /// Auto distance enabled
    bool autoDistance_;

    /// Mouse input enabled
    bool mouseInputEnabled_;

    /// Inertia enabled
    bool inertiaEnabled_;

    /// Inertia damping factor (0.0 = no inertia, 1.0 = no damping)
    float inertiaDamping_;

    // Mouse state
    bool isDragging_;
    int dragButton_;
    float lastMouseX_;
    float lastMouseY_;

    // Rotation state (longitude/latitude around target)
    float longitude_;  // Horizontal rotation (degrees)
    float latitude_;   // Vertical rotation (degrees)

    // Inertia velocity
    float longitudeVel_;
    float latitudeVel_;
    float distanceVel_;

    // ========================================================================
    // Internal Methods
    // ========================================================================

    /// \brief Update camera position based on target, distance, and rotation
    void updateCameraPosition();

    /// \brief Apply inertia to rotation and distance
    void applyInertia();

    /// \brief Update distance automatically based on viewport and FOV
    void updateAutoDistance();
};

} // namespace oflike
