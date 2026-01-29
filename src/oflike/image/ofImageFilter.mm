#import "ofImageFilter.h"
#import <Accelerate/Accelerate.h>
#import <cmath>
#import <cstdlib>
#import <vector>

namespace oflike {

// ============================================================================
// Helper Functions
// ============================================================================

/// Create vImage_Buffer from ofPixels
static vImage_Buffer CreateBuffer(ofPixels& pixels) {
    return vImage_Buffer{
        .data = pixels.getData(),
        .height = static_cast<vImagePixelCount>(pixels.getHeight()),
        .width = static_cast<vImagePixelCount>(pixels.getWidth()),
        .rowBytes = pixels.getWidth() * pixels.getNumChannels()
    };
}

/// Create vImage_Buffer from const ofPixels (for source)
static vImage_Buffer CreateBufferConst(const ofPixels& pixels) {
    return vImage_Buffer{
        .data = const_cast<unsigned char*>(pixels.getData()),
        .height = static_cast<vImagePixelCount>(pixels.getHeight()),
        .width = static_cast<vImagePixelCount>(pixels.getWidth()),
        .rowBytes = pixels.getWidth() * pixels.getNumChannels()
    };
}

// ============================================================================
// Blur Operations
// ============================================================================

bool ofImageFilter::blur(ofPixels& pixels, float radius) {
    if (!pixels.isAllocated() || radius <= 0) {
        return false;
    }

    @autoreleasepool {
        // Calculate kernel size (must be odd)
        int kernelSize = static_cast<int>(radius * 2.0f) | 1;  // Ensure odd
        if (kernelSize < 3) kernelSize = 3;
        if (kernelSize > 255) kernelSize = 255;  // vImage limit

        const size_t width = pixels.getWidth();
        const size_t height = pixels.getHeight();
        const size_t channels = pixels.getNumChannels();

        // Create temporary buffer for processing
        ofPixels temp;
        temp.allocate(width, height, channels);
        std::memcpy(temp.getData(), pixels.getData(), pixels.getTotalBytes());

        vImage_Buffer srcBuffer = CreateBufferConst(temp);
        vImage_Buffer dstBuffer = CreateBuffer(pixels);

        vImage_Error error = kvImageNoError;

        if (channels == 1) {
            error = vImageTentConvolve_Planar8(&srcBuffer, &dstBuffer, NULL,
                                               0, 0, kernelSize, kernelSize,
                                               0, kvImageEdgeExtend);
        } else if (channels == 4) {
            error = vImageTentConvolve_ARGB8888(&srcBuffer, &dstBuffer, NULL,
                                                0, 0, kernelSize, kernelSize,
                                                0, kvImageEdgeExtend);
        } else if (channels == 3) {
            // Process each channel separately for RGB
            std::vector<unsigned char> channelSrc(width * height);
            std::vector<unsigned char> channelDst(width * height);

            for (size_t c = 0; c < 3; ++c) {
                // Extract channel
                for (size_t i = 0; i < width * height; ++i) {
                    channelSrc[i] = temp.getData()[i * 3 + c];
                }

                vImage_Buffer chSrc = {channelSrc.data(),
                                       static_cast<vImagePixelCount>(height),
                                       static_cast<vImagePixelCount>(width),
                                       width};
                vImage_Buffer chDst = {channelDst.data(),
                                       static_cast<vImagePixelCount>(height),
                                       static_cast<vImagePixelCount>(width),
                                       width};

                error = vImageTentConvolve_Planar8(&chSrc, &chDst, NULL,
                                                   0, 0, kernelSize, kernelSize,
                                                   0, kvImageEdgeExtend);
                if (error != kvImageNoError) break;

                // Write back channel
                for (size_t i = 0; i < width * height; ++i) {
                    pixels.getData()[i * 3 + c] = channelDst[i];
                }
            }
        }

        return error == kvImageNoError;
    }
}

bool ofImageFilter::blur(const ofPixels& src, ofPixels& dst, float radius) {
    if (!src.isAllocated()) return false;

    dst.allocate(src.getWidth(), src.getHeight(), src.getNumChannels());
    std::memcpy(dst.getData(), src.getData(), src.getTotalBytes());

    return blur(dst, radius);
}

bool ofImageFilter::boxBlur(ofPixels& pixels, int radius) {
    if (!pixels.isAllocated() || radius <= 0) {
        return false;
    }

    @autoreleasepool {
        int kernelSize = radius * 2 + 1;
        if (kernelSize > 255) kernelSize = 255;

        const size_t channels = pixels.getNumChannels();

        ofPixels temp;
        temp.allocate(pixels.getWidth(), pixels.getHeight(), channels);
        std::memcpy(temp.getData(), pixels.getData(), pixels.getTotalBytes());

        vImage_Buffer srcBuffer = CreateBufferConst(temp);
        vImage_Buffer dstBuffer = CreateBuffer(pixels);

        vImage_Error error = kvImageNoError;

        if (channels == 1) {
            error = vImageBoxConvolve_Planar8(&srcBuffer, &dstBuffer, NULL,
                                              0, 0, kernelSize, kernelSize,
                                              0, kvImageEdgeExtend);
        } else if (channels == 4) {
            error = vImageBoxConvolve_ARGB8888(&srcBuffer, &dstBuffer, NULL,
                                               0, 0, kernelSize, kernelSize,
                                               0, kvImageEdgeExtend);
        }

        return error == kvImageNoError;
    }
}

// ============================================================================
// Sharpen Operations
// ============================================================================

bool ofImageFilter::sharpen(ofPixels& pixels, float amount, float radius) {
    if (!pixels.isAllocated() || amount <= 0) {
        return true;  // No sharpening needed
    }

    @autoreleasepool {
        const size_t width = pixels.getWidth();
        const size_t height = pixels.getHeight();
        const size_t channels = pixels.getNumChannels();
        const size_t totalBytes = pixels.getTotalBytes();

        // Create blurred copy
        ofPixels blurred;
        blurred.allocate(width, height, channels);
        std::memcpy(blurred.getData(), pixels.getData(), totalBytes);

        if (!blur(blurred, radius)) {
            return false;
        }

        // Unsharp mask: original + amount * (original - blurred)
        unsigned char* src = pixels.getData();
        unsigned char* blurData = blurred.getData();

        for (size_t i = 0; i < totalBytes; ++i) {
            float original = static_cast<float>(src[i]);
            float blur = static_cast<float>(blurData[i]);
            float sharpened = original + amount * (original - blur);
            src[i] = clampToByte(sharpened);
        }

        return true;
    }
}

bool ofImageFilter::sharpenHighPass(ofPixels& pixels, float strength) {
    if (!pixels.isAllocated()) return false;

    // High-pass sharpening kernel
    float center = 1.0f + 4.0f * strength;
    float edge = -strength;

    // 3x3 sharpening kernel
    float kernel[9] = {
        0,      edge,   0,
        edge,   center, edge,
        0,      edge,   0
    };

    return convolve(pixels, kernel, 3, 3, 1.0f);
}

// ============================================================================
// Color Adjustments
// ============================================================================

bool ofImageFilter::contrast(ofPixels& pixels, float contrast) {
    if (!pixels.isAllocated()) return false;

    const size_t totalBytes = pixels.getTotalBytes();
    const size_t channels = pixels.getNumChannels();
    unsigned char* data = pixels.getData();

    // Contrast adjustment: (pixel - 128) * contrast + 128
    for (size_t i = 0; i < totalBytes; ++i) {
        // Skip alpha channel if RGBA
        if (channels == 4 && (i % 4) == 3) continue;

        float value = static_cast<float>(data[i]);
        value = (value - 128.0f) * contrast + 128.0f;
        data[i] = clampToByte(value);
    }

    return true;
}

bool ofImageFilter::brightness(ofPixels& pixels, float brightness) {
    if (!pixels.isAllocated()) return false;

    const size_t totalBytes = pixels.getTotalBytes();
    const size_t channels = pixels.getNumChannels();
    unsigned char* data = pixels.getData();

    for (size_t i = 0; i < totalBytes; ++i) {
        // Skip alpha channel if RGBA
        if (channels == 4 && (i % 4) == 3) continue;

        int value = static_cast<int>(data[i]) + static_cast<int>(brightness);
        data[i] = clampToByte(value);
    }

    return true;
}

bool ofImageFilter::brightnessContrast(ofPixels& pixels, float brightness, float contrast) {
    if (!pixels.isAllocated()) return false;

    const size_t totalBytes = pixels.getTotalBytes();
    const size_t channels = pixels.getNumChannels();
    unsigned char* data = pixels.getData();

    for (size_t i = 0; i < totalBytes; ++i) {
        // Skip alpha channel if RGBA
        if (channels == 4 && (i % 4) == 3) continue;

        float value = static_cast<float>(data[i]);
        value = (value - 128.0f) * contrast + 128.0f + brightness;
        data[i] = clampToByte(value);
    }

    return true;
}

bool ofImageFilter::saturation(ofPixels& pixels, float saturation) {
    if (!pixels.isAllocated()) return false;

    const size_t width = pixels.getWidth();
    const size_t height = pixels.getHeight();
    const size_t channels = pixels.getNumChannels();

    if (channels < 3) {
        return true;  // Can't adjust saturation on grayscale
    }

    unsigned char* data = pixels.getData();

    for (size_t i = 0; i < width * height; ++i) {
        size_t idx = i * channels;

        float r = static_cast<float>(data[idx + 0]);
        float g = static_cast<float>(data[idx + 1]);
        float b = static_cast<float>(data[idx + 2]);

        // Calculate luminance
        float gray = 0.299f * r + 0.587f * g + 0.114f * b;

        // Interpolate between gray and original
        r = gray + saturation * (r - gray);
        g = gray + saturation * (g - gray);
        b = gray + saturation * (b - gray);

        data[idx + 0] = clampToByte(r);
        data[idx + 1] = clampToByte(g);
        data[idx + 2] = clampToByte(b);
    }

    return true;
}

bool ofImageFilter::grayscale(ofPixels& pixels) {
    return saturation(pixels, 0.0f);
}

bool ofImageFilter::invert(ofPixels& pixels) {
    if (!pixels.isAllocated()) return false;

    const size_t totalBytes = pixels.getTotalBytes();
    const size_t channels = pixels.getNumChannels();
    unsigned char* data = pixels.getData();

    for (size_t i = 0; i < totalBytes; ++i) {
        // Skip alpha channel if RGBA
        if (channels == 4 && (i % 4) == 3) continue;

        data[i] = 255 - data[i];
    }

    return true;
}

bool ofImageFilter::gamma(ofPixels& pixels, float gamma) {
    if (!pixels.isAllocated() || gamma <= 0) return false;

    // Pre-compute gamma lookup table
    unsigned char lut[256];
    float invGamma = 1.0f / gamma;

    for (int i = 0; i < 256; ++i) {
        float normalized = static_cast<float>(i) / 255.0f;
        float corrected = std::pow(normalized, invGamma);
        lut[i] = clampToByte(corrected * 255.0f);
    }

    const size_t totalBytes = pixels.getTotalBytes();
    const size_t channels = pixels.getNumChannels();
    unsigned char* data = pixels.getData();

    for (size_t i = 0; i < totalBytes; ++i) {
        // Skip alpha channel if RGBA
        if (channels == 4 && (i % 4) == 3) continue;

        data[i] = lut[data[i]];
    }

    return true;
}

// ============================================================================
// Edge Detection
// ============================================================================

bool ofImageFilter::sobel(ofPixels& pixels) {
    if (!pixels.isAllocated()) return false;

    @autoreleasepool {
        const size_t width = pixels.getWidth();
        const size_t height = pixels.getHeight();
        const size_t channels = pixels.getNumChannels();

        // Convert to grayscale if needed
        ofPixels gray;
        if (channels > 1) {
            gray.allocate(width, height, 1);
            for (size_t i = 0; i < width * height; ++i) {
                size_t idx = i * channels;
                float r = static_cast<float>(pixels.getData()[idx + 0]);
                float g = static_cast<float>(pixels.getData()[idx + 1]);
                float b = static_cast<float>(pixels.getData()[idx + 2]);
                gray.getData()[i] = clampToByte(0.299f * r + 0.587f * g + 0.114f * b);
            }
        } else {
            gray.allocate(width, height, 1);
            std::memcpy(gray.getData(), pixels.getData(), width * height);
        }

        // Sobel kernels
        int16_t sobelX[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
        int16_t sobelY[9] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};

        ofPixels resultX, resultY;
        resultX.allocate(width, height, 1);
        resultY.allocate(width, height, 1);

        vImage_Buffer srcBuffer = CreateBuffer(gray);
        vImage_Buffer dstBufferX = CreateBuffer(resultX);
        vImage_Buffer dstBufferY = CreateBuffer(resultY);

        vImage_Error error = vImageConvolve_Planar8(&srcBuffer, &dstBufferX, NULL,
                                                    0, 0, sobelX, 3, 3,
                                                    1, 128, kvImageEdgeExtend);
        if (error != kvImageNoError) return false;

        error = vImageConvolve_Planar8(&srcBuffer, &dstBufferY, NULL,
                                       0, 0, sobelY, 3, 3,
                                       1, 128, kvImageEdgeExtend);
        if (error != kvImageNoError) return false;

        // Combine gradients: sqrt(Gx^2 + Gy^2)
        for (size_t i = 0; i < width * height; ++i) {
            float gx = static_cast<float>(resultX.getData()[i]) - 128.0f;
            float gy = static_cast<float>(resultY.getData()[i]) - 128.0f;
            float magnitude = std::sqrt(gx * gx + gy * gy);

            unsigned char value = clampToByte(magnitude);

            // Write to all channels
            for (size_t c = 0; c < channels; ++c) {
                if (channels == 4 && c == 3) {
                    // Keep alpha
                    continue;
                }
                pixels.getData()[i * channels + c] = value;
            }
        }

        return true;
    }
}

bool ofImageFilter::laplacian(ofPixels& pixels) {
    if (!pixels.isAllocated()) return false;

    // Laplacian kernel
    float kernel[9] = {
        0,  1,  0,
        1, -4,  1,
        0,  1,  0
    };

    // Make a grayscale copy first
    const size_t width = pixels.getWidth();
    const size_t height = pixels.getHeight();
    const size_t channels = pixels.getNumChannels();

    ofPixels result;
    result.allocate(width, height, channels);
    std::memcpy(result.getData(), pixels.getData(), pixels.getTotalBytes());

    if (!convolve(result, kernel, 3, 3, 1.0f)) {
        return false;
    }

    // Copy result back
    std::memcpy(pixels.getData(), result.getData(), pixels.getTotalBytes());
    return true;
}

// ============================================================================
// Morphological Operations
// ============================================================================

bool ofImageFilter::dilate(ofPixels& pixels, int radius) {
    if (!pixels.isAllocated() || radius <= 0) return false;

    @autoreleasepool {
        int kernelSize = radius * 2 + 1;
        const size_t channels = pixels.getNumChannels();

        ofPixels temp;
        temp.allocate(pixels.getWidth(), pixels.getHeight(), channels);
        std::memcpy(temp.getData(), pixels.getData(), pixels.getTotalBytes());

        vImage_Buffer srcBuffer = CreateBufferConst(temp);
        vImage_Buffer dstBuffer = CreateBuffer(pixels);

        vImage_Error error = kvImageNoError;

        if (channels == 1) {
            error = vImageDilate_Planar8(&srcBuffer, &dstBuffer, 0, 0,
                                         NULL, kernelSize, kernelSize,
                                         kvImageEdgeExtend);
        } else if (channels == 4) {
            error = vImageDilate_ARGB8888(&srcBuffer, &dstBuffer, 0, 0,
                                          NULL, kernelSize, kernelSize,
                                          kvImageEdgeExtend);
        }

        return error == kvImageNoError;
    }
}

bool ofImageFilter::erode(ofPixels& pixels, int radius) {
    if (!pixels.isAllocated() || radius <= 0) return false;

    @autoreleasepool {
        int kernelSize = radius * 2 + 1;
        const size_t channels = pixels.getNumChannels();

        ofPixels temp;
        temp.allocate(pixels.getWidth(), pixels.getHeight(), channels);
        std::memcpy(temp.getData(), pixels.getData(), pixels.getTotalBytes());

        vImage_Buffer srcBuffer = CreateBufferConst(temp);
        vImage_Buffer dstBuffer = CreateBuffer(pixels);

        vImage_Error error = kvImageNoError;

        if (channels == 1) {
            error = vImageErode_Planar8(&srcBuffer, &dstBuffer, 0, 0,
                                        NULL, kernelSize, kernelSize,
                                        kvImageEdgeExtend);
        } else if (channels == 4) {
            error = vImageErode_ARGB8888(&srcBuffer, &dstBuffer, 0, 0,
                                         NULL, kernelSize, kernelSize,
                                         kvImageEdgeExtend);
        }

        return error == kvImageNoError;
    }
}

// ============================================================================
// Custom Convolution
// ============================================================================

bool ofImageFilter::convolve(ofPixels& pixels,
                             const float* kernel,
                             int kernelWidth,
                             int kernelHeight,
                             float divisor) {
    if (!pixels.isAllocated() || !kernel) return false;
    if (kernelWidth < 1 || kernelHeight < 1) return false;
    if ((kernelWidth % 2) == 0 || (kernelHeight % 2) == 0) return false;  // Must be odd

    @autoreleasepool {
        const size_t width = pixels.getWidth();
        const size_t height = pixels.getHeight();
        const size_t channels = pixels.getNumChannels();

        // Calculate divisor if not provided
        if (divisor == 0.0f) {
            for (int i = 0; i < kernelWidth * kernelHeight; ++i) {
                divisor += kernel[i];
            }
            if (divisor == 0.0f) divisor = 1.0f;
        }

        // Convert float kernel to int16 for vImage
        std::vector<int16_t> intKernel(kernelWidth * kernelHeight);
        int32_t scale = 256;  // Scale factor for fixed-point

        for (int i = 0; i < kernelWidth * kernelHeight; ++i) {
            intKernel[i] = static_cast<int16_t>(kernel[i] * scale / divisor);
        }

        ofPixels temp;
        temp.allocate(width, height, channels);
        std::memcpy(temp.getData(), pixels.getData(), pixels.getTotalBytes());

        vImage_Buffer srcBuffer = CreateBufferConst(temp);
        vImage_Buffer dstBuffer = CreateBuffer(pixels);

        vImage_Error error = kvImageNoError;

        if (channels == 1) {
            error = vImageConvolve_Planar8(&srcBuffer, &dstBuffer, NULL,
                                           0, 0, intKernel.data(),
                                           kernelHeight, kernelWidth,
                                           scale, 0, kvImageEdgeExtend);
        } else if (channels == 4) {
            error = vImageConvolve_ARGB8888(&srcBuffer, &dstBuffer, NULL,
                                            0, 0, intKernel.data(),
                                            kernelHeight, kernelWidth,
                                            scale, NULL, kvImageEdgeExtend);
        } else if (channels == 3) {
            // Process each channel separately
            std::vector<unsigned char> channelSrc(width * height);
            std::vector<unsigned char> channelDst(width * height);

            for (size_t c = 0; c < 3; ++c) {
                // Extract channel
                for (size_t i = 0; i < width * height; ++i) {
                    channelSrc[i] = temp.getData()[i * 3 + c];
                }

                vImage_Buffer chSrc = {channelSrc.data(),
                                       static_cast<vImagePixelCount>(height),
                                       static_cast<vImagePixelCount>(width),
                                       width};
                vImage_Buffer chDst = {channelDst.data(),
                                       static_cast<vImagePixelCount>(height),
                                       static_cast<vImagePixelCount>(width),
                                       width};

                error = vImageConvolve_Planar8(&chSrc, &chDst, NULL,
                                               0, 0, intKernel.data(),
                                               kernelHeight, kernelWidth,
                                               scale, 0, kvImageEdgeExtend);
                if (error != kvImageNoError) break;

                // Write back channel
                for (size_t i = 0; i < width * height; ++i) {
                    pixels.getData()[i * 3 + c] = channelDst[i];
                }
            }
        }

        return error == kvImageNoError;
    }
}

// ============================================================================
// Thresholding
// ============================================================================

bool ofImageFilter::threshold(ofPixels& pixels, int threshold) {
    if (!pixels.isAllocated()) return false;

    const size_t totalBytes = pixels.getTotalBytes();
    const size_t channels = pixels.getNumChannels();
    unsigned char* data = pixels.getData();

    threshold = std::max(0, std::min(255, threshold));

    for (size_t i = 0; i < totalBytes; ++i) {
        // Skip alpha channel if RGBA
        if (channels == 4 && (i % 4) == 3) continue;

        data[i] = (data[i] >= threshold) ? 255 : 0;
    }

    return true;
}

bool ofImageFilter::adaptiveThreshold(ofPixels& pixels, int blockSize, int constant) {
    if (!pixels.isAllocated()) return false;
    if (blockSize < 3) blockSize = 3;
    if ((blockSize % 2) == 0) blockSize++;  // Must be odd

    @autoreleasepool {
        // Create blurred version for local mean
        ofPixels blurred;
        blurred.allocate(pixels.getWidth(), pixels.getHeight(), pixels.getNumChannels());
        std::memcpy(blurred.getData(), pixels.getData(), pixels.getTotalBytes());

        if (!boxBlur(blurred, blockSize / 2)) {
            return false;
        }

        const size_t totalBytes = pixels.getTotalBytes();
        const size_t channels = pixels.getNumChannels();
        unsigned char* data = pixels.getData();
        unsigned char* blurData = blurred.getData();

        for (size_t i = 0; i < totalBytes; ++i) {
            // Skip alpha channel if RGBA
            if (channels == 4 && (i % 4) == 3) continue;

            int threshold = static_cast<int>(blurData[i]) - constant;
            data[i] = (data[i] >= threshold) ? 255 : 0;
        }

        return true;
    }
}

// ============================================================================
// Noise
// ============================================================================

bool ofImageFilter::addNoise(ofPixels& pixels, float amount) {
    if (!pixels.isAllocated() || amount <= 0) return true;

    amount = std::min(1.0f, amount) * 255.0f;

    const size_t totalBytes = pixels.getTotalBytes();
    const size_t channels = pixels.getNumChannels();
    unsigned char* data = pixels.getData();

    for (size_t i = 0; i < totalBytes; ++i) {
        // Skip alpha channel if RGBA
        if (channels == 4 && (i % 4) == 3) continue;

        // Random noise in range [-amount, amount]
        float noise = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * amount;
        int value = static_cast<int>(data[i]) + static_cast<int>(noise);
        data[i] = clampToByte(value);
    }

    return true;
}

bool ofImageFilter::median(ofPixels& pixels, int radius) {
    if (!pixels.isAllocated() || radius <= 0) return false;

    @autoreleasepool {
        const size_t width = pixels.getWidth();
        const size_t height = pixels.getHeight();
        const size_t channels = pixels.getNumChannels();
        const int kernelSize = radius * 2 + 1;

        ofPixels temp;
        temp.allocate(width, height, channels);
        std::memcpy(temp.getData(), pixels.getData(), pixels.getTotalBytes());

        // Process each channel
        for (size_t c = 0; c < channels; ++c) {
            // Skip alpha
            if (channels == 4 && c == 3) continue;

            for (size_t y = 0; y < height; ++y) {
                for (size_t x = 0; x < width; ++x) {
                    std::vector<unsigned char> neighborhood;
                    neighborhood.reserve(kernelSize * kernelSize);

                    // Collect neighborhood values
                    for (int ky = -radius; ky <= radius; ++ky) {
                        for (int kx = -radius; kx <= radius; ++kx) {
                            int nx = static_cast<int>(x) + kx;
                            int ny = static_cast<int>(y) + ky;

                            // Clamp to edge
                            nx = std::max(0, std::min(static_cast<int>(width) - 1, nx));
                            ny = std::max(0, std::min(static_cast<int>(height) - 1, ny));

                            neighborhood.push_back(temp.getData()[ny * width * channels + nx * channels + c]);
                        }
                    }

                    // Find median
                    std::sort(neighborhood.begin(), neighborhood.end());
                    pixels.getData()[y * width * channels + x * channels + c] =
                        neighborhood[neighborhood.size() / 2];
                }
            }
        }

        return true;
    }
}

} // namespace oflike
