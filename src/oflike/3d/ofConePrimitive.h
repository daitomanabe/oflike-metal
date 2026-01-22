#pragma once

#include "of3dPrimitive.h"

namespace oflike {

// Cone primitive - tapers from base radius to apex (point)
class ofConePrimitive : public of3dPrimitive {
public:
    ofConePrimitive();

    // Set all parameters at once
    void set(float radius, float height, int radiusRes = 12, int heightRes = 6, int capRes = 2);

    // Dimension control
    void setRadius(float radius);
    void setHeight(float height);
    float getRadius() const { return radius_; }
    float getHeight() const { return height_; }

    // Resolution control
    void setResolutionRadius(int radiusRes);
    void setResolutionHeight(int heightRes);
    void setResolutionCap(int capRes);
    int getResolutionRadius() const { return radiusRes_; }
    int getResolutionHeight() const { return heightRes_; }
    int getResolutionCap() const { return capRes_; }

    // Mesh extraction
    ofMesh getCapMesh() const;
    ofMesh getConeMesh() const;

private:
    void regenerateMesh();

    float radius_ = 50.0f;
    float height_ = 100.0f;
    int radiusRes_ = 12;     // Segments around circumference
    int heightRes_ = 6;      // Rings from base to apex
    int capRes_ = 2;         // Radial subdivisions in cap
};

} // namespace oflike
