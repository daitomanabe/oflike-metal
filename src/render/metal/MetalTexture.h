#pragma once

#include <cstdint>
#include <memory>

namespace render {
namespace metal {

// ============================================================================
// TextureFormat - Texture Pixel Format
// ============================================================================

enum class TextureFormat {
    RGBA8,      // 8-bit per channel RGBA
    BGRA8,      // 8-bit per channel BGRA (common on macOS)
    RGB8,       // 8-bit per channel RGB (not directly supported, converted to RGBA8)
    R8,         // 8-bit single channel
    RG8,        // 8-bit dual channel
    RGBA16F,    // 16-bit float per channel RGBA
    RGBA32F,    // 32-bit float per channel RGBA
    Depth32F,   // 32-bit float depth
    Stencil8,   // 8-bit stencil
};

// ============================================================================
// TextureWrap - Texture Wrapping Mode
// ============================================================================

enum class TextureWrap {
    Clamp,      // Clamp to edge
    Repeat,     // Repeat/wrap
    Mirror,     // Mirror repeat
};

// ============================================================================
// TextureFilter - Texture Filtering Mode
// ============================================================================

enum class TextureFilter {
    Nearest,    // Nearest neighbor
    Linear,     // Linear interpolation
};

// ============================================================================
// MetalTexture - Metal Texture Wrapper
// ============================================================================

/**
 * Wraps MTLTexture with convenient API for texture creation and loading.
 *
 * Features:
 * - Create textures from raw pixel data
 * - Load textures from image files using MTKTextureLoader
 * - Configure texture wrap and filter modes
 * - Support multiple pixel formats
 *
 * Implementation:
 * - Uses pImpl pattern to hide Objective-C++ Metal code
 * - Thread-safety: Main thread only
 * - Automatic cleanup on destruction
 *
 * Usage:
 *   MetalTexture texture(device);
 *   texture.create(width, height, format, data);
 *   void* mtlTexture = texture.getTexture();
 */
class MetalTexture {
public:
    /**
     * Constructor.
     * @param device Metal device (id<MTLDevice>)
     */
    explicit MetalTexture(void* device);

    ~MetalTexture();

    // Disable copy and move
    MetalTexture(const MetalTexture&) = delete;
    MetalTexture& operator=(const MetalTexture&) = delete;
    MetalTexture(MetalTexture&&) = delete;
    MetalTexture& operator=(MetalTexture&&) = delete;

    // ========================================================================
    // Texture Creation
    // ========================================================================

    /**
     * Create a texture from raw pixel data.
     * @param width Texture width in pixels
     * @param height Texture height in pixels
     * @param format Pixel format
     * @param data Pointer to pixel data, or nullptr for empty texture
     * @return true on success, false on failure
     */
    bool create(uint32_t width, uint32_t height, TextureFormat format, const void* data = nullptr);

    /**
     * Load a texture from an image file.
     * @param path File path to image (supports PNG, JPG, etc.)
     * @return true on success, false on failure
     */
    bool loadFromFile(const char* path);

    /**
     * Update texture data.
     * @param data Pointer to pixel data
     * @param region Optional region to update (nullptr = entire texture)
     * @return true on success, false on failure
     */
    bool updateData(const void* data, const void* region = nullptr);

    // ========================================================================
    // Texture Properties
    // ========================================================================

    /**
     * Get Metal texture handle.
     * @return MTLTexture handle (id<MTLTexture>), or nullptr if not created
     */
    void* getTexture() const;

    /**
     * Get texture width.
     */
    uint32_t getWidth() const;

    /**
     * Get texture height.
     */
    uint32_t getHeight() const;

    /**
     * Get texture format.
     */
    TextureFormat getFormat() const;

    /**
     * Check if texture is valid.
     */
    bool isValid() const;

    // ========================================================================
    // Texture Settings
    // ========================================================================

    /**
     * Set texture wrap mode.
     * @param wrapS Wrap mode for S (horizontal) axis
     * @param wrapT Wrap mode for T (vertical) axis
     */
    void setWrap(TextureWrap wrapS, TextureWrap wrapT);

    /**
     * Set texture filter mode.
     * @param minFilter Minification filter
     * @param magFilter Magnification filter
     */
    void setFilter(TextureFilter minFilter, TextureFilter magFilter);

    /**
     * Get sampler state for this texture.
     * @return MTLSamplerState handle (id<MTLSamplerState>), or nullptr if not created
     */
    void* getSamplerState();

    // ========================================================================
    // Utility
    // ========================================================================

    /**
     * Release texture resources.
     */
    void release();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// ============================================================================
// MetalTextureCache - Texture Cache Management
// ============================================================================

/**
 * Manages a cache of loaded textures to avoid redundant loads.
 *
 * Features:
 * - Path-based texture caching
 * - Automatic texture cleanup when unused
 * - Statistics tracking
 *
 * Implementation:
 * - Uses pImpl pattern
 * - Thread-safety: Main thread only
 *
 * Usage:
 *   MetalTextureCache cache(device);
 *   MetalTexture* texture = cache.load("image.png");
 *   cache.clear();
 */
class MetalTextureCache {
public:
    /**
     * Constructor.
     * @param device Metal device (id<MTLDevice>)
     */
    explicit MetalTextureCache(void* device);

    ~MetalTextureCache();

    // Disable copy and move
    MetalTextureCache(const MetalTextureCache&) = delete;
    MetalTextureCache& operator=(const MetalTextureCache&) = delete;
    MetalTextureCache(MetalTextureCache&&) = delete;
    MetalTextureCache& operator=(MetalTextureCache&&) = delete;

    // ========================================================================
    // Cache Management
    // ========================================================================

    /**
     * Load a texture from file, using cache if available.
     * @param path File path to image
     * @return MetalTexture pointer, or nullptr on failure
     */
    MetalTexture* load(const char* path);

    /**
     * Create a texture with raw data (not cached).
     * @param width Texture width
     * @param height Texture height
     * @param format Pixel format
     * @param data Pixel data
     * @return MetalTexture pointer, or nullptr on failure
     */
    MetalTexture* create(uint32_t width, uint32_t height, TextureFormat format, const void* data);

    /**
     * Remove a texture from the cache.
     * @param path File path
     */
    void remove(const char* path);

    /**
     * Clear all cached textures.
     */
    void clear();

    /**
     * Get number of textures in the cache.
     */
    size_t getCacheSize() const;

    /**
     * Check if a texture is in the cache.
     * @param path File path
     * @return true if cached, false otherwise
     */
    bool isCached(const char* path) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace metal
} // namespace render
