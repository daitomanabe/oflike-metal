# Sharp::VideoExporter - Video Export Guide

## Overview

The `Sharp::VideoExporter` class provides high-quality video export of animated Gaussian Splatting scenes using Apple's VideoToolbox framework with hardware acceleration.

## Features

- **Hardware Accelerated**: Uses Apple Silicon Media Engine for encoding
- **Multiple Codecs**:
  - H.264/AVC - Standard quality, wide compatibility
  - H.265/HEVC - Better compression, modern (recommended)
  - Apple ProRes 422 - Professional quality
  - Apple ProRes 4444 - Professional quality with alpha channel
- **Resolution Presets**: 720p, 1080p, 1440p, 4K, 5K, 8K, or custom
- **Quality Control**: Low, Medium, High, or Lossless (ProRes only)
- **Progress Callbacks**: Real-time progress monitoring for UI feedback
- **HDR Support**: HDR output with H.265 codec
- **Zero-Copy**: Direct Metal texture export (no CPU readback overhead)

## Basic Usage

### 1. Simple Frame-by-Frame Export

```cpp
#include "SharpVideoExporter.h"
#include "ofFbo.h"

// Setup
Sharp::VideoExporter exporter;
Sharp::VideoExportSettings settings;
settings.codec = Sharp::VideoCodec::H265;
settings.resolution = Sharp::VideoResolution::UHD_4K;
settings.framerate = 60;
settings.quality = Sharp::VideoQuality::High;

exporter.setup(settings);

// Set progress callback
exporter.setProgressCallback([](float progress, size_t frame, size_t total) {
    std::cout << "Progress: " << (progress * 100.0f) << "% (Frame "
              << frame << "/" << total << ")" << std::endl;
});

// Export frames
exporter.setExpectedTotalFrames(300); // Optional: for accurate progress
exporter.beginExport("output.mov");

for (int i = 0; i < 300; i++) {
    // Render your scene to an FBO
    fbo.begin();
    // ... render Gaussian Splatting scene here ...
    fbo.end();

    // Add frame to video
    exporter.addFrame(fbo.getTexture());
}

exporter.endExport();
```

### 2. Export with Camera Animation

```cpp
#include "SharpVideoExporter.h"
#include "SharpCameraPath.h"
#include "SharpGaussianCloud.h"
#include "SharpRenderer.h"

// Load Gaussian cloud
Sharp::GaussianCloud cloud;
cloud.loadPLY("model.ply");

// Setup camera path
Sharp::CameraPath cameraPath;
cameraPath.setOrbitPath(
    ofVec3f(0, 0, 0),  // center
    5.0f,               // radius
    10.0f               // duration (seconds)
);

// Setup renderer and exporter
Sharp::GaussianRenderer renderer;
renderer.setup(3840, 2160); // 4K

Sharp::VideoExporter exporter;
Sharp::VideoExportSettings settings;
settings.codec = Sharp::VideoCodec::H265;
settings.resolution = Sharp::VideoResolution::UHD_4K;
settings.framerate = 60;

exporter.setup(settings);

// Calculate total frames
float duration = cameraPath.getDuration();
int totalFrames = duration * settings.framerate;
exporter.setExpectedTotalFrames(totalFrames);

// Export
exporter.beginExport("orbit.mov");

ofCamera camera;
cameraPath.play();

for (int i = 0; i < totalFrames; i++) {
    float deltaTime = 1.0f / settings.framerate;
    cameraPath.update(deltaTime);
    cameraPath.applyToCamera(camera);

    // Render frame
    ofFbo fbo;
    fbo.allocate(3840, 2160);
    fbo.begin();
    ofClear(0);
    camera.begin();
    renderer.draw(cloud);
    camera.end();
    fbo.end();

    exporter.addFrame(fbo.getTexture());
}

exporter.endExport();
```

## Configuration Options

### Codec Selection

```cpp
settings.codec = Sharp::VideoCodec::H264;       // Standard H.264
settings.codec = Sharp::VideoCodec::H265;       // H.265 (recommended)
settings.codec = Sharp::VideoCodec::ProRes422;  // ProRes 422
settings.codec = Sharp::VideoCodec::ProRes4444; // ProRes 4444 (with alpha)
```

### Resolution Presets

```cpp
settings.resolution = Sharp::VideoResolution::HD_720p;   // 1280x720
settings.resolution = Sharp::VideoResolution::HD_1080p;  // 1920x1080
settings.resolution = Sharp::VideoResolution::QHD_1440p; // 2560x1440
settings.resolution = Sharp::VideoResolution::UHD_4K;    // 3840x2160
settings.resolution = Sharp::VideoResolution::UHD_5K;    // 5120x2880
settings.resolution = Sharp::VideoResolution::UHD_8K;    // 7680x4320

// Custom resolution
settings.resolution = Sharp::VideoResolution::Custom;
settings.customWidth = 2048;
settings.customHeight = 1024;
```

### Quality Settings

```cpp
settings.quality = Sharp::VideoQuality::Low;      // Smaller file
settings.quality = Sharp::VideoQuality::Medium;   // Balanced
settings.quality = Sharp::VideoQuality::High;     // Larger file (recommended)
settings.quality = Sharp::VideoQuality::Lossless; // ProRes only
```

### Frame Rate

```cpp
settings.framerate = 24;  // Cinematic
settings.framerate = 30;  // Standard
settings.framerate = 60;  // Smooth (recommended for 3DGS)
settings.framerate = 120; // High frame rate
```

### Advanced Settings

```cpp
// Custom bitrate (Mbps, 0 = auto)
settings.customBitrate = 50.0f;

// Hardware acceleration (always enabled on Apple Silicon)
settings.useHardwareAcceleration = true;

// Alpha channel export (ProRes4444 only)
settings.exportAlpha = true;

// HDR output (H.265 only)
settings.useHDR = true;
```

## Progress Monitoring

### Progress Callback

```cpp
exporter.setProgressCallback([](float progress, size_t frame, size_t total) {
    // Update UI
    progressBar.setValue(progress);
    statusLabel.setText(
        "Encoding frame " + std::to_string(frame) + " of " + std::to_string(total)
    );
});
```

### Statistics

```cpp
// During export
float progress = exporter.getProgress();           // 0.0 - 1.0
size_t framesEncoded = exporter.getEncodedFrameCount();
float speed = exporter.getEncodingSpeed();         // fps
float timeRemaining = exporter.getEstimatedTimeRemaining(); // seconds
float elapsedTime = exporter.getElapsedTime();     // seconds
size_t fileSize = exporter.getEstimatedFileSize(); // bytes
```

### Status Monitoring

```cpp
Sharp::ExportStatus status = exporter.getStatus();

switch (status) {
    case Sharp::ExportStatus::Idle:
        // Ready to export
        break;
    case Sharp::ExportStatus::Preparing:
        // Setting up encoder
        break;
    case Sharp::ExportStatus::Encoding:
        // Encoding frames
        break;
    case Sharp::ExportStatus::Finalizing:
        // Finalizing video file
        break;
    case Sharp::ExportStatus::Completed:
        // Export successful
        break;
    case Sharp::ExportStatus::Cancelled:
        // Export cancelled
        break;
    case Sharp::ExportStatus::Error:
        // Error occurred
        std::cerr << "Export error: " << exporter.getLastError() << std::endl;
        break;
}
```

## Error Handling

```cpp
exporter.setup(settings);

if (!exporter.beginExport("output.mov")) {
    std::cerr << "Failed to start export: " << exporter.getLastError() << std::endl;
    return;
}

for (int i = 0; i < totalFrames; i++) {
    if (!exporter.addFrame(texture)) {
        std::cerr << "Failed to add frame: " << exporter.getLastError() << std::endl;
        exporter.cancelExport();
        return;
    }
}

if (!exporter.endExport()) {
    std::cerr << "Failed to finalize export: " << exporter.getLastError() << std::endl;
    return;
}

std::cout << "Export completed successfully!" << std::endl;
```

## Cancellation

```cpp
// User clicked cancel button
exporter.cancelExport();

// This will:
// 1. Abort encoding
// 2. Delete incomplete file
// 3. Set status to Cancelled
```

## Utility Functions

### Get Resolution Dimensions

```cpp
size_t width, height;
Sharp::VideoExporter::getResolutionDimensions(
    Sharp::VideoResolution::UHD_4K,
    width,
    height
);
// width = 3840, height = 2160
```

### Get Codec Name

```cpp
std::string name = Sharp::VideoExporter::getCodecName(
    Sharp::VideoCodec::H265
);
// name = "H.265/HEVC"
```

### Get File Extension

```cpp
std::string ext = Sharp::VideoExporter::getFileExtension(
    Sharp::VideoCodec::H265
);
// ext = ".mp4"
```

### Get Recommended Bitrate

```cpp
float bitrate = Sharp::VideoExporter::getRecommendedBitrate(
    Sharp::VideoResolution::UHD_4K,
    Sharp::VideoQuality::High,
    Sharp::VideoCodec::H265
);
// bitrate in Mbps
```

## Performance Tips

1. **Use H.265**: Better compression than H.264, hardware accelerated
2. **Hardware Acceleration**: Always enabled on Apple Silicon (automatic)
3. **Zero-Copy Export**: Pass `ofTexture` directly instead of `ofPixels` when possible
4. **Batch Rendering**: Render frames in advance if memory allows
5. **Expected Total Frames**: Set this for accurate progress reporting
6. **Quality vs Size**: High quality is recommended for 3DGS content

## Codec Comparison

| Codec | Quality | File Size | Speed | Compatibility | Alpha |
|-------|---------|-----------|-------|---------------|-------|
| H.264 | Good | Medium | Fast | Excellent | No |
| H.265 | Excellent | Small | Fast | Good | No |
| ProRes 422 | Excellent | Large | Very Fast | Professional | No |
| ProRes 4444 | Excellent | Very Large | Very Fast | Professional | Yes |

## Example: Export Settings Presets

```cpp
// Web/Social Media (YouTube, Twitter)
Sharp::VideoExportSettings webSettings;
webSettings.codec = Sharp::VideoCodec::H265;
webSettings.resolution = Sharp::VideoResolution::HD_1080p;
webSettings.framerate = 60;
webSettings.quality = Sharp::VideoQuality::High;

// Professional Workflow (DaVinci Resolve, Final Cut Pro)
Sharp::VideoExportSettings proSettings;
proSettings.codec = Sharp::VideoCodec::ProRes422;
proSettings.resolution = Sharp::VideoResolution::UHD_4K;
proSettings.framerate = 60;
proSettings.quality = Sharp::VideoQuality::High;

// Archive/Master
Sharp::VideoExportSettings archiveSettings;
archiveSettings.codec = Sharp::VideoCodec::ProRes4444;
archiveSettings.resolution = Sharp::VideoResolution::UHD_8K;
archiveSettings.framerate = 60;
archiveSettings.quality = Sharp::VideoQuality::Lossless;
archiveSettings.exportAlpha = true;
```

## Thread Safety

- `setup()`, `beginExport()`, `addFrame()`, `endExport()` are **not thread-safe**
- Call all methods from the same thread (typically main/render thread)
- Progress callback is called on encoding thread - use appropriate synchronization

## Requirements

- macOS 13.0 (Ventura) or later
- Apple Silicon or Intel Mac with VideoToolbox support
- Metal-compatible GPU

## Known Limitations

1. `exportScene()` and `exportCloud()` convenience methods not yet implemented
2. Maximum frame size limited by Metal texture size (16384x16384)
3. HDR output only supported with H.265 codec
4. Alpha channel only supported with ProRes 4444 codec

## See Also

- `SharpCameraPath` - Camera animation
- `SharpRenderer` - Gaussian Splatting renderer
- `SharpScene` - Scene management
- `ofFbo` - Offscreen rendering target
