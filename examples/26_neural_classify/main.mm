/// \file main.mm
/// \brief Example 26: Image Classification with Core ML and Neural Engine
///
/// Demonstrates machine learning inference using ofxNeuralEngine:
/// - Loading Core ML image classification models (.mlmodelc)
/// - Classifying images with Neural Engine acceleration
/// - Displaying top-K classification results with confidence scores
/// - Understanding Core ML model requirements and preprocessing
///
/// This example uses Apple's Core ML framework with Neural Engine,
/// providing hardware-accelerated ML inference on Apple Silicon.
///
/// Model Requirements:
/// - Input: Image (typically 224x224 or 299x299 RGB)
/// - Output: Probabilities for each class label
///
/// Example Models:
/// - MobileNetV3 (fast, good for real-time)
/// - ResNet50 (accurate, larger model)
/// - EfficientNet (balanced accuracy/speed)
///
/// Note: This example creates synthetic test data. In a real application,
/// you would load images from files, camera, or other sources.

#include "../../src/core/AppBase.h"
#include "../../src/oflike/graphics/ofGraphics.h"
#include "../../src/oflike/image/ofPixels.h"
#include "../../src/oflike/image/ofImage.h"
#include "../../src/oflike/types/ofColor.h"
#include "../../src/oflike/utils/ofUtils.h"
#include "../../src/oflike/utils/ofLog.h"
#include "../../src/addons/apple_native/ofxNeuralEngine/ofxNeuralEngine.h"
#include "../../src/core/Context.h"
#include <string>
#include <vector>

using namespace oflike;
using namespace NeuralEngine;

class NeuralClassifyApp : public ofBaseApp {
public:
    ImageClassifier classifier;
    ofPixels testPixels;
    ofImage testImage;
    std::vector<ClassificationResult> results;

    bool modelLoaded = false;
    bool imageReady = false;
    bool resultsReady = false;
    float classificationTime = 0.0f;

    // Test image parameters
    int imageWidth = 224;
    int imageHeight = 224;

    // UI state
    std::string statusMessage = "Press SPACE to load model and classify";
    int topK = 5;  // Number of top results to display

    void setup() override {
        ofSetFrameRate(60);
        ofSetBackgroundColor(40, 40, 45);

        ofLog() << "=== Neural Image Classification Example ===";
        ofLog() << "";
        ofLog() << "This example demonstrates the ImageClassifier API from ofxNeuralEngine.";
        ofLog() << "It uses Core ML with Neural Engine acceleration for fast inference.";
        ofLog() << "";
        ofLog() << "Controls:";
        ofLog() << "  SPACE - Load model and run classification";
        ofLog() << "  1-9   - Set top-K results to display (1-9)";
        ofLog() << "  R     - Run classification again";
        ofLog() << "";
        ofLog() << "Note: This example uses synthetic test data for demonstration.";
        ofLog() << "In a real application, you would load images from files or camera.";
        ofLog() << "";

        // Create synthetic test image
        createTestImage();
    }

    void createTestImage() {
        // Create RGB test image with a simple pattern
        testPixels.allocate(imageWidth, imageHeight, 3);

        // Create a circular gradient pattern
        int centerX = imageWidth / 2;
        int centerY = imageHeight / 2;
        float maxDist = sqrtf(centerX * centerX + centerY * centerY);

        for (int y = 0; y < imageHeight; ++y) {
            for (int x = 0; x < imageWidth; ++x) {
                int index = (y * imageWidth + x) * 3;

                // Distance from center
                float dx = x - centerX;
                float dy = y - centerY;
                float dist = sqrtf(dx * dx + dy * dy);
                float normalizedDist = dist / maxDist;

                // Create gradient pattern
                testPixels.getData()[index + 0] = (uint8_t)(255 * (1.0f - normalizedDist)); // R
                testPixels.getData()[index + 1] = (uint8_t)(128 + 127 * sinf(normalizedDist * 3.14159f * 4)); // G
                testPixels.getData()[index + 2] = (uint8_t)(128 + 127 * cosf(normalizedDist * 3.14159f * 4)); // B
            }
        }

        // Load into ofImage for display
        testImage.setFromPixels(testPixels);

        imageReady = true;
        ofLog() << "Created test image: " << imageWidth << "x" << imageHeight << " RGB";
    }

    void loadModelAndClassify() {
        // NOTE: This is a demonstration of the API.
        // In a real application, you would provide a path to an actual .mlmodelc file.
        // Example: "models/MobileNetV3.mlmodelc"

        std::string modelPath = "MobileNetV3.mlmodelc";

        ofLog() << "";
        ofLog() << "=== Loading Model ===";
        ofLog() << "Path: " << modelPath;
        ofLog() << "Note: Model file not included in example (demonstration only)";
        ofLog() << "";

        // Attempt to load model
        // This will fail in the demo since we don't include actual model files
        bool loaded = classifier.load(modelPath);

        if (!loaded) {
            ofLog() << "⚠️  Model loading failed (expected - model file not included)";
            ofLog() << "";
            ofLog() << "To use this example with a real model:";
            ofLog() << "1. Download a Core ML image classification model";
            ofLog() << "   Example: MobileNetV3 from Apple's Core ML Models";
            ofLog() << "   https://developer.apple.com/machine-learning/models/";
            ofLog() << "";
            ofLog() << "2. Compile the model (.mlmodel → .mlmodelc):";
            ofLog() << "   xcrun coremlcompiler compile MyModel.mlmodel ./";
            ofLog() << "";
            ofLog() << "3. Update modelPath in this example to point to your .mlmodelc";
            ofLog() << "";
            ofLog() << "4. Ensure the model input size matches imageWidth/imageHeight";
            ofLog() << "   (typically 224x224 for MobileNet, 299x299 for Inception)";
            ofLog() << "";

            statusMessage = "Model not found - see console for instructions";
            modelLoaded = false;
            return;
        }

        modelLoaded = true;
        ofLog() << "✓ Model loaded successfully";
        ofLog() << "  Neural Engine: " << (classifier.isNeuralEngineSupported() ? "Supported" : "Not available");
        ofLog() << "";

        // Run classification
        runClassification();
    }

    void runClassification() {
        if (!modelLoaded) {
            ofLog() << "Model not loaded. Press SPACE to load model.";
            return;
        }

        if (!imageReady) {
            ofLog() << "Image not ready.";
            return;
        }

        ofLog() << "=== Running Classification ===";
        float startTime = ofGetElapsedTimef();

        // Classify image
        results = classifier.classify(testPixels, topK);

        classificationTime = ofGetElapsedTimef() - startTime;

        ofLog() << "✓ Classification complete in " << (classificationTime * 1000.0f) << " ms";
        ofLog() << "";
        ofLog() << "Top " << topK << " predictions:";

        for (size_t i = 0; i < results.size(); ++i) {
            ofLog() << "  " << (i + 1) << ". " << results[i].label
                    << " (" << (results[i].confidence * 100.0f) << "%)";
        }
        ofLog() << "";

        resultsReady = true;
        statusMessage = "Classification complete - Press R to run again";
    }

    void update() override {
        // Update logic if needed
    }

    void draw() override {
        ofSetColor(255);

        // Draw test image if available
        if (imageReady) {
            float scale = 2.0f;
            testImage.draw(20, 20, imageWidth * scale, imageHeight * scale);

            // Draw border around image
            ofNoFill();
            ofSetColor(100, 255, 100);
            ofDrawRectangle(20, 20, imageWidth * scale, imageHeight * scale);
            ofFill();
        }

        // Draw status and instructions
        ofSetColor(255);
        float textX = 20;
        float textY = imageReady ? 20 + imageHeight * 2.0f + 40 : 20;

        // Status message
        ofDrawBitmapString("Status: " + statusMessage, textX, textY);
        textY += 30;

        // Model info
        if (modelLoaded) {
            ofSetColor(100, 255, 100);
            ofDrawBitmapString("Model: Loaded", textX, textY);
            textY += 20;
            ofDrawBitmapString("Neural Engine: " +
                std::string(classifier.isNeuralEngineSupported() ? "Available" : "Not available"),
                textX, textY);
            textY += 30;
        } else {
            ofSetColor(255, 100, 100);
            ofDrawBitmapString("Model: Not loaded", textX, textY);
            textY += 30;
        }

        // Classification results
        if (resultsReady && !results.empty()) {
            ofSetColor(255);
            textY += 10;
            ofDrawBitmapString("Classification Results (Top " + ofToString(topK) + "):", textX, textY);
            textY += 20;
            ofDrawBitmapString("Time: " + ofToString(classificationTime * 1000.0f, 2) + " ms", textX, textY);
            textY += 30;

            for (size_t i = 0; i < results.size(); ++i) {
                // Color code by confidence
                float conf = results[i].confidence;
                if (conf > 0.7f) {
                    ofSetColor(100, 255, 100);  // High confidence - green
                } else if (conf > 0.3f) {
                    ofSetColor(255, 200, 100);  // Medium confidence - yellow
                } else {
                    ofSetColor(255, 100, 100);  // Low confidence - red
                }

                std::string text = ofToString(i + 1) + ". " + results[i].label +
                                 " (" + ofToString(results[i].confidence * 100.0f, 1) + "%)";
                ofDrawBitmapString(text, textX, textY);
                textY += 20;
            }
        }

        // Controls
        ofSetColor(150);
        textY += 30;
        ofDrawBitmapString("Controls:", textX, textY);
        textY += 20;
        ofDrawBitmapString("  SPACE - Load model and classify", textX, textY);
        textY += 20;
        ofDrawBitmapString("  1-9   - Set top-K results", textX, textY);
        textY += 20;
        ofDrawBitmapString("  R     - Run classification again", textX, textY);

        // FPS
        ofSetColor(100);
        ofDrawBitmapString("FPS: " + ofToString(ofGetFrameRate(), 1), ofGetWidth() - 100, 20);
    }

    void keyPressed(int key) override {
        if (key == ' ') {
            loadModelAndClassify();
        } else if (key == 'r' || key == 'R') {
            if (modelLoaded) {
                runClassification();
            } else {
                ofLog() << "Model not loaded. Press SPACE first.";
            }
        } else if (key >= '1' && key <= '9') {
            topK = key - '0';
            ofLog() << "Top-K set to: " << topK;
            if (modelLoaded && resultsReady) {
                runClassification();
            }
        }
    }
};

// Entry point
int main() {
    NeuralClassifyApp app;
    auto engine = oflike::Engine::getInstance();
    engine->setup(&app, 1024, 768, "Neural Image Classification Example");
    engine->run();
    return 0;
}
