#pragma once

// oflike-metal VboMesh - Modern Metal GPU Mesh
// Fully leverages Metal 3 features for high-performance rendering

#include <memory>
#include "../math/ofVec2f.h"
#include "../math/ofVec3f.h"
#include "../types/ofColor.h"
#include "ofMesh.h"

namespace oflike {

// ============================================================================
// Forward Declarations
// ============================================================================

class VboMesh;

// ============================================================================
// Enums and Configuration
// ============================================================================

/// Storage mode for GPU buffers
enum class VboStorageMode {
    /// GPU-only storage - fastest for static meshes
    /// Data is uploaded once and stays on GPU
    Private,

    /// CPU/GPU shared memory - best for dynamic meshes updated every frame
    /// No explicit sync needed
    Shared,

    /// Managed storage - explicit sync, good for infrequent updates
    /// Use sync() after CPU modifications
    Managed,

    /// Auto-select based on usage pattern
    Auto
};

/// Usage hint for automatic storage mode selection
enum class VboUsageHint {
    /// Data never changes after initial upload
    Static,

    /// Data changes occasionally (every few frames)
    Dynamic,

    /// Data changes every frame
    Stream
};

/// Vertex attribute layout strategy
enum class VboAttributeLayout {
    /// All attributes interleaved in single buffer (better cache locality)
    /// [pos0, norm0, uv0, col0, pos1, norm1, uv1, col1, ...]
    Interleaved,

    /// Separate buffer per attribute (flexible partial updates)
    /// Positions: [pos0, pos1, ...]
    /// Normals:   [norm0, norm1, ...]
    Separate
};

/// Draw mode for rendering
enum class VboDrawMode {
    /// Standard draw call
    Direct,

    /// Instanced rendering with instance count
    Instanced,

    /// GPU-driven indirect draw (arguments from GPU buffer)
    Indirect
};

// ============================================================================
// Vertex Attribute Descriptor
// ============================================================================

/// Describes a custom vertex attribute
struct VboVertexAttribute {
    uint32_t index;         ///< Attribute index (buffer binding)
    uint32_t offset;        ///< Byte offset within vertex (interleaved only)
    uint32_t stride;        ///< Byte stride between vertices
    uint32_t components;    ///< Number of components (1-4)
    bool normalized;        ///< Normalize integer values to [0,1] or [-1,1]

    enum class Format {
        Float,      ///< 32-bit float
        Half,       ///< 16-bit float
        Int,        ///< 32-bit signed integer
        UInt,       ///< 32-bit unsigned integer
        Short,      ///< 16-bit signed integer
        UShort,     ///< 16-bit unsigned integer
        Byte,       ///< 8-bit signed integer
        UByte,      ///< 8-bit unsigned integer
    } format;
};

// ============================================================================
// Instance Data Structure
// ============================================================================

/// Per-instance data for instanced rendering
struct VboInstanceData {
    simd_float4x4 modelMatrix;      ///< Instance transform
    simd_float4 color;              ///< Instance color (optional tint)
    simd_float4 userData;           ///< Custom per-instance data

    VboInstanceData()
        : modelMatrix(matrix_identity_float4x4)
        , color(simd_make_float4(1, 1, 1, 1))
        , userData(simd_make_float4(0, 0, 0, 0)) {}

    VboInstanceData(const simd_float4x4& m)
        : modelMatrix(m)
        , color(simd_make_float4(1, 1, 1, 1))
        , userData(simd_make_float4(0, 0, 0, 0)) {}

    VboInstanceData(const simd_float4x4& m, const simd_float4& c)
        : modelMatrix(m)
        , color(c)
        , userData(simd_make_float4(0, 0, 0, 0)) {}
};

// ============================================================================
// Indirect Draw Arguments (matches MTLDrawIndexedPrimitivesIndirectArguments)
// ============================================================================

/// Arguments for indirect indexed draw calls
struct VboIndirectArguments {
    uint32_t indexCount;        ///< Number of indices to draw
    uint32_t instanceCount;     ///< Number of instances
    uint32_t indexStart;        ///< First index
    int32_t  baseVertex;        ///< Added to each index value
    uint32_t baseInstance;      ///< First instance ID
};

// ============================================================================
// VboMesh - Modern Metal GPU Mesh
// ============================================================================

/**
 * VboMesh - High-performance GPU mesh with modern Metal features
 *
 * Features:
 * - Triple buffering for CPU-GPU concurrency
 * - Automatic storage mode selection
 * - Instanced rendering (thousands of objects, 1 draw call)
 * - Indirect draw (GPU-driven rendering)
 * - Flexible vertex attribute layout
 * - Efficient partial buffer updates
 *
 * Example Usage:
 *
 *   // Create from ofMesh
 *   VboMesh vbo;
 *   vbo.setMesh(mesh, VboUsageHint::Static);
 *   vbo.draw();
 *
 *   // Dynamic mesh (updated every frame)
 *   VboMesh dynamic;
 *   dynamic.allocate(maxVertices, maxIndices, VboUsageHint::Stream);
 *   dynamic.updateVertices(vertices.data(), count);
 *   dynamic.draw();
 *
 *   // Instanced rendering
 *   VboMesh instanced;
 *   instanced.setMesh(mesh, VboUsageHint::Static);
 *   instanced.setInstances(instanceData, 10000);
 *   instanced.drawInstanced(10000);
 *
 *   // Indirect draw (GPU fills arguments)
 *   VboMesh indirect;
 *   indirect.setMesh(mesh, VboUsageHint::Static);
 *   indirect.drawIndirect(argumentBuffer);
 */
class VboMesh {
public:
    // ========================================================================
    // Construction / Destruction
    // ========================================================================

    VboMesh();
    ~VboMesh();

    // Move semantics
    VboMesh(VboMesh&& other) noexcept;
    VboMesh& operator=(VboMesh&& other) noexcept;

    // No copy (GPU resources)
    VboMesh(const VboMesh&) = delete;
    VboMesh& operator=(const VboMesh&) = delete;

    // ========================================================================
    // Initialization
    // ========================================================================

    /// Initialize from ofMesh data
    /// @param mesh Source mesh data
    /// @param usage Usage hint for storage mode selection
    /// @param layout Vertex attribute layout strategy
    /// @return true on success
    bool setMesh(const ofMesh& mesh,
                 VboUsageHint usage = VboUsageHint::Static,
                 VboAttributeLayout layout = VboAttributeLayout::Interleaved);

    /// Pre-allocate buffers for dynamic use
    /// @param maxVertices Maximum number of vertices
    /// @param maxIndices Maximum number of indices (0 for non-indexed)
    /// @param usage Usage hint (should be Dynamic or Stream)
    /// @param layout Vertex attribute layout strategy
    /// @return true on success
    bool allocate(size_t maxVertices,
                  size_t maxIndices = 0,
                  VboUsageHint usage = VboUsageHint::Dynamic,
                  VboAttributeLayout layout = VboAttributeLayout::Interleaved);

    /// Explicitly set storage mode (overrides auto selection)
    void setStorageMode(VboStorageMode mode);

    /// Check if VBO is allocated and ready to use
    bool isAllocated() const;

    /// Release all GPU resources
    void clear();

    // ========================================================================
    // Data Updates
    // ========================================================================

    /// Update all mesh data from ofMesh
    /// @param mesh Source mesh (must be same or smaller size than allocated)
    void updateMesh(const ofMesh& mesh);

    /// Update vertex positions
    /// @param data Pointer to vertex positions
    /// @param count Number of vertices
    /// @param offset First vertex to update (default: 0)
    void updateVertices(const ofVec3f* data, size_t count, size_t offset = 0);

    /// Update vertex normals
    void updateNormals(const ofVec3f* data, size_t count, size_t offset = 0);

    /// Update texture coordinates
    void updateTexCoords(const ofVec2f* data, size_t count, size_t offset = 0);

    /// Update vertex colors
    void updateColors(const ofColor* data, size_t count, size_t offset = 0);

    /// Update indices
    void updateIndices(const uint32_t* data, size_t count, size_t offset = 0);

    /// Set active vertex/index count (for partial draws)
    void setVertexCount(size_t count);
    void setIndexCount(size_t count);

    /// Synchronize managed storage mode buffers
    /// Call after CPU modifications if using Managed storage mode
    void sync();

    // ========================================================================
    // Instance Data (for Instanced Rendering)
    // ========================================================================

    /// Set instance data for instanced rendering
    /// @param data Array of per-instance data
    /// @param count Number of instances
    void setInstances(const VboInstanceData* data, size_t count);

    /// Update instance data
    /// @param data Array of per-instance data
    /// @param count Number of instances to update
    /// @param offset First instance to update
    void updateInstances(const VboInstanceData* data, size_t count, size_t offset = 0);

    /// Get maximum allocated instances
    size_t getMaxInstances() const;

    // ========================================================================
    // Indirect Draw Arguments
    // ========================================================================

    /// Set indirect draw arguments (CPU-side, then uploaded to GPU)
    void setIndirectArguments(const VboIndirectArguments& args);

    /// Get pointer to indirect argument buffer for GPU modification
    /// Returns native MTLBuffer handle
    void* getIndirectArgumentBuffer();

    // ========================================================================
    // Drawing
    // ========================================================================

    /// Draw the mesh
    void draw() const;

    /// Draw with explicit primitive mode
    void draw(ofPrimitiveMode mode) const;

    /// Draw a range of vertices/indices
    /// @param start First vertex/index
    /// @param count Number of primitives
    void drawRange(size_t start, size_t count) const;

    /// Draw instances
    /// @param instanceCount Number of instances to draw
    void drawInstanced(size_t instanceCount) const;

    /// Draw instances with explicit primitive mode
    void drawInstanced(size_t instanceCount, ofPrimitiveMode mode) const;

    /// Indirect draw (arguments from GPU buffer)
    void drawIndirect() const;

    /// Indirect draw with external argument buffer
    /// @param argumentBuffer MTLBuffer containing draw arguments
    /// @param argumentOffset Byte offset to arguments in buffer
    void drawIndirect(void* argumentBuffer, size_t argumentOffset = 0) const;

    // ========================================================================
    // Information
    // ========================================================================

    /// Get number of vertices
    size_t getNumVertices() const;

    /// Get number of indices
    size_t getNumIndices() const;

    /// Get primitive mode
    ofPrimitiveMode getMode() const;

    /// Set primitive mode
    void setMode(ofPrimitiveMode mode);

    /// Check if mesh uses indexed rendering
    bool hasIndices() const;

    /// Check if mesh has normals
    bool hasNormals() const;

    /// Check if mesh has texture coordinates
    bool hasTexCoords() const;

    /// Check if mesh has colors
    bool hasColors() const;

    /// Get current storage mode
    VboStorageMode getStorageMode() const;

    /// Get vertex attribute layout
    VboAttributeLayout getAttributeLayout() const;

    // ========================================================================
    // Advanced: Direct Buffer Access
    // ========================================================================

    /// Get native Metal vertex buffer
    /// @param frameIndex Frame index (for triple-buffered dynamic meshes)
    /// @return id<MTLBuffer> cast to void*
    void* getVertexBuffer(uint32_t frameIndex = 0) const;

    /// Get native Metal index buffer
    void* getIndexBuffer(uint32_t frameIndex = 0) const;

    /// Get native Metal instance buffer
    void* getInstanceBuffer(uint32_t frameIndex = 0) const;

    /// Get vertex buffer offset (for interleaved attributes)
    size_t getVertexBufferOffset() const;

    /// Get stride between vertices
    size_t getVertexStride() const;

    // ========================================================================
    // Static Utility
    // ========================================================================

    /// Create VboMesh from primitive (convenience)
    static VboMesh createPlane(float width, float height, int columns = 2, int rows = 2);
    static VboMesh createSphere(float radius, int resolution = 20);
    static VboMesh createBox(float width, float height, float depth);
    static VboMesh createCone(float radius, float height, int segments = 20);
    static VboMesh createCylinder(float radius, float height, int segments = 20);
    static VboMesh createIcosphere(float radius, int subdivisions = 2);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    // Internal helpers
    void markDirty();
    void uploadIfNeeded() const;
};

// ============================================================================
// VboMeshBatch - Batch multiple VboMesh draws
// ============================================================================

/**
 * VboMeshBatch - Efficient batching of multiple mesh draws
 *
 * Uses indirect drawing to batch multiple mesh draws into fewer draw calls.
 * Ideal for rendering many different meshes efficiently.
 */
class VboMeshBatch {
public:
    VboMeshBatch();
    ~VboMeshBatch();

    /// Add a mesh to the batch with transform
    void add(VboMesh& mesh, const simd_float4x4& transform);

    /// Clear all meshes from batch
    void clear();

    /// Draw all batched meshes
    void draw();

    /// Get number of meshes in batch
    size_t getCount() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace oflike
