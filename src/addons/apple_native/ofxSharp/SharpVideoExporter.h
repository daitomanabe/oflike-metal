#pragma once

// SharpVideoExporter - Video export for Gaussian Splatting scenes
//
// This class provides high-quality video export of animated Gaussian Splatting scenes
// using Apple's VideoToolbox framework. Features:
// - H.265/HEVC encoding (hardware accelerated)
// - ProRes 422/4444 encoding (professional quality)
// - Hardware acceleration on Apple Silicon
// - Progress callbacks for UI feedback
// - Frame-by-frame rendering from camera paths
// - Multiple resolution presets (HD, 4K, 8K)
// - Customizable bitrate and quality settings
// - Zero-copy texture export via Metal
//
// Usage:
//   Sharp::VideoExporter exporter;
//   Sharp::VideoExportSettings settings;
//   settings.codec = Sharp::VideoCodec::H265;
//   settings.resolution = Sharp::VideoResolution::UHD_4K;
//   settings.framerate = 60;
//
//   exporter.setup(settings);
//   exporter.setProgressCallback([](float progress) {
//       std::cout << "Progress: " << (progress * 100) << "%" << std::endl;
//   });
//
//   exporter.beginExport("output.mov");
//   for (int i = 0; i < totalFrames; i++) {
//       // Render frame
//       exporter.addFrame(fbo.getTexture());
//   }
//   exporter.endExport();

#include <string>
#include <memory>
#include <functional>

// Forward declarations
namespace oflike {
    class ofTexture;
    class ofPixels;
    class ofFbo;
}

namespace Sharp {

// Forward declarations
class GaussianCloud;
class CameraPath;
class SharpScene;

// ============================================================================
// Enums
// ============================================================================

/// Video codec type
enum class VideoCodec {
    H264,           ///< H.264/AVC (standard quality, wide compatibility)
    H265,           ///< H.265/HEVC (better compression, modern)
    ProRes422,      ///< Apple ProRes 422 (professional quality)
    ProRes4444      ///< Apple ProRes 4444 (with alpha channel)
};

/// Video resolution presets
enum class VideoResolution {
    HD_720p,        ///< 1280x720
    HD_1080p,       ///< 1920x1080
    QHD_1440p,      ///< 2560x1440
    UHD_4K,         ///< 3840x2160
    UHD_5K,         ///< 5120x2880
    UHD_8K,         ///< 7680x4320
    Custom          ///< Custom resolution (specify width/height manually)
};

/// Video quality preset
enum class VideoQuality {
    Low,            ///< Low quality (smaller file size)
    Medium,         ///< Medium quality (balanced)
    High,           ///< High quality (larger file size)
    Lossless        ///< Lossless quality (ProRes only, very large)
};

/// Export status
enum class ExportStatus {
    Idle,           ///< Not exporting
    Preparing,      ///< Preparing export
    Encoding,       ///< Encoding frames
    Finalizing,     ///< Finalizing video file
    Completed,      ///< Export completed successfully
    Cancelled,      ///< Export cancelled by user
    Error           ///< Error occurred
};

// ============================================================================
// Settings Structure
// ============================================================================

/// Video export settings
struct VideoExportSettings {
    // Resolution
    VideoResolution resolution = VideoResolution::HD_1080p;
    size_t customWidth = 1920;      ///< Used when resolution = Custom
    size_t customHeight = 1080;     ///< Used when resolution = Custom

    // Codec
    VideoCodec codec = VideoCodec::H265;

    // Quality
    VideoQuality quality = VideoQuality::High;
    float customBitrate = 0.0f;     ///< Mbps, 0 = auto based on quality

    // Frame rate
    int framerate = 30;

    // Hardware acceleration
    bool useHardwareAcceleration = true;

    // Alpha channel (ProRes4444 only)
    bool exportAlpha = false;

    // Color space
    bool useHDR = false;            ///< Use HDR color space (H.265 only)

    VideoExportSettings() = default;
};

// ============================================================================
// VideoExporter Class
// ============================================================================

class VideoExporter {
public:
    // ========================================================================
    // Construction / Destruction
    // ========================================================================

    VideoExporter();
    ~VideoExporter();

    // Move semantics
    VideoExporter(VideoExporter&& other) noexcept;
    VideoExporter& operator=(VideoExporter&& other) noexcept;

    // Disable copy
    VideoExporter(const VideoExporter&) = delete;
    VideoExporter& operator=(const VideoExporter&) = delete;

    // ========================================================================
    // Setup / Configuration
    // ========================================================================

    /**
     * Setup exporter with settings.
     * Call this before beginExport().
     * @param settings Export settings
     * @return true if setup successful
     */
    bool setup(const VideoExportSettings& settings);

    /**
     * Get current settings.
     */
    VideoExportSettings getSettings() const;

    /**
     * Check if hardware acceleration is available for the current codec.
     */
    bool isHardwareAccelerationAvailable() const;

    /**
     * Get maximum supported resolution for current codec.
     */
    VideoResolution getMaxSupportedResolution() const;

    // ========================================================================
    // Export Control
    // ========================================================================

    /**
     * Begin video export session.
     * @param outputPath Output file path (.mov, .mp4)
     * @return true if export started successfully
     */
    bool beginExport(const std::string& outputPath);

    /**
     * Add a frame to the video.
     * Call this repeatedly for each frame during export.
     * @param texture Source texture to encode
     * @return true if frame added successfully
     */
    bool addFrame(const oflike::ofTexture& texture);

    /**
     * Add a frame from pixels.
     * @param pixels Source pixels to encode
     * @return true if frame added successfully
     */
    bool addFrame(const oflike::ofPixels& pixels);

    /**
     * End export session and finalize video file.
     * This may take some time as the video is finalized.
     * @return true if export completed successfully
     */
    bool endExport();

    /**
     * Cancel ongoing export.
     * Aborts export and deletes incomplete file.
     */
    void cancelExport();

    /**
     * Check if currently exporting.
     */
    bool isExporting() const;

    /**
     * Get current export status.
     */
    ExportStatus getStatus() const;

    // ========================================================================
    // Progress Monitoring
    // ========================================================================

    /// Progress callback function type
    /// @param progress Normalized progress (0.0 - 1.0)
    /// @param frameIndex Current frame index
    /// @param totalFrames Total frames (if known, 0 if unknown)
    using ProgressCallback = std::function<void(float progress, size_t frameIndex, size_t totalFrames)>;

    /**
     * Set progress callback.
     * Called periodically during export with progress updates.
     * @param callback Callback function (called on encoding thread)
     */
    void setProgressCallback(ProgressCallback callback);

    /**
     * Get current progress (0.0 - 1.0).
     * Returns 0.0 if not exporting or totalFrames unknown.
     */
    float getProgress() const;

    /**
     * Get number of frames encoded so far.
     */
    size_t getEncodedFrameCount() const;

    /**
     * Set expected total frames for progress calculation.
     * Optional - improves progress accuracy if known in advance.
     * @param totalFrames Total number of frames to encode
     */
    void setExpectedTotalFrames(size_t totalFrames);

    // ========================================================================
    // Convenience Methods - Scene Export
    // ========================================================================

    /**
     * Export a Gaussian Splatting scene with camera animation.
     * This is a convenience method that handles the entire export process.
     * @param scene Scene to render
     * @param cameraPath Camera animation path
     * @param outputPath Output file path
     * @param settings Export settings
     * @return true if export completed successfully
     */
    bool exportScene(const SharpScene& scene,
                     const CameraPath& cameraPath,
                     const std::string& outputPath,
                     const VideoExportSettings& settings);

    /**
     * Export a single Gaussian cloud with camera animation.
     * @param cloud Gaussian cloud to render
     * @param cameraPath Camera animation path
     * @param outputPath Output file path
     * @param settings Export settings
     * @return true if export completed successfully
     */
    bool exportCloud(const GaussianCloud& cloud,
                     const CameraPath& cameraPath,
                     const std::string& outputPath,
                     const VideoExportSettings& settings);

    // ========================================================================
    // Error Handling
    // ========================================================================

    /**
     * Get last error message.
     */
    std::string getLastError() const;

    /**
     * Check if an error occurred.
     */
    bool hasError() const;

    // ========================================================================
    // Statistics
    // ========================================================================

    /**
     * Get estimated file size in bytes (updated during export).
     */
    size_t getEstimatedFileSize() const;

    /**
     * Get current encoding speed (frames per second).
     */
    float getEncodingSpeed() const;

    /**
     * Get estimated time remaining in seconds.
     * Returns 0.0 if unknown.
     */
    float getEstimatedTimeRemaining() const;

    /**
     * Get elapsed time since export started (seconds).
     */
    float getElapsedTime() const;

    // ========================================================================
    // Utility
    // ========================================================================

    /**
     * Get resolution dimensions for a preset.
     * @param resolution Resolution preset
     * @param outWidth Output width
     * @param outHeight Output height
     */
    static void getResolutionDimensions(VideoResolution resolution,
                                       size_t& outWidth,
                                       size_t& outHeight);

    /**
     * Get string name for codec.
     */
    static std::string getCodecName(VideoCodec codec);

    /**
     * Get file extension for codec.
     */
    static std::string getFileExtension(VideoCodec codec);

    /**
     * Get recommended bitrate in Mbps for resolution and quality.
     */
    static float getRecommendedBitrate(VideoResolution resolution,
                                      VideoQuality quality,
                                      VideoCodec codec);

private:
    // pImpl pattern to hide VideoToolbox/Objective-C++ implementation
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace Sharp
