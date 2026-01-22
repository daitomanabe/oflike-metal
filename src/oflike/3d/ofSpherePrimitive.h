#pragma once

#include "of3dPrimitive.h"

/// ofSpherePrimitive - Sphere primitive
/// Inherits from of3dPrimitive, generates sphere mesh using UV sphere algorithm
class ofSpherePrimitive : public of3dPrimitive {
public:
  ofSpherePrimitive();
  ofSpherePrimitive(float radius, int resolution);

  // Dimensions
  void set(float radius, int resolution);
  void setRadius(float radius);
  float getRadius() const;

  // Resolution (tesselation quality)
  void setResolution(int resolution);
  int getResolution() const;

private:
  float radius_;
  int resolution_;

  void generateMesh();
};
