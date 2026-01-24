#pragma once

// ofxOpenCv - Vision.framework-based computer vision addon for oflike-metal
//
// This addon provides computer vision capabilities using Apple's Vision.framework
// instead of OpenCV, allowing for:
// - Native performance on Apple Silicon
// - Neural Engine acceleration
// - Zero-copy integration with Metal
//
// Features:
// - Face detection (VNDetectFaceRectanglesRequest)
// - Human detection (VNDetectHumanRectanglesRequest)
// - Text recognition (VNRecognizeTextRequest)
// - Barcode detection (VNDetectBarcodesRequest)
//
// All bounding boxes are returned in normalized coordinates [0,1]
// with origin at top-left (oflike coordinate system)

#include "ofxCvVisionDetector.h"

namespace ofxCv {
    // Re-export main types
    using VisionDetector = VisionDetector;
    using FaceDetection = FaceDetection;
    using HumanDetection = HumanDetection;
    using TextDetection = TextDetection;
    using BarcodeDetection = BarcodeDetection;
    using DetectionType = DetectionType;
    using TextRecognitionLevel = TextRecognitionLevel;
}
