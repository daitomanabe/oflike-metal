#pragma once

#include "../IRenderer.h"
#include <memory>

namespace render {
namespace metal {

// ============================================================================
// MetalRenderer - Metal Backend Implementation
// ============================================================================

/**
 * Metal implementation of the IRenderer interface.
 *
 * Features:
 * - Triple buffering for smooth rendering
 * - DrawList command execution
 * - State management (blend, depth, culling)
 * - Texture creation and loading via MTKTextureLoader
 * - Performance statistics tracking
 *
 * Implementation:
 * - Uses pImpl pattern to hide Objective-C++ Metal code
 * - All Metal-specific code in .mm file
 * - Thread-safety: Main thread only
 *
 * Usage:
 *   auto renderer = std::make_unique<MetalRenderer>(device, view);
 *   renderer->initialize();
 *   renderer->beginFrame();
 *   renderer->executeDrawList(drawList);
 *   renderer->endFrame();
 */
class MetalRenderer : public IRenderer {
public:
    /**
     * Constructor.
     * @param device Metal device (id<MTLDevice>)
     * @param view Metal view (MTKView)
     */
    MetalRenderer(void* device, void* view);

    ~MetalRenderer() override;

    // Disable copy and move
    MetalRenderer(const MetalRenderer&) = delete;
    MetalRenderer& operator=(const MetalRenderer&) = delete;
    MetalRenderer(MetalRenderer&&) = delete;
    MetalRenderer& operator=(MetalRenderer&&) = delete;

    // ========================================================================
    // IRenderer Implementation
    // ========================================================================

    // Initialization & Lifecycle
    bool initialize() override;
    void shutdown() override;
    bool isInitialized() const override;

    // Frame Management
    bool beginFrame() override;
    bool endFrame() override;
    uint32_t getCurrentFrameIndex() const override;

    // DrawList Execution
    bool executeDrawList(const DrawList& drawList) override;

    // Render State
    void setViewport(float x, float y, float width, float height) override;
    void setScissor(float x, float y, float width, float height) override;
    void setScissorEnabled(bool enabled) override;
    void clear(const simd_float4& color, bool clearColor = true,
               bool clearDepth = false, float depth = 1.0f) override;
    void setBlendMode(BlendMode mode) override;
    void setDepthTestEnabled(bool enabled) override;
    void setDepthWriteEnabled(bool enabled) override;
    void setCullingMode(bool cullBack, bool enabled) override;

    // Render Target Management
    bool setRenderTarget(void* renderTarget) override;
    void* getDefaultRenderTarget() const override;

    // Texture Management
    void* createTexture(uint32_t width, uint32_t height, const void* data) override;
    void* loadTexture(const char* path) override;
    void destroyTexture(void* texture) override;

    // Information
    const char* getRendererName() const override;
    uint32_t getViewportWidth() const override;
    uint32_t getViewportHeight() const override;
    void getStatistics(uint32_t& outDrawCalls, uint32_t& outVertices) const override;

    // Performance monitoring
    double getLastGPUTime() const;  // Returns GPU time in milliseconds

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace metal
} // namespace render
