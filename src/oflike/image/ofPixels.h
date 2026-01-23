#pragma once

// oflike-metal ofPixels - openFrameworks API compatible pixel buffer
// Provides in-memory pixel data storage and manipulation

#include <cstdint>
#include <cstring>
#include <algorithm>
#include "../types/ofColor.h"

namespace oflike {

/// \brief Image format types
enum ofImageType {
    OF_IMAGE_GRAYSCALE = 1,     ///< Single channel (8-bit)
    OF_IMAGE_COLOR = 3,         ///< RGB (24-bit)
    OF_IMAGE_COLOR_ALPHA = 4    ///< RGBA (32-bit)
};

/// \brief Template pixel buffer class for storing and manipulating image data
/// \details ofPixels_ provides openFrameworks-compatible pixel buffer operations
/// using a template for different pixel types (uint8_t, uint16_t, float)
template<typename PixelType>
class ofPixels_ {
public:
    // ========================================================================
    // Constructors & Destructor
    // ========================================================================

    /// \brief Default constructor
    ofPixels_()
        : pixels_(nullptr)
        , width_(0)
        , height_(0)
        , channels_(0)
        , pixelsOwner_(true)
        , bAllocated_(false) {}

    /// \brief Copy constructor
    ofPixels_(const ofPixels_& mom) {
        copyFrom(mom);
    }

    /// \brief Move constructor
    ofPixels_(ofPixels_&& mom) noexcept {
        pixels_ = mom.pixels_;
        width_ = mom.width_;
        height_ = mom.height_;
        channels_ = mom.channels_;
        pixelsOwner_ = mom.pixelsOwner_;
        bAllocated_ = mom.bAllocated_;

        mom.pixels_ = nullptr;
        mom.width_ = 0;
        mom.height_ = 0;
        mom.channels_ = 0;
        mom.pixelsOwner_ = false;
        mom.bAllocated_ = false;
    }

    /// \brief Destructor
    ~ofPixels_() {
        clear();
    }

    // ========================================================================
    // Assignment Operators
    // ========================================================================

    /// \brief Copy assignment operator
    ofPixels_& operator=(const ofPixels_& mom) {
        if (this != &mom) {
            copyFrom(mom);
        }
        return *this;
    }

    /// \brief Move assignment operator
    ofPixels_& operator=(ofPixels_&& mom) noexcept {
        if (this != &mom) {
            clear();
            pixels_ = mom.pixels_;
            width_ = mom.width_;
            height_ = mom.height_;
            channels_ = mom.channels_;
            pixelsOwner_ = mom.pixelsOwner_;
            bAllocated_ = mom.bAllocated_;

            mom.pixels_ = nullptr;
            mom.width_ = 0;
            mom.height_ = 0;
            mom.channels_ = 0;
            mom.pixelsOwner_ = false;
            mom.bAllocated_ = false;
        }
        return *this;
    }

    // ========================================================================
    // Allocation
    // ========================================================================

    /// \brief Allocate pixel buffer memory
    /// \param w Width in pixels
    /// \param h Height in pixels
    /// \param channels Number of channels (1=grayscale, 3=RGB, 4=RGBA)
    void allocate(size_t w, size_t h, size_t channels) {
        if (w == width_ && h == height_ && channels == channels_ && bAllocated_) {
            return;  // Already allocated with same dimensions
        }

        clear();

        width_ = w;
        height_ = h;
        channels_ = channels;

        if (width_ > 0 && height_ > 0 && channels_ > 0) {
            const size_t totalSize = width_ * height_ * channels_;
            pixels_ = new PixelType[totalSize];
            std::memset(pixels_, 0, totalSize * sizeof(PixelType));
            pixelsOwner_ = true;
            bAllocated_ = true;
        }
    }

    /// \brief Allocate from image type
    void allocate(size_t w, size_t h, ofImageType type) {
        allocate(w, h, static_cast<size_t>(type));
    }

    /// \brief Clear pixel buffer and free memory
    void clear() {
        if (pixels_ && pixelsOwner_) {
            delete[] pixels_;
        }
        pixels_ = nullptr;
        width_ = 0;
        height_ = 0;
        channels_ = 0;
        pixelsOwner_ = false;
        bAllocated_ = false;
    }

    /// \brief Check if pixels are allocated
    bool isAllocated() const {
        return bAllocated_;
    }

    // ========================================================================
    // Data Access
    // ========================================================================

    /// \brief Get pointer to pixel data
    PixelType* getData() {
        return pixels_;
    }

    /// \brief Get const pointer to pixel data
    const PixelType* getData() const {
        return pixels_;
    }

    /// \brief Get pixel value at index
    PixelType& operator[](size_t pos) {
        return pixels_[pos];
    }

    /// \brief Get const pixel value at index
    const PixelType& operator[](size_t pos) const {
        return pixels_[pos];
    }

    // ========================================================================
    // Dimensions
    // ========================================================================

    /// \brief Get width in pixels
    size_t getWidth() const {
        return width_;
    }

    /// \brief Get height in pixels
    size_t getHeight() const {
        return height_;
    }

    /// \brief Get number of channels
    size_t getNumChannels() const {
        return channels_;
    }

    /// \brief Get bytes per pixel
    size_t getBytesPerPixel() const {
        return channels_ * sizeof(PixelType);
    }

    /// \brief Get bytes per channel
    size_t getBytesPerChannel() const {
        return sizeof(PixelType);
    }

    /// \brief Get total number of pixels
    size_t size() const {
        return width_ * height_;
    }

    /// \brief Get total size in bytes
    size_t getTotalBytes() const {
        return width_ * height_ * channels_ * sizeof(PixelType);
    }

    // ========================================================================
    // Pixel Access by Coordinates
    // ========================================================================

    /// \brief Get index for pixel at (x, y)
    size_t getPixelIndex(size_t x, size_t y) const {
        return (y * width_ + x) * channels_;
    }

    /// \brief Get color at pixel (x, y)
    ofColor_<PixelType> getColor(size_t x, size_t y) const {
        if (x >= width_ || y >= height_ || !pixels_) {
            return ofColor_<PixelType>(0, 0, 0, 0);
        }

        const size_t index = getPixelIndex(x, y);

        if (channels_ == 1) {
            // Grayscale
            return ofColor_<PixelType>(pixels_[index]);
        } else if (channels_ == 3) {
            // RGB
            return ofColor_<PixelType>(
                pixels_[index],
                pixels_[index + 1],
                pixels_[index + 2]
            );
        } else if (channels_ == 4) {
            // RGBA
            return ofColor_<PixelType>(
                pixels_[index],
                pixels_[index + 1],
                pixels_[index + 2],
                pixels_[index + 3]
            );
        }

        return ofColor_<PixelType>(0, 0, 0, 0);
    }

    /// \brief Set color at pixel (x, y)
    void setColor(size_t x, size_t y, const ofColor_<PixelType>& color) {
        if (x >= width_ || y >= height_ || !pixels_) {
            return;
        }

        const size_t index = getPixelIndex(x, y);

        if (channels_ == 1) {
            // Grayscale - use luminance
            const float lum = 0.299f * color.r + 0.587f * color.g + 0.114f * color.b;
            pixels_[index] = static_cast<PixelType>(lum);
        } else if (channels_ == 3) {
            // RGB
            pixels_[index] = color.r;
            pixels_[index + 1] = color.g;
            pixels_[index + 2] = color.b;
        } else if (channels_ == 4) {
            // RGBA
            pixels_[index] = color.r;
            pixels_[index + 1] = color.g;
            pixels_[index + 2] = color.b;
            pixels_[index + 3] = color.a;
        }
    }

    /// \brief Set all pixels to a color
    void setColor(const ofColor_<PixelType>& color) {
        if (!pixels_) return;

        for (size_t y = 0; y < height_; ++y) {
            for (size_t x = 0; x < width_; ++x) {
                setColor(x, y, color);
            }
        }
    }

    // ========================================================================
    // Pixel Data Operations
    // ========================================================================

    /// \brief Set pixels from raw data (copies data)
    void setFromPixels(const PixelType* newPixels, size_t w, size_t h, size_t channels) {
        allocate(w, h, channels);
        if (pixels_ && newPixels) {
            std::memcpy(pixels_, newPixels, getTotalBytes());
        }
    }

    /// \brief Set pixels from raw data with same dimensions
    void setFromPixels(const PixelType* newPixels, size_t w, size_t h, ofImageType type) {
        setFromPixels(newPixels, w, h, static_cast<size_t>(type));
    }

    /// \brief Use external pixel data (does not copy, does not own memory)
    void setFromExternalPixels(PixelType* newPixels, size_t w, size_t h, size_t channels) {
        clear();
        pixels_ = newPixels;
        width_ = w;
        height_ = h;
        channels_ = channels;
        pixelsOwner_ = false;  // Don't own this memory
        bAllocated_ = true;
    }

    /// \brief Swap channels (e.g., RGB to BGR)
    void swapRgb() {
        if (!pixels_ || channels_ < 3) return;

        for (size_t i = 0; i < width_ * height_; ++i) {
            const size_t idx = i * channels_;
            std::swap(pixels_[idx], pixels_[idx + 2]);  // Swap R and B
        }
    }

    // ========================================================================
    // Image Type Conversion
    // ========================================================================

    /// \brief Get image type
    ofImageType getImageType() const {
        return static_cast<ofImageType>(channels_);
    }

    /// \brief Set image type (converts channels)
    void setImageType(ofImageType newType) {
        const size_t newChannels = static_cast<size_t>(newType);
        if (newChannels == channels_) return;

        ofPixels_ newPixels;
        newPixels.allocate(width_, height_, newChannels);

        for (size_t y = 0; y < height_; ++y) {
            for (size_t x = 0; x < width_; ++x) {
                ofColor_<PixelType> color = getColor(x, y);
                newPixels.setColor(x, y, color);
            }
        }

        *this = std::move(newPixels);
    }

private:
    // ========================================================================
    // Internal Methods
    // ========================================================================

    void copyFrom(const ofPixels_& mom) {
        if (mom.isAllocated()) {
            allocate(mom.width_, mom.height_, mom.channels_);
            if (pixels_ && mom.pixels_) {
                std::memcpy(pixels_, mom.pixels_, getTotalBytes());
            }
        } else {
            clear();
        }
    }

    // ========================================================================
    // Member Variables
    // ========================================================================

    PixelType* pixels_;      ///< Pointer to pixel data
    size_t width_;           ///< Width in pixels
    size_t height_;          ///< Height in pixels
    size_t channels_;        ///< Number of channels (1, 3, or 4)
    bool pixelsOwner_;       ///< True if we own the pixel memory
    bool bAllocated_;        ///< True if pixels are allocated
};

// ========================================================================
// Type Aliases
// ========================================================================

using ofPixels = ofPixels_<uint8_t>;          ///< 8-bit pixels (0-255)
using ofShortPixels = ofPixels_<uint16_t>;    ///< 16-bit pixels (0-65535)
using ofFloatPixels = ofPixels_<float>;       ///< Float pixels (0.0-1.0)

}  // namespace oflike
