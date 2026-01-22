#include "of3dPrimitive.h"
#include "../graphics/ofGraphics.h"
#include "../../core/Context.h"

of3dPrimitive::of3dPrimitive() {
  mesh_.setMode(OF_PRIMITIVE_TRIANGLES);
}

of3dPrimitive::~of3dPrimitive() {
}

ofMesh& of3dPrimitive::getMesh() {
  return mesh_;
}

const ofMesh& of3dPrimitive::getMesh() const {
  return mesh_;
}

void of3dPrimitive::draw() const {
  // Apply transformation
  transformGL();

  // Draw mesh
  mesh_.draw();

  // Restore transformation
  restoreTransformGL();
}

void of3dPrimitive::drawWireframe() const {
  // Apply transformation
  transformGL();

  // Draw mesh as wireframe
  mesh_.drawWireframe();

  // Restore transformation
  restoreTransformGL();
}

void of3dPrimitive::drawAxes(float size) const {
  // Apply transformation
  transformGL();

  auto& dl = oflike::engine().drawList();
  auto savedColor = oflike::engine().getStyle().color;

  // X axis - red
  dl.setColor(oflike::Color4f{1.0f, 0.0f, 0.0f, 1.0f});
  dl.addLine(0, 0, size, 0);

  // Y axis - green
  dl.setColor(oflike::Color4f{0.0f, 1.0f, 0.0f, 1.0f});
  dl.addLine(0, 0, 0, size);

  // Z axis - blue (3D - stub for future 3D rendering)
  dl.setColor(oflike::Color4f{0.0f, 0.0f, 1.0f, 1.0f});
  // TODO: Implement 3D line drawing when full 3D pipeline is ready

  // Restore color
  dl.setColor(savedColor);

  // Restore transformation
  restoreTransformGL();
}

void of3dPrimitive::drawNormals(float length, bool split) const {
  // Apply transformation
  transformGL();

  auto& dl = oflike::engine().drawList();
  auto savedColor = oflike::engine().getStyle().color;

  // Draw normal vectors from vertices
  const auto& vertices = mesh_.getVertices();
  const auto& normals = mesh_.getNormals();

  if (normals.size() == vertices.size()) {
    dl.setColor(oflike::Color4f{0.0f, 1.0f, 1.0f, 1.0f}); // Cyan for normals

    for (size_t i = 0; i < vertices.size(); i++) {
      const auto& v = vertices[i];
      const auto& n = normals[i];

      glm::vec3 end = v + n * length;

      // TODO: Implement 3D line drawing when full 3D pipeline is ready
      // For now, draw 2D projection
      dl.addLine(v.x, v.y, end.x, end.y);
    }
  }

  // Restore color
  dl.setColor(savedColor);

  // Restore transformation
  restoreTransformGL();
}

void of3dPrimitive::mapTexCoords(float u1, float v1, float u2, float v2) {
  // Remap texture coordinates to the specified range
  auto& texCoords = mesh_.getTexCoords();

  if (texCoords.empty()) {
    return;
  }

  // Find current min/max
  float minU = texCoords[0].x, maxU = texCoords[0].x;
  float minV = texCoords[0].y, maxV = texCoords[0].y;

  for (const auto& tc : texCoords) {
    if (tc.x < minU) minU = tc.x;
    if (tc.x > maxU) maxU = tc.x;
    if (tc.y < minV) minV = tc.y;
    if (tc.y > maxV) maxV = tc.y;
  }

  float rangeU = maxU - minU;
  float rangeV = maxV - minV;

  if (rangeU < 0.0001f) rangeU = 1.0f;
  if (rangeV < 0.0001f) rangeV = 1.0f;

  // Remap each coordinate
  for (auto& tc : texCoords) {
    float normalizedU = (tc.x - minU) / rangeU;
    float normalizedV = (tc.y - minV) / rangeV;

    tc.x = u1 + normalizedU * (u2 - u1);
    tc.y = v1 + normalizedV * (v2 - v1);
  }
}

void of3dPrimitive::mapTexCoordsFromTexture(const ofTexture& tex) {
  // Map texture coordinates based on texture dimensions
  // Normalized coordinates (0-1) are standard, so this is typically a no-op
  // unless we want to account for non-power-of-two textures
  mapTexCoords(0.0f, 0.0f, 1.0f, 1.0f);
}

void of3dPrimitive::resizeToTexture(const ofTexture& tex, float scale) {
  // Resize the primitive to match texture aspect ratio
  if (!tex.isAllocated()) {
    return;
  }

  float width = static_cast<float>(tex.getWidth()) * scale;
  float height = static_cast<float>(tex.getHeight()) * scale;

  // Scale the primitive to match texture dimensions
  // This is a simple uniform scaling approach
  float aspectRatio = width / height;

  setScale(width, height, (width + height) * 0.5f);
}

void of3dPrimitive::setMode(ofPrimitiveMode mode) {
  mesh_.setMode(mode);
}

ofPrimitiveMode of3dPrimitive::getMode() const {
  return mesh_.getMode();
}
