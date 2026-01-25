#pragma once

// GenericModel - Generic Core ML model wrapper
//
// This class provides a flexible interface for any Core ML model.
// Unlike specialized wrappers (ImageClassifier, StyleTransfer, etc.),
// this class allows you to work with arbitrary inputs and outputs.
//
// Features:
// - Support for any Core ML model (.mlmodelc or .mlpackage)
// - Dynamic input/output introspection
// - Multiple input types: images, arrays, dictionaries
// - Multiple output types: arrays, dictionaries, images
// - Neural Engine acceleration
// - Configurable compute units (CPU, GPU, Neural Engine)
// - Batch prediction support
//
// Example usage:
//   GenericModel model;
//   model.load("MyModel.mlmodelc");
//
//   // Set inputs
//   model.setInput("image", myImage);
//   model.setInput("threshold", 0.5f);
//
//   // Run prediction
//   model.predict();
//
//   // Get outputs
//   auto scores = model.getOutputArray<float>("scores");
//   auto labels = model.getOutputArray<std::string>("labels");

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <any>

// Forward declarations
namespace oflike {
    template<typename T> class ofPixels_;
    using ofPixels = ofPixels_<uint8_t>;
    class ofTexture;
}

namespace NeuralEngine {

// Feature types supported by Core ML
enum class FeatureType {
    Invalid,
    Int64,
    Double,
    String,
    Image,
    MultiArray,
    Dictionary,
    Sequence
};

// Multi-array data type
enum class MultiArrayDataType {
    Invalid,
    Double,
    Float32,
    Float16,
    Int32
};

// Feature description from model
struct FeatureInfo {
    std::string name;
    FeatureType type = FeatureType::Invalid;
    bool isOptional = false;

    // For MultiArray features
    MultiArrayDataType arrayDataType = MultiArrayDataType::Invalid;
    std::vector<size_t> arrayShape;

    // For Image features
    size_t imageWidth = 0;
    size_t imageHeight = 0;

    // For Dictionary features
    FeatureType dictionaryKeyType = FeatureType::Invalid;
    FeatureType dictionaryValueType = FeatureType::Invalid;
};

// Configuration for generic model
struct GenericModelConfig {
    // Compute units preference
    enum class ComputeUnits {
        All,                    // All available (Neural Engine preferred)
        CPUOnly,                // CPU only
        CPUAndGPU,              // CPU and GPU only
        CPUAndNeuralEngine      // CPU and Neural Engine only (recommended)
    };

    ComputeUnits computeUnits = ComputeUnits::All;

    // Whether to allow fallback to CPU if preferred compute unit is unavailable
    bool allowFallback = true;
};

// Information about loaded model
struct GenericModelInfo {
    std::string modelPath;
    std::string modelDescription;
    std::string modelAuthor;
    std::string modelLicense;
    std::string modelVersion;

    std::vector<FeatureInfo> inputs;
    std::vector<FeatureInfo> outputs;

    bool isNeuralEngineSupported = false;
    bool isLoaded = false;
};

class GenericModel {
public:
    // ============================================================================
    // Constructors / Destructor
    // ============================================================================

    GenericModel();
    ~GenericModel();

    // Move semantics (disable copy due to Core ML resources)
    GenericModel(GenericModel&& other) noexcept;
    GenericModel& operator=(GenericModel&& other) noexcept;

    // Disable copy
    GenericModel(const GenericModel&) = delete;
    GenericModel& operator=(const GenericModel&) = delete;

    // ============================================================================
    // Model Loading
    // ============================================================================

    // Load Core ML model from file path
    // Accepts .mlmodelc (compiled) or .mlpackage paths
    // Returns true if successful
    bool load(const std::string& modelPath);

    // Load Core ML model with custom configuration
    // Returns true if successful
    bool load(const std::string& modelPath, const GenericModelConfig& config);

    // Unload current model and free resources
    void unload();

    // ============================================================================
    // Input Setting
    // ============================================================================

    // Set input from ofPixels (for image inputs)
    void setInput(const std::string& name, const oflike::ofPixels& pixels);

    // Set input from ofTexture (for image inputs)
    void setInput(const std::string& name, const oflike::ofTexture& texture);

    // Set input from int64
    void setInput(const std::string& name, int64_t value);

    // Set input from double
    void setInput(const std::string& name, double value);

    // Set input from float (converted to double)
    void setInput(const std::string& name, float value);

    // Set input from int (converted to int64)
    void setInput(const std::string& name, int value);

    // Set input from string
    void setInput(const std::string& name, const std::string& value);

    // Set input from multi-array (float array)
    void setInput(const std::string& name, const std::vector<float>& data,
                  const std::vector<size_t>& shape);

    // Set input from multi-array (double array)
    void setInput(const std::string& name, const std::vector<double>& data,
                  const std::vector<size_t>& shape);

    // Set input from multi-array (int32 array)
    void setInput(const std::string& name, const std::vector<int32_t>& data,
                  const std::vector<size_t>& shape);

    // Set input from dictionary (string -> double)
    void setInput(const std::string& name, const std::map<std::string, double>& dict);

    // Clear all inputs
    void clearInputs();

    // ============================================================================
    // Prediction
    // ============================================================================

    // Run prediction with current inputs
    // Returns true if successful
    bool predict();

    // Run batch prediction with multiple input sets
    // Each map in the vector represents one set of inputs
    // Returns true if successful
    bool predictBatch(const std::vector<std::map<std::string, std::any>>& batchInputs);

    // ============================================================================
    // Output Retrieval
    // ============================================================================

    // Get output as int64
    int64_t getOutputInt64(const std::string& name) const;

    // Get output as double
    double getOutputDouble(const std::string& name) const;

    // Get output as float (converted from double)
    float getOutputFloat(const std::string& name) const;

    // Get output as string
    std::string getOutputString(const std::string& name) const;

    // Get output as ofPixels (for image outputs)
    oflike::ofPixels getOutputImage(const std::string& name) const;

    // Get output as multi-array (float)
    std::vector<float> getOutputArray(const std::string& name) const;

    // Get output as multi-array with shape information
    std::pair<std::vector<float>, std::vector<size_t>> getOutputArrayWithShape(const std::string& name) const;

    // Get output as dictionary (string -> double)
    std::map<std::string, double> getOutputDictionary(const std::string& name) const;

    // Get raw output shape for multi-array outputs
    std::vector<size_t> getOutputShape(const std::string& name) const;

    // Check if output exists
    bool hasOutput(const std::string& name) const;

    // Get all output names
    std::vector<std::string> getOutputNames() const;

    // ============================================================================
    // Information
    // ============================================================================

    // Check if model is loaded
    bool isLoaded() const;

    // Get model information
    GenericModelInfo getInfo() const;

    // Get input feature info by name
    FeatureInfo getInputInfo(const std::string& name) const;

    // Get output feature info by name
    FeatureInfo getOutputInfo(const std::string& name) const;

    // Get all input names
    std::vector<std::string> getInputNames() const;

    // Get last error message
    std::string getLastError() const;

    // ============================================================================
    // Advanced
    // ============================================================================

    // Get native Core ML model handle (MLModel*)
    // Returns nullptr if not loaded
    // WARNING: This is for advanced use only
    void* getNativeHandle() const;

private:
    // pImpl pattern - hide Objective-C++ implementation
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace NeuralEngine
