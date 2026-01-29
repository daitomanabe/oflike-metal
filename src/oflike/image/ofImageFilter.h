#pragma once

// oflike-metal ofImageFilter - Image filtering operations
// Provides blur, sharpen, contrast, and other image filters
// Uses Accelerate framework (vImage) for CPU processing

#include "ofPixels.h"
#include <string>

namespace oflike {

/// \brief Image filtering operations using Accelerate framework
/// \details ofImageFilter provides various image processing operations
/// optimized using the vImage API from Accelerate framework.
///
/// Features:
/// - Gaussian blur
/// - Box blur
/// - Sharpen
/// - Contrast adjustment
/// - Brightness adjustment
/// - Saturation adjustment
/// - Invert
/// - Edge detection (Sobel, Laplacian)
/// - Convolution with custom kernels
///
/// All operations work in-place on ofPixels or return a new ofPixels.
///
/// Example:
/// \code
///     ofPixels pixels;
///     pixels.allocate(512, 512, 4);
///     // ... fill with data ...
///
///     ofImageFilter::blur(pixels, 5.0f);      // Gaussian blur
///     ofImageFilter::sharpen(pixels, 1.5f);   // Sharpen
///     ofImageFilter::contrast(pixels, 1.2f); // Increase contrast
/// \endcode
class ofImageFilter {
public:
    // ========================================================================
    // Blur Operations
    // ========================================================================

    /// \brief Apply Gaussian blur
    /// \param pixels Pixel data to modify (in-place)
    /// \param radius Blur radius in pixels (0.0 to ~100.0)
    /// \return true on success
    static bool blur(ofPixels& pixels, float radius);

    /// \brief Apply Gaussian blur (returns new pixels)
    /// \param src Source pixel data
    /// \param dst Destination pixel data
    /// \param radius Blur radius in pixels
    /// \return true on success
    static bool blur(const ofPixels& src, ofPixels& dst, float radius);

    /// \brief Apply box blur (fast, lower quality)
    /// \param pixels Pixel data to modify (in-place)
    /// \param radius Blur radius in pixels
    /// \return true on success
    static bool boxBlur(ofPixels& pixels, int radius);

    // ========================================================================
    // Sharpen Operations
    // ========================================================================

    /// \brief Apply unsharp mask sharpening
    /// \param pixels Pixel data to modify (in-place)
    /// \param amount Sharpening amount (0.0 = none, 1.0 = normal, >1.0 = strong)
    /// \param radius Blur radius for the unsharp mask
    /// \return true on success
    static bool sharpen(ofPixels& pixels, float amount, float radius = 1.0f);

    /// \brief Apply high-pass sharpening
    /// \param pixels Pixel data to modify (in-place)
    /// \param strength Sharpening strength (0.0 to 2.0)
    /// \return true on success
    static bool sharpenHighPass(ofPixels& pixels, float strength);

    // ========================================================================
    // Color Adjustments
    // ========================================================================

    /// \brief Adjust contrast
    /// \param pixels Pixel data to modify (in-place)
    /// \param contrast Contrast multiplier (1.0 = no change, <1.0 = less, >1.0 = more)
    /// \return true on success
    static bool contrast(ofPixels& pixels, float contrast);

    /// \brief Adjust brightness
    /// \param pixels Pixel data to modify (in-place)
    /// \param brightness Brightness offset (-255 to 255)
    /// \return true on success
    static bool brightness(ofPixels& pixels, float brightness);

    /// \brief Adjust brightness and contrast together
    /// \param pixels Pixel data to modify (in-place)
    /// \param brightness Brightness offset (-255 to 255)
    /// \param contrast Contrast multiplier (1.0 = no change)
    /// \return true on success
    static bool brightnessContrast(ofPixels& pixels, float brightness, float contrast);

    /// \brief Adjust saturation
    /// \param pixels Pixel data to modify (in-place)
    /// \param saturation Saturation multiplier (0.0 = grayscale, 1.0 = normal, >1.0 = more)
    /// \return true on success
    static bool saturation(ofPixels& pixels, float saturation);

    /// \brief Convert to grayscale
    /// \param pixels Pixel data to modify (in-place, keeps same channel count)
    /// \return true on success
    static bool grayscale(ofPixels& pixels);

    /// \brief Invert colors
    /// \param pixels Pixel data to modify (in-place)
    /// \return true on success
    static bool invert(ofPixels& pixels);

    /// \brief Apply gamma correction
    /// \param pixels Pixel data to modify (in-place)
    /// \param gamma Gamma value (1.0 = no change, <1.0 = brighten, >1.0 = darken)
    /// \return true on success
    static bool gamma(ofPixels& pixels, float gamma);

    // ========================================================================
    // Edge Detection
    // ========================================================================

    /// \brief Apply Sobel edge detection
    /// \param pixels Pixel data to modify (in-place)
    /// \return true on success
    static bool sobel(ofPixels& pixels);

    /// \brief Apply Laplacian edge detection
    /// \param pixels Pixel data to modify (in-place)
    /// \return true on success
    static bool laplacian(ofPixels& pixels);

    // ========================================================================
    // Morphological Operations
    // ========================================================================

    /// \brief Apply dilation (expand bright regions)
    /// \param pixels Pixel data to modify (in-place)
    /// \param radius Dilation radius
    /// \return true on success
    static bool dilate(ofPixels& pixels, int radius);

    /// \brief Apply erosion (expand dark regions)
    /// \param pixels Pixel data to modify (in-place)
    /// \param radius Erosion radius
    /// \return true on success
    static bool erode(ofPixels& pixels, int radius);

    // ========================================================================
    // Custom Convolution
    // ========================================================================

    /// \brief Apply custom convolution kernel
    /// \param pixels Pixel data to modify (in-place)
    /// \param kernel Convolution kernel data (row-major order)
    /// \param kernelWidth Kernel width (must be odd)
    /// \param kernelHeight Kernel height (must be odd)
    /// \param divisor Divisor for normalization (sum of kernel values if 0)
    /// \return true on success
    static bool convolve(ofPixels& pixels,
                         const float* kernel,
                         int kernelWidth,
                         int kernelHeight,
                         float divisor = 0.0f);

    // ========================================================================
    // Thresholding
    // ========================================================================

    /// \brief Apply threshold
    /// \param pixels Pixel data to modify (in-place)
    /// \param threshold Threshold value (0-255)
    /// \return true on success
    static bool threshold(ofPixels& pixels, int threshold);

    /// \brief Apply adaptive threshold
    /// \param pixels Pixel data to modify (in-place)
    /// \param blockSize Size of local region for threshold calculation
    /// \param constant Constant subtracted from mean
    /// \return true on success
    static bool adaptiveThreshold(ofPixels& pixels, int blockSize, int constant);

    // ========================================================================
    // Noise
    // ========================================================================

    /// \brief Add random noise
    /// \param pixels Pixel data to modify (in-place)
    /// \param amount Noise amount (0.0 to 1.0)
    /// \return true on success
    static bool addNoise(ofPixels& pixels, float amount);

    /// \brief Apply median filter (noise reduction)
    /// \param pixels Pixel data to modify (in-place)
    /// \param radius Filter radius
    /// \return true on success
    static bool median(ofPixels& pixels, int radius);

private:
    /// \brief Clamp value to byte range
    static inline unsigned char clampToByte(int value) {
        return static_cast<unsigned char>(std::max(0, std::min(255, value)));
    }

    /// \brief Clamp value to byte range (float version)
    static inline unsigned char clampToByte(float value) {
        return static_cast<unsigned char>(std::max(0.0f, std::min(255.0f, value)));
    }
};

} // namespace oflike
