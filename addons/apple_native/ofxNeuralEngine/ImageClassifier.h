#pragma once

// ImageClassifier - Core ML image classification wrapper
//
// This class provides easy-to-use image classification using Core ML models.
// Supports any Core ML image classification model and can leverage the Neural Engine
// for accelerated inference on Apple Silicon.
//
// Features:
// - Top-K classification results
// - Neural Engine acceleration
// - ofPixels and ofTexture input support
// - Configurable compute units (CPU, GPU, Neural Engine)
//
// Example usage:
//   ImageClassifier classifier;
//   classifier.load("MobileNetV3.mlmodelc");
//   auto results = classifier.classify(myImage);
//   for (const auto& result : results) {
//       ofLog() << result.label << ": " << result.confidence;
//   }

#include <string>
#include <vector>
#include <memory>

// Forward declarations
namespace oflike {
    class ofPixels;
    class ofTexture;
}

namespace NeuralEngine {

// Single classification result
struct ClassificationResult {
    std::string label;      // Class label (e.g., "dog", "cat")
    float confidence;       // Confidence score 0.0 to 1.0
    int classIndex;        // Class index in model

    ClassificationResult() : confidence(0.0f), classIndex(-1) {}
    ClassificationResult(const std::string& l, float c, int idx)
        : label(l), confidence(c), classIndex(idx) {}
};

// Configuration for classifier
struct ClassifierConfig {
    // Compute units preference
    enum class ComputeUnits {
        All,                    // All available (Neural Engine preferred)
        CPUOnly,                // CPU only
        CPUAndGPU,              // CPU and GPU only
        CPUAndNeuralEngine      // CPU and Neural Engine only (recommended)
    };

    ComputeUnits computeUnits = ComputeUnits::All;

    // Number of top results to return (default: 5)
    size_t topK = 5;

    // Minimum confidence threshold (0.0 to 1.0)
    float minConfidence = 0.0f;
};

// Information about loaded model
struct ClassifierInfo {
    std::string modelPath;
    size_t inputWidth = 0;
    size_t inputHeight = 0;
    size_t numClasses = 0;
    bool isNeuralEngineSupported = false;
    bool isLoaded = false;
};

class ImageClassifier {
public:
    // ============================================================================
    // Constructors / Destructor
    // ============================================================================

    ImageClassifier();
    ~ImageClassifier();

    // Move semantics (disable copy due to Core ML resources)
    ImageClassifier(ImageClassifier&& other) noexcept;
    ImageClassifier& operator=(ImageClassifier&& other) noexcept;

    // Disable copy
    ImageClassifier(const ImageClassifier&) = delete;
    ImageClassifier& operator=(const ImageClassifier&) = delete;

    // ============================================================================
    // Model Loading
    // ============================================================================

    // Load Core ML model from file path
    // Accepts .mlmodelc (compiled) or .mlpackage paths
    // Returns true if successful
    bool load(const std::string& modelPath);

    // Load Core ML model with custom configuration
    // Returns true if successful
    bool load(const std::string& modelPath, const ClassifierConfig& config);

    // Unload current model and free resources
    void unload();

    // ============================================================================
    // Classification
    // ============================================================================

    // Classify image from ofPixels
    // Returns top-K classification results sorted by confidence (highest first)
    std::vector<ClassificationResult> classify(const oflike::ofPixels& pixels);

    // Classify image from ofTexture
    // Returns top-K classification results sorted by confidence (highest first)
    std::vector<ClassificationResult> classify(const oflike::ofTexture& texture);

    // ============================================================================
    // Configuration
    // ============================================================================

    // Set number of top results to return (default: 5)
    void setTopK(size_t k);

    // Get current top-K setting
    size_t getTopK() const;

    // Set minimum confidence threshold (0.0 to 1.0)
    void setMinConfidence(float threshold);

    // Get current confidence threshold
    float getMinConfidence() const;

    // ============================================================================
    // Information
    // ============================================================================

    // Check if model is loaded
    bool isLoaded() const;

    // Get model information
    ClassifierInfo getInfo() const;

    // Get all class labels (if available in model)
    std::vector<std::string> getClassLabels() const;

    // Get last error message
    std::string getLastError() const;

private:
    // pImpl pattern - hide Objective-C++ implementation
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace NeuralEngine
