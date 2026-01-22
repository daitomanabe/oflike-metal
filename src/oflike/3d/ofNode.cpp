#include "ofNode.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

ofNode::ofNode() = default;
ofNode::~ofNode() = default;

// Position
void ofNode::setPosition(float x, float y, float z) {
  position_ = glm::vec3(x, y, z);
}

void ofNode::setPosition(const glm::vec3& p) {
  position_ = p;
}

void ofNode::setGlobalPosition(const glm::vec3& p) {
  if (parent_) {
    // Convert global position to local space
    glm::mat4 parentGlobal = parent_->getGlobalTransformMatrix();
    // For simplicity, we'll just use the translation component
    // A full implementation would require matrix inversion
    glm::vec3 parentPos = parent_->getGlobalPosition();
    position_ = p - parentPos;
  } else {
    position_ = p;
  }
}

glm::vec3 ofNode::getGlobalPosition() const {
  if (parent_) {
    glm::vec3 parentPos = parent_->getGlobalPosition();
    glm::quat parentOri = parent_->getOrientationQuat();
    glm::vec3 parentScale = parent_->getScale();

    // Transform local position to global
    glm::vec3 scaled(position_.x * parentScale.x,
                     position_.y * parentScale.y,
                     position_.z * parentScale.z);
    return parentPos + glm::rotate(parentOri, scaled);
  }
  return position_;
}

// Rotation
void ofNode::setOrientation(const glm::quat& q) {
  orientation_ = q;
}

void ofNode::setOrientation(const glm::vec3& eulerAngles) {
  // GLM expects euler angles in radians
  orientation_ = glm::quat(eulerAngles);
}

void ofNode::rotateDeg(float degrees, const glm::vec3& axis) {
  float radians = degrees * static_cast<float>(M_PI) / 180.0f;
  rotateRad(radians, axis);
}

void ofNode::rotateDeg(float degrees, float axisX, float axisY, float axisZ) {
  rotateDeg(degrees, glm::vec3(axisX, axisY, axisZ));
}

void ofNode::rotateRad(float radians, const glm::vec3& axis) {
  glm::quat rotation = glm::angleAxis(radians, glm::normalize(axis));
  orientation_ = orientation_ * rotation;
}

void ofNode::tiltDeg(float degrees) {
  rotateDeg(degrees, glm::vec3(1, 0, 0));
}

void ofNode::panDeg(float degrees) {
  rotateDeg(degrees, glm::vec3(0, 1, 0));
}

void ofNode::rollDeg(float degrees) {
  rotateDeg(degrees, glm::vec3(0, 0, 1));
}

// Scale
void ofNode::setScale(float s) {
  scale_ = glm::vec3(s, s, s);
}

void ofNode::setScale(float x, float y, float z) {
  scale_ = glm::vec3(x, y, z);
}

void ofNode::setScale(const glm::vec3& s) {
  scale_ = s;
}

// Direction vectors
glm::vec3 ofNode::getXAxis() const {
  return glm::rotate(orientation_, glm::vec3(1, 0, 0));
}

glm::vec3 ofNode::getYAxis() const {
  return glm::rotate(orientation_, glm::vec3(0, 1, 0));
}

glm::vec3 ofNode::getZAxis() const {
  return glm::rotate(orientation_, glm::vec3(0, 0, 1));
}

// Look at
void ofNode::lookAt(const glm::vec3& target) {
  lookAt(target, glm::vec3(0, 1, 0));
}

void ofNode::lookAt(const glm::vec3& target, const glm::vec3& up) {
  glm::vec3 direction = glm::normalize(target - getGlobalPosition());
  if (glm::length(direction) < 0.001f) return;

  glm::vec3 right = glm::normalize(glm::cross(up, direction));
  glm::vec3 newUp = glm::normalize(glm::cross(direction, right));

  // Build rotation matrix from basis vectors
  // This is a simplified implementation
  // In practice, you'd convert the basis vectors to a quaternion

  // For now, just rotate to face the target
  glm::vec3 forward(0, 0, -1);
  glm::vec3 axis = glm::cross(forward, direction);
  float dotProduct = glm::dot(forward, direction);
  float angle = std::acos(glm::clamp(dotProduct, -1.0f, 1.0f));

  if (glm::length(axis) > 0.001f) {
    orientation_ = glm::angleAxis(angle, glm::normalize(axis));
  }
}

void ofNode::lookAt(const ofNode& target) {
  lookAt(target.getGlobalPosition());
}

// Matrices
glm::mat4 ofNode::getLocalTransformMatrix() const {
  glm::mat4 translation = glm::translate(glm::mat4(1.0f), position_);
  glm::mat4 rotation = glm::mat4_cast(orientation_);
  glm::mat4 scale = glm::scale(glm::mat4(1.0f), scale_);

  // TRS order: translation * rotation * scale
  return translation * rotation * scale;
}

glm::mat4 ofNode::getGlobalTransformMatrix() const {
  glm::mat4 local = getLocalTransformMatrix();
  if (parent_) {
    return parent_->getGlobalTransformMatrix() * local;
  }
  return local;
}

// Hierarchy
void ofNode::setParent(ofNode& parent, bool keepGlobalTransform) {
  glm::vec3 globalPos;
  glm::quat globalOri;
  glm::vec3 globalScale;

  if (keepGlobalTransform) {
    globalPos = getGlobalPosition();
    globalOri = orientation_; // Simplified - should get global orientation
    globalScale = scale_;
  }

  parent_ = &parent;

  if (keepGlobalTransform) {
    setGlobalPosition(globalPos);
    // Note: This is simplified - proper implementation would transform
    // orientation and scale to parent's local space
  }
}

void ofNode::clearParent(bool keepGlobalTransform) {
  if (!parent_) return;

  glm::vec3 globalPos;
  glm::quat globalOri;
  glm::vec3 globalScale;

  if (keepGlobalTransform) {
    globalPos = getGlobalPosition();
    globalOri = orientation_; // Simplified
    globalScale = scale_;
  }

  parent_ = nullptr;

  if (keepGlobalTransform) {
    position_ = globalPos;
    orientation_ = globalOri;
    scale_ = globalScale;
  }
}

// Transform application (placeholder for rendering integration)
void ofNode::transformGL() const {
  // This will be integrated with the DrawList transform stack
  // For now, it's a placeholder
}

void ofNode::restoreTransformGL() const {
  // This will be integrated with the DrawList transform stack
  // For now, it's a placeholder
}

// Drawing
void ofNode::draw() {
  transformGL();
  customDraw();
  restoreTransformGL();
}

void ofNode::customDraw() {
  // Override in subclasses
}
