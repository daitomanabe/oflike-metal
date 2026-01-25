#pragma once

// ofxSharp - 3D Gaussian Splatting addon for oflike-metal
//
// This addon provides a simplified, ofApp-friendly wrapper around the Sharp namespace
// for generating and rendering 3D Gaussian Splatting representations from single images.
//
// Quick Start:
// ------------
// class ofApp : public ofBaseApp {
//     ofxSharp sharp;
//
//     void setup() {
//         sharp.setup();
//         sharp.generateFromImage(myImage);
//     }
//
//     void draw() {
//         camera.begin();
//         sharp.drawCloud(camera);
//         camera.end();
//     }
// };
//
// Features:
// - ⚡ Neural Engine acceleration (< 1 second inference)
// - 🖼️ Single image to 3D conversion
// - 🎨 Real-time rendering (60+ FPS)
// - 📹 Video export support
// - 🎥 Camera path animation
//
// Reference: https://github.com/apple/ml-sharp

#include <string>
#include <memory>
#include <functional>

// Forward declarations
namespace oflike {
    class ofPixels;
    class ofTexture;
    class ofCamera;
    class ofImage;
}

namespace Sharp {
    class GaussianCloud;
    class SharpModel;
    class SharpRenderer;
    class SharpScene;
    class CameraPath;
    class VideoExporter;
    struct RenderConfig;
    struct RenderStats;
}

/// Main ofxSharp class - simplified API for 3D Gaussian Splatting
class ofxSharp {
public:
    // ============================================================================
    // Constructors / Destructor
    // ============================================================================

    ofxSharp();
    ~ofxSharp();

    // Move semantics (disable copy due to Metal/Core ML resources)
    ofxSharp(ofxSharp&& other) noexcept;
    ofxSharp& operator=(ofxSharp&& other) noexcept;

    // Disable copy
    ofxSharp(const ofxSharp&) = delete;
    ofxSharp& operator=(const ofxSharp&) = delete;

    // ============================================================================
    // Setup
    // ============================================================================

    /**
     * Initialize ofxSharp with default settings.
     * Loads the bundled SHARP Core ML model and initializes the renderer.
     * @return true if successful
     */
    bool setup();

    /**
     * Initialize ofxSharp with custom model path.
     * @param modelPath Path to .mlmodelc file (e.g., "sharp.mlmodelc")
     * @return true if successful
     */
    bool setup(const std::string& modelPath);

    /**
     * Initialize ofxSharp with custom resolution.
     * @param width Render target width
     * @param height Render target height
     * @return true if successful
     */
    bool setup(int width, int height);

    /**
     * Initialize ofxSharp with custom model and resolution.
     * @param modelPath Path to .mlmodelc file
     * @param width Render target width
     * @param height Render target height
     * @return true if successful
     */
    bool setup(const std::string& modelPath, int width, int height);

    /**
     * Check if ofxSharp is initialized.
     */
    bool isSetup() const;

    // ============================================================================
    // Generation (Synchronous)
    // ============================================================================

    /**
     * Generate 3D Gaussian cloud from image pixels.
     * This replaces any existing cloud.
     * @param pixels Input image pixels
     * @return true if successful
     */
    bool generateFromImage(const oflike::ofPixels& pixels);

    /**
     * Generate 3D Gaussian cloud from texture.
     * This replaces any existing cloud.
     * @param texture Input texture
     * @return true if successful
     */
    bool generateFromImage(const oflike::ofTexture& texture);

    /**
     * Generate 3D Gaussian cloud from ofImage.
     * This replaces any existing cloud.
     * @param image Input image
     * @return true if successful
     */
    bool generateFromImage(const oflike::ofImage& image);

    /**
     * Load pre-generated Gaussian cloud from PLY file.
     * @param filepath Path to PLY file
     * @return true if successful
     */
    bool loadCloud(const std::string& filepath);

    /**
     * Save current Gaussian cloud to PLY file.
     * @param filepath Path to output PLY file
     * @return true if successful
     */
    bool saveCloud(const std::string& filepath) const;

    // ============================================================================
    // Generation (Asynchronous)
    // ============================================================================

    /**
     * Callback type for async generation.
     * Parameters: success (bool)
     */
    using GenerateCallback = std::function<void(bool success)>;

    /**
     * Generate 3D Gaussian cloud asynchronously from image pixels.
     * Callback is called on main thread when inference completes.
     * @param pixels Input image pixels
     * @param callback Completion callback
     */
    void generateFromImageAsync(const oflike::ofPixels& pixels, GenerateCallback callback);

    /**
     * Generate 3D Gaussian cloud asynchronously from texture.
     * Callback is called on main thread when inference completes.
     * @param texture Input texture
     * @param callback Completion callback
     */
    void generateFromImageAsync(const oflike::ofTexture& texture, GenerateCallback callback);

    /**
     * Generate 3D Gaussian cloud asynchronously from ofImage.
     * Callback is called on main thread when inference completes.
     * @param image Input image
     * @param callback Completion callback
     */
    void generateFromImageAsync(const oflike::ofImage& image, GenerateCallback callback);

    /**
     * Check if async generation is in progress.
     */
    bool isGenerating() const;

    /**
     * Cancel ongoing async generation.
     */
    void cancelGeneration();

    // ============================================================================
    // Rendering
    // ============================================================================

    /**
     * Draw the current Gaussian cloud with the given camera.
     * @param camera Camera for view/projection matrices
     */
    void drawCloud(const oflike::ofCamera& camera);

    /**
     * Draw the current Gaussian cloud at the current camera position.
     * Uses the active camera from the oflike context.
     */
    void drawCloud();

    /**
     * Check if a Gaussian cloud is loaded and ready to render.
     */
    bool hasCloud() const;

    /**
     * Get number of Gaussians in current cloud.
     */
    size_t getGaussianCount() const;

    // ============================================================================
    // Cloud Manipulation
    // ============================================================================

    /**
     * Translate the cloud.
     * @param x, y, z Translation offset
     */
    void translate(float x, float y, float z);

    /**
     * Rotate the cloud around its center.
     * @param angle Rotation angle in radians
     * @param ax, ay, az Rotation axis
     */
    void rotate(float angle, float ax, float ay, float az);

    /**
     * Scale the cloud uniformly.
     * @param scale Scale factor
     */
    void scale(float scale);

    /**
     * Scale the cloud non-uniformly.
     * @param sx, sy, sz Scale factors
     */
    void scale(float sx, float sy, float sz);

    /**
     * Reset cloud transform to identity.
     */
    void resetTransform();

    // ============================================================================
    // Rendering Configuration
    // ============================================================================

    /**
     * Enable/disable depth sorting (default: true).
     * Depth sorting is required for correct transparency but has a performance cost.
     */
    void setDepthSortEnabled(bool enabled);

    /**
     * Set splat scale multiplier (default: 1.0).
     * Values > 1.0 make splats larger, < 1.0 make them smaller.
     */
    void setSplatScale(float scale);

    /**
     * Set opacity multiplier (default: 1.0).
     * Values < 1.0 make the entire cloud more transparent.
     */
    void setOpacityScale(float scale);

    /**
     * Enable/disable anti-aliasing (default: true).
     */
    void setAntiAliasingEnabled(bool enabled);

    /**
     * Enable/disable spherical harmonics (default: true).
     * When disabled, only DC component (diffuse color) is used.
     */
    void setSphericalHarmonicsEnabled(bool enabled);

    /**
     * Set maximum spherical harmonics degree (default: 3, range: 0-3).
     * 0 = diffuse only, 3 = full view-dependent appearance.
     */
    void setMaxSHDegree(int degree);

    /**
     * Get current render configuration.
     */
    const Sharp::RenderConfig& getRenderConfig() const;

    /**
     * Get rendering statistics from last frame.
     */
    const Sharp::RenderStats& getRenderStats() const;

    // ============================================================================
    // Filtering
    // ============================================================================

    /**
     * Filter Gaussians by opacity threshold.
     * Removes Gaussians with opacity below threshold.
     */
    void filterByOpacity(float minOpacity);

    /**
     * Filter Gaussians by size (scale magnitude).
     */
    void filterBySize(float minSize, float maxSize);

    /**
     * Remove invisible Gaussians (opacity < 0.01).
     */
    void removeInvisible();

    // ============================================================================
    // Error Handling
    // ============================================================================

    /**
     * Get last error message.
     * Returns empty string if no error.
     */
    std::string getLastError() const;

    /**
     * Check if Neural Engine is being used.
     * Returns true if the last inference used Neural Engine.
     */
    bool isUsingNeuralEngine() const;

    /**
     * Get last inference time in milliseconds.
     */
    double getLastInferenceTime() const;

    // ============================================================================
    // Advanced: Direct Access to Sharp Classes
    // ============================================================================

    /**
     * Get direct access to SharpModel for advanced operations.
     * Returns nullptr if not initialized.
     */
    Sharp::SharpModel* getModel();

    /**
     * Get direct access to SharpRenderer for advanced operations.
     * Returns nullptr if not initialized.
     */
    Sharp::SharpRenderer* getRenderer();

    /**
     * Get direct access to current GaussianCloud for advanced operations.
     * Returns nullptr if no cloud is loaded.
     */
    Sharp::GaussianCloud* getCloud();

    /**
     * Get const access to current GaussianCloud.
     * Returns nullptr if no cloud is loaded.
     */
    const Sharp::GaussianCloud* getCloud() const;

private:
    // pImpl pattern to hide Sharp/Objective-C++ implementation
    class Impl;
    std::unique_ptr<Impl> impl_;
};
