#pragma once

// oflike-metal ofTexture - openFrameworks API compatible texture
// Provides GPU texture storage and rendering

#include <memory>
#include <string>
#include "ofPixels.h"

namespace oflike {

// Forward declarations
class ofTexture;

/// \brief Texture wrap mode for U/V coordinates
enum ofTexwrapMode_t {
    OF_TEXTURE_WRAP_REPEAT,         ///< Repeat texture
    OF_TEXTURE_WRAP_CLAMP_TO_EDGE,  ///< Clamp to edge
    OF_TEXTURE_WRAP_MIRRORED_REPEAT ///< Mirror repeat
};

/// \brief Texture filter mode for minification and magnification
enum ofTexFilterMode_t {
    OF_TEXTURE_FILTER_NEAREST, ///< Nearest neighbor
    OF_TEXTURE_FILTER_LINEAR   ///< Linear interpolation
};

/// \brief GPU texture for rendering images
/// \details ofTexture provides openFrameworks-compatible texture operations
/// using Metal backend. Supports loading from files, uploading from ofPixels,
/// and drawing to screen.
///
/// Features:
/// - Load textures from image files (PNG, JPG, etc.)
/// - Upload pixel data from ofPixels
/// - Allocate empty textures
/// - Configure texture wrap and filter modes
/// - Draw textures with positioning
/// - Bind/unbind for shader usage
///
/// Implementation:
/// - Uses pImpl pattern to hide Metal/Objective-C++ details
/// - Pure C++ header (.h), Objective-C++ implementation (.mm)
/// - Internally wraps MetalTexture from render/metal layer
/// - Thread-safety: Main thread only (Metal requirement)
///
/// Example:
/// \code
///     ofTexture texture;
///     texture.allocate(512, 512, OF_IMAGE_COLOR_ALPHA);
///     ofPixels pixels;
///     pixels.allocate(512, 512, OF_IMAGE_COLOR_ALPHA);
///     // ... fill pixels with data ...
///     texture.loadData(pixels);
///     texture.draw(100, 100);
/// \endcode
class ofTexture {
public:
    // ========================================================================
    // Constructors & Destructor
    // ========================================================================

    /// \brief Default constructor
    ofTexture();

    /// \brief Destructor
    ~ofTexture();

    /// \brief Move constructor
    ofTexture(ofTexture&& other) noexcept;

    /// \brief Move assignment
    ofTexture& operator=(ofTexture&& other) noexcept;

    // Delete copy constructor and assignment (textures can't be copied, only moved)
    ofTexture(const ofTexture&) = delete;
    ofTexture& operator=(const ofTexture&) = delete;

    // ========================================================================
    // Allocation
    // ========================================================================

    /// \brief Allocate empty texture with specified dimensions
    /// \param w Width in pixels
    /// \param h Height in pixels
    /// \param internalFormat Image format (OF_IMAGE_GRAYSCALE, OF_IMAGE_COLOR, OF_IMAGE_COLOR_ALPHA)
    void allocate(int w, int h, int internalFormat);

    /// \brief Allocate empty texture with same format as ofPixels
    /// \param pix ofPixels to match dimensions and format
    void allocate(const ofPixels& pix);

    /// \brief Check if texture is allocated
    /// \return true if texture has been allocated, false otherwise
    bool isAllocated() const;

    /// \brief Clear texture and free GPU memory
    void clear();

    // ========================================================================
    // Data Upload
    // ========================================================================

    /// \brief Upload pixel data to texture
    /// \param pix Pixel data to upload
    void loadData(const ofPixels& pix);

    /// \brief Upload pixel data to texture (16-bit)
    void loadData(const ofShortPixels& pix);

    /// \brief Upload pixel data to texture (float)
    void loadData(const ofFloatPixels& pix);

    /// \brief Upload raw pixel data to texture
    /// \param data Pointer to pixel data
    /// \param w Width in pixels
    /// \param h Height in pixels
    /// \param glFormat Image format (OF_IMAGE_GRAYSCALE, OF_IMAGE_COLOR, OF_IMAGE_COLOR_ALPHA)
    void loadData(const void* data, int w, int h, int glFormat);

    // ========================================================================
    // Data Readback
    // ========================================================================

    /// \brief Read pixel data from GPU texture to CPU memory
    /// \param pix Destination pixel buffer (will be allocated if needed)
    /// \return true on success, false on failure
    bool readToPixels(ofPixels& pix) const;

    // ========================================================================
    // Drawing
    // ========================================================================

    /// \brief Draw texture at position
    /// \param x X coordinate
    /// \param y Y coordinate
    void draw(float x, float y) const;

    /// \brief Draw texture at position with size
    /// \param x X coordinate
    /// \param y Y coordinate
    /// \param w Width to draw
    /// \param h Height to draw
    void draw(float x, float y, float w, float h) const;

    /// \brief Draw texture at position with z-coordinate
    /// \param x X coordinate
    /// \param y Y coordinate
    /// \param z Z coordinate
    void draw(float x, float y, float z) const;

    /// \brief Draw texture at position with size and z-coordinate
    /// \param x X coordinate
    /// \param y Y coordinate
    /// \param z Z coordinate
    /// \param w Width to draw
    /// \param h Height to draw
    void draw(float x, float y, float z, float w, float h) const;

    // ========================================================================
    // Binding (for shader use)
    // ========================================================================

    /// \brief Bind texture to texture unit
    /// \param textureUnit Texture unit to bind to (default 0)
    void bind(int textureUnit = 0) const;

    /// \brief Unbind texture from texture unit
    /// \param textureUnit Texture unit to unbind from (default 0)
    void unbind(int textureUnit = 0) const;

    // ========================================================================
    // Texture Settings
    // ========================================================================

    /// \brief Set texture wrap mode
    /// \param wrapModeHorizontal Wrap mode for horizontal (U) axis
    /// \param wrapModeVertical Wrap mode for vertical (V) axis
    void setTextureWrap(ofTexwrapMode_t wrapModeHorizontal, ofTexwrapMode_t wrapModeVertical);

    /// \brief Set texture minification and magnification filter
    /// \param minFilter Minification filter mode
    /// \param magFilter Magnification filter mode
    void setTextureMinMagFilter(ofTexFilterMode_t minFilter, ofTexFilterMode_t magFilter);

    // ========================================================================
    // Mipmap (Phase 2)
    // ========================================================================

    /// \brief Enable mipmap generation for this texture
    /// \details When enabled, mipmaps will be generated when texture data is loaded.
    /// Must be called before allocate() or loadData().
    void enableMipmap();

    /// \brief Disable mipmap generation
    void disableMipmap();

    /// \brief Check if mipmaps are enabled
    /// \return true if mipmaps are enabled
    bool hasMipmap() const;

    /// \brief Generate mipmaps for current texture data
    /// \details Explicitly generates mipmaps. Called automatically if enabled
    /// when loadData() is called. Requires texture to be allocated first.
    void generateMipmap();

    /// \brief Set mipmap filter mode
    /// \param filter Filter mode for mipmap sampling (LINEAR for trilinear filtering)
    void setMipmapFilter(ofTexFilterMode_t filter);

    /// \brief Get number of mipmap levels
    /// \return Number of mipmap levels (1 if no mipmaps)
    int getNumMipmapLevels() const;

    // ========================================================================
    // Anisotropic Filtering (Phase 2)
    // ========================================================================

    /// \brief Set maximum anisotropic filtering level
    /// \param level Anisotropy level (1 = disabled, 2-16 = enabled)
    void setMaxAnisotropy(int level);

    /// \brief Get current anisotropic filtering level
    /// \return Current anisotropy level
    int getMaxAnisotropy() const;

    // ========================================================================
    // Properties
    // ========================================================================

    /// \brief Get texture width
    /// \return Width in pixels
    int getWidth() const;

    /// \brief Get texture height
    /// \return Height in pixels
    int getHeight() const;

    // ========================================================================
    // Native Handle Access
    // ========================================================================

    /// \brief Get native Metal texture handle
    /// \details Returns the underlying id<MTLTexture> as a void pointer.
    /// Use with caution - requires knowledge of Metal API.
    /// Cast to id<MTLTexture> using __bridge in Objective-C++ code.
    /// \return Metal texture handle (id<MTLTexture>), or nullptr if not allocated
    void* getNativeHandle() const;

private:
    // ========================================================================
    // pImpl Pattern
    // ========================================================================

    struct Impl;
    std::unique_ptr<Impl> impl_;

    // ========================================================================
    // Internal Helper Methods
    // ========================================================================

    /// \brief Initialize impl if needed
    void ensureImpl();
};

} // namespace oflike
