#pragma once

#include <memory>
#include "../../oflike/image/ofPixels.h"

// Forward declarations to avoid including Objective-C headers
typedef struct __CVBuffer* CVPixelBufferRef;
typedef struct __CVBuffer* CVImageBufferRef;

// Optional OpenCV support
#ifdef OFXCV_USE_OPENCV
namespace cv {
    class Mat;
}
#endif

namespace ofxCv {

/// Image conversion utilities for ofPixels, CVPixelBuffer, and cv::Mat
/// Uses pImpl pattern to hide Objective-C++ details
class ImageConverter {
public:
    ImageConverter();
    ~ImageConverter();

    // ofPixels ↔ CVPixelBuffer conversions

    /// Convert ofPixels to CVPixelBuffer
    /// @param pixels Source pixel data
    /// @param pixelBuffer Output CVPixelBuffer (caller must release)
    /// @return true if conversion succeeded
    bool pixelsToCVPixelBuffer(const oflike::ofPixels& pixels, CVPixelBufferRef& pixelBuffer);

    /// Convert CVPixelBuffer to ofPixels
    /// @param pixelBuffer Source CVPixelBuffer
    /// @param pixels Output pixel data
    /// @return true if conversion succeeded
    bool cvPixelBufferToPixels(CVPixelBufferRef pixelBuffer, oflike::ofPixels& pixels);

#ifdef OFXCV_USE_OPENCV
    // ofPixels ↔ cv::Mat conversions (optional)

    /// Convert ofPixels to cv::Mat
    /// @param pixels Source pixel data
    /// @param mat Output cv::Mat
    /// @return true if conversion succeeded
    bool pixelsToMat(const oflike::ofPixels& pixels, cv::Mat& mat);

    /// Convert cv::Mat to ofPixels
    /// @param mat Source cv::Mat
    /// @param pixels Output pixel data
    /// @return true if conversion succeeded
    bool matToPixels(const cv::Mat& mat, oflike::ofPixels& pixels);
#endif

    // Error handling
    bool hasError() const;
    std::string getLastError() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

// Convenience functions for simple conversions

/// Create CVPixelBuffer from ofPixels (caller must release with CVPixelBufferRelease)
CVPixelBufferRef toCv(const oflike::ofPixels& pixels);

/// Create ofPixels from CVPixelBuffer
oflike::ofPixels toOf(CVPixelBufferRef pixelBuffer);

#ifdef OFXCV_USE_OPENCV
/// Create cv::Mat from ofPixels (shares memory, does not copy)
cv::Mat toCv(const oflike::ofPixels& pixels);

/// Create ofPixels from cv::Mat (copies data)
oflike::ofPixels toOf(const cv::Mat& mat);
#endif

} // namespace ofxCv
