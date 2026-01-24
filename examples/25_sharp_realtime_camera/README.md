# Example 25: Sharp Realtime Camera

This example demonstrates **real-time 3D Gaussian Splatting generation from live camera input** using ofxSharp. It captures frames from your Mac's built-in camera and generates 3D Gaussian cloud representations on-the-fly using Apple's Neural Engine.

## Features

- 📹 **Live Camera Capture** - Captures frames from built-in camera using AVFoundation
- ⚡ **Neural Engine Acceleration** - Sub-second inference times (typically 200-800ms)
- 🎨 **Real-time 3D Viewing** - Interactive 3D visualization of generated Gaussian clouds
- 🔄 **Auto-generation Mode** - Automatically generate 3D from camera at regular intervals
- 🎮 **Interactive Controls** - Adjust rendering parameters in real-time
- 💾 **Export Support** - Save generated clouds to PLY format

## How It Works

### Pipeline

```
Camera → AVFoundation → CVPixelBuffer → ofImage → SHARP Model → Gaussian Cloud → Metal Renderer
```

1. **Capture**: AVFoundation captures camera frames at 640x480
2. **Convert**: BGRA pixel format is converted to RGB for ofImage
3. **Inference**: SHARP Core ML model generates 3D Gaussian parameters (typically 500-2000 Gaussians)
4. **Render**: Metal-based renderer displays Gaussian splats in 3D

### Performance

- **Inference**: 200-800ms per frame (Neural Engine)
- **Rendering**: 60+ FPS with depth sorting and anti-aliasing
- **Memory**: ~50-100MB per cloud depending on Gaussian count

## Controls

### Camera Control
- `SPACE` - Start/Stop camera
- `V` - Toggle camera preview

### Generation
- `G` - Generate 3D from current camera frame
- `A` - Toggle auto-generate mode
- `</,` - Decrease cooldown between auto-generations
- `>/.` - Increase cooldown between auto-generations

### Rendering
- `+/-` - Adjust splat scale
- `[/]` - Adjust opacity
- `1-4` - Set spherical harmonics degree (0-3)
- `R` - Reset transform

### Save
- `S` - Save current cloud to PLY file (timestamped)

### 3D Camera
- `Mouse Drag` - Rotate camera
- `Mouse Wheel` - Zoom in/out

## UI Layout

```
┌─────────────┬──────────────────────────────┐
│             │                              │
│   Camera    │        3D View               │
│   Preview   │                              │
│  (640x480)  │    (Interactive Gaussian     │
│             │      Splatting Render)       │
│             │                              │
└─────────────┴──────────────────────────────┘
      1/3              2/3 of window
```

## Usage Examples

### Basic Usage

1. **Start the application**
2. **Press SPACE** to start camera
3. **Press G** to generate 3D from current frame
4. **Use mouse** to rotate and explore the 3D cloud

### Auto-generation Mode

1. **Press A** to enable auto-generate
2. Camera frames will automatically be converted to 3D
3. Adjust cooldown with `<` and `>` keys to control frequency
4. Recommended cooldown: 2-5 seconds

### Capture Workflow

```cpp
// Manual capture
1. SPACE - Start camera
2. Position yourself
3. G - Generate 3D
4. S - Save to PLY

// Auto-capture
1. SPACE - Start camera
2. A - Enable auto-generate
3. Move around while it captures
4. S - Save interesting results
```

## Technical Details

### Camera Setup

The example uses AVFoundation for camera capture:
- **Resolution**: 640x480 (AVCaptureSessionPreset640x480)
- **Format**: BGRA (kCVPixelFormatType_32BGRA)
- **Frame Rate**: Up to 30 FPS (limited by cooldown)

### Pixel Format Conversion

```cpp
// BGRA (AVFoundation) → RGB (ofImage)
for each pixel:
    R = BGRA[2]
    G = BGRA[1]
    B = BGRA[0]
```

### Async Generation Pipeline

```cpp
processCameraFrame(CVPixelBufferRef) → ofImage buffer
    ↓
generateFromImageAsync(ofImage, callback)
    ↓
SHARP Model inference (Neural Engine, ~500ms)
    ↓
Callback on main thread
    ↓
Cloud ready for rendering
```

### Cooldown System

To prevent overwhelming the Neural Engine:
- Minimum time between generations (default: 2 seconds)
- Adjustable via `<` and `>` keys
- Only triggers in auto-generate mode

## Performance Tips

### Optimize for Real-time

1. **Cooldown**: Keep cooldown at 2+ seconds for smooth operation
2. **Resolution**: 640x480 is optimal for real-time inference
3. **SH Degree**: Lower SH degree (0-1) for faster rendering
4. **Splat Count**: SHARP model generates 500-2000 Gaussians per frame

### Reduce Latency

```cpp
// Lower cooldown for faster updates
cooldownDuration = 1.0f; // Faster but may lag

// Higher quality but slower
sharp.setMaxSHDegree(3); // Full quality
```

### Memory Management

- Camera frames are double-buffered
- Only one cloud is kept in memory at a time
- Each generation replaces previous cloud

## Camera Permissions

On macOS 10.14+, camera access requires permission:

1. First launch will prompt for camera access
2. Grant permission in System Preferences → Security & Privacy → Camera
3. Restart application after granting permission

### Info.plist Entry

If building your own app, add:

```xml
<key>NSCameraUsageDescription</key>
<string>This app needs camera access for real-time 3D generation</string>
```

## Troubleshooting

### Camera Not Starting

- **Check permissions**: System Preferences → Security & Privacy → Camera
- **Check device**: Ensure camera is not in use by another app
- **Console log**: Check for AVFoundation error messages

### Slow Generation

- **Neural Engine**: Verify with status display (should show "YES")
- **Cooldown**: Increase cooldown duration
- **Resolution**: Camera uses 640x480 which is optimal

### Memory Issues

- **Single cloud**: Only one cloud exists at a time
- **Auto-generate**: Disable if memory warnings occur
- **Save regularly**: Export to PLY and clear if needed

## Examples Use Cases

### 1. Face Scanning

```
1. Position face in front of camera
2. Press G to generate
3. Rotate 3D view to see reconstruction
4. S to save
```

### 2. Object Capture

```
1. Place object in view
2. Enable auto-generate (A)
3. Move camera around object
4. Save best results (S)
```

### 3. Scene Exploration

```
1. Point camera at scene
2. Generate 3D (G)
3. Explore spatial structure in 3D
4. Compare with camera preview
```

## Advanced Configuration

### Custom Resolution

Modify in `startCamera()`:

```objc
[captureSession setSessionPreset:AVCaptureSessionPreset1280x720]; // Higher res
```

Note: Higher resolution = longer inference time

### Custom Cooldown

```cpp
cooldownDuration = 5.0f; // 5 seconds between generations
```

### Rendering Quality

```cpp
sharp.setSplatScale(2.0f);      // Larger splats
sharp.setOpacityScale(0.8f);    // More transparent
sharp.setMaxSHDegree(2);        // Medium quality SH
sharp.setAntiAliasingEnabled(true);
sharp.setDepthSortEnabled(true);
```

## Known Limitations

1. **Single Image Input**: SHARP model works best with single, well-lit images
2. **Static Scenes**: Moving subjects may not reconstruct well
3. **Lighting**: Good lighting improves reconstruction quality
4. **Camera Quality**: Built-in webcam quality affects results

## Related Examples

- **22_sharp_basic** - Basic SHARP usage with static images
- **23_sharp_scene** - Multi-object scene management
- **24_sharp_video_export** - Export animations to video

## References

- Apple SHARP: https://github.com/apple/ml-sharp
- 3D Gaussian Splatting: https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/
- AVFoundation: https://developer.apple.com/av-foundation/
- Core ML: https://developer.apple.com/documentation/coreml

## Credits

- **ofxSharp**: Daito Manabe / YCAM
- **SHARP Model**: Apple Machine Learning Research
- **3D Gaussian Splatting**: Kerbl et al., SIGGRAPH 2023
