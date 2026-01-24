#pragma once

// SharpRenderer - Metal-based renderer for 3D Gaussian Splatting
//
// This class provides a high-performance GPU-accelerated renderer for 3D Gaussian Splatting
// point clouds. It implements the following features:
// - GPU-based depth sorting using Metal compute shaders
// - Alpha blending with proper transparency
// - Efficient covariance to 2D projection
// - Anti-aliasing through Gaussian splatting
// - Integration with oflike render pipeline
//
// Algorithm:
// 1. Transform Gaussians from world space to screen space
// 2. Compute 2D covariance matrices from 3D Gaussians
// 3. Sort Gaussians by depth (back-to-front for alpha blending)
// 4. Render each Gaussian as a billboard quad
// 5. Evaluate spherical harmonics for view-dependent color
// 6. Apply Gaussian splatting with proper alpha blending
//
// Reference: https://github.com/graphdeco-inria/gaussian-splatting
// Paper: "3D Gaussian Splatting for Real-Time Radiance Field Rendering"

#include "SharpGaussianCloud.h"
#include <memory>
#include <functional>

// Forward declarations
namespace oflike {
    class ofCamera;
    class ofTexture;
    class ofMatrix4x4;
}

namespace Sharp {

// Rendering configuration
struct RenderConfig {
    // Enable depth sorting (required for correct transparency)
    bool enableDepthSort = true;

    // Enable anti-aliasing
    bool enableAntialiasing = true;

    // Enable spherical harmonics (view-dependent appearance)
    bool enableSphericalHarmonics = true;

    // Maximum spherical harmonics degree to use (0-3)
    // 0 = diffuse only, 3 = full view-dependent appearance
    int maxSHDegree = 3;

    // Scale factor for Gaussian splats (larger = bigger splats)
    float splatScale = 1.0f;

    // Opacity multiplier (0-1)
    float opacityScale = 1.0f;

    // Minimum opacity threshold (Gaussians below this are culled)
    float minOpacity = 0.01f;

    // Enable frustum culling
    bool enableFrustumCulling = true;

    // Enable backface culling (culls Gaussians facing away)
    bool enableBackfaceCulling = false;
};

// Rendering statistics
struct RenderStats {
    uint32_t totalGaussians = 0;
    uint32_t visibleGaussians = 0;
    uint32_t culledGaussians = 0;
    double sortTimeMs = 0.0;
    double renderTimeMs = 0.0;
    double totalTimeMs = 0.0;
    uint32_t frameIndex = 0;
};

class SharpRenderer {
public:
    // ============================================================================
    // Constructors / Destructor
    // ============================================================================

    SharpRenderer();
    ~SharpRenderer();

    // Move semantics (disable copy due to Metal resources)
    SharpRenderer(SharpRenderer&& other) noexcept;
    SharpRenderer& operator=(SharpRenderer&& other) noexcept;
    SharpRenderer(const SharpRenderer&) = delete;
    SharpRenderer& operator=(const SharpRenderer&) = delete;

    // ============================================================================
    // Initialization
    // ============================================================================

    /**
     * Initialize renderer with Metal device and command queue.
     * @param device Metal device (id<MTLDevice>)
     * @param commandQueue Metal command queue (id<MTLCommandQueue>)
     * @return true if initialization succeeded
     */
    bool initialize(void* device, void* commandQueue);

    /**
     * Check if renderer is initialized.
     */
    bool isInitialized() const;

    /**
     * Shutdown renderer and release resources.
     */
    void shutdown();

    // ============================================================================
    // Rendering
    // ============================================================================

    /**
     * Render a Gaussian cloud with the given camera and render target.
     * @param cloud Gaussian cloud to render
     * @param camera Camera for view/projection matrices
     * @param renderTarget Metal texture to render to (id<MTLTexture>)
     * @param commandBuffer Metal command buffer (id<MTLCommandBuffer>)
     * @return true if rendering succeeded
     */
    bool render(const GaussianCloud& cloud,
                const oflike::ofMatrix4x4& viewMatrix,
                const oflike::ofMatrix4x4& projectionMatrix,
                void* renderTarget,
                void* commandBuffer);

    /**
     * Render a Gaussian cloud with ofCamera.
     * @param cloud Gaussian cloud to render
     * @param camera ofCamera for view/projection
     * @param renderTarget Metal texture to render to (id<MTLTexture>)
     * @param commandBuffer Metal command buffer (id<MTLCommandBuffer>)
     * @return true if rendering succeeded
     */
    bool render(const GaussianCloud& cloud,
                const oflike::ofCamera& camera,
                void* renderTarget,
                void* commandBuffer);

    // ============================================================================
    // Configuration
    // ============================================================================

    /**
     * Set rendering configuration.
     */
    void setConfig(const RenderConfig& config);

    /**
     * Get current rendering configuration.
     */
    const RenderConfig& getConfig() const;

    /**
     * Enable/disable depth sorting.
     * When disabled, Gaussians are rendered in the order they appear in the cloud.
     * This is faster but may produce incorrect transparency.
     */
    void setDepthSortEnabled(bool enabled);

    /**
     * Set splat scale multiplier.
     * Values > 1.0 make splats larger, < 1.0 make them smaller.
     */
    void setSplatScale(float scale);

    /**
     * Set opacity scale multiplier.
     * Values < 1.0 make the entire cloud more transparent.
     */
    void setOpacityScale(float scale);

    /**
     * Enable/disable spherical harmonics.
     * When disabled, only DC component (diffuse color) is used.
     */
    void setSphericalHarmonicsEnabled(bool enabled);

    /**
     * Set maximum spherical harmonics degree (0-3).
     * 0 = diffuse only, 3 = full view-dependent appearance.
     */
    void setMaxSHDegree(int degree);

    // ============================================================================
    // Statistics
    // ============================================================================

    /**
     * Get rendering statistics from the last frame.
     */
    const RenderStats& getStats() const;

    /**
     * Reset statistics counters.
     */
    void resetStats();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace Sharp
