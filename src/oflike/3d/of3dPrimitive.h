#pragma once

#include "ofNode.h"
#include "ofMesh.h"

namespace oflike {

/// \brief Base class for 3D primitives with mesh support
/// \details of3dPrimitive combines ofNode's transform capabilities with
/// an ofMesh for rendering. Subclasses implement specific geometric shapes.
class of3dPrimitive : public ofNode {
public:
    of3dPrimitive();
    virtual ~of3dPrimitive();

    // ========================================
    // Mesh Access
    // ========================================

    /// \brief Get the mesh for rendering or modification
    ofMesh& getMesh();

    /// \brief Get the mesh (const version)
    const ofMesh& getMesh() const;

    /// \brief Get a pointer to the mesh (for legacy compatibility)
    ofMesh* getMeshPtr();

    // ========================================
    // Drawing
    // ========================================

    /// \brief Draw the primitive with current transform
    virtual void draw() const;

    /// \brief Draw the primitive as wireframe
    virtual void drawWireframe() const;

    /// \brief Draw vertex normals for debugging
    void drawNormals(float length = 10.0f) const;

    /// \brief Draw coordinate axes at the primitive's position
    void drawAxes(float length = 10.0f) const;

    // ========================================
    // Texturing
    // ========================================

    /// \brief Map texture coordinates to mesh vertices
    /// \param u1 Left texture coordinate (0-1)
    /// \param v1 Top texture coordinate (0-1)
    /// \param u2 Right texture coordinate (0-1)
    /// \param v2 Bottom texture coordinate (0-1)
    void mapTexCoords(float u1, float v1, float u2, float v2);

    // ========================================
    // Resolution (subclass implementation)
    // ========================================

    /// \brief Set the resolution/detail level
    /// \param res Resolution value (interpretation depends on subclass)
    virtual void setResolution(int res);

    /// \brief Get the current resolution
    virtual int getResolution() const;

protected:
    ofMesh mesh_;
    int resolution_ = 12;
};

// ============================================================================
// ofBoxPrimitive
// ============================================================================

/// \brief A box/cube primitive
class ofBoxPrimitive : public of3dPrimitive {
public:
    /// \brief Create a unit box
    ofBoxPrimitive();

    /// \brief Create a uniform box
    /// \param size Box size (width = height = depth)
    ofBoxPrimitive(float size);

    /// \brief Create a box with specific dimensions
    /// \param width Box width (X)
    /// \param height Box height (Y)
    /// \param depth Box depth (Z)
    ofBoxPrimitive(float width, float height, float depth);

    /// \brief Set box dimensions
    void set(float width, float height, float depth);

    /// \brief Set uniform box size
    void set(float size);

    float getWidth() const { return width_; }
    float getHeight() const { return height_; }
    float getDepth() const { return depth_; }

    void setWidth(float w);
    void setHeight(float h);
    void setDepth(float d);

    void setResolution(int res) override;

private:
    void buildMesh();
    float width_ = 1.0f;
    float height_ = 1.0f;
    float depth_ = 1.0f;
};

// ============================================================================
// ofSpherePrimitive
// ============================================================================

/// \brief A sphere primitive
class ofSpherePrimitive : public of3dPrimitive {
public:
    /// \brief Create a unit sphere
    ofSpherePrimitive();

    /// \brief Create a sphere with specified radius
    /// \param radius Sphere radius
    /// \param resolution Number of subdivisions (higher = smoother)
    ofSpherePrimitive(float radius, int resolution = 24);

    /// \brief Set sphere radius
    void setRadius(float radius);

    float getRadius() const { return radius_; }

    void setResolution(int res) override;

private:
    void buildMesh();
    float radius_ = 0.5f;
};

// ============================================================================
// ofCylinderPrimitive
// ============================================================================

/// \brief A cylinder primitive
class ofCylinderPrimitive : public of3dPrimitive {
public:
    /// \brief Create a unit cylinder
    ofCylinderPrimitive();

    /// \brief Create a cylinder
    /// \param radius Cylinder radius
    /// \param height Cylinder height
    /// \param radiusSegments Number of radial segments
    /// \param heightSegments Number of height segments
    /// \param bCapped Whether to cap the ends
    ofCylinderPrimitive(float radius, float height,
                        int radiusSegments = 24, int heightSegments = 1,
                        bool bCapped = true);

    void set(float radius, float height, int radiusSegments = 24,
             int heightSegments = 1, bool bCapped = true);

    void setRadius(float radius);
    void setHeight(float height);
    void setCapped(bool capped);

    float getRadius() const { return radius_; }
    float getHeight() const { return height_; }
    bool getCapped() const { return capped_; }

    void setResolution(int res) override;

private:
    void buildMesh();
    float radius_ = 0.5f;
    float height_ = 1.0f;
    int radiusSegments_ = 24;
    int heightSegments_ = 1;
    bool capped_ = true;
};

// ============================================================================
// ofConePrimitive
// ============================================================================

/// \brief A cone primitive
class ofConePrimitive : public of3dPrimitive {
public:
    /// \brief Create a unit cone
    ofConePrimitive();

    /// \brief Create a cone
    /// \param radius Base radius
    /// \param height Cone height
    /// \param radiusSegments Number of radial segments
    /// \param heightSegments Number of height segments
    /// \param bCapped Whether to cap the base
    ofConePrimitive(float radius, float height,
                    int radiusSegments = 24, int heightSegments = 1,
                    bool bCapped = true);

    void set(float radius, float height, int radiusSegments = 24,
             int heightSegments = 1, bool bCapped = true);

    void setRadius(float radius);
    void setHeight(float height);
    void setCapped(bool capped);

    float getRadius() const { return radius_; }
    float getHeight() const { return height_; }
    bool getCapped() const { return capped_; }

    void setResolution(int res) override;

private:
    void buildMesh();
    float radius_ = 0.5f;
    float height_ = 1.0f;
    int radiusSegments_ = 24;
    int heightSegments_ = 1;
    bool capped_ = true;
};

// ============================================================================
// ofPlanePrimitive
// ============================================================================

/// \brief A flat plane primitive
class ofPlanePrimitive : public of3dPrimitive {
public:
    /// \brief Create a unit plane
    ofPlanePrimitive();

    /// \brief Create a plane
    /// \param width Plane width
    /// \param height Plane height
    /// \param columns Number of column subdivisions
    /// \param rows Number of row subdivisions
    ofPlanePrimitive(float width, float height, int columns = 2, int rows = 2);

    void set(float width, float height, int columns = 2, int rows = 2);

    void setWidth(float width);
    void setHeight(float height);

    float getWidth() const { return width_; }
    float getHeight() const { return height_; }
    int getNumColumns() const { return columns_; }
    int getNumRows() const { return rows_; }

    void setResolution(int res) override;

private:
    void buildMesh();
    float width_ = 1.0f;
    float height_ = 1.0f;
    int columns_ = 2;
    int rows_ = 2;
};

// ============================================================================
// ofIcoSpherePrimitive
// ============================================================================

/// \brief An icosphere primitive (geodesic sphere)
class ofIcoSpherePrimitive : public of3dPrimitive {
public:
    /// \brief Create a unit icosphere
    ofIcoSpherePrimitive();

    /// \brief Create an icosphere
    /// \param radius Sphere radius
    /// \param iterations Number of subdivision iterations (0-5)
    ofIcoSpherePrimitive(float radius, int iterations = 2);

    void setRadius(float radius);
    float getRadius() const { return radius_; }

    void setResolution(int iterations) override;
    int getResolution() const override { return iterations_; }

private:
    void buildMesh();
    float radius_ = 0.5f;
    int iterations_ = 2;
};

} // namespace oflike
