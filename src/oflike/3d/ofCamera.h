#pragma once

#include "ofNode.h"
#include "../types/ofRectangle.h"
#include "../math/ofVectorMath.h"
#include <cmath>

// Forward declaration
namespace oflike { class DrawList; }

namespace oflike {

/// ofCamera - 3D camera with View and Projection matrix management
/// Inherits from ofNode for position/rotation/scale hierarchy
class ofCamera : public ofNode {
public:
  ofCamera();
  virtual ~ofCamera() = default;

  // Projection settings
  void setFov(float fov) { fov_ = fov; }
  float getFov() const { return fov_; }
  void setNearClip(float near) { nearClip_ = near; }
  void setFarClip(float far) { farClip_ = far; }
  float getNearClip() const { return nearClip_; }
  float getFarClip() const { return farClip_; }
  void setAspectRatio(float aspectRatio) { aspectRatio_ = aspectRatio; }
  float getAspectRatio() const { return aspectRatio_; }

  // Orthographic projection
  void enableOrtho() { isOrtho_ = true; }
  void disableOrtho() { isOrtho_ = false; }
  bool isOrtho() const { return isOrtho_; }

  // Target management
  void setTarget(const glm::vec3& target);
  void setTarget(ofNode& target);
  glm::vec3 getTarget() const { return target_; }
  bool hasTarget() const { return hasTarget_; }
  float getDistance() const;
  void setDistance(float distance);

  // Matrix calculations
  glm::mat4 getProjectionMatrix() const;
  glm::mat4 getProjectionMatrix(const ofRectangle& viewport) const;
  glm::mat4 getModelViewMatrix() const;
  glm::mat4 getModelViewProjectionMatrix() const;
  glm::mat4 getModelViewProjectionMatrix(const ofRectangle& viewport) const;

  // Coordinate transformations
  glm::vec3 worldToScreen(const glm::vec3& worldPoint) const;
  glm::vec3 worldToScreen(const glm::vec3& worldPoint, const ofRectangle& viewport) const;
  glm::vec3 screenToWorld(const glm::vec3& screenPoint) const;
  glm::vec3 screenToWorld(const glm::vec3& screenPoint, const ofRectangle& viewport) const;

  // Rendering interface - applies camera matrices to DrawList
  void begin();
  void begin(const ofRectangle& viewport);
  void end();

  // Utility functions
  float getImagePlaneDistance(const ofRectangle& viewport) const;
  void drawFrustum(const ofRectangle& viewport) const;

protected:
  float fov_ = 60.0f;           // Field of view in degrees
  float nearClip_ = 0.1f;       // Near clipping plane
  float farClip_ = 10000.0f;    // Far clipping plane
  float aspectRatio_ = 4.0f / 3.0f;
  bool isOrtho_ = false;        // Orthographic vs perspective

  glm::vec3 target_{0, 0, 0};   // Look-at target
  bool hasTarget_ = false;      // Whether target is set

  ofRectangle currentViewport_; // Viewport during begin/end
  bool isActive_ = false;       // Whether begin() has been called

  // Helper: Create perspective projection matrix
  glm::mat4 makePerspectiveMatrix(float fov, float aspect, float near, float far) const;

  // Helper: Create orthographic projection matrix
  glm::mat4 makeOrthoMatrix(float width, float height, float near, float far) const;

  // Helper: Get view matrix (inverse of global transform)
  glm::mat4 getViewMatrix() const;
};

} // namespace oflike
