#pragma once

#include "of3dPrimitive.h"

/// ofBoxPrimitive - Box/cube primitive
/// Inherits from of3dPrimitive, generates box mesh with configurable dimensions and resolution
class ofBoxPrimitive : public of3dPrimitive {
public:
  enum Side {
    SIDE_FRONT,
    SIDE_RIGHT,
    SIDE_LEFT,
    SIDE_BACK,
    SIDE_TOP,
    SIDE_BOTTOM,
    SIDES_TOTAL
  };

  ofBoxPrimitive();
  ofBoxPrimitive(float size);
  ofBoxPrimitive(float width, float height, float depth);

  // Dimensions
  void set(float size);
  void set(float width, float height, float depth);
  void setWidth(float w);
  void setHeight(float h);
  void setDepth(float d);

  float getWidth() const;
  float getHeight() const;
  float getDepth() const;

  // Resolution
  void setResolution(int res);
  void setResolutionWidth(int res);
  void setResolutionHeight(int res);
  void setResolutionDepth(int res);
  int getResolutionWidth() const;
  int getResolutionHeight() const;
  int getResolutionDepth() const;

  // Side mesh extraction
  ofMesh getSideMesh(int side) const;

private:
  float width_;
  float height_;
  float depth_;
  int resolutionWidth_;
  int resolutionHeight_;
  int resolutionDepth_;

  void generateMesh();
};
