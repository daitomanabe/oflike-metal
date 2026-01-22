#pragma once

#include "of3dPrimitive.h"

namespace oflike {

// Cylinder primitive
class ofCylinderPrimitive : public of3dPrimitive {
public:
    ofCylinderPrimitive();

    // Set all parameters at once
    void set(float radius, float height, int radiusRes = 12, int heightRes = 6, int capRes = 2, bool capped = true);

    // Dimension control
    void setRadius(float radius);
    void setHeight(float height);
    float getRadius() const { return radius_; }
    float getHeight() const { return height_; }

    // Resolution control
    void setResolutionRadius(int res);
    void setResolutionHeight(int res);
    void setResolutionCap(int res);
    int getResolutionRadius() const { return radiusRes_; }
    int getResolutionHeight() const { return heightRes_; }
    int getResolutionCap() const { return capRes_; }

    // Mesh extraction
    ofMesh getTopCapMesh() const;
    ofMesh getBottomCapMesh() const;
    ofMesh getCylinderMesh() const;

private:
    float radius_;
    float height_;
    int radiusRes_;
    int heightRes_;
    int capRes_;
    bool capped_;

    void regenerateMesh();
};

} // namespace oflike
