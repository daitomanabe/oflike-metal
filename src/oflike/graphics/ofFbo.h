#pragma once

// oflike-metal ofFbo - openFrameworks API compatible framebuffer object
// Provides offscreen rendering target functionality

#include <memory>
#include <vector>
#include "../image/ofTexture.h"
#include "../image/ofPixels.h"

namespace oflike {

// ============================================================================
// ofFboSettings - FBO Configuration
// ============================================================================

/// \brief Configuration settings for ofFbo allocation
/// \details Specifies the properties of the framebuffer including dimensions,
/// color attachments, depth buffer, stencil buffer, and multisampling.
struct ofFboSettings {
    int width = 0;                      ///< Width in pixels
    int height = 0;                     ///< Height in pixels
    int numColorAttachments = 1;        ///< Number of color textures (1-4)
    int internalFormat = OF_IMAGE_COLOR_ALPHA; ///< Color texture format
    bool useDepth = false;              ///< Enable depth buffer
    bool useStencil = false;            ///< Enable stencil buffer
    int numSamples = 0;                 ///< MSAA samples (0=off, 2/4/8=on)

    ofFboSettings()
        : width(0)
        , height(0)
        , numColorAttachments(1)
        , internalFormat(OF_IMAGE_COLOR_ALPHA)
        , useDepth(false)
        , useStencil(false)
        , numSamples(0) {}
};

// ============================================================================
// ofFbo - Framebuffer Object
// ============================================================================

/// \brief Offscreen rendering target (Framebuffer Object)
/// \details ofFbo provides openFrameworks-compatible FBO operations using
/// Metal render textures. Supports rendering to textures, depth buffers,
/// multiple color attachments, and multisampling.
///
/// Features:
/// - Offscreen rendering to textures
/// - Optional depth buffer support
/// - Multiple color attachments (MRT)
/// - Multisampling anti-aliasing (MSAA)
/// - Read back to ofPixels
/// - Draw FBO contents to screen
///
/// Implementation:
/// - Uses pImpl pattern to hide Metal/Objective-C++ details
/// - Pure C++ header (.h), Objective-C++ implementation (.mm)
/// - Internally uses MTLTexture render targets
/// - Thread-safety: Main thread only (Metal requirement)
///
/// Example:
/// \code
///     ofFbo fbo;
///     fbo.allocate(1024, 768);
///
///     fbo.begin();
///     // Draw content here
///     ofClear(0);
///     ofDrawCircle(512, 384, 100);
///     fbo.end();
///
///     // Draw FBO to screen
///     fbo.draw(0, 0);
/// \endcode
class ofFbo {
public:
    // ========================================================================
    // Constructors & Destructor
    // ========================================================================

    /// \brief Default constructor
    ofFbo();

    /// \brief Destructor
    ~ofFbo();

    /// \brief Move constructor
    ofFbo(ofFbo&& other) noexcept;

    /// \brief Move assignment
    ofFbo& operator=(ofFbo&& other) noexcept;

    // Delete copy constructor and assignment
    ofFbo(const ofFbo&) = delete;
    ofFbo& operator=(const ofFbo&) = delete;

    // ========================================================================
    // Allocation
    // ========================================================================

    /// \brief Allocate FBO with specified dimensions
    /// \param width Width in pixels
    /// \param height Height in pixels
    /// \param internalFormat Color texture format (default: OF_IMAGE_COLOR_ALPHA)
    /// \param numSamples MSAA samples (0=off, 2/4/8=MSAA)
    void allocate(int width, int height,
                  int internalFormat = OF_IMAGE_COLOR_ALPHA,
                  int numSamples = 0);

    /// \brief Allocate FBO with detailed settings
    /// \param settings FBO configuration settings
    void allocateWithSettings(const ofFboSettings& settings);

    /// \brief Check if FBO is allocated
    /// \return true if allocated, false otherwise
    bool isAllocated() const;

    /// \brief Clear FBO and free GPU memory
    void clear();

    // ========================================================================
    // Rendering
    // ========================================================================

    /// \brief Begin rendering to this FBO
    /// \details All subsequent draw calls will render to this FBO until end() is called.
    /// Automatically sets viewport to FBO dimensions.
    void begin();

    /// \brief End rendering to this FBO
    /// \details Restores previous render target and viewport.
    void end();

    // ========================================================================
    // Drawing
    // ========================================================================

    /// \brief Draw FBO contents to screen at position (x, y)
    /// \param x X position in pixels
    /// \param y Y position in pixels
    void draw(float x, float y) const;

    /// \brief Draw FBO contents with specified dimensions
    /// \param x X position in pixels
    /// \param y Y position in pixels
    /// \param width Width to draw (scales texture if different from FBO size)
    /// \param height Height to draw (scales texture if different from FBO size)
    void draw(float x, float y, float width, float height) const;

    // ========================================================================
    // Texture Access
    // ========================================================================

    /// \brief Get color texture (attachment 0)
    /// \return Reference to color texture
    ofTexture& getTexture();

    /// \brief Get color texture (attachment 0) - const version
    /// \return Const reference to color texture
    const ofTexture& getTexture() const;

    /// \brief Get specific color texture by index
    /// \param attachmentIndex Attachment index (0-3)
    /// \return Reference to color texture
    ofTexture& getTexture(int attachmentIndex);

    /// \brief Get depth texture if allocated
    /// \return Reference to depth texture (throws if depth not enabled)
    ofTexture& getDepthTexture();

    /// \brief Get depth texture if allocated - const version
    /// \return Const reference to depth texture (throws if depth not enabled)
    const ofTexture& getDepthTexture() const;

    // ========================================================================
    // Readback
    // ========================================================================

    /// \brief Read FBO color contents to ofPixels
    /// \param pixels Destination pixel buffer
    void readToPixels(ofPixels& pixels) const;

    /// \brief Read specific color attachment to ofPixels
    /// \param pixels Destination pixel buffer
    /// \param attachmentIndex Attachment index (0-3)
    void readToPixels(ofPixels& pixels, int attachmentIndex) const;

    // ========================================================================
    // Properties
    // ========================================================================

    /// \brief Get FBO width
    /// \return Width in pixels
    int getWidth() const;

    /// \brief Get FBO height
    /// \return Height in pixels
    int getHeight() const;

    /// \brief Get number of color attachments
    /// \return Number of color textures
    int getNumTextures() const;

    /// \brief Check if depth buffer is enabled
    /// \return true if depth buffer exists
    bool hasDepthBuffer() const;

    /// \brief Check if stencil buffer is enabled
    /// \return true if stencil buffer exists
    bool hasStencilBuffer() const;

    // ========================================================================
    // Multi-Attachment Control (MRT)
    // ========================================================================

    /// \brief Set which color attachment to draw to
    /// \param attachmentIndex Attachment index (0-3)
    /// \details Only relevant when using multiple color attachments.
    /// By default, all attachments are active.
    void setActiveDrawBuffer(int attachmentIndex);

    /// \brief Enable all color attachments for drawing
    void setActiveDrawBuffers(const std::vector<int>& attachmentIndices);

    // ========================================================================
    // Native Handle Access (Advanced)
    // ========================================================================

    /// \brief Get native Metal texture handle for color attachment
    /// \param attachmentIndex Attachment index (default: 0)
    /// \return MTLTexture handle (id<MTLTexture>), or nullptr if not allocated
    void* getNativeTextureHandle(int attachmentIndex = 0) const;

    /// \brief Get native Metal depth texture handle
    /// \return MTLTexture handle (id<MTLTexture>), or nullptr if no depth buffer
    void* getNativeDepthHandle() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    void ensureImpl();
};

} // namespace oflike
