#pragma once

#include <memory>
#include <vector>
#include <string>
#include "../../oflike/image/ofPixels.h"
#include "../../oflike/graphics/ofPolyline.h"  // Contains ofRectangle

namespace ofxCv {

/// Detection type for Vision.framework requests
enum class DetectionType {
    Face,
    Human,
    Text,
    Barcode
};

/// Recognition level for text detection
enum class TextRecognitionLevel {
    Accurate,  // More accurate, slower
    Fast       // Faster, less accurate
};

/// Result from face detection
struct FaceDetection {
    oflike::ofRectangle boundingBox;  // Normalized coordinates [0,1]
    float confidence;                  // 0.0 - 1.0
};

/// Result from human detection
struct HumanDetection {
    oflike::ofRectangle boundingBox;  // Normalized coordinates [0,1]
    float confidence;                  // 0.0 - 1.0
};

/// Result from text recognition
struct TextDetection {
    oflike::ofRectangle boundingBox;  // Normalized coordinates [0,1]
    std::string text;
    float confidence;                  // 0.0 - 1.0
};

/// Result from barcode detection
struct BarcodeDetection {
    oflike::ofRectangle boundingBox;  // Normalized coordinates [0,1]
    std::string payload;               // Barcode data
    std::string type;                  // QR, EAN13, etc.
    float confidence;                  // 0.0 - 1.0
};

/// Vision.framework detector wrapper
/// Uses pImpl pattern to hide Objective-C++ from C++ headers
class VisionDetector {
public:
    VisionDetector();
    ~VisionDetector();

    // Face Detection
    bool detectFaces(const oflike::ofPixels& pixels, std::vector<FaceDetection>& results);

    // Human Detection
    bool detectHumans(const oflike::ofPixels& pixels, std::vector<HumanDetection>& results);

    // Text Recognition
    bool recognizeText(const oflike::ofPixels& pixels, std::vector<TextDetection>& results,
                       TextRecognitionLevel level = TextRecognitionLevel::Accurate);
    bool setTextLanguages(const std::vector<std::string>& languages);

    // Barcode Detection
    bool detectBarcodes(const oflike::ofPixels& pixels, std::vector<BarcodeDetection>& results);

    // Configuration
    void setMinimumConfidence(float confidence); // Default: 0.5
    float getMinimumConfidence() const;

    // Error handling
    bool hasError() const;
    std::string getLastError() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace ofxCv
