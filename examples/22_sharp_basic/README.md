# Example 22: Sharp Basic - 3D Gaussian Splatting

This example demonstrates **ofxSharp**, Apple's SHARP (3D Gaussian Splatting) integration for oflike-metal. It shows how to generate and render 3D representations from single 2D images using Neural Engine acceleration.

## What This Example Shows

- **Neural Engine Inference**: Generate 3D Gaussian clouds from single images in < 1 second
- **Real-time Rendering**: Render Gaussian splats at 60+ FPS using Metal
- **Camera Control**: Interactive camera navigation with ofEasyCam
- **Parameter Adjustment**: Real-time tweaking of rendering parameters
- **PLY Import/Export**: Save and load Gaussian clouds
- **Performance Monitoring**: View inference time, Gaussian count, render stats

## Visual Elements

The example renders:
1. **3D Gaussian Cloud**: Generated from input image using Neural Engine
2. **Coordinate Axes**: RGB lines showing X, Y, Z axes for orientation
3. **UI Overlay**: Status, statistics, and controls
4. **FPS Counter**: Real-time performance monitoring

## Key Concepts

### Setup
```cpp
void setup() override {
    // Initialize ofxSharp
    sharp.setup();

    // Configure camera
    camera.setDistance(5.0f);
    camera.enableMouseInput();
}
```

### Generation (Asynchronous)
```cpp
void loadAndGenerate() {
    // Load source image
    sourceImage.load("test_image.jpg");

    // Generate 3D representation asynchronously
    sharp.generateFromImageAsync(sourceImage, [this](bool success) {
        if (success) {
            // Cloud is ready to render
            lastInferenceTime = sharp.getLastInferenceTime();
        }
    });
}
```

### Rendering
```cpp
void draw() override {
    camera.begin();

    // Draw Gaussian cloud with current camera
    sharp.drawCloud(camera);

    camera.end();
}
```

## Interactive Controls

### Generation
- **SPACE** - Load image and generate 3D Gaussian cloud
- **S** - Save current cloud to PLY file (timestamped)
- **L** - Load cloud from PLY file (not implemented - needs file dialog)

### Rendering Parameters
- **+/-** - Adjust splat scale (0.1 - 5.0)
- **[/]** - Adjust opacity (0.1 - 1.0)
- **D** - Toggle depth sorting (required for correct transparency)
- **A** - Toggle anti-aliasing
- **1-4** - Set spherical harmonics degree (0-3)
  - 0: Diffuse color only
  - 3: Full view-dependent appearance (default)

### Camera Control
- **Mouse Drag** - Rotate camera around target
- **Mouse Wheel** - Zoom in/out
- **R** - Reset cloud transform to identity

## Technical Details

### Neural Engine Acceleration

ofxSharp uses Apple's Core ML and Neural Engine for fast inference:
- **Inference Time**: < 1 second on Apple Silicon
- **Model**: Pre-trained SHARP model (bundled as .mlmodelc)
- **Input**: Single RGB image (any resolution)
- **Output**: 3D Gaussian cloud (positions, colors, covariances)

### Gaussian Splatting

Each Gaussian splat is defined by:
- **Position** (μ): 3D center point
- **Color** (SH): Spherical harmonics coefficients (view-dependent)
- **Opacity** (α): Transparency value
- **Covariance** (Σ): 3x3 matrix defining shape and orientation
- **Scale**: Size of the splat

### Rendering Pipeline

1. **Depth Sort**: Sort Gaussians back-to-front (GPU)
2. **Projection**: Project 3D covariance to 2D screen space
3. **Rasterization**: Draw each splat as textured quad
4. **Alpha Blending**: Composite splats with over operator
5. **Anti-aliasing**: Apply Gaussian kernel for smooth edges

### Performance

Expected performance on Apple Silicon:
- **Inference**: 500-1000ms (Neural Engine)
- **Rendering**: 60+ FPS at 1080p
- **Gaussian Count**: ~100k-500k splats (depends on scene)
- **Memory**: ~50-200 MB (depends on cloud size)

## API Functions Used

### ofxSharp
- `setup()` - Initialize Sharp with default settings
- `generateFromImageAsync()` - Generate 3D from image (async)
- `drawCloud()` - Render Gaussian cloud
- `hasCloud()` - Check if cloud is loaded
- `getGaussianCount()` - Get number of Gaussians
- `isGenerating()` - Check if inference is running
- `getLastInferenceTime()` - Get inference duration
- `isUsingNeuralEngine()` - Check if Neural Engine was used
- `saveCloud()` / `loadCloud()` - PLY import/export
- `setSplatScale()` - Adjust splat size
- `setOpacityScale()` - Adjust transparency
- `setDepthSortEnabled()` - Toggle depth sorting
- `setAntiAliasingEnabled()` - Toggle anti-aliasing
- `setMaxSHDegree()` - Set spherical harmonics degree
- `resetTransform()` - Reset cloud transform

### ofEasyCam
- `setDistance()` - Set camera distance
- `setTarget()` - Set look-at target
- `enableMouseInput()` - Enable interactive control
- `begin()` / `end()` - Activate camera

## File Requirements

This example expects a test image file:
- **File**: `test_image.jpg` (or any image format)
- **Location**: Same directory as executable
- **Format**: RGB image (any resolution)
- **Fallback**: Creates a gradient test image if not found

To use your own image:
1. Place an image file in the build directory
2. Press SPACE to load and generate

## Expected Output

When running, you should see:
1. Initial status message prompting to press SPACE
2. After pressing SPACE:
   - Status changes to "Generating..."
   - Neural Engine processes the image (~1 second)
   - Status updates to "Generation complete!"
3. 3D Gaussian cloud appears
4. Interactive camera control enabled
5. Real-time rendering at 60+ FPS

## Rendering Quality

The quality of the 3D reconstruction depends on:
- **Image Content**: Objects with clear depth cues work best
- **Image Quality**: High resolution produces better results
- **Scene Complexity**: Simple scenes are more reliable
- **View-dependent Effects**: Spherical harmonics capture reflections/highlights

## Advanced Usage

### Direct Access to Sharp Classes

For advanced control, you can access the underlying Sharp classes:

```cpp
// Get direct access to components
Sharp::SharpModel* model = sharp.getModel();
Sharp::SharpRenderer* renderer = sharp.getRenderer();
Sharp::GaussianCloud* cloud = sharp.getCloud();

// Advanced filtering
cloud->filterByOpacity(0.1f);
cloud->filterBySize(0.01f, 10.0f);
```

### Custom Resolution

```cpp
// Initialize with custom render resolution
sharp.setup(1920, 1080);
```

### Custom Model

```cpp
// Use custom trained model
sharp.setup("path/to/custom_model.mlmodelc");
```

## Troubleshooting

### "Failed to initialize ofxSharp"
- Ensure SHARP model file (.mlmodelc) is bundled with app
- Check macOS version (requires macOS 13.0+)
- Verify Core ML framework is linked

### "Generation failed"
- Check image format and size
- Ensure Neural Engine is available (Apple Silicon)
- Check Console for detailed error messages

### Low FPS
- Reduce splat scale
- Disable depth sorting (but transparency will be incorrect)
- Filter invisible Gaussians with `removeInvisible()`

### Black Screen
- Ensure cloud was generated successfully
- Check camera distance (try adjusting with mouse wheel)
- Verify depth test is enabled

## Next Steps

After mastering this example, try:
- **Example 23: Sharp Scene** - Multi-object scene management
- **Example 24: Sharp Video Export** - Export camera animations
- **Example 25: Sharp Realtime Camera** - Live camera feed to 3D

## References

- [Apple SHARP Repository](https://github.com/apple/ml-sharp)
- [3D Gaussian Splatting Paper](https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/)
- [ofxSharp Documentation](../../../src/addons/apple_native/ofxSharp/README.md)

## Notes

- **Neural Engine**: Only available on Apple Silicon (M1/M2/M3)
- **Intel Macs**: Falls back to GPU/CPU (slower inference)
- **Model Size**: ~50-100 MB (included in bundle)
- **License**: SHARP model follows Apple Sample Code License
