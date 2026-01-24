#pragma once

// StyleTransfer - Core ML style transfer wrapper
//
// This class provides easy-to-use style transfer using Core ML models.
// Supports neural style transfer models that transform images with artistic styles.
// Leverages Neural Engine for accelerated inference on Apple Silicon.
//
// Features:
// - Artistic style transfer
// - Neural Engine acceleration
// - ofPixels and ofTexture input support
// - GPU-accelerated processing
// - Configurable compute units (CPU, GPU, Neural Engine)
// - Style intensity control
//
// Example usage:
//   StyleTransfer transfer;
//   transfer.load("CandyStyle.mlmodelc");
//   ofPixels stylized = transfer.transfer(myImage);
//   ofTexture result;
//   result.loadData(stylized);
//   result.draw(0, 0);

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

// Configuration for style transfer
struct StyleTransferConfig {
    // Compute units preference
    enum class ComputeUnits {
        All,                    // All available (Neural Engine preferred)
        CPUOnly,                // CPU only
        CPUAndGPU,              // CPU and GPU only
        CPUAndNeuralEngine      // CPU and Neural Engine only (recommended)
    };

    ComputeUnits computeUnits = ComputeUnits::All;

    // Style intensity (0.0 = original, 1.0 = full style) - if model supports
    float styleIntensity = 1.0f;

    // Whether to preserve original image colors (colorize only)
    bool preserveColors = false;
};

// Information about loaded model
struct StyleTransferInfo {
    std::string modelPath;
    size_t inputWidth = 0;
    size_t inputHeight = 0;
    size_t outputWidth = 0;
    size_t outputHeight = 0;
    bool isNeuralEngineSupported = false;
    bool isLoaded = false;
    bool supportsStyleIntensity = false;
};

class StyleTransfer {
public:
    // ============================================================================
    // Constructors / Destructor
    // ============================================================================

    StyleTransfer();
    ~StyleTransfer();

    // Move semantics (disable copy due to Core ML resources)
    StyleTransfer(StyleTransfer&& other) noexcept;
    StyleTransfer& operator=(StyleTransfer&& other) noexcept;

    // Disable copy
    StyleTransfer(const StyleTransfer&) = delete;
    StyleTransfer& operator=(const StyleTransfer&) = delete;

    // ============================================================================
    // Model Loading
    // ============================================================================

    // Load Core ML style transfer model from file path
    // Accepts .mlmodelc (compiled) or .mlpackage paths
    // Returns true if successful
    bool load(const std::string& modelPath);

    // Load Core ML model with custom configuration
    // Returns true if successful
    bool load(const std::string& modelPath, const StyleTransferConfig& config);

    // Unload current model and free resources
    void unload();

    // ============================================================================
    // Style Transfer
    // ============================================================================

    // Transfer style to ofPixels
    // Returns stylized image as ofPixels
    // Output has same dimensions as input (model may resize internally)
    oflike::ofPixels transfer(const oflike::ofPixels& pixels);

    // Transfer style to ofTexture (GPU direct processing)
    // Returns stylized image as ofPixels
    // More efficient than converting texture to pixels first
    oflike::ofPixels transfer(const oflike::ofTexture& texture);

    // Transfer style and output directly to texture (GPU-only path)
    // Returns true if successful
    // This is the most efficient method for real-time applications
    bool transferToTexture(const oflike::ofTexture& input, oflike::ofTexture& output);

    // Batch transfer - process multiple images
    // Returns vector of stylized images
    std::vector<oflike::ofPixels> transferBatch(const std::vector<oflike::ofPixels>& images);

    // ============================================================================
    // Configuration
    // ============================================================================

    // Set style intensity (0.0 = original, 1.0 = full style)
    // Only works if model supports intensity parameter
    void setStyleIntensity(float intensity);

    // Get current style intensity
    float getStyleIntensity() const;

    // Set whether to preserve original colors
    void setPreserveColors(bool preserve);

    // Get current preserve colors setting
    bool getPreserveColors() const;

    // ============================================================================
    // Information
    // ============================================================================

    // Check if model is loaded
    bool isLoaded() const;

    // Get model information
    StyleTransferInfo getInfo() const;

    // Get last error message
    std::string getLastError() const;

private:
    // pImpl pattern - hide Objective-C++ implementation
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace NeuralEngine
