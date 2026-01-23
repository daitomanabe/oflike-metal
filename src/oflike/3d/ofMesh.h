#pragma once

// oflike-metal ofMesh - openFrameworks API compatible mesh class
// Core mesh class with vertices, normals, texcoords, colors, indices, and drawing

#include <vector>
#include <memory>
#include <string>
#include "../math/ofVec2f.h"
#include "../math/ofVec3f.h"
#include "../types/ofColor.h"

namespace oflike {

/// \brief Primitive rendering mode for meshes
/// \details Controls how vertices are interpreted during rendering
enum ofPrimitiveMode {
    OF_PRIMITIVE_TRIANGLES = 0,
    OF_PRIMITIVE_TRIANGLE_STRIP = 1,
    OF_PRIMITIVE_TRIANGLE_FAN = 2,
    OF_PRIMITIVE_LINES = 3,
    OF_PRIMITIVE_LINE_STRIP = 4,
    OF_PRIMITIVE_LINE_LOOP = 5,
    OF_PRIMITIVE_POINTS = 6,
};

/// \brief 3D mesh class with vertices, normals, texcoords, colors, and indices
/// \details ofMesh provides an openFrameworks-compatible interface for creating
/// and rendering 3D meshes. Supports indexed and non-indexed rendering, automatic
/// normal generation, and various primitive types.
class ofMesh {
public:
    // ========================================================================
    // Construction / Destruction
    // ========================================================================

    /// \brief Default constructor
    ofMesh();

    /// \brief Construct with a specific primitive mode
    /// \param mode The primitive rendering mode
    ofMesh(ofPrimitiveMode mode);

    /// \brief Destructor
    ~ofMesh();

    // Move semantics
    ofMesh(ofMesh&& other) noexcept;
    ofMesh& operator=(ofMesh&& other) noexcept;

    // Delete copy semantics (meshes can be large)
    ofMesh(const ofMesh&) = delete;
    ofMesh& operator=(const ofMesh&) = delete;

    // ========================================================================
    // Vertex Data Management
    // ========================================================================

    /// \brief Add a vertex to the mesh
    /// \param v The vertex position
    void addVertex(const ofVec3f& v);

    /// \brief Add multiple vertices to the mesh
    /// \param verts Vector of vertex positions
    void addVertices(const std::vector<ofVec3f>& verts);

    /// \brief Add multiple vertices from a raw array
    /// \param verts Array of vertex positions
    /// \param count Number of vertices
    void addVertices(const ofVec3f* verts, size_t count);

    /// \brief Add a normal vector
    /// \param n The normal vector
    void addNormal(const ofVec3f& n);

    /// \brief Add multiple normals to the mesh
    /// \param norms Vector of normal vectors
    void addNormals(const std::vector<ofVec3f>& norms);

    /// \brief Add multiple normals from a raw array
    /// \param norms Array of normal vectors
    /// \param count Number of normals
    void addNormals(const ofVec3f* norms, size_t count);

    /// \brief Add a texture coordinate
    /// \param t The texture coordinate (UV)
    void addTexCoord(const ofVec2f& t);

    /// \brief Add multiple texture coordinates to the mesh
    /// \param tcs Vector of texture coordinates
    void addTexCoords(const std::vector<ofVec2f>& tcs);

    /// \brief Add multiple texture coordinates from a raw array
    /// \param tcs Array of texture coordinates
    /// \param count Number of texture coordinates
    void addTexCoords(const ofVec2f* tcs, size_t count);

    /// \brief Add a vertex color
    /// \param c The color
    void addColor(const ofColor& c);

    /// \brief Add multiple colors to the mesh
    /// \param cols Vector of colors
    void addColors(const std::vector<ofColor>& cols);

    /// \brief Add multiple colors from a raw array
    /// \param cols Array of colors
    /// \param count Number of colors
    void addColors(const ofColor* cols, size_t count);

    // ========================================================================
    // Index Management
    // ========================================================================

    /// \brief Add an index to the index buffer
    /// \param i The vertex index
    void addIndex(uint32_t i);

    /// \brief Add multiple indices to the mesh
    /// \param inds Vector of indices
    void addIndices(const std::vector<uint32_t>& inds);

    /// \brief Add multiple indices from a raw array
    /// \param inds Array of indices
    /// \param count Number of indices
    void addIndices(const uint32_t* inds, size_t count);

    /// \brief Add a triangle by specifying three vertex indices
    /// \param i1 First vertex index
    /// \param i2 Second vertex index
    /// \param i3 Third vertex index
    void addTriangle(uint32_t i1, uint32_t i2, uint32_t i3);

    // ========================================================================
    // Primitive Mode
    // ========================================================================

    /// \brief Set the primitive rendering mode
    /// \param mode The primitive mode
    void setMode(ofPrimitiveMode mode);

    /// \brief Get the current primitive rendering mode
    /// \return The primitive mode
    ofPrimitiveMode getMode() const;

    // ========================================================================
    // Drawing
    // ========================================================================

    /// \brief Draw the mesh
    void draw() const;

    /// \brief Draw the mesh as a wireframe
    void drawWireframe() const;

    /// \brief Draw only the vertices (as points)
    void drawVertices() const;

    /// \brief Draw only the faces (triangles)
    void drawFaces() const;

    // ========================================================================
    // Data Access
    // ========================================================================

    /// \brief Get the vertices
    /// \return Reference to the vertex vector
    std::vector<ofVec3f>& getVertices();

    /// \brief Get the vertices (const)
    /// \return Const reference to the vertex vector
    const std::vector<ofVec3f>& getVertices() const;

    /// \brief Get the normals
    /// \return Reference to the normal vector
    std::vector<ofVec3f>& getNormals();

    /// \brief Get the normals (const)
    /// \return Const reference to the normal vector
    const std::vector<ofVec3f>& getNormals() const;

    /// \brief Get the texture coordinates
    /// \return Reference to the texcoord vector
    std::vector<ofVec2f>& getTexCoords();

    /// \brief Get the texture coordinates (const)
    /// \return Const reference to the texcoord vector
    const std::vector<ofVec2f>& getTexCoords() const;

    /// \brief Get the colors
    /// \return Reference to the color vector
    std::vector<ofColor>& getColors();

    /// \brief Get the colors (const)
    /// \return Const reference to the color vector
    const std::vector<ofColor>& getColors() const;

    /// \brief Get the indices
    /// \return Reference to the index vector
    std::vector<uint32_t>& getIndices();

    /// \brief Get the indices (const)
    /// \return Const reference to the index vector
    const std::vector<uint32_t>& getIndices() const;

    // ========================================================================
    // Utility
    // ========================================================================

    /// \brief Clear all mesh data (vertices, normals, texcoords, colors, indices)
    void clear();

    /// \brief Get the number of vertices
    /// \return Vertex count
    size_t getNumVertices() const;

    /// \brief Get the number of normals
    /// \return Normal count
    size_t getNumNormals() const;

    /// \brief Get the number of texture coordinates
    /// \return Texcoord count
    size_t getNumTexCoords() const;

    /// \brief Get the number of colors
    /// \return Color count
    size_t getNumColors() const;

    /// \brief Get the number of indices
    /// \return Index count
    size_t getNumIndices() const;

    /// \brief Check if the mesh uses indexed rendering
    /// \return True if indices are present
    bool hasIndices() const;

    /// \brief Check if normals are present
    /// \return True if normals exist
    bool hasNormals() const;

    /// \brief Check if texture coordinates are present
    /// \return True if texcoords exist
    bool hasTexCoords() const;

    /// \brief Check if colors are present
    /// \return True if colors exist
    bool hasColors() const;

    // ========================================================================
    // Mesh Operations
    // ========================================================================

    /// \brief Merge duplicate vertices (vertices at the same position)
    /// \details Updates indices accordingly
    void mergeDuplicateVertices();

    /// \brief Automatically generate indices for triangle rendering
    /// \details Creates indices 0, 1, 2, 3, 4, 5, ... for all vertices
    void setupIndicesAuto();

    /// \brief Generate smooth normals by averaging face normals at each vertex
    /// \details Requires vertices and either no indices or valid triangle indices
    void smoothNormals();

    /// \brief Generate flat normals (one normal per triangle face)
    /// \details Creates unique vertices per face, no vertex sharing
    void flatNormals();

    /// \brief Append another mesh to this mesh
    /// \param mesh The mesh to append
    /// \details Combines all vertices, normals, texcoords, colors, and indices
    void append(const ofMesh& mesh);

    // ========================================================================
    // Static Mesh Generators
    // ========================================================================

    /// \brief Create a plane mesh
    /// \param width Plane width
    /// \param height Plane height
    /// \param columns Number of columns (subdivisions in X)
    /// \param rows Number of rows (subdivisions in Y)
    /// \return A plane mesh
    static ofMesh plane(float width, float height, int columns = 2, int rows = 2);

    /// \brief Create a sphere mesh (UV sphere)
    /// \param radius Sphere radius
    /// \param resolution Number of segments (latitude and longitude)
    /// \return A sphere mesh
    static ofMesh sphere(float radius, int resolution = 20);

    /// \brief Create a box mesh
    /// \param width Box width (X dimension)
    /// \param height Box height (Y dimension)
    /// \param depth Box depth (Z dimension)
    /// \return A box mesh
    static ofMesh box(float width, float height, float depth);

    /// \brief Create a cone mesh
    /// \param radius Base radius
    /// \param height Cone height
    /// \param radiusSegments Number of segments around the base
    /// \param heightSegments Number of segments along the height
    /// \return A cone mesh
    static ofMesh cone(float radius, float height, int radiusSegments = 20, int heightSegments = 1);

    /// \brief Create a cylinder mesh
    /// \param radius Cylinder radius
    /// \param height Cylinder height
    /// \param radiusSegments Number of segments around the circumference
    /// \param heightSegments Number of segments along the height
    /// \return A cylinder mesh
    static ofMesh cylinder(float radius, float height, int radiusSegments = 20, int heightSegments = 1);

    /// \brief Create an icosphere mesh (geodesic sphere)
    /// \param radius Sphere radius
    /// \param subdivisions Number of subdivision iterations (0-6)
    /// \return An icosphere mesh
    static ofMesh icosphere(float radius, int subdivisions = 2);

    // ========================================================================
    // File I/O (PLY / OBJ)
    // ========================================================================

    /// \brief Load mesh from a file
    /// \param filename Path to the mesh file (PLY or OBJ)
    /// \return True if loaded successfully
    bool load(const std::string& filename);

    /// \brief Save mesh to a file
    /// \param filename Path to save the mesh (PLY or OBJ)
    /// \return True if saved successfully
    bool save(const std::string& filename) const;

private:
    // Internal data stored in vectors (no pImpl needed - data is simple)
    ofPrimitiveMode mode_;
    std::vector<ofVec3f> vertices_;
    std::vector<ofVec3f> normals_;
    std::vector<ofVec2f> texCoords_;
    std::vector<ofColor> colors_;
    std::vector<uint32_t> indices_;
};

} // namespace oflike
