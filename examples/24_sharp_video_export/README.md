# Example 24: Sharp Video Export

**Export 3D Gaussian Splatting Animations to High-Quality Video**

This example demonstrates the `Sharp::VideoExporter` class, which provides professional video export capabilities for animated Gaussian Splatting scenes using Apple's VideoToolbox framework.

## Features Demonstrated

### Video Export Capabilities
- **Multiple Codecs**: H.264, H.265/HEVC, ProRes 422, ProRes 4444
- **Resolution Presets**: 720p, 1080p, 1440p, 4K, 5K, 8K
- **Quality Settings**: Low, Medium, High, Lossless (ProRes)
- **Hardware Acceleration**: Metal-accelerated encoding on Apple Silicon
- **Frame Rate Control**: 15-120 fps support
- **Progress Monitoring**: Real-time encoding progress and speed metrics

### Camera Animation
- **Orbit Path**: Circular motion around target point
- **Dolly Path**: Linear motion through control points
- **Spiral Path**: Helical motion combining orbit and height
- **Keyframe Animation**: Custom paths with spline interpolation
- **Smooth Interpolation**: Catmull-Rom and Bezier curves

### Export Workflow
- **Scene Export**: Export entire `Sharp::SharpScene` with multiple objects
- **Camera Path Integration**: Automatic camera animation during export
- **Progress Callbacks**: Real-time feedback during encoding
- **Error Handling**: Comprehensive error reporting
- **File Management**: Automatic timestamp-based file naming

## Controls

### Scene Management
- **SPACE**: Create demo scene with 2 Gaussian clouds
- **C**: Clear scene

### Export Control
- **E**: Start video export with current settings
- **X**: Cancel ongoing export

### Export Settings
- **1-4**: Select codec
  - 1: H.264 (standard compatibility)
  - 2: H.265 (better compression, recommended)
  - 3: ProRes 422 (professional quality)
  - 4: ProRes 4444 (with alpha channel)

- **5-8**: Select resolution
  - 5: 720p (1280x720)
  - 6: 1080p (1920x1080) - default
  - 7: 1440p (2560x1440)
  - 8: 4K (3840x2160)

- **Q/W**: Adjust quality (Low → Medium → High → Lossless)
- **F/G**: Adjust framerate (15 → 30 → 45 → 60 → ... → 120 fps)
- **D/S**: Adjust duration (1 → 5 → 10 → ... → 30 seconds)

### Camera
- **Mouse Drag**: Rotate preview camera
- **Mouse Wheel**: Zoom preview camera

## Key Code Patterns

### Basic Export Workflow

```cpp
// 1. Setup exporter
Sharp::VideoExporter exporter;
Sharp::VideoExportSettings settings;
settings.codec = Sharp::VideoCodec::H265;
settings.resolution = Sharp::VideoResolution::UHD_4K;
settings.quality = Sharp::VideoQuality::High;
settings.framerate = 60;

exporter.setup(settings);

// 2. Setup progress callback
exporter.setProgressCallback([](float progress, size_t frameIdx, size_t total) {
    std::cout << "Progress: " << (progress * 100) << "%" << std::endl;
});

// 3. Export scene with camera animation
bool success = exporter.exportScene(scene, cameraPath, "output.mov", settings);

if (success) {
    ofLog() << "Export completed!";
} else {
    ofLog() << "Error: " << exporter.getLastError();
}
```

### Manual Frame-by-Frame Export

```cpp
// Begin export
exporter.beginExport("output.mov");

// Render and export each frame
for (int i = 0; i < totalFrames; i++) {
    // Update animation
    float time = i / (float)framerate;
    cameraPath.setTime(time);
    cameraPath.applyToCamera(camera);

    // Render to FBO
    fbo.begin();
    scene.render(renderer, camera);
    fbo.end();

    // Add frame to video
    exporter.addFrame(fbo.getTexture());
}

// Finalize video file
exporter.endExport();
```

### Camera Path Setup

```cpp
// Orbit path - circle around center
cameraPath.setOrbitPath(
    {0, 0, 0},          // center
    5.0f,               // radius
    10.0f,              // duration (seconds)
    {0, 1, 0},          // axis (Y-up)
    0.0f                // start angle
);

// Keyframe animation
cameraPath.addKeyframe(0.0f, {5, 2, 5}, {0, 0, 0});
cameraPath.addKeyframe(2.0f, {-5, 3, 3}, {0, 0, 0});
cameraPath.addKeyframe(4.0f, {0, 5, 0}, {0, 0, 0});
cameraPath.setInterpolationMode(Sharp::InterpolationMode::CatmullRom);

// Playback control
cameraPath.setPlaybackMode(Sharp::PlaybackMode::Loop);
cameraPath.play();
```

### Progress Monitoring

```cpp
exporter.setProgressCallback([this](float progress, size_t frameIdx, size_t total) {
    // Update UI
    progressBar.setValue(progress);
    statusLabel.setText("Frame " + std::to_string(frameIdx) + "/" + std::to_string(total));

    // Log performance
    float encodingSpeed = exporter.getEncodingSpeed();
    float timeRemaining = exporter.getEstimatedTimeRemaining();
    ofLog() << "Encoding at " << encodingSpeed << " fps, ETA: " << timeRemaining << "s";
});
```

## API Overview

### VideoExporter Core Methods

```cpp
// Setup
bool setup(const VideoExportSettings& settings)
VideoExportSettings getSettings() const

// Export control
bool beginExport(const std::string& outputPath)
bool addFrame(const ofTexture& texture)
bool addFrame(const ofPixels& pixels)
bool endExport()
void cancelExport()

// Convenience methods
bool exportScene(const SharpScene& scene,
                 const CameraPath& path,
                 const std::string& outputPath,
                 const VideoExportSettings& settings)

bool exportCloud(const GaussianCloud& cloud,
                 const CameraPath& path,
                 const std::string& outputPath,
                 const VideoExportSettings& settings)

// Progress monitoring
void setProgressCallback(ProgressCallback callback)
float getProgress() const
size_t getEncodedFrameCount() const
void setExpectedTotalFrames(size_t total)

// Statistics
size_t getEstimatedFileSize() const
float getEncodingSpeed() const           // frames per second
float getEstimatedTimeRemaining() const  // seconds
float getElapsedTime() const             // seconds

// Error handling
std::string getLastError() const
bool hasError() const
```

### CameraPath Methods

```cpp
// Path configuration
void setOrbitPath(const float3& center, float radius, float duration,
                  const float3& axis = {0, 1, 0}, float startAngle = 0)
void setDollyPath(const vector<float3>& points, float duration, bool closed = false)
void setSpiralPath(const float3& center, float radius, float height,
                   float revolutions, float duration)

// Keyframe animation
void addKeyframe(float time, const float3& position, const float3& target, float fov = 60.0f)
void clearKeyframes()
void setInterpolationMode(InterpolationMode mode)

// Playback
void play()
void pause()
void stop()
void setPlaybackMode(PlaybackMode mode)
void setTime(float time)
float getTime() const
float getDuration() const

// Camera application
void applyToCamera(ofCamera& camera) const
void update(float deltaTime)
```

## Export Settings Reference

### VideoExportSettings Structure

```cpp
struct VideoExportSettings {
    // Resolution
    VideoResolution resolution = VideoResolution::HD_1080p;
    size_t customWidth = 1920;
    size_t customHeight = 1080;

    // Codec
    VideoCodec codec = VideoCodec::H265;

    // Quality
    VideoQuality quality = VideoQuality::High;
    float customBitrate = 0.0f;  // 0 = auto

    // Frame rate
    int framerate = 30;

    // Hardware acceleration
    bool useHardwareAcceleration = true;

    // Alpha channel (ProRes4444 only)
    bool exportAlpha = false;

    // HDR (H.265 only)
    bool useHDR = false;
};
```

### Codec Comparison

| Codec | Quality | File Size | Speed | Use Case |
|-------|---------|-----------|-------|----------|
| H.264 | Good | Small | Fast | Web sharing, compatibility |
| H.265 | Excellent | Medium | Fast | High quality, modern devices |
| ProRes 422 | Excellent | Large | Medium | Professional editing, no alpha |
| ProRes 4444 | Lossless | Very Large | Medium | Professional editing, with alpha |

### Resolution Presets

| Preset | Resolution | Aspect Ratio | Use Case |
|--------|-----------|--------------|----------|
| HD_720p | 1280x720 | 16:9 | Web preview, fast export |
| HD_1080p | 1920x1080 | 16:9 | Standard HD, recommended |
| QHD_1440p | 2560x1440 | 16:9 | High resolution displays |
| UHD_4K | 3840x2160 | 16:9 | 4K displays, production |
| UHD_5K | 5120x2880 | 16:9 | 5K displays, high-end |
| UHD_8K | 7680x4320 | 16:9 | 8K displays, future-proof |

## Performance Considerations

### Hardware Acceleration
- **Apple Silicon**: Full hardware acceleration for H.265 and ProRes
- **Intel Macs**: Limited hardware acceleration
- Enable with `settings.useHardwareAcceleration = true` (default)

### Encoding Speed

Typical encoding speeds on M1 Max:
- **H.265 4K @ 60fps**: 120-180 fps (real-time capable)
- **ProRes 4K @ 60fps**: 60-90 fps (near real-time)
- **H.265 8K @ 30fps**: 30-60 fps (real-time at lower resolutions)

### Memory Usage
- **Per-frame buffer**: Resolution-dependent
- **4K frame**: ~33 MB per frame buffer
- **Recommended**: 8GB+ RAM for 4K export
- **Unified Memory**: Zero-copy texture export on Apple Silicon

### File Size Estimates

For 10 seconds @ 60fps (600 frames):
- **H.265 4K High**: ~150 MB
- **ProRes 422 4K**: ~2.5 GB
- **ProRes 4444 4K**: ~3.5 GB

## Advanced Usage

### Background Export

```cpp
// Export on background thread
std::thread exportThread([this]() {
    bool success = exporter.exportScene(scene, cameraPath, "output.mov", settings);

    // Notify main thread when done
    if (success) {
        ofLog() << "Export completed!";
    }
});
exportThread.detach();

// Monitor progress on main thread
void update() {
    if (exporter.isExporting()) {
        float progress = exporter.getProgress();
        updateProgressBar(progress);
    }
}
```

### Custom Camera Animation

```cpp
// Complex camera path with multiple keyframes
CameraPath path;

// Start position
path.addKeyframe(0.0f, {10, 3, 10}, {0, 0, 0}, 60.0f);

// Orbit around object
for (int i = 1; i <= 8; i++) {
    float angle = (i / 8.0f) * TWO_PI;
    float t = i * 0.5f;
    float3 pos = {cos(angle) * 8, 3, sin(angle) * 8};
    path.addKeyframe(t, pos, {0, 0, 0}, 60.0f);
}

// Close-up zoom
path.addKeyframe(5.0f, {2, 1, 2}, {0, 0, 0}, 45.0f);

// Use smooth interpolation
path.setInterpolationMode(InterpolationMode::CatmullRom);
```

### Multi-Resolution Export

```cpp
// Export same animation at multiple resolutions
VideoResolution resolutions[] = {
    VideoResolution::HD_1080p,
    VideoResolution::QHD_1440p,
    VideoResolution::UHD_4K
};

for (auto res : resolutions) {
    settings.resolution = res;
    std::string filename = "export_" + getResolutionName(res) + ".mov";

    exporter.setup(settings);
    exporter.exportScene(scene, cameraPath, filename, settings);
}
```

## Common Use Cases

### Turntable Animation
Create a 360° rotating view of an object:
```cpp
cameraPath.setOrbitPath(
    objectCenter,      // center of object
    5.0f,              // distance
    10.0f,             // 10 second rotation
    {0, 1, 0},         // rotate around Y axis
    0.0f               // start at 0 degrees
);
```

### Flythrough Animation
Create a path that flies through the scene:
```cpp
vector<float3> waypoints = {
    {10, 2, 10},
    {5, 3, 0},
    {0, 4, -5},
    {-5, 3, 0},
    {-10, 2, 10}
};
cameraPath.setDollyPath(waypoints, 15.0f, false);
```

### Cinematic Reveal
Start far away and zoom into detail:
```cpp
cameraPath.addKeyframe(0.0f, {20, 10, 20}, {0, 0, 0}, 70.0f);  // Wide FOV
cameraPath.addKeyframe(3.0f, {10, 5, 10}, {0, 0, 0}, 60.0f);
cameraPath.addKeyframe(6.0f, {3, 2, 3}, {0, 0, 0}, 50.0f);
cameraPath.addKeyframe(10.0f, {1, 1, 1}, {0, 0, 0}, 35.0f);    // Narrow FOV
```

## Technical Details

### Video Codec Details
- **H.264**: AVC codec, profile=High, 8-bit color
- **H.265**: HEVC codec, profile=Main10, 10-bit color support
- **ProRes**: Apple ProRes codec, 10-bit 4:2:2 (422) or 4:4:4:4 (4444)

### Color Space
- **SDR**: Rec.709 color space (default)
- **HDR**: P3 color space with HDR10 metadata (H.265 only)

### File Format
- **MOV**: QuickTime container (ProRes)
- **MP4**: MPEG-4 container (H.264, H.265)

### Thread Safety
- `setup()`, `beginExport()`, `endExport()`: Call from any thread
- `addFrame()`: Must be called from encoding thread (same thread as `beginExport()`)
- Progress callbacks: Called from encoding thread

## Troubleshooting

### Export Fails to Start
- Check codec/resolution compatibility
- Verify output path is writable
- Ensure scene is not empty
- Check `getLastError()` for details

### Slow Encoding Speed
- Enable hardware acceleration
- Lower resolution or framerate
- Use H.265 instead of ProRes for faster encoding
- Check Activity Monitor for thermal throttling

### Large File Sizes
- Use H.265 instead of ProRes
- Lower quality setting
- Reduce resolution or framerate
- Use custom bitrate to limit file size

## Next Steps

- **Example 25**: Sharp Real-time Camera - Generate Gaussian clouds from live camera
- See `src/addons/apple_native/ofxSharp/SharpVideoExporter.h` for complete API
- See `src/addons/apple_native/ofxSharp/SharpCameraPath.h` for camera animation API
- See `src/addons/apple_native/ofxSharp/README.md` for ofxSharp overview

## Related Examples
- **Example 22**: Sharp Basic - Single Gaussian cloud rendering
- **Example 23**: Sharp Scene - Multi-object scene management
- **Example 10**: FBO - Offscreen rendering for video export
