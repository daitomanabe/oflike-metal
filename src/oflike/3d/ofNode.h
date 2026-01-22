#pragma once

#include "../math/ofVectorMath.h"
#include <vector>

// Forward declaration
class ofNode;

/// ofNode - 3D transformation node with position, rotation, scale, and hierarchy support
class ofNode {
public:
  ofNode();
  virtual ~ofNode();

  // Position
  void setPosition(float x, float y, float z);
  void setPosition(const glm::vec3& p);
  void setGlobalPosition(const glm::vec3& p);
  glm::vec3 getPosition() const { return position_; }
  glm::vec3 getGlobalPosition() const;
  float getX() const { return position_.x; }
  float getY() const { return position_.y; }
  float getZ() const { return position_.z; }

  // Rotation
  void setOrientation(const glm::quat& q);
  void setOrientation(const glm::vec3& eulerAngles);
  glm::quat getOrientationQuat() const { return orientation_; }
  glm::vec3 getOrientationEuler() const { return glm::eulerAngles(orientation_); }

  // Rotation operations
  void rotateDeg(float degrees, const glm::vec3& axis);
  void rotateDeg(float degrees, float axisX, float axisY, float axisZ);
  void rotateRad(float radians, const glm::vec3& axis);
  void tiltDeg(float degrees);   // X-axis rotation
  void panDeg(float degrees);    // Y-axis rotation
  void rollDeg(float degrees);   // Z-axis rotation

  // Scale
  void setScale(float s);
  void setScale(float x, float y, float z);
  void setScale(const glm::vec3& s);
  glm::vec3 getScale() const { return scale_; }

  // Direction vectors
  glm::vec3 getXAxis() const;
  glm::vec3 getYAxis() const;
  glm::vec3 getZAxis() const;
  glm::vec3 getLookAtDir() const { return getZAxis() * -1.0f; }

  // Look at
  void lookAt(const glm::vec3& target);
  void lookAt(const glm::vec3& target, const glm::vec3& up);
  void lookAt(const ofNode& target);

  // Matrices
  glm::mat4 getLocalTransformMatrix() const;
  glm::mat4 getGlobalTransformMatrix() const;

  // Hierarchy
  void setParent(ofNode& parent, bool keepGlobalTransform = false);
  void clearParent(bool keepGlobalTransform = false);
  ofNode* getParent() const { return parent_; }

  // Transform application (for use with rendering)
  void transformGL() const;
  void restoreTransformGL() const;

  // Drawing
  void draw();
  virtual void customDraw();

protected:
  glm::vec3 position_{0, 0, 0};
  glm::quat orientation_{1, 0, 0, 0};  // Identity quaternion
  glm::vec3 scale_{1, 1, 1};
  ofNode* parent_ = nullptr;
};
