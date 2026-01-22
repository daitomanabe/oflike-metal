#include "ofCamera.h"
#include "../../render/DrawList.h"
#include "../../core/Context.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

namespace oflike {

ofCamera::ofCamera()
  : ofNode()
  , fov_(60.0f)
  , nearClip_(0.1f)
  , farClip_(10000.0f)
  , aspectRatio_(4.0f / 3.0f)
  , isOrtho_(false)
  , hasTarget_(false)
  , isActive_(false)
{
}

// Target management
void ofCamera::setTarget(const glm::vec3& target) {
  target_ = target;
  hasTarget_ = true;
  lookAt(target);
}

void ofCamera::setTarget(ofNode& target) {
  setTarget(target.getGlobalPosition());
}

float ofCamera::getDistance() const {
  if (hasTarget_) {
    glm::vec3 pos = getGlobalPosition();
    glm::vec3 diff = target_ - pos;
    return glm::length(diff);
  }
  return 0.0f;
}

void ofCamera::setDistance(float distance) {
  if (hasTarget_) {
    glm::vec3 pos = getGlobalPosition();
    glm::vec3 dir = target_ - pos;
    float currentDist = glm::length(dir);
    if (currentDist > 0) {
      dir = glm::normalize(dir);
      setGlobalPosition(target_ - dir * distance);
    }
  }
}

// Matrix calculations
glm::mat4 ofCamera::makePerspectiveMatrix(float fov, float aspect, float near, float far) const {
  float fovRad = glm::radians(fov);
  return glm::perspective(fovRad, aspect, near, far);
}

glm::mat4 ofCamera::makeOrthoMatrix(float width, float height, float near, float far) const {
  float left = -width / 2.0f;
  float right = width / 2.0f;
  float bottom = -height / 2.0f;
  float top = height / 2.0f;
  return glm::ortho(left, right, bottom, top, near, far);
}

glm::mat4 ofCamera::getViewMatrix() const {
  // View matrix is the inverse of the camera's global transform
  glm::vec3 pos = getGlobalPosition();
  glm::quat orient = getOrientationQuat();

  // Inverse rotation: conjugate of quaternion
  glm::quat invOrient = glm::conjugate(orient);
  glm::mat4 invRotation = glm::mat4_cast(invOrient);

  // Inverse translation: -position rotated by inverse rotation
  glm::vec3 invPos = glm::rotate(invOrient, pos * -1.0f);
  glm::mat4 invTranslation = glm::translate(glm::mat4(1.0f), invPos);

  return invTranslation * invRotation;
}

glm::mat4 ofCamera::getProjectionMatrix() const {
  if (isOrtho_) {
    // Default ortho size
    float width = 1024.0f;
    float height = 768.0f;
    return makeOrthoMatrix(width, height, nearClip_, farClip_);
  } else {
    return makePerspectiveMatrix(fov_, aspectRatio_, nearClip_, farClip_);
  }
}

glm::mat4 ofCamera::getProjectionMatrix(const ofRectangle& viewport) const {
  float aspect = viewport.width / viewport.height;
  if (isOrtho_) {
    return makeOrthoMatrix(viewport.width, viewport.height, nearClip_, farClip_);
  } else {
    return makePerspectiveMatrix(fov_, aspect, nearClip_, farClip_);
  }
}

glm::mat4 ofCamera::getModelViewMatrix() const {
  return getViewMatrix();
}

glm::mat4 ofCamera::getModelViewProjectionMatrix() const {
  return getProjectionMatrix() * getViewMatrix();
}

glm::mat4 ofCamera::getModelViewProjectionMatrix(const ofRectangle& viewport) const {
  return getProjectionMatrix(viewport) * getViewMatrix();
}

// Coordinate transformations
glm::vec3 ofCamera::worldToScreen(const glm::vec3& worldPoint) const {
  return worldToScreen(worldPoint, currentViewport_);
}

glm::vec3 ofCamera::worldToScreen(const glm::vec3& worldPoint, const ofRectangle& viewport) const {
  glm::mat4 mvp = getModelViewProjectionMatrix(viewport);

  // Transform to clip space
  glm::vec4 clipSpace = mvp * glm::vec4(worldPoint, 1.0f);

  // Perspective divide
  if (clipSpace.w != 0) {
    clipSpace /= clipSpace.w;
  }

  // NDC to screen space
  float screenX = viewport.x + (clipSpace.x + 1.0f) * viewport.width * 0.5f;
  float screenY = viewport.y + (1.0f - clipSpace.y) * viewport.height * 0.5f; // Flip Y
  float screenZ = (clipSpace.z + 1.0f) * 0.5f; // Map [-1, 1] to [0, 1]

  return glm::vec3(screenX, screenY, screenZ);
}

glm::vec3 ofCamera::screenToWorld(const glm::vec3& screenPoint) const {
  return screenToWorld(screenPoint, currentViewport_);
}

glm::vec3 ofCamera::screenToWorld(const glm::vec3& screenPoint, const ofRectangle& viewport) const {
  // Screen to NDC
  float x = (screenPoint.x - viewport.x) / viewport.width * 2.0f - 1.0f;
  float y = 1.0f - (screenPoint.y - viewport.y) / viewport.height * 2.0f; // Flip Y
  float z = screenPoint.z * 2.0f - 1.0f;

  // Get inverse MVP
  glm::mat4 mvp = getModelViewProjectionMatrix(viewport);
  glm::mat4 invMvp = glm::inverse(mvp);

  // Transform back to world space
  glm::vec4 worldSpace = invMvp * glm::vec4(x, y, z, 1.0f);

  // Perspective divide
  if (worldSpace.w != 0) {
    worldSpace /= worldSpace.w;
  }

  return glm::vec3(worldSpace);
}

// Rendering interface
void ofCamera::begin() {
  // Use default viewport (get from engine)
  Engine& eng = engine();
  float w = static_cast<float>(eng.getWindowWidth());
  float h = static_cast<float>(eng.getWindowHeight());
  begin(ofRectangle{0, 0, w, h});
}

void ofCamera::begin(const ofRectangle& viewport) {
  currentViewport_ = viewport;
  isActive_ = true;

  Engine& eng = engine();
  DrawList& dl = eng.drawList();

  // Apply camera matrices to DrawList
  dl.setViewMatrix(getViewMatrix());
  dl.setProjectionMatrix(getProjectionMatrix(viewport));
}

void ofCamera::end() {
  if (isActive_) {
    Engine& eng = engine();
    DrawList& dl = eng.drawList();

    // Reset matrices to identity
    dl.setViewMatrix(glm::mat4(1.0f));
    dl.setProjectionMatrix(glm::mat4(1.0f));

    isActive_ = false;
  }
}

// Utility functions
float ofCamera::getImagePlaneDistance(const ofRectangle& viewport) const {
  float fovRad = glm::radians(fov_);
  return viewport.height / (2.0f * std::tan(fovRad / 2.0f));
}

void ofCamera::drawFrustum(const ofRectangle& viewport) const {
  // TODO: Implement frustum visualization
  // This would draw lines representing the camera frustum
  (void)viewport;
}

} // namespace oflike
