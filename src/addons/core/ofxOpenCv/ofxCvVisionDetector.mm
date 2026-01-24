#import "ofxCvVisionDetector.h"
#import <Vision/Vision.h>
#import <CoreGraphics/CoreGraphics.h>
#import <ImageIO/ImageIO.h>

namespace ofxCv {

// Helper: Convert ofPixels to CVPixelBuffer
static CVPixelBufferRef createPixelBuffer(const oflike::ofPixels& pixels) {
    @autoreleasepool {
        size_t width = pixels.getWidth();
        size_t height = pixels.getHeight();
        size_t channels = pixels.getNumChannels();

        OSType pixelFormat;
        switch (channels) {
            case 1: pixelFormat = kCVPixelFormatType_OneComponent8; break;
            case 3: pixelFormat = kCVPixelFormatType_24RGB; break;
            case 4: pixelFormat = kCVPixelFormatType_32RGBA; break;
            default: return nullptr;
        }

        CVPixelBufferRef pixelBuffer = nullptr;
        NSDictionary* options = @{
            (id)kCVPixelBufferCGImageCompatibilityKey: @YES,
            (id)kCVPixelBufferCGBitmapContextCompatibilityKey: @YES,
            (id)kCVPixelBufferMetalCompatibilityKey: @YES
        };

        CVReturn status = CVPixelBufferCreate(
            kCFAllocatorDefault,
            width, height,
            pixelFormat,
            (__bridge CFDictionaryRef)options,
            &pixelBuffer
        );

        if (status != kCVReturnSuccess || !pixelBuffer) {
            return nullptr;
        }

        CVPixelBufferLockBaseAddress(pixelBuffer, 0);

        void* baseAddress = CVPixelBufferGetBaseAddress(pixelBuffer);
        size_t bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer);
        size_t srcBytesPerRow = width * channels;

        const unsigned char* src = pixels.getData();
        unsigned char* dst = (unsigned char*)baseAddress;

        for (size_t y = 0; y < height; y++) {
            memcpy(dst + y * bytesPerRow, src + y * srcBytesPerRow, srcBytesPerRow);
        }

        CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);

        return pixelBuffer;
    }
}

// Helper: Convert normalized Vision rect to ofRectangle
static oflike::ofRectangle visionRectToOf(CGRect visionRect) {
    // Vision.framework uses normalized coordinates [0,1]
    // with origin at bottom-left
    return oflike::ofRectangle(
        visionRect.origin.x,
        1.0f - (visionRect.origin.y + visionRect.size.height), // Flip Y
        visionRect.size.width,
        visionRect.size.height
    );
}

class VisionDetector::Impl {
public:
    float minimumConfidence = 0.5f;
    std::string lastError;
    NSArray<NSString*>* textLanguages = nil;

    Impl() {
        @autoreleasepool {
            // Default to English
            textLanguages = @[@"en-US"];
        }
    }

    ~Impl() {
        @autoreleasepool {
            textLanguages = nil;
        }
    }

    bool detectFaces(const oflike::ofPixels& pixels, std::vector<FaceDetection>& results) {
        @autoreleasepool {
            results.clear();

            CVPixelBufferRef pixelBuffer = createPixelBuffer(pixels);
            if (!pixelBuffer) {
                lastError = "Failed to create pixel buffer";
                return false;
            }

            VNDetectFaceRectanglesRequest* request = [[VNDetectFaceRectanglesRequest alloc] init];

            VNImageRequestHandler* handler = [[VNImageRequestHandler alloc]
                initWithCVPixelBuffer:pixelBuffer
                options:@{}];

            NSError* error = nil;
            [handler performRequests:@[request] error:&error];

            CVPixelBufferRelease(pixelBuffer);

            if (error) {
                lastError = std::string([[error localizedDescription] UTF8String]);
                return false;
            }

            for (VNFaceObservation* observation in request.results) {
                if (observation.confidence >= minimumConfidence) {
                    FaceDetection detection;
                    detection.boundingBox = visionRectToOf(observation.boundingBox);
                    detection.confidence = observation.confidence;
                    results.push_back(detection);
                }
            }

            return true;
        }
    }

    bool detectHumans(const oflike::ofPixels& pixels, std::vector<HumanDetection>& results) {
        @autoreleasepool {
            results.clear();

            CVPixelBufferRef pixelBuffer = createPixelBuffer(pixels);
            if (!pixelBuffer) {
                lastError = "Failed to create pixel buffer";
                return false;
            }

            VNDetectHumanRectanglesRequest* request = [[VNDetectHumanRectanglesRequest alloc] init];

            VNImageRequestHandler* handler = [[VNImageRequestHandler alloc]
                initWithCVPixelBuffer:pixelBuffer
                options:@{}];

            NSError* error = nil;
            [handler performRequests:@[request] error:&error];

            CVPixelBufferRelease(pixelBuffer);

            if (error) {
                lastError = std::string([[error localizedDescription] UTF8String]);
                return false;
            }

            for (VNHumanObservation* observation in request.results) {
                if (observation.confidence >= minimumConfidence) {
                    HumanDetection detection;
                    detection.boundingBox = visionRectToOf(observation.boundingBox);
                    detection.confidence = observation.confidence;
                    results.push_back(detection);
                }
            }

            return true;
        }
    }

    bool recognizeText(const oflike::ofPixels& pixels, std::vector<TextDetection>& results,
                       TextRecognitionLevel level) {
        @autoreleasepool {
            results.clear();

            CVPixelBufferRef pixelBuffer = createPixelBuffer(pixels);
            if (!pixelBuffer) {
                lastError = "Failed to create pixel buffer";
                return false;
            }

            VNRecognizeTextRequest* request = [[VNRecognizeTextRequest alloc] init];

            // Set recognition level
            if (level == TextRecognitionLevel::Accurate) {
                request.recognitionLevel = VNRequestTextRecognitionLevelAccurate;
            } else {
                request.recognitionLevel = VNRequestTextRecognitionLevelFast;
            }

            // Set languages
            if (textLanguages) {
                request.recognitionLanguages = textLanguages;
            }

            VNImageRequestHandler* handler = [[VNImageRequestHandler alloc]
                initWithCVPixelBuffer:pixelBuffer
                options:@{}];

            NSError* error = nil;
            [handler performRequests:@[request] error:&error];

            CVPixelBufferRelease(pixelBuffer);

            if (error) {
                lastError = std::string([[error localizedDescription] UTF8String]);
                return false;
            }

            for (VNRecognizedTextObservation* observation in request.results) {
                if (observation.confidence >= minimumConfidence) {
                    VNRecognizedText* recognizedText = [observation topCandidates:1].firstObject;
                    if (recognizedText) {
                        TextDetection detection;
                        detection.boundingBox = visionRectToOf(observation.boundingBox);
                        detection.text = std::string([recognizedText.string UTF8String]);
                        detection.confidence = observation.confidence;
                        results.push_back(detection);
                    }
                }
            }

            return true;
        }
    }

    bool detectBarcodes(const oflike::ofPixels& pixels, std::vector<BarcodeDetection>& results) {
        @autoreleasepool {
            results.clear();

            CVPixelBufferRef pixelBuffer = createPixelBuffer(pixels);
            if (!pixelBuffer) {
                lastError = "Failed to create pixel buffer";
                return false;
            }

            VNDetectBarcodesRequest* request = [[VNDetectBarcodesRequest alloc] init];

            VNImageRequestHandler* handler = [[VNImageRequestHandler alloc]
                initWithCVPixelBuffer:pixelBuffer
                options:@{}];

            NSError* error = nil;
            [handler performRequests:@[request] error:&error];

            CVPixelBufferRelease(pixelBuffer);

            if (error) {
                lastError = std::string([[error localizedDescription] UTF8String]);
                return false;
            }

            for (VNBarcodeObservation* observation in request.results) {
                if (observation.confidence >= minimumConfidence) {
                    BarcodeDetection detection;
                    detection.boundingBox = visionRectToOf(observation.boundingBox);

                    if (observation.payloadStringValue) {
                        detection.payload = std::string([observation.payloadStringValue UTF8String]);
                    }

                    // Convert symbology to string
                    NSString* symbology = observation.symbology;
                    if ([symbology isEqualToString:VNBarcodeSymbologyQR]) {
                        detection.type = "QR";
                    } else if ([symbology isEqualToString:VNBarcodeSymbologyEAN13]) {
                        detection.type = "EAN13";
                    } else if ([symbology isEqualToString:VNBarcodeSymbologyEAN8]) {
                        detection.type = "EAN8";
                    } else if ([symbology isEqualToString:VNBarcodeSymbologyCode128]) {
                        detection.type = "Code128";
                    } else if ([symbology isEqualToString:VNBarcodeSymbologyCode39]) {
                        detection.type = "Code39";
                    } else if ([symbology isEqualToString:VNBarcodeSymbologyCode93]) {
                        detection.type = "Code93";
                    } else if ([symbology isEqualToString:VNBarcodeSymbologyDataMatrix]) {
                        detection.type = "DataMatrix";
                    } else if ([symbology isEqualToString:VNBarcodeSymbologyPDF417]) {
                        detection.type = "PDF417";
                    } else if ([symbology isEqualToString:VNBarcodeSymbologyAztec]) {
                        detection.type = "Aztec";
                    } else {
                        detection.type = std::string([symbology UTF8String]);
                    }

                    detection.confidence = observation.confidence;
                    results.push_back(detection);
                }
            }

            return true;
        }
    }

    bool setTextLanguages(const std::vector<std::string>& languages) {
        @autoreleasepool {
            NSMutableArray<NSString*>* nsLanguages = [NSMutableArray array];
            for (const auto& lang : languages) {
                [nsLanguages addObject:[NSString stringWithUTF8String:lang.c_str()]];
            }
            textLanguages = [nsLanguages copy];
            return true;
        }
    }
};

// VisionDetector implementation

VisionDetector::VisionDetector()
    : pImpl(std::make_unique<Impl>()) {
}

VisionDetector::~VisionDetector() = default;

bool VisionDetector::detectFaces(const oflike::ofPixels& pixels, std::vector<FaceDetection>& results) {
    return pImpl->detectFaces(pixels, results);
}

bool VisionDetector::detectHumans(const oflike::ofPixels& pixels, std::vector<HumanDetection>& results) {
    return pImpl->detectHumans(pixels, results);
}

bool VisionDetector::recognizeText(const oflike::ofPixels& pixels, std::vector<TextDetection>& results,
                                   TextRecognitionLevel level) {
    return pImpl->recognizeText(pixels, results, level);
}

bool VisionDetector::setTextLanguages(const std::vector<std::string>& languages) {
    return pImpl->setTextLanguages(languages);
}

bool VisionDetector::detectBarcodes(const oflike::ofPixels& pixels, std::vector<BarcodeDetection>& results) {
    return pImpl->detectBarcodes(pixels, results);
}

void VisionDetector::setMinimumConfidence(float confidence) {
    pImpl->minimumConfidence = confidence;
}

float VisionDetector::getMinimumConfidence() const {
    return pImpl->minimumConfidence;
}

bool VisionDetector::hasError() const {
    return !pImpl->lastError.empty();
}

std::string VisionDetector::getLastError() const {
    return pImpl->lastError;
}

} // namespace ofxCv
