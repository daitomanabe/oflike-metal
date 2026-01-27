#pragma once

#include "DrawList.h"
#include "RenderTypes.h"
#include <cstdint>

namespace render {

// ============================================================================
// IRenderer - Abstract Renderer Interface
// ============================================================================

/**
 * Abstract interface for rendering backends.
 *
 * Responsibilities:
 * - Execute DrawList commands
 * - Manage render state (viewport, blend mode, depth test)
 * - Handle render target switching
 * - Frame synchronization and buffering
 *
 * Implementation notes:
 * - Metal implementation uses pImpl pattern
 * - All coordinates are in pixels
 * - Origin is top-left (2D) or standard 3D (right-hand)
 * - Thread-safety: Main thread only for draw calls
 */
class IRenderer {
public:
    virtual ~IRenderer() = default;

    // ========================================================================
    // Initialization & Lifecycle
    // ========================================================================

    /**
     * Initialize the renderer.
     * @return true on success, false on failure
     */
    virtual bool initialize() = 0;

    /**
     * Shutdown the renderer and release resources.
     */
    virtual void shutdown() = 0;

    /**
     * Check if the renderer is initialized.
     * @return true if initialized
     */
    virtual bool isInitialized() const = 0;

    // ========================================================================
    // Frame Management
    // ========================================================================

    /**
     * Begin a new frame.
     * Must be called before any draw commands.
     * @return true on success
     */
    virtual bool beginFrame() = 0;

    /**
     * End the current frame and present to screen.
     * @return true on success
     */
    virtual bool endFrame() = 0;

    /**
     * Get the current frame index (for triple buffering).
     * @return Frame index (0, 1, or 2)
     */
    virtual uint32_t getCurrentFrameIndex() const = 0;

    // ========================================================================
    // DrawList Execution
    // ========================================================================

    /**
     * Execute all commands in a DrawList.
     * @param drawList The DrawList to execute
     * @return true on success
     */
    virtual bool executeDrawList(const DrawList& drawList) = 0;

    // ========================================================================
    // Render State
    // ========================================================================

    /**
     * Set the viewport rectangle.
     * @param x Left edge in pixels
     * @param y Top edge in pixels
     * @param width Width in pixels
     * @param height Height in pixels
     */
    virtual void setViewport(float x, float y, float width, float height) = 0;

    /**
     * Set the scissor test rectangle.
     * @param x Left edge in pixels
     * @param y Top edge in pixels
     * @param width Width in pixels
     * @param height Height in pixels
     */
    virtual void setScissor(float x, float y, float width, float height) = 0;

    /**
     * Enable or disable scissor testing.
     * @param enabled true to enable scissor test
     */
    virtual void setScissorEnabled(bool enabled) = 0;

    /**
     * Clear the current render target.
     * @param color Clear color (RGBA, 0.0-1.0)
     * @param clearColor true to clear color buffer
     * @param clearDepth true to clear depth buffer
     * @param depth Clear depth value (0.0-1.0)
     */
    virtual void clear(const simd_float4& color, bool clearColor = true,
                       bool clearDepth = false, float depth = 1.0f) = 0;

    /**
     * Set the blend mode.
     * @param mode Blend mode
     */
    virtual void setBlendMode(BlendMode mode) = 0;

    /**
     * Enable or disable depth testing.
     * @param enabled true to enable depth test
     */
    virtual void setDepthTestEnabled(bool enabled) = 0;

    /**
     * Enable or disable depth write.
     * @param enabled true to enable depth write
     */
    virtual void setDepthWriteEnabled(bool enabled) = 0;

    /**
     * Set culling mode.
     * @param cullBack true to cull back faces, false to cull front faces
     * @param enabled true to enable culling
     */
    virtual void setCullingMode(bool cullBack, bool enabled) = 0;

    // ========================================================================
    // Render Target Management
    // ========================================================================

    /**
     * Set the current render target.
     * @param renderTarget Handle to render target (MTLTexture), or nullptr for screen
     * @return true on success
     */
    virtual bool setRenderTarget(void* renderTarget) = 0;

    /**
     * Get the default screen render target.
     * @return Handle to the screen render target
     */
    virtual void* getDefaultRenderTarget() const = 0;

    // ========================================================================
    // Texture Management
    // ========================================================================

    /**
     * Create a texture from pixel data.
     * @param width Texture width in pixels
     * @param height Texture height in pixels
     * @param data Pixel data (RGBA format)
     * @return Handle to the created texture, or nullptr on failure
     */
    virtual void* createTexture(uint32_t width, uint32_t height, const void* data) = 0;

    /**
     * Load a texture from file.
     * @param path File path
     * @return Handle to the loaded texture, or nullptr on failure
     */
    virtual void* loadTexture(const char* path) = 0;

    /**
     * Destroy a texture.
     * @param texture Handle to the texture
     */
    virtual void destroyTexture(void* texture) = 0;

    /**
     * Read pixels from a texture (GPU->CPU readback).
     * @param texture Handle to the texture
     * @param data Destination buffer (must be pre-allocated)
     * @param width Texture width
     * @param height Texture height
     * @param bytesPerRow Bytes per row
     * @return true on success, false on failure
     */
    virtual bool readTexturePixels(void* texture, void* data, uint32_t width, uint32_t height, size_t bytesPerRow) const = 0;

    // ========================================================================
    // Device Access
    // ========================================================================

    /**
     * Get the underlying graphics device.
     * @return Device handle (e.g., id<MTLDevice> for Metal), or nullptr if not available
     */
    virtual void* getDevice() const = 0;

    // ========================================================================
    // Information
    // ========================================================================

    /**
     * Get the renderer name (e.g., "Metal").
     * @return Renderer name string
     */
    virtual const char* getRendererName() const = 0;

    /**
     * Get the current viewport width.
     * @return Viewport width in pixels
     */
    virtual uint32_t getViewportWidth() const = 0;

    /**
     * Get the current viewport height.
     * @return Viewport height in pixels
     */
    virtual uint32_t getViewportHeight() const = 0;

    /**
     * Get rendering statistics for the last frame.
     * @param outDrawCalls Number of draw calls
     * @param outVertices Number of vertices rendered
     */
    virtual void getStatistics(uint32_t& outDrawCalls, uint32_t& outVertices) const = 0;

    /**
     * Get the last GPU frame time in milliseconds.
     * @return GPU time in ms, or 0.0 if unsupported
     */
    virtual double getLastGPUTime() const { return 0.0; }
};

} // namespace render
