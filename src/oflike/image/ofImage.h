#pragma once

// oflike-metal ofImage - openFrameworks API compatible image
// Provides high-level image loading, saving, and manipulation

#include <string>
#include <memory>
#include "ofPixels.h"
#include "ofTexture.h"

namespace oflike {

/// \brief High-level image class combining pixel data and texture
/// \details ofImage provides openFrameworks-compatible image operations
/// including loading from files, saving to files, pixel manipulation,
/// and rendering to screen. It combines ofPixels (CPU data) and ofTexture
/// (GPU data) in a single convenient class.
///
/// Features:
/// - Load images from files (PNG, JPG, GIF, BMP, TIFF, etc. via ImageIO)
/// - Save images to files (PNG, JPG, TIFF via CGImageDestination)
/// - Access and modify pixel data
/// - Draw images to screen
/// - Resize, crop, rotate, and mirror operations
/// - Convert between image types (grayscale, RGB, RGBA)
///
/// Implementation:
/// - Uses ImageIO framework for loading (hardware accelerated)
/// - Uses MTKTextureLoader for direct GPU loading (fastest path)
/// - Uses CGImageDestination for saving
/// - pImpl pattern to hide Objective-C++ details
///
/// Example:
/// \code
///     ofImage image;
///     image.load("photo.jpg");
///     image.resize(512, 512);
///     image.draw(100, 100);
///     image.save("resized.png");
/// \endcode
class ofImage {
public:
    // ========================================================================
    // Constructors & Destructor
    // ========================================================================

    /// \brief Default constructor
    ofImage();

    /// \brief Construct and load image from file
    /// \param fileName Path to image file
    explicit ofImage(const std::string& fileName);

    /// \brief Destructor
    ~ofImage();

    /// \brief Move constructor
    ofImage(ofImage&& other) noexcept;

    /// \brief Move assignment
    ofImage& operator=(ofImage&& other) noexcept;

    // Delete copy constructor and assignment (images contain large data)
    ofImage(const ofImage&) = delete;
    ofImage& operator=(const ofImage&) = delete;

    // ========================================================================
    // Loading & Saving
    // ========================================================================

    /// \brief Load image from file
    /// \details Supports formats: PNG, JPG, GIF, BMP, TIFF, and more (via ImageIO)
    /// Uses MTKTextureLoader for direct GPU upload, falls back to ImageIO
    /// \param fileName Path to image file (absolute or relative)
    /// \return true if successful, false otherwise
    bool load(const std::string& fileName);

    /// \brief Save image to file
    /// \details Supported formats: PNG, JPG, TIFF
    /// Format is determined by file extension
    /// Uses CGImageDestination with hardware acceleration
    /// \param fileName Path to save image file
    /// \param quality JPEG quality 0.0-1.0 (default 0.9), ignored for PNG/TIFF
    /// \return true if successful, false otherwise
    bool save(const std::string& fileName, float quality = 0.9f);

    // ========================================================================
    // Drawing
    // ========================================================================

    /// \brief Draw image at position
    /// \param x X coordinate
    /// \param y Y coordinate
    void draw(float x, float y) const;

    /// \brief Draw image at position with size
    /// \param x X coordinate
    /// \param y Y coordinate
    /// \param w Width to draw
    /// \param h Height to draw
    void draw(float x, float y, float w, float h) const;

    /// \brief Draw image at position with z-coordinate
    /// \param x X coordinate
    /// \param y Y coordinate
    /// \param z Z coordinate
    void draw(float x, float y, float z) const;

    /// \brief Draw image at position with size and z-coordinate
    /// \param x X coordinate
    /// \param y Y coordinate
    /// \param z Z coordinate
    /// \param w Width to draw
    /// \param h Height to draw
    void draw(float x, float y, float z, float w, float h) const;

    // ========================================================================
    // Data Access
    // ========================================================================

    /// \brief Get reference to pixel data
    /// \details Pixels are synchronized from GPU if needed
    /// \return Reference to ofPixels containing image data
    ofPixels& getPixels();

    /// \brief Get const reference to pixel data
    /// \return Const reference to ofPixels containing image data
    const ofPixels& getPixels() const;

    /// \brief Get reference to texture
    /// \details Texture is synchronized from pixels if needed
    /// \return Reference to ofTexture for GPU rendering
    ofTexture& getTexture();

    /// \brief Get const reference to texture
    /// \return Const reference to ofTexture for GPU rendering
    const ofTexture& getTexture() const;

    // ========================================================================
    // Pixel Manipulation
    // ========================================================================

    /// \brief Set image from pixel data
    /// \details Copies pixel data and uploads to GPU
    /// \param pixels Source pixel data
    void setFromPixels(const ofPixels& pixels);

    /// \brief Set image from pixel data (16-bit)
    void setFromPixels(const ofShortPixels& pixels);

    /// \brief Set image from pixel data (float)
    void setFromPixels(const ofFloatPixels& pixels);

    /// \brief Set image from raw pixel data
    /// \param pixels Pointer to raw pixel data
    /// \param w Width in pixels
    /// \param h Height in pixels
    /// \param type Image type (OF_IMAGE_GRAYSCALE, OF_IMAGE_COLOR, OF_IMAGE_COLOR_ALPHA)
    void setFromPixels(const unsigned char* pixels, int w, int h, ofImageType type);

    /// \brief Update texture from modified pixels
    /// \details Call this after modifying pixels to sync GPU texture
    void update();

    // ========================================================================
    // Image Manipulation
    // ========================================================================

    /// \brief Resize image to new dimensions
    /// \details Uses high-quality Lanczos resampling via vImage (Accelerate framework)
    /// \param newWidth New width in pixels
    /// \param newHeight New height in pixels
    void resize(int newWidth, int newHeight);

    /// \brief Crop image to specified rectangle
    /// \param x Left position
    /// \param y Top position
    /// \param w Crop width
    /// \param h Crop height
    void crop(int x, int y, int w, int h);

    /// \brief Rotate image 90 degrees clockwise
    /// \param nClockwiseTurns Number of 90-degree clockwise rotations (default 1)
    void rotate90(int nClockwiseTurns = 1);

    /// \brief Mirror image horizontally or vertically
    /// \param vertical If true, mirror vertically; if false, mirror horizontally
    void mirror(bool vertical = false);

    // ========================================================================
    // Format Conversion
    // ========================================================================

    /// \brief Set image type (convert channels)
    /// \details Converts between grayscale, RGB, and RGBA
    /// \param newType Target image type
    void setImageType(ofImageType newType);

    /// \brief Get current image type
    /// \return Current image type
    ofImageType getImageType() const;

    // ========================================================================
    // Properties
    // ========================================================================

    /// \brief Get image width
    /// \return Width in pixels
    float getWidth() const;

    /// \brief Get image height
    /// \return Height in pixels
    float getHeight() const;

    /// \brief Check if image is allocated
    /// \return true if image has data, false otherwise
    bool isAllocated() const;

    /// \brief Check if image was loaded and is valid
    /// \return true if image is loaded, false otherwise
    bool isLoaded() const;

    /// \brief Clear image data and free memory
    void clear();

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

    /// \brief Synchronize pixels from texture
    void syncPixelsFromTexture() const;

    /// \brief Synchronize texture from pixels
    void syncTextureFromPixels();
};

} // namespace oflike
