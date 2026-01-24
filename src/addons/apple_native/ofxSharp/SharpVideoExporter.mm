#import "SharpVideoExporter.h"
#import "SharpGaussianCloud.h"
#import "SharpCameraPath.h"
#import "SharpScene.h"
#import "SharpRenderer.h"
#import "oflike/image/ofTexture.h"
#import "oflike/image/ofPixels.h"
#import "oflike/graphics/ofFbo.h"
#import "oflike/3d/ofCamera.h"
#import "core/Context.h"

#import <AVFoundation/AVFoundation.h>
#import <VideoToolbox/VideoToolbox.h>
#import <CoreVideo/CoreVideo.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <os/log.h>
#import <chrono>

using namespace oflike;

namespace Sharp {

// ============================================================================
// Implementation
// ============================================================================

struct VideoExporter::Impl {
    // Settings
    VideoExportSettings settings_;

    // AVFoundation objects
    AVAssetWriter* assetWriter_ = nil;
    AVAssetWriterInput* writerInput_ = nil;
    AVAssetWriterInputPixelBufferAdaptor* pixelBufferAdaptor_ = nil;

    // State
    ExportStatus status_ = ExportStatus::Idle;
    std::string outputPath_;
    size_t frameIndex_ = 0;
    size_t expectedTotalFrames_ = 0;
    bool hasError_ = false;
    std::string lastError_;

    // Progress callback
    ProgressCallback progressCallback_;

    // Statistics
    std::chrono::steady_clock::time_point startTime_;
    size_t estimatedFileSize_ = 0;
    float encodingSpeed_ = 0.0f;

    // Metal resources for texture conversion
    id<MTLDevice> device_ = nil;
    id<MTLCommandQueue> commandQueue_ = nil;

    // ========================================================================
    // Constructor / Destructor
    // ========================================================================

    Impl() {
        @autoreleasepool {
            device_ = MTLCreateSystemDefaultDevice();
            if (device_) {
                commandQueue_ = [device_ newCommandQueue];
            }
        }
    }

    ~Impl() {
        @autoreleasepool {
            cleanup();
            commandQueue_ = nil;
            device_ = nil;
        }
    }

    // ========================================================================
    // Setup
    // ========================================================================

    bool setup(const VideoExportSettings& settings) {
        @autoreleasepool {
            settings_ = settings;

            // Validate settings
            if (settings_.codec == VideoCodec::ProRes4444 && !settings_.useHardwareAcceleration) {
                lastError_ = "ProRes encoding requires hardware acceleration";
                hasError_ = true;
                return false;
            }

            if (settings_.useHDR && settings_.codec != VideoCodec::H265) {
                lastError_ = "HDR output only supported with H.265 codec";
                hasError_ = true;
                return false;
            }

            hasError_ = false;
            lastError_.clear();
            return true;
        }
    }

    // ========================================================================
    // Export Control
    // ========================================================================

    bool beginExport(const std::string& outputPath) {
        @autoreleasepool {
            if (status_ != ExportStatus::Idle) {
                lastError_ = "Export already in progress";
                hasError_ = true;
                return false;
            }

            status_ = ExportStatus::Preparing;
            outputPath_ = outputPath;
            frameIndex_ = 0;
            estimatedFileSize_ = 0;
            encodingSpeed_ = 0.0f;

            // Get resolution
            size_t width, height;
            getResolutionDimensions(settings_.resolution, width, height);
            if (settings_.resolution == VideoResolution::Custom) {
                width = settings_.customWidth;
                height = settings_.customHeight;
            }

            // Create output URL
            NSString* path = [NSString stringWithUTF8String:outputPath.c_str()];
            NSURL* outputURL = [NSURL fileURLWithPath:path];

            // Delete existing file if present
            [[NSFileManager defaultManager] removeItemAtURL:outputURL error:nil];

            // Create asset writer
            NSError* error = nil;
            assetWriter_ = [[AVAssetWriter alloc] initWithURL:outputURL
                                                      fileType:AVFileTypeQuickTimeMovie
                                                         error:&error];
            if (error) {
                lastError_ = std::string([[error localizedDescription] UTF8String]);
                hasError_ = true;
                status_ = ExportStatus::Error;
                return false;
            }

            // Configure video settings
            NSMutableDictionary* videoSettings = [NSMutableDictionary dictionary];
            videoSettings[AVVideoWidthKey] = @(width);
            videoSettings[AVVideoHeightKey] = @(height);

            // Codec configuration
            configureCodec(videoSettings, width, height);

            // Create writer input
            writerInput_ = [[AVAssetWriterInput alloc] initWithMediaType:AVMediaTypeVideo
                                                          outputSettings:videoSettings];
            writerInput_.expectsMediaDataInRealTime = NO;

            // Create pixel buffer adaptor
            NSDictionary* pixelBufferAttributes = @{
                (NSString*)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA),
                (NSString*)kCVPixelBufferWidthKey: @(width),
                (NSString*)kCVPixelBufferHeightKey: @(height),
                (NSString*)kCVPixelBufferMetalCompatibilityKey: @YES,
            };

            pixelBufferAdaptor_ = [[AVAssetWriterInputPixelBufferAdaptor alloc]
                                  initWithAssetWriterInput:writerInput_
                                  sourcePixelBufferAttributes:pixelBufferAttributes];

            // Add input to writer
            if ([assetWriter_ canAddInput:writerInput_]) {
                [assetWriter_ addInput:writerInput_];
            } else {
                lastError_ = "Cannot add video input to asset writer";
                hasError_ = true;
                status_ = ExportStatus::Error;
                cleanup();
                return false;
            }

            // Start writing
            if (![assetWriter_ startWriting]) {
                lastError_ = std::string([[assetWriter_.error localizedDescription] UTF8String]);
                hasError_ = true;
                status_ = ExportStatus::Error;
                cleanup();
                return false;
            }

            [assetWriter_ startSessionAtSourceTime:kCMTimeZero];

            status_ = ExportStatus::Encoding;
            startTime_ = std::chrono::steady_clock::now();

            os_log_info(OS_LOG_DEFAULT, "VideoExporter: Started export to %s (%zux%zu, %d fps)",
                       outputPath.c_str(), width, height, settings_.framerate);

            return true;
        }
    }

    bool addFrame(const ofTexture& texture) {
        @autoreleasepool {
            if (status_ != ExportStatus::Encoding) {
                lastError_ = "Not in encoding state";
                hasError_ = true;
                return false;
            }

            // Wait for writer to be ready
            while (!writerInput_.readyForMoreMediaData) {
                [NSThread sleepForTimeInterval:0.01];
            }

            // Get texture dimensions
            size_t width = texture.getWidth();
            size_t height = texture.getHeight();

            // Create pixel buffer
            CVPixelBufferRef pixelBuffer = nullptr;
            CVReturn status = CVPixelBufferPoolCreatePixelBuffer(nullptr,
                                                                 pixelBufferAdaptor_.pixelBufferPool,
                                                                 &pixelBuffer);
            if (status != kCVReturnSuccess) {
                lastError_ = "Failed to create pixel buffer";
                hasError_ = true;
                return false;
            }

            // Lock pixel buffer
            CVPixelBufferLockBaseAddress(pixelBuffer, 0);
            void* baseAddress = CVPixelBufferGetBaseAddress(pixelBuffer);
            size_t bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer);

            // Read texture pixels via Metal blit
            id<MTLTexture> metalTexture = (__bridge id<MTLTexture>)texture.getNativeHandle();
            if (metalTexture) {
                // Create temporary texture for readback
                MTLTextureDescriptor* desc = [MTLTextureDescriptor
                    texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                 width:width
                                                height:height
                                             mipmapped:NO];
                desc.usage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;
                desc.storageMode = MTLStorageModeManaged;

                id<MTLTexture> tempTexture = [device_ newTextureWithDescriptor:desc];

                // Blit texture
                id<MTLCommandBuffer> commandBuffer = [commandQueue_ commandBuffer];
                id<MTLBlitCommandEncoder> blitEncoder = [commandBuffer blitCommandEncoder];

                [blitEncoder copyFromTexture:metalTexture
                                 sourceSlice:0
                                 sourceLevel:0
                                sourceOrigin:MTLOriginMake(0, 0, 0)
                                  sourceSize:MTLSizeMake(width, height, 1)
                                   toTexture:tempTexture
                            destinationSlice:0
                            destinationLevel:0
                           destinationOrigin:MTLOriginMake(0, 0, 0)];

                [blitEncoder synchronizeResource:tempTexture];
                [blitEncoder endEncoding];
                [commandBuffer commit];
                [commandBuffer waitUntilCompleted];

                // Copy to pixel buffer
                [tempTexture getBytes:baseAddress
                          bytesPerRow:bytesPerRow
                           fromRegion:MTLRegionMake2D(0, 0, width, height)
                          mipmapLevel:0];
            }

            CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);

            // Calculate presentation time
            CMTime presentationTime = CMTimeMake(frameIndex_, settings_.framerate);

            // Append pixel buffer
            BOOL success = [pixelBufferAdaptor_ appendPixelBuffer:pixelBuffer
                                             withPresentationTime:presentationTime];

            CVPixelBufferRelease(pixelBuffer);

            if (!success) {
                lastError_ = "Failed to append pixel buffer";
                hasError_ = true;
                return false;
            }

            frameIndex_++;

            // Update statistics
            updateStatistics();

            // Call progress callback
            if (progressCallback_) {
                float progress = 0.0f;
                if (expectedTotalFrames_ > 0) {
                    progress = static_cast<float>(frameIndex_) / static_cast<float>(expectedTotalFrames_);
                }
                progressCallback_(progress, frameIndex_, expectedTotalFrames_);
            }

            return true;
        }
    }

    bool addFrame(const ofPixels& pixels) {
        @autoreleasepool {
            if (status_ != ExportStatus::Encoding) {
                lastError_ = "Not in encoding state";
                hasError_ = true;
                return false;
            }

            // Wait for writer to be ready
            while (!writerInput_.readyForMoreMediaData) {
                [NSThread sleepForTimeInterval:0.01];
            }

            // Get pixel dimensions
            size_t width = pixels.getWidth();
            size_t height = pixels.getHeight();

            // Create pixel buffer
            CVPixelBufferRef pixelBuffer = nullptr;
            CVReturn status = CVPixelBufferPoolCreatePixelBuffer(nullptr,
                                                                 pixelBufferAdaptor_.pixelBufferPool,
                                                                 &pixelBuffer);
            if (status != kCVReturnSuccess) {
                lastError_ = "Failed to create pixel buffer";
                hasError_ = true;
                return false;
            }

            // Lock pixel buffer
            CVPixelBufferLockBaseAddress(pixelBuffer, 0);
            void* baseAddress = CVPixelBufferGetBaseAddress(pixelBuffer);
            size_t bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer);

            // Copy pixels (convert RGBA to BGRA if needed)
            const unsigned char* srcData = pixels.getData();
            unsigned char* dstData = static_cast<unsigned char*>(baseAddress);

            size_t numChannels = pixels.getNumChannels();

            for (size_t y = 0; y < height; y++) {
                for (size_t x = 0; x < width; x++) {
                    size_t srcIdx = (y * width + x) * numChannels;
                    size_t dstIdx = y * bytesPerRow + x * 4;

                    if (numChannels >= 3) {
                        // Convert RGBA to BGRA
                        dstData[dstIdx + 0] = srcData[srcIdx + 2]; // B
                        dstData[dstIdx + 1] = srcData[srcIdx + 1]; // G
                        dstData[dstIdx + 2] = srcData[srcIdx + 0]; // R
                        dstData[dstIdx + 3] = (numChannels == 4) ? srcData[srcIdx + 3] : 255; // A
                    } else {
                        // Grayscale
                        dstData[dstIdx + 0] = srcData[srcIdx];
                        dstData[dstIdx + 1] = srcData[srcIdx];
                        dstData[dstIdx + 2] = srcData[srcIdx];
                        dstData[dstIdx + 3] = 255;
                    }
                }
            }

            CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);

            // Calculate presentation time
            CMTime presentationTime = CMTimeMake(frameIndex_, settings_.framerate);

            // Append pixel buffer
            BOOL success = [pixelBufferAdaptor_ appendPixelBuffer:pixelBuffer
                                             withPresentationTime:presentationTime];

            CVPixelBufferRelease(pixelBuffer);

            if (!success) {
                lastError_ = "Failed to append pixel buffer";
                hasError_ = true;
                return false;
            }

            frameIndex_++;

            // Update statistics
            updateStatistics();

            // Call progress callback
            if (progressCallback_) {
                float progress = 0.0f;
                if (expectedTotalFrames_ > 0) {
                    progress = static_cast<float>(frameIndex_) / static_cast<float>(expectedTotalFrames_);
                }
                progressCallback_(progress, frameIndex_, expectedTotalFrames_);
            }

            return true;
        }
    }

    bool endExport() {
        @autoreleasepool {
            if (status_ != ExportStatus::Encoding) {
                lastError_ = "Not in encoding state";
                hasError_ = true;
                return false;
            }

            status_ = ExportStatus::Finalizing;

            // Finish writing
            [writerInput_ markAsFinished];

            // Finalize asset writer (this may take time)
            __block BOOL success = NO;
            __block NSError* error = nil;

            dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);

            [assetWriter_ finishWritingWithCompletionHandler:^{
                success = (assetWriter_.status == AVAssetWriterStatusCompleted);
                error = assetWriter_.error;
                dispatch_semaphore_signal(semaphore);
            }];

            dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);

            if (!success) {
                lastError_ = error ? std::string([[error localizedDescription] UTF8String]) : "Unknown error";
                hasError_ = true;
                status_ = ExportStatus::Error;
                cleanup();
                return false;
            }

            // Calculate final statistics
            auto endTime = std::chrono::steady_clock::now();
            float elapsedSeconds = std::chrono::duration<float>(endTime - startTime_).count();

            // Get file size
            NSString* path = [NSString stringWithUTF8String:outputPath_.c_str()];
            NSDictionary* attrs = [[NSFileManager defaultManager] attributesOfItemAtPath:path error:nil];
            estimatedFileSize_ = [attrs fileSize];

            status_ = ExportStatus::Completed;

            os_log_info(OS_LOG_DEFAULT, "VideoExporter: Export completed - %zu frames in %.2f seconds (%.1f fps, %zu MB)",
                       frameIndex_, elapsedSeconds, frameIndex_ / elapsedSeconds, estimatedFileSize_ / (1024 * 1024));

            cleanup();
            return true;
        }
    }

    void cancelExport() {
        @autoreleasepool {
            if (status_ != ExportStatus::Encoding && status_ != ExportStatus::Finalizing) {
                return;
            }

            [assetWriter_ cancelWriting];
            status_ = ExportStatus::Cancelled;

            // Delete incomplete file
            NSString* path = [NSString stringWithUTF8String:outputPath_.c_str()];
            [[NSFileManager defaultManager] removeItemAtPath:path error:nil];

            os_log_info(OS_LOG_DEFAULT, "VideoExporter: Export cancelled");

            cleanup();
        }
    }

    // ========================================================================
    // Convenience Methods
    // ========================================================================

    bool exportScene(const SharpScene& scene,
                    const CameraPath& cameraPath,
                    const std::string& outputPath,
                    const VideoExportSettings& settings) {
        // This is a high-level convenience method
        // In a real implementation, this would:
        // 1. Setup renderer and FBO
        // 2. Iterate through camera path
        // 3. Render each frame
        // 4. Add frame to exporter
        // For now, return error as it requires full renderer integration
        lastError_ = "exportScene not yet implemented - use manual frame export";
        hasError_ = true;
        return false;
    }

    bool exportCloud(const GaussianCloud& cloud,
                    const CameraPath& cameraPath,
                    const std::string& outputPath,
                    const VideoExportSettings& settings) {
        // Similar to exportScene - requires renderer integration
        lastError_ = "exportCloud not yet implemented - use manual frame export";
        hasError_ = true;
        return false;
    }

    // ========================================================================
    // Helper Methods
    // ========================================================================

    void configureCodec(NSMutableDictionary* videoSettings, size_t width, size_t height) {
        switch (settings_.codec) {
            case VideoCodec::H264: {
                videoSettings[AVVideoCodecKey] = AVVideoCodecTypeH264;

                NSMutableDictionary* compressionProps = [NSMutableDictionary dictionary];
                compressionProps[AVVideoAverageBitRateKey] = @(getBitrateForQuality(width, height) * 1000000);
                compressionProps[AVVideoProfileLevelKey] = AVVideoProfileLevelH264HighAutoLevel;
                compressionProps[AVVideoExpectedSourceFrameRateKey] = @(settings_.framerate);

                if (settings_.useHardwareAcceleration) {
                    compressionProps[AVVideoEncoderSpecificationKey] = @{
                        (__bridge NSString*)kVTVideoEncoderSpecification_EnableHardwareAcceleratedVideoEncoder: @YES
                    };
                }

                videoSettings[AVVideoCompressionPropertiesKey] = compressionProps;
                break;
            }

            case VideoCodec::H265: {
                videoSettings[AVVideoCodecKey] = AVVideoCodecTypeHEVC;

                NSMutableDictionary* compressionProps = [NSMutableDictionary dictionary];
                compressionProps[AVVideoAverageBitRateKey] = @(getBitrateForQuality(width, height) * 1000000);
                compressionProps[AVVideoExpectedSourceFrameRateKey] = @(settings_.framerate);

                if (settings_.useHardwareAcceleration) {
                    compressionProps[AVVideoEncoderSpecificationKey] = @{
                        (__bridge NSString*)kVTVideoEncoderSpecification_EnableHardwareAcceleratedVideoEncoder: @YES
                    };
                }

                videoSettings[AVVideoCompressionPropertiesKey] = compressionProps;
                break;
            }

            case VideoCodec::ProRes422: {
                videoSettings[AVVideoCodecKey] = AVVideoCodecTypeAppleProRes422;
                break;
            }

            case VideoCodec::ProRes4444: {
                videoSettings[AVVideoCodecKey] = AVVideoCodecTypeAppleProRes4444;
                break;
            }
        }
    }

    float getBitrateForQuality(size_t width, size_t height) {
        if (settings_.customBitrate > 0.0f) {
            return settings_.customBitrate;
        }

        // Calculate bitrate based on resolution and quality
        float pixelCount = static_cast<float>(width * height);
        float bitsPerPixel = 0.0f;

        switch (settings_.quality) {
            case VideoQuality::Low:      bitsPerPixel = 0.1f; break;
            case VideoQuality::Medium:   bitsPerPixel = 0.2f; break;
            case VideoQuality::High:     bitsPerPixel = 0.3f; break;
            case VideoQuality::Lossless: bitsPerPixel = 1.0f; break;
        }

        // Mbps = (pixels * bits_per_pixel * framerate) / 1,000,000
        float bitrate = (pixelCount * bitsPerPixel * settings_.framerate) / 1000000.0f;

        return bitrate;
    }

    void updateStatistics() {
        auto currentTime = std::chrono::steady_clock::now();
        float elapsedSeconds = std::chrono::duration<float>(currentTime - startTime_).count();

        if (elapsedSeconds > 0.0f) {
            encodingSpeed_ = static_cast<float>(frameIndex_) / elapsedSeconds;
        }
    }

    void cleanup() {
        @autoreleasepool {
            pixelBufferAdaptor_ = nil;
            writerInput_ = nil;
            assetWriter_ = nil;
        }
    }
};

// ============================================================================
// VideoExporter Public Interface
// ============================================================================

VideoExporter::VideoExporter()
    : impl_(std::make_unique<Impl>())
{}

VideoExporter::~VideoExporter() = default;

VideoExporter::VideoExporter(VideoExporter&& other) noexcept = default;
VideoExporter& VideoExporter::operator=(VideoExporter&& other) noexcept = default;

// Setup / Configuration
bool VideoExporter::setup(const VideoExportSettings& settings) {
    return impl_->setup(settings);
}

VideoExportSettings VideoExporter::getSettings() const {
    return impl_->settings_;
}

bool VideoExporter::isHardwareAccelerationAvailable() const {
    // Hardware acceleration is available on all modern Macs
    return true;
}

VideoResolution VideoExporter::getMaxSupportedResolution() const {
    // Apple Silicon supports up to 8K
    return VideoResolution::UHD_8K;
}

// Export Control
bool VideoExporter::beginExport(const std::string& outputPath) {
    return impl_->beginExport(outputPath);
}

bool VideoExporter::addFrame(const ofTexture& texture) {
    return impl_->addFrame(texture);
}

bool VideoExporter::addFrame(const ofPixels& pixels) {
    return impl_->addFrame(pixels);
}

bool VideoExporter::endExport() {
    return impl_->endExport();
}

void VideoExporter::cancelExport() {
    impl_->cancelExport();
}

bool VideoExporter::isExporting() const {
    return impl_->status_ == ExportStatus::Encoding ||
           impl_->status_ == ExportStatus::Finalizing;
}

ExportStatus VideoExporter::getStatus() const {
    return impl_->status_;
}

// Progress Monitoring
void VideoExporter::setProgressCallback(ProgressCallback callback) {
    impl_->progressCallback_ = callback;
}

float VideoExporter::getProgress() const {
    if (impl_->expectedTotalFrames_ == 0) {
        return 0.0f;
    }
    return static_cast<float>(impl_->frameIndex_) / static_cast<float>(impl_->expectedTotalFrames_);
}

size_t VideoExporter::getEncodedFrameCount() const {
    return impl_->frameIndex_;
}

void VideoExporter::setExpectedTotalFrames(size_t totalFrames) {
    impl_->expectedTotalFrames_ = totalFrames;
}

// Convenience Methods
bool VideoExporter::exportScene(const SharpScene& scene,
                               const CameraPath& cameraPath,
                               const std::string& outputPath,
                               const VideoExportSettings& settings) {
    return impl_->exportScene(scene, cameraPath, outputPath, settings);
}

bool VideoExporter::exportCloud(const GaussianCloud& cloud,
                               const CameraPath& cameraPath,
                               const std::string& outputPath,
                               const VideoExportSettings& settings) {
    return impl_->exportCloud(cloud, cameraPath, outputPath, settings);
}

// Error Handling
std::string VideoExporter::getLastError() const {
    return impl_->lastError_;
}

bool VideoExporter::hasError() const {
    return impl_->hasError_;
}

// Statistics
size_t VideoExporter::getEstimatedFileSize() const {
    return impl_->estimatedFileSize_;
}

float VideoExporter::getEncodingSpeed() const {
    return impl_->encodingSpeed_;
}

float VideoExporter::getEstimatedTimeRemaining() const {
    if (impl_->expectedTotalFrames_ == 0 || impl_->encodingSpeed_ <= 0.0f) {
        return 0.0f;
    }

    size_t framesRemaining = impl_->expectedTotalFrames_ - impl_->frameIndex_;
    return static_cast<float>(framesRemaining) / impl_->encodingSpeed_;
}

float VideoExporter::getElapsedTime() const {
    if (impl_->status_ == ExportStatus::Idle) {
        return 0.0f;
    }

    auto currentTime = std::chrono::steady_clock::now();
    return std::chrono::duration<float>(currentTime - impl_->startTime_).count();
}

// Utility
void VideoExporter::getResolutionDimensions(VideoResolution resolution,
                                           size_t& outWidth,
                                           size_t& outHeight) {
    switch (resolution) {
        case VideoResolution::HD_720p:
            outWidth = 1280;
            outHeight = 720;
            break;
        case VideoResolution::HD_1080p:
            outWidth = 1920;
            outHeight = 1080;
            break;
        case VideoResolution::QHD_1440p:
            outWidth = 2560;
            outHeight = 1440;
            break;
        case VideoResolution::UHD_4K:
            outWidth = 3840;
            outHeight = 2160;
            break;
        case VideoResolution::UHD_5K:
            outWidth = 5120;
            outHeight = 2880;
            break;
        case VideoResolution::UHD_8K:
            outWidth = 7680;
            outHeight = 4320;
            break;
        case VideoResolution::Custom:
            outWidth = 1920;
            outHeight = 1080;
            break;
    }
}

std::string VideoExporter::getCodecName(VideoCodec codec) {
    switch (codec) {
        case VideoCodec::H264:       return "H.264/AVC";
        case VideoCodec::H265:       return "H.265/HEVC";
        case VideoCodec::ProRes422:  return "Apple ProRes 422";
        case VideoCodec::ProRes4444: return "Apple ProRes 4444";
        default:                     return "Unknown";
    }
}

std::string VideoExporter::getFileExtension(VideoCodec codec) {
    switch (codec) {
        case VideoCodec::H264:
        case VideoCodec::H265:
            return ".mp4";
        case VideoCodec::ProRes422:
        case VideoCodec::ProRes4444:
            return ".mov";
        default:
            return ".mov";
    }
}

float VideoExporter::getRecommendedBitrate(VideoResolution resolution,
                                          VideoQuality quality,
                                          VideoCodec codec) {
    size_t width, height;
    getResolutionDimensions(resolution, width, height);

    float pixelCount = static_cast<float>(width * height);
    float bitsPerPixel = 0.0f;

    // Base bits per pixel for H.265
    switch (quality) {
        case VideoQuality::Low:      bitsPerPixel = 0.05f; break;
        case VideoQuality::Medium:   bitsPerPixel = 0.1f;  break;
        case VideoQuality::High:     bitsPerPixel = 0.2f;  break;
        case VideoQuality::Lossless: bitsPerPixel = 0.5f;  break;
    }

    // Adjust for codec
    if (codec == VideoCodec::H264) {
        bitsPerPixel *= 1.5f; // H.264 needs more bits than H.265
    } else if (codec == VideoCodec::ProRes422 || codec == VideoCodec::ProRes4444) {
        bitsPerPixel *= 10.0f; // ProRes is much higher bitrate
    }

    // Calculate Mbps at 30fps
    float bitrate = (pixelCount * bitsPerPixel * 30.0f) / 1000000.0f;

    return bitrate;
}

} // namespace Sharp
