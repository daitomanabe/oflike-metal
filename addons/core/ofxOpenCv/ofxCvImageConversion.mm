#import "ofxCvImageConversion.h"
#import <CoreVideo/CoreVideo.h>
#import <Foundation/Foundation.h>
#import <string>

#ifdef OFXCV_USE_OPENCV
#include <opencv2/core.hpp>
#endif

namespace ofxCv {

// ============================================================================
// ImageConverter::Impl
// ============================================================================

class ImageConverter::Impl {
public:
    std::string lastError;

    bool pixelsToCVPixelBuffer(const oflike::ofPixels& pixels, CVPixelBufferRef& pixelBuffer) {
        @autoreleasepool {
            if (pixels.getData() == nullptr || pixels.getWidth() == 0 || pixels.getHeight() == 0) {
                lastError = "Invalid ofPixels: empty or null data";
                return false;
            }

            // Determine pixel format
            OSType pixelFormat;
            int bytesPerPixel;

            switch (pixels.getNumChannels()) {
                case 1:
                    pixelFormat = kCVPixelFormatType_OneComponent8;
                    bytesPerPixel = 1;
                    break;
                case 3:
                    pixelFormat = kCVPixelFormatType_24RGB;
                    bytesPerPixel = 3;
                    break;
                case 4:
                    pixelFormat = kCVPixelFormatType_32RGBA;
                    bytesPerPixel = 4;
                    break;
                default:
                    lastError = "Unsupported channel count: " + std::to_string(pixels.getNumChannels());
                    return false;
            }

            NSDictionary* options = @{
                (id)kCVPixelBufferCGImageCompatibilityKey: @YES,
                (id)kCVPixelBufferCGBitmapContextCompatibilityKey: @YES,
                (id)kCVPixelBufferMetalCompatibilityKey: @YES
            };

            CVReturn status = CVPixelBufferCreate(
                kCFAllocatorDefault,
                pixels.getWidth(),
                pixels.getHeight(),
                pixelFormat,
                (__bridge CFDictionaryRef)options,
                &pixelBuffer
            );

            if (status != kCVReturnSuccess) {
                lastError = "CVPixelBufferCreate failed with status: " + std::to_string(status);
                return false;
            }

            // Lock pixel buffer for writing
            CVPixelBufferLockBaseAddress(pixelBuffer, 0);

            void* baseAddress = CVPixelBufferGetBaseAddress(pixelBuffer);
            size_t bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer);

            // Copy pixel data row by row
            const unsigned char* src = pixels.getData();
            unsigned char* dst = static_cast<unsigned char*>(baseAddress);

            for (size_t y = 0; y < pixels.getHeight(); ++y) {
                memcpy(dst + y * bytesPerRow, src + y * pixels.getWidth() * bytesPerPixel,
                       pixels.getWidth() * bytesPerPixel);
            }

            CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);

            return true;
        }
    }

    bool cvPixelBufferToPixels(CVPixelBufferRef pixelBuffer, oflike::ofPixels& pixels) {
        @autoreleasepool {
            if (pixelBuffer == nullptr) {
                lastError = "CVPixelBuffer is null";
                return false;
            }

            CVPixelBufferLockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);

            size_t width = CVPixelBufferGetWidth(pixelBuffer);
            size_t height = CVPixelBufferGetHeight(pixelBuffer);
            OSType pixelFormat = CVPixelBufferGetPixelFormatType(pixelBuffer);

            // Determine number of channels
            int numChannels;
            switch (pixelFormat) {
                case kCVPixelFormatType_OneComponent8:
                    numChannels = 1;
                    break;
                case kCVPixelFormatType_24RGB:
                    numChannels = 3;
                    break;
                case kCVPixelFormatType_32RGBA:
                case kCVPixelFormatType_32BGRA:
                    numChannels = 4;
                    break;
                default:
                    CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
                    lastError = "Unsupported CVPixelBuffer format: " + std::to_string(pixelFormat);
                    return false;
            }

            // Allocate ofPixels
            pixels.allocate(width, height, numChannels);

            void* baseAddress = CVPixelBufferGetBaseAddress(pixelBuffer);
            size_t bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer);

            unsigned char* src = static_cast<unsigned char*>(baseAddress);
            unsigned char* dst = pixels.getData();

            // Handle BGRA → RGBA conversion if needed
            bool needsSwizzle = (pixelFormat == kCVPixelFormatType_32BGRA);

            // Copy pixel data row by row
            for (size_t y = 0; y < height; ++y) {
                if (needsSwizzle) {
                    // Convert BGRA to RGBA
                    for (size_t x = 0; x < width; ++x) {
                        size_t srcIdx = y * bytesPerRow + x * 4;
                        size_t dstIdx = y * width * 4 + x * 4;
                        dst[dstIdx + 0] = src[srcIdx + 2]; // R ← B
                        dst[dstIdx + 1] = src[srcIdx + 1]; // G ← G
                        dst[dstIdx + 2] = src[srcIdx + 0]; // B ← R
                        dst[dstIdx + 3] = src[srcIdx + 3]; // A ← A
                    }
                } else {
                    memcpy(dst + y * width * numChannels, src + y * bytesPerRow,
                           width * numChannels);
                }
            }

            CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);

            return true;
        }
    }

#ifdef OFXCV_USE_OPENCV
    bool pixelsToMat(const oflike::ofPixels& pixels, cv::Mat& mat) {
        if (pixels.getData() == nullptr || pixels.getWidth() == 0 || pixels.getHeight() == 0) {
            lastError = "Invalid ofPixels: empty or null data";
            return false;
        }

        // Determine OpenCV type
        int cvType;
        switch (pixels.getNumChannels()) {
            case 1:
                cvType = CV_8UC1;
                break;
            case 3:
                cvType = CV_8UC3;
                break;
            case 4:
                cvType = CV_8UC4;
                break;
            default:
                lastError = "Unsupported channel count: " + std::to_string(pixels.getNumChannels());
                return false;
        }

        // Create Mat that wraps ofPixels data (no copy)
        mat = cv::Mat(pixels.getHeight(), pixels.getWidth(), cvType,
                      const_cast<unsigned char*>(pixels.getData()),
                      pixels.getWidth() * pixels.getNumChannels());

        // Note: RGB → BGR conversion if needed
        if (pixels.getNumChannels() == 3) {
            cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);
        } else if (pixels.getNumChannels() == 4) {
            cv::cvtColor(mat, mat, cv::COLOR_RGBA2BGRA);
        }

        return true;
    }

    bool matToPixels(const cv::Mat& mat, oflike::ofPixels& pixels) {
        if (mat.empty()) {
            lastError = "cv::Mat is empty";
            return false;
        }

        // Determine number of channels
        int numChannels = mat.channels();
        if (numChannels < 1 || numChannels > 4) {
            lastError = "Unsupported cv::Mat channel count: " + std::to_string(numChannels);
            return false;
        }

        // Allocate ofPixels
        pixels.allocate(mat.cols, mat.rows, numChannels);

        // Handle BGR → RGB conversion if needed
        if (numChannels == 3) {
            cv::Mat temp;
            cv::cvtColor(mat, temp, cv::COLOR_BGR2RGB);
            memcpy(pixels.getData(), temp.data, temp.total() * temp.elemSize());
        } else if (numChannels == 4) {
            cv::Mat temp;
            cv::cvtColor(mat, temp, cv::COLOR_BGRA2RGBA);
            memcpy(pixels.getData(), temp.data, temp.total() * temp.elemSize());
        } else {
            // Direct copy for grayscale or single channel
            memcpy(pixels.getData(), mat.data, mat.total() * mat.elemSize());
        }

        return true;
    }
#endif
};

// ============================================================================
// ImageConverter (pImpl wrapper)
// ============================================================================

ImageConverter::ImageConverter() : pImpl(std::make_unique<Impl>()) {}
ImageConverter::~ImageConverter() = default;

bool ImageConverter::pixelsToCVPixelBuffer(const oflike::ofPixels& pixels, CVPixelBufferRef& pixelBuffer) {
    return pImpl->pixelsToCVPixelBuffer(pixels, pixelBuffer);
}

bool ImageConverter::cvPixelBufferToPixels(CVPixelBufferRef pixelBuffer, oflike::ofPixels& pixels) {
    return pImpl->cvPixelBufferToPixels(pixelBuffer, pixels);
}

#ifdef OFXCV_USE_OPENCV
bool ImageConverter::pixelsToMat(const oflike::ofPixels& pixels, cv::Mat& mat) {
    return pImpl->pixelsToMat(pixels, mat);
}

bool ImageConverter::matToPixels(const cv::Mat& mat, oflike::ofPixels& pixels) {
    return pImpl->matToPixels(mat, pixels);
}
#endif

bool ImageConverter::hasError() const {
    return !pImpl->lastError.empty();
}

std::string ImageConverter::getLastError() const {
    return pImpl->lastError;
}

// ============================================================================
// Convenience functions
// ============================================================================

CVPixelBufferRef toCv(const oflike::ofPixels& pixels) {
    CVPixelBufferRef pixelBuffer = nullptr;
    ImageConverter converter;
    if (converter.pixelsToCVPixelBuffer(pixels, pixelBuffer)) {
        return pixelBuffer;
    }
    return nullptr;
}

oflike::ofPixels toOf(CVPixelBufferRef pixelBuffer) {
    oflike::ofPixels pixels;
    ImageConverter converter;
    converter.cvPixelBufferToPixels(pixelBuffer, pixels);
    return pixels;
}

#ifdef OFXCV_USE_OPENCV
cv::Mat toCv(const oflike::ofPixels& pixels) {
    cv::Mat mat;
    ImageConverter converter;
    converter.pixelsToMat(pixels, mat);
    return mat;
}

oflike::ofPixels toOf(const cv::Mat& mat) {
    oflike::ofPixels pixels;
    ImageConverter converter;
    converter.matToPixels(mat, pixels);
    return pixels;
}
#endif

} // namespace ofxCv
