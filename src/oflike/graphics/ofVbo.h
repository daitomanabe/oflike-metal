#pragma once

// oflike-metal ofVbo - openFrameworks API compatible Vertex Buffer Object
// Provides GPU-resident vertex data management for efficient rendering

#include <memory>
#include <vector>
#include "../math/ofVec2f.h"
#include "../math/ofVec3f.h"
#include "../types/ofColor.h"

namespace oflike {

// Forward declarations
class ofMesh;

/// \brief GPU Vertex Buffer Object for efficient rendering
/// \details ofVbo provides openFrameworks-compatible VBO operations using
/// Metal buffers. Supports vertex positions, normals, colors, and texture
/// coordinates with optional index buffers.
///
/// Features:
/// - GPU-resident vertex data storage
/// - Separate buffers for positions, normals, colors, texCoords
/// - Index buffer support for indexed drawing
/// - Dynamic or static usage hints
/// - Interleaved or separate attribute layouts
///
/// Implementation:
/// - Uses pImpl pattern to hide Metal/Objective-C++ details
/// - Internally uses MTLBuffer for GPU storage
/// - Supports both immediate upload and deferred updates
///
/// Example:
/// \code
///     ofVbo vbo;
///
///     // Setup vertex data
///     vector<ofVec3f> vertices = {...};
///     vector<ofVec3f> normals = {...};
///     vector<unsigned int> indices = {...};
///
///     vbo.setVertexData(vertices);
///     vbo.setNormalData(normals);
///     vbo.setIndexData(indices);
///
///     // Draw
///     vbo.draw(OF_MESH_FILL, 0, vertices.size());
/// \endcode
class ofVbo {
public:
    // ========================================================================
    // Construction / Destruction
    // ========================================================================

    /// \brief Default constructor
    ofVbo();

    /// \brief Destructor
    ~ofVbo();

    /// \brief Move constructor
    ofVbo(ofVbo&& other) noexcept;

    /// \brief Move assignment
    ofVbo& operator=(ofVbo&& other) noexcept;

    // Delete copy
    ofVbo(const ofVbo&) = delete;
    ofVbo& operator=(const ofVbo&) = delete;

    // ========================================================================
    // Vertex Data
    // ========================================================================

    /// \brief Set vertex position data
    /// \param vertices Pointer to vertex positions
    /// \param total Number of vertices
    /// \param usage GL_STATIC_DRAW or GL_DYNAMIC_DRAW (mapped to Metal)
    void setVertexData(const float* vertices, int total, int usage = 0);

    /// \brief Set vertex position data from ofVec3f array
    /// \param vertices Pointer to vertex positions
    /// \param total Number of vertices
    /// \param usage Usage hint
    void setVertexData(const ofVec3f* vertices, int total, int usage = 0);

    /// \brief Set vertex position data from vector
    /// \param vertices Vector of vertex positions
    /// \param usage Usage hint
    void setVertexData(const std::vector<ofVec3f>& vertices, int usage = 0);

    /// \brief Get number of vertices
    /// \return Number of vertices in buffer
    int getNumVertices() const;

    /// \brief Check if vertex data has been set
    /// \return true if vertex data exists
    bool hasVertices() const;

    // ========================================================================
    // Normal Data
    // ========================================================================

    /// \brief Set normal data
    /// \param normals Pointer to normal vectors
    /// \param total Number of normals
    /// \param usage Usage hint
    void setNormalData(const float* normals, int total, int usage = 0);

    /// \brief Set normal data from ofVec3f array
    /// \param normals Pointer to normal vectors
    /// \param total Number of normals
    /// \param usage Usage hint
    void setNormalData(const ofVec3f* normals, int total, int usage = 0);

    /// \brief Set normal data from vector
    /// \param normals Vector of normal vectors
    /// \param usage Usage hint
    void setNormalData(const std::vector<ofVec3f>& normals, int usage = 0);

    /// \brief Get number of normals
    /// \return Number of normals in buffer
    int getNumNormals() const;

    /// \brief Check if normal data has been set
    /// \return true if normal data exists
    bool hasNormals() const;

    // ========================================================================
    // Color Data
    // ========================================================================

    /// \brief Set color data (RGBA float)
    /// \param colors Pointer to color data (4 floats per color)
    /// \param total Number of colors
    /// \param usage Usage hint
    void setColorData(const float* colors, int total, int usage = 0);

    /// \brief Set color data from ofFloatColor array
    /// \param colors Pointer to colors
    /// \param total Number of colors
    /// \param usage Usage hint
    void setColorData(const ofFloatColor* colors, int total, int usage = 0);

    /// \brief Set color data from vector
    /// \param colors Vector of colors
    /// \param usage Usage hint
    void setColorData(const std::vector<ofFloatColor>& colors, int usage = 0);

    /// \brief Get number of colors
    /// \return Number of colors in buffer
    int getNumColors() const;

    /// \brief Check if color data has been set
    /// \return true if color data exists
    bool hasColors() const;

    // ========================================================================
    // Texture Coordinate Data
    // ========================================================================

    /// \brief Set texture coordinate data
    /// \param texCoords Pointer to texture coordinates (2 floats per coord)
    /// \param total Number of texture coordinates
    /// \param usage Usage hint
    void setTexCoordData(const float* texCoords, int total, int usage = 0);

    /// \brief Set texture coordinate data from ofVec2f array
    /// \param texCoords Pointer to texture coordinates
    /// \param total Number of texture coordinates
    /// \param usage Usage hint
    void setTexCoordData(const ofVec2f* texCoords, int total, int usage = 0);

    /// \brief Set texture coordinate data from vector
    /// \param texCoords Vector of texture coordinates
    /// \param usage Usage hint
    void setTexCoordData(const std::vector<ofVec2f>& texCoords, int usage = 0);

    /// \brief Get number of texture coordinates
    /// \return Number of texture coordinates in buffer
    int getNumTexCoords() const;

    /// \brief Check if texture coordinate data has been set
    /// \return true if texture coordinate data exists
    bool hasTexCoords() const;

    // ========================================================================
    // Index Data
    // ========================================================================

    /// \brief Set index data
    /// \param indices Pointer to index data
    /// \param total Number of indices
    /// \param usage Usage hint
    void setIndexData(const unsigned int* indices, int total, int usage = 0);

    /// \brief Set index data from vector
    /// \param indices Vector of indices
    /// \param usage Usage hint
    void setIndexData(const std::vector<unsigned int>& indices, int usage = 0);

    /// \brief Get number of indices
    /// \return Number of indices in buffer
    int getNumIndices() const;

    /// \brief Check if index data has been set
    /// \return true if index data exists
    bool hasIndices() const;

    // ========================================================================
    // Update Data (for dynamic buffers)
    // ========================================================================

    /// \brief Update a portion of vertex data
    /// \param offset Offset into buffer (in vertices)
    /// \param vertices Pointer to new vertex data
    /// \param count Number of vertices to update
    void updateVertexData(int offset, const ofVec3f* vertices, int count);

    /// \brief Update a portion of normal data
    /// \param offset Offset into buffer (in normals)
    /// \param normals Pointer to new normal data
    /// \param count Number of normals to update
    void updateNormalData(int offset, const ofVec3f* normals, int count);

    /// \brief Update a portion of color data
    /// \param offset Offset into buffer (in colors)
    /// \param colors Pointer to new color data
    /// \param count Number of colors to update
    void updateColorData(int offset, const ofFloatColor* colors, int count);

    /// \brief Update a portion of texture coordinate data
    /// \param offset Offset into buffer (in texCoords)
    /// \param texCoords Pointer to new texCoord data
    /// \param count Number of texCoords to update
    void updateTexCoordData(int offset, const ofVec2f* texCoords, int count);

    /// \brief Update a portion of index data
    /// \param offset Offset into buffer (in indices)
    /// \param indices Pointer to new index data
    /// \param count Number of indices to update
    void updateIndexData(int offset, const unsigned int* indices, int count);

    // ========================================================================
    // Drawing
    // ========================================================================

    /// \brief Draw the VBO
    /// \param drawMode Drawing mode (OF_MESH_POINTS, OF_MESH_WIREFRAME, OF_MESH_FILL)
    /// \param first First vertex to draw
    /// \param total Number of vertices to draw
    void draw(int drawMode, int first, int total) const;

    /// \brief Draw using indices
    /// \param drawMode Drawing mode
    /// \param first First index to use
    /// \param total Number of indices to use
    void drawElements(int drawMode, int first, int total) const;

    /// \brief Draw using instance rendering
    /// \param drawMode Drawing mode
    /// \param first First vertex
    /// \param total Number of vertices
    /// \param instanceCount Number of instances
    void drawInstanced(int drawMode, int first, int total, int instanceCount) const;

    // ========================================================================
    // Mesh Integration
    // ========================================================================

    /// \brief Set all data from an ofMesh
    /// \param mesh The mesh to copy data from
    /// \param usage Usage hint
    void setMesh(const ofMesh& mesh, int usage = 0);

    // ========================================================================
    // Clear / Reset
    // ========================================================================

    /// \brief Clear all buffer data
    void clear();

    /// \brief Clear vertex data only
    void clearVertices();

    /// \brief Clear normal data only
    void clearNormals();

    /// \brief Clear color data only
    void clearColors();

    /// \brief Clear texture coordinate data only
    void clearTexCoords();

    /// \brief Clear index data only
    void clearIndices();

    // ========================================================================
    // Binding (Advanced)
    // ========================================================================

    /// \brief Bind VBO for rendering
    /// \details Call before custom draw operations
    void bind() const;

    /// \brief Unbind VBO
    void unbind() const;

    // ========================================================================
    // Native Access (Advanced)
    // ========================================================================

    /// \brief Get native vertex buffer handle
    /// \return id<MTLBuffer> or nullptr
    void* getNativeVertexBuffer() const;

    /// \brief Get native normal buffer handle
    /// \return id<MTLBuffer> or nullptr
    void* getNativeNormalBuffer() const;

    /// \brief Get native color buffer handle
    /// \return id<MTLBuffer> or nullptr
    void* getNativeColorBuffer() const;

    /// \brief Get native texCoord buffer handle
    /// \return id<MTLBuffer> or nullptr
    void* getNativeTexCoordBuffer() const;

    /// \brief Get native index buffer handle
    /// \return id<MTLBuffer> or nullptr
    void* getNativeIndexBuffer() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    void ensureImpl();
};

} // namespace oflike

// Draw mode constants (matching openFrameworks)
#ifndef OF_MESH_POINTS
#define OF_MESH_POINTS 0
#define OF_MESH_WIREFRAME 1
#define OF_MESH_FILL 2
#endif
