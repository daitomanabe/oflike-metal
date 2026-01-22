#include "ofEasyCam.h"

namespace oflike {

ofEasyCam::ofEasyCam()
  : ofCamera()
  , bMouseInputEnabled_(true)
  , drag_(0.9f)
  , lastMouse_(0, 0, 0)
  , bIsMousePressed_(false)
  , lastButton_(-1)
  , rotationSensitivity_(1.0f, 1.0f, 1.0f)
  , translationSensitivity_(1.0f, 1.0f, 1.0f)
{
  // Default distance for easy camera
  setDistance(100.0f);
}

void ofEasyCam::enableMouseInput() {
  bMouseInputEnabled_ = true;
}

void ofEasyCam::disableMouseInput() {
  bMouseInputEnabled_ = false;
}

void ofEasyCam::setRotationSensitivity(float x, float y, float z) {
  rotationSensitivity_ = glm::vec3(x, y, z);
}

void ofEasyCam::setRotationSensitivity(const glm::vec3& sensitivity) {
  rotationSensitivity_ = sensitivity;
}

void ofEasyCam::setTranslationSensitivity(float x, float y, float z) {
  translationSensitivity_ = glm::vec3(x, y, z);
}

void ofEasyCam::setTranslationSensitivity(const glm::vec3& sensitivity) {
  translationSensitivity_ = sensitivity;
}

void ofEasyCam::mousePressed(int x, int y, int button) {
  if (!bMouseInputEnabled_) return;

  lastMouse_ = glm::vec3(x, y, 0);
  bIsMousePressed_ = true;
  lastButton_ = button;
}

void ofEasyCam::mouseReleased(int x, int y, int button) {
  if (!bMouseInputEnabled_) return;

  bIsMousePressed_ = false;
  lastButton_ = -1;
}

void ofEasyCam::mouseDragged(int x, int y, int button) {
  if (!bMouseInputEnabled_ || !bIsMousePressed_) return;

  // Calculate mouse delta
  float deltaX = x - lastMouse_.x;
  float deltaY = y - lastMouse_.y;

  // Apply drag rotation
  updateRotation(deltaX, deltaY);

  // Update last mouse position
  lastMouse_ = glm::vec3(x, y, 0);
}

void ofEasyCam::mouseScrolled(float x, float y) {
  if (!bMouseInputEnabled_) return;

  // Scroll Y controls zoom
  updateZoom(y);
}

void ofEasyCam::updateRotation(float deltaX, float deltaY) {
  // Apply rotation sensitivity
  deltaX *= rotationSensitivity_.x;
  deltaY *= rotationSensitivity_.y;

  // Convert pixel deltas to radians (scale factor for reasonable rotation speed)
  float rotationSpeed = 0.005f; // radians per pixel
  float angleX = deltaX * rotationSpeed;
  float angleY = deltaY * rotationSpeed;

  // Get current orientation
  glm::quat currentOrientation = getOrientationQuat();

  // Rotate around world Y axis (horizontal mouse movement)
  glm::quat rotationY = glm::angleAxis(-angleX, glm::vec3(0, 1, 0));

  // Rotate around local X axis (vertical mouse movement)
  glm::vec3 rightAxis = getXAxis();
  glm::quat rotationX = glm::angleAxis(-angleY, rightAxis);

  // Combine rotations
  glm::quat newOrientation = rotationY * currentOrientation * rotationX;
  setOrientation(newOrientation);

  // If we have a target, orbit around it
  if (hasTarget()) {
    glm::vec3 targetPos = getTarget();
    float distance = getDistance();

    // Calculate new position based on orientation and distance
    glm::vec3 forward = getLookAtDir();
    glm::vec3 newPos = targetPos - forward * distance;
    setPosition(newPos);
  }
}

void ofEasyCam::updateZoom(float scrollY) {
  // Get current distance from target
  float currentDistance = getDistance();

  // Apply zoom sensitivity and drag
  float zoomSpeed = 5.0f; // distance change per scroll unit
  float distanceDelta = -scrollY * zoomSpeed * drag_;

  // Calculate new distance (clamp to positive values)
  float newDistance = currentDistance + distanceDelta;
  if (newDistance < nearClip_) {
    newDistance = nearClip_;
  }
  if (newDistance > farClip_) {
    newDistance = farClip_;
  }

  // Update distance (this will move camera along look direction)
  setDistance(newDistance);
}

} // namespace oflike
