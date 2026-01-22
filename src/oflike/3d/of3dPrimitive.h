#pragma once

#include "ofNode.h"
#include "ofMesh.h"
#include "../image/ofTexture.h"

/// of3dPrimitive - Base class for 3D primitives (box, sphere, cylinder, etc.)
/// Inherits from ofNode for 3D transformation capabilities
/// Manages internal mesh generation and drawing
class of3dPrimitive : public ofNode {
public:
  of3dPrimitive();
  virtual ~of3dPrimitive();

  // Mesh access
  ofMesh& getMesh();
  const ofMesh& getMesh() const;

  // Drawing
  void draw() const;
  void drawWireframe() const;
  void drawAxes(float size) const;
  void drawNormals(float length, bool split = false) const;

  // Texture coordinate mapping
  void mapTexCoords(float u1, float v1, float u2, float v2);
  void mapTexCoordsFromTexture(const ofTexture& tex);
  void resizeToTexture(const ofTexture& tex, float scale = 1.0f);

  // Primitive mode
  void setMode(ofPrimitiveMode mode);
  ofPrimitiveMode getMode() const;

protected:
  ofMesh mesh_;
};
