/// \file main.cpp
/// \brief Example 21: Face Detection with Vision.framework
///
/// Demonstrates computer vision using ofxOpenCv (Vision.framework):
/// - Creating test pixels programmatically
/// - Detecting faces using VisionDetector
/// - Drawing detection results with bounding boxes
/// - Displaying confidence scores
/// - Understanding normalized coordinates
///
/// This example uses Apple's Vision.framework for face detection,
/// providing native performance and Neural Engine acceleration.
///
/// Note: This is a demonstration example. In a real application,
/// you would load images from files or camera input.

#include "../../src/core/AppBase.h"
#include "../../src/oflike/graphics/ofGraphics.h"
#include "../../src/oflike/image/ofPixels.h"
#include "../../src/oflike/types/ofColor.h"
#include "../../src/oflike/utils/ofUtils.h"
#include "../../src/oflike/utils/ofLog.h"
#include "../../src/addons/core/ofxOpenCv/ofxOpenCv.h"
#include "../../src/core/Context.h"
#include <string>
#include <vector>

using namespace oflike;

class FaceDetectionApp : public ofBaseApp {
public:
    ofPixels testPixels;
    ofxCv::VisionDetector detector;
    std::vector<ofxCv::FaceDetection> faces;
    bool pixelsReady = false;
    bool detectionsReady = false;
    float detectionTime = 0.0f;
    int imageWidth = 640;
    int imageHeight = 480;

    void setup() override {
        ofSetFrameRate(60);
        ofSetBackgroundColor(40, 40, 45);

        // Configure detector
        detector.setMinimumConfidence(0.5f);  // 50% confidence threshold

        ofLog() << "=== Face Detection Example ===";
        ofLog() << "This example demonstrates the ofxCv::VisionDetector API.";
        ofLog() << "";
        ofLog() << "To test with real images:";
        ofLog() << "1. Load an image into ofPixels (from file or camera)";
        ofLog() << "2. Call detector.detectFaces(pixels, faces)";
        ofLog() << "3. Process the detected face bounding boxes";
        ofLog() << "";
        ofLog() << "Press SPACE to run test detection";
        ofLog() << "Note: Test with empty pixels (no faces will be found)";
    }

    void createTestPixels() {
        // Create RGB test image
        testPixels.allocate(imageWidth, imageHeight, 3);

        // Fill with gradient
        for (int y = 0; y < imageHeight; ++y) {
            for (int x = 0; x < imageWidth; ++x) {
                int index = (y * imageWidth + x) * 3;
                testPixels.getData()[index + 0] = (x * 255) / imageWidth;  // R
                testPixels.getData()[index + 1] = (y * 255) / imageHeight; // G
                testPixels.getData()[index + 2] = 128;                      // B
            }
        }

        pixelsReady = true;
        ofLog() << "Created test pixels: " << imageWidth << "x" << imageHeight << " RGB";
    }

    void runDetection() {
        if (!pixelsReady) {
            createTestPixels();
        }

        ofLog() << "Running face detection...";
        float startTime = ofGetElapsedTimef();

        // Detect faces
        faces.clear();
        bool success = detector.detectFaces(testPixels, faces);

        detectionTime = ofGetElapsedTimef() - startTime;

        if (success) {
            ofLog() << "Detection complete: Found " << faces.size() << " face(s) in "
                    << (detectionTime * 1000.0f) << " ms";

            if (faces.size() == 0) {
                ofLog() << "No faces found (expected for test gradient image)";
                ofLog() << "";
                ofLog() << "To test with real faces:";
                ofLog() << "1. Load an image with faces into ofPixels";
                ofLog() << "2. Call detector.detectFaces(pixels, faces)";
                ofLog() << "3. Bounding boxes are in normalized [0,1] coordinates";
            }

            for (size_t i = 0; i < faces.size(); ++i) {
                const auto& face = faces[i];
                ofLog() << "  Face " << (i+1) << ": "
                        << "x=" << face.boundingBox.x << " "
                        << "y=" << face.boundingBox.y << " "
                        << "w=" << face.boundingBox.width << " "
                        << "h=" << face.boundingBox.height << " "
                        << "confidence=" << face.confidence;
            }

            detectionsReady = true;
        } else {
            ofLogError() << "Face detection failed: " << detector.getLastError();
            detectionsReady = false;
        }
    }

    void update() override {
        // Update logic here if needed
    }

    void draw() override {
        // Draw test image visualization
        if (pixelsReady) {
            ofSetColor(255);

            // Draw pixel data as rectangles (simplified visualization)
            float rectWidth = ofGetWidth() / 64.0f;
            float rectHeight = ofGetHeight() / 48.0f;

            for (int y = 0; y < 48; ++y) {
                for (int x = 0; x < 64; ++x) {
                    int px = (x * imageWidth) / 64;
                    int py = (y * imageHeight) / 48;
                    int index = (py * imageWidth + px) * 3;

                    ofSetColor(
                        testPixels.getData()[index + 0],
                        testPixels.getData()[index + 1],
                        testPixels.getData()[index + 2]
                    );

                    ofDrawRectangle(x * rectWidth, y * rectHeight, rectWidth, rectHeight);
                }
            }

            // Draw detection results (if any)
            if (detectionsReady && faces.size() > 0) {
                ofNoFill();
                ofSetLineWidth(3.0f);
                ofSetColor(0, 255, 0);

                for (const auto& face : faces) {
                    // Convert normalized [0,1] to screen coordinates
                    float x = face.boundingBox.x * ofGetWidth();
                    float y = face.boundingBox.y * ofGetHeight();
                    float w = face.boundingBox.width * ofGetWidth();
                    float h = face.boundingBox.height * ofGetHeight();

                    ofDrawRectangle(x, y, w, h);
                }

                ofFill();
            }
        }

        // Draw info text overlay
        ofSetColor(255, 255, 255, 200);
        ofDrawRectangle(10, 10, 600, 180);

        ofSetColor(0);
        std::string info = "Face Detection Example (Vision.framework)\n\n";
        info += "Test Image: " + ofToString(imageWidth) + "x" + ofToString(imageHeight) + " RGB\n";
        info += "Faces found: " + ofToString(faces.size()) + "\n";

        if (detectionsReady) {
            info += "Detection time: " + ofToString(detectionTime * 1000.0f, 1) + " ms\n";
        }

        info += "\nPress SPACE to run detection\n";
        info += "\nNote: This example uses test gradient pixels.\n";
        info += "Load real images with faces for actual detection.";

        ofDrawBitmapString(info, 20, 30);
    }

    void keyPressed(int key) override {
        if (key == ' ') {
            runDetection();
        }
    }
};

//
// How to run this example:
//
// 1. Replace TestApp in src/platform/bridge/SwiftBridge.mm:
//    #include "../../examples/21_face_detection/main.cpp"
//    testApp_ = std::make_unique<FaceDetectionApp>();
//
// 2. Build and run
//
// Controls:
// - Space: Run face detection on test pixels
//
// Note: This example uses test gradient pixels for demonstration.
// To test with real images:
// 1. Load an image into ofPixels (using image loading functionality)
// 2. Call detector.detectFaces(pixels, faces)
// 3. Process the faces vector with normalized [0,1] bounding boxes
