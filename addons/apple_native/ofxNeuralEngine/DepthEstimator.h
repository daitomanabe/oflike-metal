#pragma once

// DepthEstimator - Core ML depth estimation wrapper
//
// This class provides easy-to-use depth estimation using Core ML models.
// Supports depth estimation models that output depth maps from single images.
// Leverages Neural Engine for accelerated inference on Apple Silicon.
//
// Features:
// - Single-image depth estimation
// - Neural Engine acceleration
// - ofPixels and ofTexture input support
// - Direct GPU processing
// - Configurable compute units (CPU, GPU, Neural Engine)
//
// Example usage:
//   DepthEstimator estimator;
//   estimator.load("DepthAnything.mlmodelc");
//   ofPixels depthMap = estimator.estimate(myImage);
//   ofTexture depthTexture;
//   depthTexture.loadData(depthMap);
//   depthTexture.draw(0, 0);

#include <string>
#include <vector>
#include <memory>

// Forward declarations
namespace oflike {
    template<typename T> class ofPixels_;
    using ofPixels = ofPixels_<uint8_t>;
    class ofTexture;
}

namespace NeuralEngine {

// Configuration for depth estimator
struct DepthEstimatorConfig {
    // Compute units preference
    enum class ComputeUnits {
        All,                    // All available (Neural Engine preferred)
        CPUOnly,                // CPU only
        CPUAndGPU,              // CPU and GPU only
        CPUAndNeuralEngine      // CPU and Neural Engine only (recommended)
    };

    ComputeUnits computeUnits = ComputeUnits::All;

    // Whether to normalize output to 0-255 range (default: true)
    bool normalizeOutput = true;

    // Whether to invert depth (near=white, far=black)
    bool invertDepth = false;
};

// Information about loaded model
struct DepthEstimatorInfo {
    std::string modelPath;
    size_t inputWidth = 0;
    size_t inputHeight = 0;
    size_t outputWidth = 0;
    size_t outputHeight = 0;
    bool isNeuralEngineSupported = false;
    bool isLoaded = false;
};

class DepthEstimator {
public:
    // ============================================================================
    // Constructors / Destructor
    // ============================================================================

    DepthEstimator();
    ~DepthEstimator();

    // Move semantics (disable copy due to Core ML resources)
    DepthEstimator(DepthEstimator&& other) noexcept;
    DepthEstimator& operator=(DepthEstimator&& other) noexcept;

    // Disable copy
    DepthEstimator(const DepthEstimator&) = delete;
    DepthEstimator& operator=(const DepthEstimator&) = delete;

    // ============================================================================
    // Model Loading
    // ============================================================================

    // Load Core ML depth estimation model from file path
    // Accepts .mlmodelc (compiled) or .mlpackage paths
    // Returns true if successful
    bool load(const std::string& modelPath);

    // Load Core ML model with custom configuration
    // Returns true if successful
    bool load(const std::string& modelPath, const DepthEstimatorConfig& config);

    // Unload current model and free resources
    void unload();

    // ============================================================================
    // Depth Estimation
    // ============================================================================

    // Estimate depth from ofPixels
    // Returns depth map as grayscale ofPixels (single channel)
    // Depth values: near=0 (black), far=255 (white) by default
    // If invertDepth=true: near=255 (white), far=0 (black)
    oflike::ofPixels estimate(const oflike::ofPixels& pixels);

    // Estimate depth from ofTexture (GPU direct processing)
    // Returns depth map as grayscale ofPixels
    // More efficient than converting texture to pixels first
    oflike::ofPixels estimate(const oflike::ofTexture& texture);

    // Estimate depth and output directly to texture (GPU-only path)
    // Returns true if successful
    // This is the most efficient method for real-time applications
    bool estimateToTexture(const oflike::ofTexture& input, oflike::ofTexture& output);

    // ============================================================================
    // Configuration
    // ============================================================================

    // Set whether to normalize output to 0-255 range
    void setNormalizeOutput(bool normalize);

    // Get current normalize setting
    bool getNormalizeOutput() const;

    // Set whether to invert depth values
    void setInvertDepth(bool invert);

    // Get current invert depth setting
    bool getInvertDepth() const;

    // ============================================================================
    // Information
    // ============================================================================

    // Check if model is loaded
    bool isLoaded() const;

    // Get model information
    DepthEstimatorInfo getInfo() const;

    // Get last error message
    std::string getLastError() const;

private:
    // pImpl pattern - hide Objective-C++ implementation
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace NeuralEngine
