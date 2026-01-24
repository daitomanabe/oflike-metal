#pragma once

// SharpGaussianCloud - Container for managing a collection of 3D Gaussians
//
// This class manages a collection of Gaussian primitives and provides:
// - Memory management for large-scale Gaussian clouds (millions of primitives)
// - Metal buffer management for GPU rendering
// - Spatial transformations (translate, rotate, scale)
// - Filtering and culling operations
// - Import/export functionality (PLY format)
// - Bounding box computation
//
// Reference: https://github.com/apple/ml-sharp
// Paper: "3D Gaussian Splatting for Real-Time Radiance Field Rendering"

#include "SharpGaussian.h"
#include "math/Types.h"
#include <vector>
#include <string>
#include <memory>

// Forward declarations
namespace oflike {
    class ofMatrix4x4;
}

namespace Sharp {

class GaussianCloud {
public:
    // ============================================================================
    // Constructors / Destructor
    // ============================================================================

    GaussianCloud();
    ~GaussianCloud();

    // Move semantics (disable copy due to Metal resources)
    GaussianCloud(GaussianCloud&& other) noexcept;
    GaussianCloud& operator=(GaussianCloud&& other) noexcept;

    // Disable copy
    GaussianCloud(const GaussianCloud&) = delete;
    GaussianCloud& operator=(const GaussianCloud&) = delete;

    // ============================================================================
    // Data Management
    // ============================================================================

    // Add a single Gaussian
    void addGaussian(const Gaussian& gaussian);

    // Add multiple Gaussians
    void addGaussians(const std::vector<Gaussian>& gaussians);

    // Reserve capacity for performance
    void reserve(size_t count);

    // Clear all Gaussians
    void clear();

    // Get number of Gaussians
    size_t size() const;

    // Check if empty
    bool empty() const;

    // Get Gaussian by index
    const Gaussian& getGaussian(size_t index) const;
    Gaussian& getGaussian(size_t index);

    // Get all Gaussians
    const std::vector<Gaussian>& getGaussians() const;

    // ============================================================================
    // Spatial Transformations
    // ============================================================================

    // Apply translation to all Gaussians
    void translate(const oflike::float3& offset);

    // Apply rotation to all Gaussians (around center)
    void rotate(const oflike::quatf& rotation);

    // Apply rotation to all Gaussians (around specified point)
    void rotateAround(const oflike::quatf& rotation, const oflike::float3& center);

    // Apply uniform scale to all Gaussians
    void scale(float factor);

    // Apply non-uniform scale to all Gaussians
    void scale(const oflike::float3& factors);

    // Apply 4x4 transformation matrix to all Gaussians
    void transform(const oflike::ofMatrix4x4& matrix);

    // ============================================================================
    // Filtering & Culling
    // ============================================================================

    // Filter Gaussians by opacity threshold
    void filterByOpacity(float minOpacity);

    // Filter Gaussians by size (scale magnitude)
    void filterBySize(float minSize, float maxSize);

    // Filter Gaussians by bounding box
    void filterByBounds(const oflike::float3& minBounds, const oflike::float3& maxBounds);

    // Remove invisible Gaussians (opacity < threshold)
    void removeInvisible(float threshold = 0.01f);

    // ============================================================================
    // Bounding Box
    // ============================================================================

    // Get axis-aligned bounding box minimum point
    oflike::float3 getBoundsMin() const;

    // Get axis-aligned bounding box maximum point
    oflike::float3 getBoundsMax() const;

    // Get bounding box center
    oflike::float3 getCenter() const;

    // Get bounding box size
    oflike::float3 getSize() const;

    // Recalculate bounding box (called automatically when needed)
    void updateBounds();

    // ============================================================================
    // Metal Buffer Management
    // ============================================================================

    // Update Metal buffer with current Gaussian data
    // Returns true if successful
    bool updateMetalBuffer();

    // Get Metal buffer for rendering (opaque handle)
    // Returns nullptr if buffer not created
    void* getMetalBuffer() const;

    // Get buffer size in bytes
    size_t getBufferSize() const;

    // Check if Metal buffer is up-to-date
    bool isBufferDirty() const;

    // ============================================================================
    // Import / Export
    // ============================================================================

    // Load from PLY file
    // Returns true if successful
    bool loadFromPLY(const std::string& filepath);

    // Save to PLY file
    // Returns true if successful
    bool saveToPLY(const std::string& filepath) const;

    // ============================================================================
    // Statistics
    // ============================================================================

    // Get total memory usage (CPU + GPU)
    size_t getMemoryUsage() const;

    // Get average opacity
    float getAverageOpacity() const;

    // Get average scale magnitude
    float getAverageScale() const;

private:
    // pImpl pattern to hide Metal/Objective-C++ implementation
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace Sharp
