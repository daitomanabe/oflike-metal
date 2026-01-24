#pragma once

// SharpModel - Core ML model wrapper for SHARP inference
//
// This class wraps Apple's SHARP (Single-image High-fidelity 3D Asset Reconstruction with Priors)
// Core ML model for generating 3D Gaussian Splatting representations from single images.
//
// The model takes a single RGB image as input and produces:
// - 3D Gaussian primitives (positions, scales, rotations, opacities)
// - Spherical harmonics coefficients for view-dependent appearance
// - Optimized for Apple Neural Engine execution
//
// Reference: https://github.com/apple/ml-sharp
// Paper: "SHARP: Single-image to High-fidelity 3D Asset Reconstruction with Priors"

#include "SharpGaussianCloud.h"
#include <string>
#include <memory>
#include <functional>

// Forward declarations
namespace oflike {
    class ofPixels;
    class ofTexture;
}

namespace Sharp {

// Status codes for model operations
enum class ModelStatus {
    Success = 0,
    ErrorModelNotLoaded,
    ErrorInvalidInput,
    ErrorInferenceFailed,
    ErrorNeuralEngineUnavailable,
    ErrorInvalidModelFormat
};

// Configuration for model loading
struct ModelConfig {
    // Compute units preference
    enum class ComputeUnits {
        All,            // All available (Neural Engine preferred)
        CPUOnly,        // CPU only
        CPUAndGPU,      // CPU and GPU only
        CPUAndNeuralEngine  // CPU and Neural Engine only
    };

    ComputeUnits computeUnits = ComputeUnits::All;

    // Enable low-power mode (may reduce performance)
    bool lowPowerMode = false;

    // Maximum number of Gaussians to generate (0 = no limit)
    size_t maxGaussians = 0;

    // Minimum opacity threshold for generated Gaussians
    float minOpacity = 0.01f;
};

// Information about loaded model
struct ModelInfo {
    std::string modelPath;
    size_t inputWidth = 0;
    size_t inputHeight = 0;
    size_t maxOutputGaussians = 0;
    bool isNeuralEngineSupported = false;
    bool isLoaded = false;
};

class SharpModel {
public:
    // ============================================================================
    // Constructors / Destructor
    // ============================================================================

    SharpModel();
    ~SharpModel();

    // Move semantics (disable copy due to Metal/Core ML resources)
    SharpModel(SharpModel&& other) noexcept;
    SharpModel& operator=(SharpModel&& other) noexcept;

    // Disable copy
    SharpModel(const SharpModel&) = delete;
    SharpModel& operator=(const SharpModel&) = delete;

    // ============================================================================
    // Model Loading
    // ============================================================================

    // Load Core ML model from file path
    // Returns true if successful
    bool load(const std::string& modelPath);

    // Load Core ML model with custom configuration
    // Returns true if successful
    bool load(const std::string& modelPath, const ModelConfig& config);

    // Unload model and free resources
    void unload();

    // Check if model is loaded and ready
    bool isLoaded() const;

    // Get model information
    ModelInfo getModelInfo() const;

    // ============================================================================
    // Inference (Synchronous)
    // ============================================================================

    // Generate Gaussian cloud from image (ofPixels)
    // Returns empty cloud on error (check getLastStatus())
    GaussianCloud predict(const oflike::ofPixels& image);

    // Generate Gaussian cloud from texture (ofTexture)
    // Returns empty cloud on error (check getLastStatus())
    GaussianCloud predict(const oflike::ofTexture& texture);

    // ============================================================================
    // Inference (Asynchronous)
    // ============================================================================

    // Callback type for async inference
    // Parameters: GaussianCloud result, ModelStatus status
    using PredictCallback = std::function<void(GaussianCloud&& cloud, ModelStatus status)>;

    // Generate Gaussian cloud asynchronously from image (ofPixels)
    // Callback is called on background thread when inference completes
    void predictAsync(const oflike::ofPixels& image, PredictCallback callback);

    // Generate Gaussian cloud asynchronously from texture (ofTexture)
    // Callback is called on background thread when inference completes
    void predictAsync(const oflike::ofTexture& texture, PredictCallback callback);

    // Check if async inference is in progress
    bool isInferenceInProgress() const;

    // Cancel ongoing async inference
    void cancelInference();

    // ============================================================================
    // Error Handling
    // ============================================================================

    // Get status of last operation
    ModelStatus getLastStatus() const;

    // Get human-readable error message for last operation
    std::string getLastError() const;

    // Convert status code to string
    static std::string statusToString(ModelStatus status);

    // ============================================================================
    // Performance Metrics
    // ============================================================================

    // Get last inference time in milliseconds
    double getLastInferenceTime() const;

    // Get average inference time in milliseconds (over all predictions)
    double getAverageInferenceTime() const;

    // Get total number of predictions performed
    size_t getPredictionCount() const;

    // Check if Neural Engine was used for last inference
    bool wasNeuralEngineUsed() const;

    // ============================================================================
    // Configuration
    // ============================================================================

    // Set minimum opacity threshold for output Gaussians
    void setMinOpacity(float threshold);

    // Set maximum number of output Gaussians (0 = no limit)
    void setMaxGaussians(size_t maxCount);

    // Enable/disable post-processing filtering
    void setPostProcessingEnabled(bool enabled);

private:
    // pImpl pattern to hide Core ML/Objective-C++ implementation
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace Sharp
