#pragma once

// =============================================================================
// ofVectorMath.h - GLM-based math types for openFrameworks compatibility
// =============================================================================
// Uses GLM (OpenGL Mathematics) for vector, matrix, and quaternion operations.
// Provides oF-compatible type aliases.
// =============================================================================

// Enable GLM experimental extensions (required for gtx/quaternion, gtx/euler_angles)
#define GLM_ENABLE_EXPERIMENTAL

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

// =============================================================================
// oF-compatible type aliases
// =============================================================================

// Vector types
using ofVec2f = glm::vec2;
using ofVec3f = glm::vec3;
using ofVec4f = glm::vec4;

// Matrix types
using ofMatrix3x3 = glm::mat3;
using ofMatrix4x4 = glm::mat4;

// Quaternion
using ofQuaternion = glm::quat;

// Point (alias for vec3)
using ofPoint = glm::vec3;

// =============================================================================
// Helper functions for oF compatibility
// =============================================================================

// Rotate a 2D vector by angle (radians)
inline glm::vec2 ofVec2fRotated(const glm::vec2& v, float radians) {
  float c = std::cos(radians);
  float s = std::sin(radians);
  return glm::vec2(v.x * c - v.y * s, v.x * s + v.y * c);
}

// =============================================================================
// GLM helper function re-exports for convenience
// =============================================================================

// These are already available via glm:: namespace, but we provide them
// for code that expects them in global scope:
//
//   glm::dot(a, b)
//   glm::cross(a, b)
//   glm::normalize(v)
//   glm::length(v)
//   glm::distance(a, b)
//   glm::mix(a, b, t)      // linear interpolation
//   glm::radians(degrees)
//   glm::degrees(radians)
//   glm::angleAxis(angle, axis)
//   glm::eulerAngles(quat)
//   glm::toMat4(quat)
//   glm::slerp(q1, q2, t)
//
