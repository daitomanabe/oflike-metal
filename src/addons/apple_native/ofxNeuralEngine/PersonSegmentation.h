#pragma once

// PersonSegmentation - Vision framework person segmentation wrapper
//
// This class provides easy-to-use person segmentation using Apple's Vision framework.
// Leverages VNGeneratePersonSegmentationRequest for real-time person masking.
// Supports separating people from background for matting, background replacement, etc.
//
// Features:
// - Person mask generation (alpha matte)
// - Multi-quality modes (fast, balanced, accurate)
// - Support for multiple people
// - Real-time performance
// - Neural Engine acceleration
// - Direct GPU texture output
//
// Example usage:
//   PersonSegmentation seg;
//   seg.setup();
//   ofPixels mask = seg.segment(myImage);
//   // Use mask for compositing, background replacement, etc.

#include <string>
#include <vector>
#include <memory>
#include <functional>

// Forward declarations
namespace oflike {
    template<typename T> class ofPixels_;
    using ofPixels = ofPixels_<uint8_t>;
    class ofTexture;
}

namespace NeuralEngine {

// Quality level for person segmentation
enum class SegmentationQuality {
    Fast,       // Fastest processing, lower quality
    Balanced,   // Balance between speed and quality (default)
    Accurate    // Best quality, slower processing
};

// Configuration for person segmentation
struct PersonSegmentationConfig {
    // Quality level (affects performance vs accuracy tradeoff)
    SegmentationQuality quality = SegmentationQuality::Balanced;

    // Output resolution scale (1.0 = same as input, 0.5 = half resolution)
    // Lower values improve performance at cost of mask quality
    float outputScale = 1.0f;

    // Minimum confidence threshold for person detection (0.0 to 1.0)
    float minConfidence = 0.3f;

    // Whether to detect multiple people (false = single person only)
    bool multiPerson = true;

    // Smooth mask edges (reduces aliasing)
    bool smoothEdges = true;
};

// Information about segmentation result
struct SegmentationInfo {
    size_t inputWidth = 0;
    size_t inputHeight = 0;
    size_t maskWidth = 0;
    size_t maskHeight = 0;
    size_t numPeopleDetected = 0;
    float averageConfidence = 0.0f;
    bool isValid = false;
};

// Result of person segmentation
struct SegmentationResult {
    oflike::ofPixels mask;          // Alpha mask (grayscale, 0=background, 255=person)
    SegmentationInfo info;          // Metadata about segmentation
    std::string errorMessage;       // Empty if successful

    SegmentationResult();

    // Check if segmentation was successful
    bool isValid() const { return info.isValid; }

    // Apply mask to input image (extract person, remove background)
    oflike::ofPixels applyMask(const oflike::ofPixels& input) const;

    // Invert mask (person=0, background=255)
    oflike::ofPixels getInvertedMask() const;

    // Get blur feathered mask (soft edges)
    oflike::ofPixels getFeatheredMask(float radius = 5.0f) const;
};

class PersonSegmentation {
public:
    // ============================================================================
    // Constructors / Destructor
    // ============================================================================

    PersonSegmentation();
    ~PersonSegmentation();

    // Move semantics (disable copy due to Vision resources)
    PersonSegmentation(PersonSegmentation&& other) noexcept;
    PersonSegmentation& operator=(PersonSegmentation&& other) noexcept;

    // Disable copy
    PersonSegmentation(const PersonSegmentation&) = delete;
    PersonSegmentation& operator=(const PersonSegmentation&) = delete;

    // ============================================================================
    // Setup
    // ============================================================================

    // Setup segmentation with default configuration
    bool setup();

    // Setup segmentation with custom configuration
    bool setup(const PersonSegmentationConfig& config);

    // Shutdown and release resources
    void shutdown();

    // Check if segmentation is ready
    bool isSetup() const;

    // ============================================================================
    // Segmentation
    // ============================================================================

    // Generate person mask from ofPixels
    // Returns segmentation result with mask and metadata
    SegmentationResult segment(const oflike::ofPixels& pixels);

    // Generate person mask from ofTexture (GPU-accelerated)
    // Returns segmentation result with mask and metadata
    SegmentationResult segment(oflike::ofTexture& texture);

    // ============================================================================
    // Async Segmentation
    // ============================================================================

    // Callback for async segmentation completion
    using SegmentationCallback = std::function<void(const SegmentationResult&)>;

    // Async segmentation from ofPixels
    // Callback is invoked on completion (runs on background thread)
    void segmentAsync(const oflike::ofPixels& pixels, SegmentationCallback callback);

    // Async segmentation from ofTexture
    // Callback is invoked on completion (runs on background thread)
    void segmentAsync(oflike::ofTexture& texture, SegmentationCallback callback);

    // Cancel any pending async operations
    void cancelAsync();

    // Check if async operation is in progress
    bool isProcessing() const;

    // ============================================================================
    // Configuration
    // ============================================================================

    // Update configuration (requires setup)
    bool setConfig(const PersonSegmentationConfig& config);

    // Get current configuration
    PersonSegmentationConfig getConfig() const;

    // Set quality level
    void setQuality(SegmentationQuality quality);

    // Set output scale
    void setOutputScale(float scale);

    // Set minimum confidence threshold
    void setMinConfidence(float confidence);

    // Enable/disable multi-person detection
    void setMultiPerson(bool enabled);

    // Enable/disable edge smoothing
    void setSmoothEdges(bool enabled);

    // ============================================================================
    // Information
    // ============================================================================

    // Check if Neural Engine is available (Apple Silicon M1+)
    static bool isNeuralEngineAvailable();

    // Check if person segmentation is supported on this device
    static bool isSupported();

    // Get last error message
    std::string getLastError() const;

    // ============================================================================
    // Utility Functions
    // ============================================================================

    // Composite person over new background
    // Uses mask to blend person from foreground with background image
    static oflike::ofPixels composite(
        const oflike::ofPixels& foreground,
        const oflike::ofPixels& background,
        const oflike::ofPixels& mask
    );

    // Extract person from image (transparent background)
    // Returns RGBA image with alpha channel from mask
    static oflike::ofPixels extractPerson(
        const oflike::ofPixels& image,
        const oflike::ofPixels& mask
    );

    // Blur background only (keep person sharp)
    static oflike::ofPixels blurBackground(
        const oflike::ofPixels& image,
        const oflike::ofPixels& mask,
        float blurRadius = 10.0f
    );

private:
    // pImpl pattern to hide Objective-C++ implementation
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace NeuralEngine
