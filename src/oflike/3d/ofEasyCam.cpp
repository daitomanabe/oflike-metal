#include "ofEasyCam.h"
#include <cmath>

namespace oflike {

// ========================================================================
// Construction / Destruction
// ========================================================================

ofEasyCam::ofEasyCam()
    : ofCamera()
    , target_(0, 0, 0)
    , distance_(100.0f)
    , autoDistance_(false)
    , mouseInputEnabled_(true)
    , inertiaEnabled_(false)
    , inertiaDamping_(0.9f)
    , isDragging_(false)
    , dragButton_(-1)
    , lastMouseX_(0)
    , lastMouseY_(0)
    , longitude_(0)
    , latitude_(0)
    , longitudeVel_(0)
    , latitudeVel_(0)
    , distanceVel_(0)
{
    // Set initial camera position based on target and distance
    updateCameraPosition();
}

ofEasyCam::~ofEasyCam() {
}

// ========================================================================
// Target & Distance
// ========================================================================

void ofEasyCam::setDistance(float distance) {
    distance_ = distance;
    updateCameraPosition();
}

float ofEasyCam::getDistance() const {
    return distance_;
}

void ofEasyCam::setTarget(const ofVec3f& target) {
    target_ = target;
    updateCameraPosition();
}

void ofEasyCam::setTarget(float x, float y, float z) {
    setTarget(ofVec3f(x, y, z));
}

ofVec3f ofEasyCam::getTarget() const {
    return target_;
}

// ========================================================================
// Auto Distance
// ========================================================================

void ofEasyCam::setAutoDistance(bool autoDistance) {
    autoDistance_ = autoDistance;
    // TODO: Implement auto distance calculation based on scene bounds
}

bool ofEasyCam::getAutoDistance() const {
    return autoDistance_;
}

// ========================================================================
// Mouse Input
// ========================================================================

void ofEasyCam::enableMouseInput() {
    mouseInputEnabled_ = true;
}

void ofEasyCam::disableMouseInput() {
    mouseInputEnabled_ = false;
}

bool ofEasyCam::getMouseInputEnabled() const {
    return mouseInputEnabled_;
}

// ========================================================================
// Inertia
// ========================================================================

void ofEasyCam::enableInertia() {
    inertiaEnabled_ = true;
}

void ofEasyCam::disableInertia() {
    inertiaEnabled_ = false;
    // Reset velocities
    longitudeVel_ = 0;
    latitudeVel_ = 0;
    distanceVel_ = 0;
}

bool ofEasyCam::getInertiaEnabled() const {
    return inertiaEnabled_;
}

void ofEasyCam::setInertiaDamping(float damping) {
    inertiaDamping_ = damping;
}

float ofEasyCam::getInertiaDamping() const {
    return inertiaDamping_;
}

// ========================================================================
// Mouse Event Handlers
// ========================================================================

void ofEasyCam::onMousePressed(float x, float y, int button) {
    if (!mouseInputEnabled_) return;

    isDragging_ = true;
    dragButton_ = button;
    lastMouseX_ = x;
    lastMouseY_ = y;

    // Reset velocities on new drag
    longitudeVel_ = 0;
    latitudeVel_ = 0;
    distanceVel_ = 0;
}

void ofEasyCam::onMouseDragged(float x, float y, int button) {
    if (!mouseInputEnabled_ || !isDragging_) return;

    float deltaX = x - lastMouseX_;
    float deltaY = y - lastMouseY_;

    // Left button (0): Rotate around target
    if (button == 0) {
        // Horizontal rotation (longitude)
        longitude_ -= deltaX * 0.5f;

        // Vertical rotation (latitude)
        latitude_ += deltaY * 0.5f;

        // Clamp latitude to prevent flipping
        if (latitude_ > 89.0f) latitude_ = 89.0f;
        if (latitude_ < -89.0f) latitude_ = -89.0f;

        // Store velocities for inertia
        if (inertiaEnabled_) {
            longitudeVel_ = -deltaX * 0.5f;
            latitudeVel_ = deltaY * 0.5f;
        }

        updateCameraPosition();
    }
    // Right button (2) or middle button (1): Zoom
    else if (button == 1 || button == 2) {
        // Vertical drag controls zoom
        float zoomSpeed = distance_ * 0.01f;
        distance_ += deltaY * zoomSpeed;

        // Clamp distance to reasonable values
        if (distance_ < 1.0f) distance_ = 1.0f;
        if (distance_ > 10000.0f) distance_ = 10000.0f;

        // Store velocity for inertia
        if (inertiaEnabled_) {
            distanceVel_ = deltaY * zoomSpeed;
        }

        updateCameraPosition();
    }

    lastMouseX_ = x;
    lastMouseY_ = y;
}

void ofEasyCam::onMouseReleased(float x, float y, int button) {
    if (!mouseInputEnabled_) return;

    isDragging_ = false;
    dragButton_ = -1;
}

void ofEasyCam::onMouseScrolled(float x, float y) {
    if (!mouseInputEnabled_) return;

    // Mouse wheel controls zoom
    float zoomSpeed = distance_ * 0.1f;
    distance_ -= y * zoomSpeed;

    // Clamp distance
    if (distance_ < 1.0f) distance_ = 1.0f;
    if (distance_ > 10000.0f) distance_ = 10000.0f;

    // Store velocity for inertia
    if (inertiaEnabled_) {
        distanceVel_ = -y * zoomSpeed;
    }

    updateCameraPosition();
}

// ========================================================================
// Update
// ========================================================================

void ofEasyCam::update() {
    if (inertiaEnabled_ && !isDragging_) {
        applyInertia();
    }
}

// ========================================================================
// Internal Methods
// ========================================================================

void ofEasyCam::updateCameraPosition() {
    // Convert longitude/latitude to radians
    float lonRad = longitude_ * 3.14159265f / 180.0f;
    float latRad = latitude_ * 3.14159265f / 180.0f;

    // Calculate camera position in spherical coordinates
    // X: longitude (horizontal rotation)
    // Y: latitude (vertical rotation)
    // Z: distance (radius)
    float cosLat = std::cos(latRad);
    float sinLat = std::sin(latRad);
    float cosLon = std::cos(lonRad);
    float sinLon = std::sin(lonRad);

    // Position relative to target
    float x = distance_ * cosLat * sinLon;
    float y = distance_ * sinLat;
    float z = distance_ * cosLat * cosLon;

    // Set camera position
    setPosition(target_.x + x, target_.y + y, target_.z + z);

    // Look at target
    lookAt(target_);
}

void ofEasyCam::applyInertia() {
    // Apply damping to velocities
    longitudeVel_ *= inertiaDamping_;
    latitudeVel_ *= inertiaDamping_;
    distanceVel_ *= inertiaDamping_;

    // Stop if velocities are very small
    const float threshold = 0.01f;
    if (std::abs(longitudeVel_) < threshold) longitudeVel_ = 0;
    if (std::abs(latitudeVel_) < threshold) latitudeVel_ = 0;
    if (std::abs(distanceVel_) < threshold) distanceVel_ = 0;

    // Update rotation and distance
    bool needsUpdate = false;

    if (longitudeVel_ != 0) {
        longitude_ += longitudeVel_;
        needsUpdate = true;
    }

    if (latitudeVel_ != 0) {
        latitude_ += latitudeVel_;
        // Clamp latitude
        if (latitude_ > 89.0f) {
            latitude_ = 89.0f;
            latitudeVel_ = 0;
        }
        if (latitude_ < -89.0f) {
            latitude_ = -89.0f;
            latitudeVel_ = 0;
        }
        needsUpdate = true;
    }

    if (distanceVel_ != 0) {
        distance_ += distanceVel_;
        // Clamp distance
        if (distance_ < 1.0f) {
            distance_ = 1.0f;
            distanceVel_ = 0;
        }
        if (distance_ > 10000.0f) {
            distance_ = 10000.0f;
            distanceVel_ = 0;
        }
        needsUpdate = true;
    }

    if (needsUpdate) {
        updateCameraPosition();
    }
}

} // namespace oflike
