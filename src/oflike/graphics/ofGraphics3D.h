#pragma once

// 3D drawing function implementations
// Include this AFTER ofMesh.h and primitive headers to avoid circular dependencies

#include "../3d/ofBoxPrimitive.h"
#include "../3d/ofSpherePrimitive.h"
#include "../3d/ofConePrimitive.h"
#include "../3d/ofCylinderPrimitive.h"
#include "../3d/ofPlanePrimitive.h"
#include "ofGraphics.h"

// ============== Sphere ==============

inline void ofDrawSphere(float x, float y, float z, float radius) {
  ofPushMatrix();
  ofTranslate(x, y, z);
  ofSpherePrimitive sphere(radius, 32);
  sphere.draw();
  ofPopMatrix();
}

inline void ofDrawSphere(const glm::vec3& position, float radius) {
  ofDrawSphere(position.x, position.y, position.z, radius);
}

inline void ofDrawSphere(float radius) {
  ofDrawSphere(0, 0, 0, radius);
}

// ============== Box ==============

inline void ofDrawBox(float x, float y, float z, float size) {
  ofPushMatrix();
  ofTranslate(x, y, z);
  ofBoxPrimitive box(size);
  box.draw();
  ofPopMatrix();
}

inline void ofDrawBox(float x, float y, float z, float width, float height, float depth) {
  ofPushMatrix();
  ofTranslate(x, y, z);
  ofBoxPrimitive box(width, height, depth);
  box.draw();
  ofPopMatrix();
}

inline void ofDrawBox(const glm::vec3& position, float size) {
  ofDrawBox(position.x, position.y, position.z, size);
}

inline void ofDrawBox(float size) {
  ofDrawBox(0, 0, 0, size);
}

// ============== Cone ==============

inline void ofDrawCone(float x, float y, float z, float radius, float height) {
  ofPushMatrix();
  ofTranslate(x, y, z);
  oflike::ofConePrimitive cone;
  cone.set(radius, height);
  cone.draw();
  ofPopMatrix();
}

inline void ofDrawCone(const glm::vec3& position, float radius, float height) {
  ofDrawCone(position.x, position.y, position.z, radius, height);
}

// ============== Cylinder ==============

inline void ofDrawCylinder(float x, float y, float z, float radius, float height) {
  ofPushMatrix();
  ofTranslate(x, y, z);
  oflike::ofCylinderPrimitive cylinder;
  cylinder.set(radius, height);
  cylinder.draw();
  ofPopMatrix();
}

inline void ofDrawCylinder(const glm::vec3& position, float radius, float height) {
  ofDrawCylinder(position.x, position.y, position.z, radius, height);
}

// ============== Plane ==============

inline void ofDrawPlane(float x, float y, float z, float width, float height) {
  ofPushMatrix();
  ofTranslate(x, y, z);
  oflike::ofPlanePrimitive plane;
  plane.set(width, height);
  plane.draw();
  ofPopMatrix();
}
