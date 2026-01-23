# 10_fbo - Framebuffer Objects (Offscreen Rendering)

Comprehensive demonstration of ofFbo (Framebuffer Object) API for offscreen rendering, multi-pass rendering, texture readback, multisampling, depth buffers, and multiple color attachments.

## Features

### Six FBO Demonstration Modes

#### Mode 1: Basic Offscreen Rendering
- Single FBO rendering 3D scene
- Central rotating sphere with orbiting cubes
- Camera and lighting setup
- FBO as texture source

#### Mode 2: Two-Pass Rendering
- First pass: Render scene to fbo1
- Second pass: Apply post-processing using fbo1 as input texture
- Simple "glow" effect by drawing offset copies
- Demonstrates FBO chaining

#### Mode 3: Multisample FBO (MSAA 4x)
- 4x multisampling for anti-aliased rendering
- Smooth edges on geometric primitives
- Demonstrates MSAA setup with `ofFboSettings::numSamples`
- Comparison with non-MSAA rendering

#### Mode 4: Multiple Color Attachments
- FBO with 2 color attachments
- `ofFboSettings::numColorbuffers = 2`
- Demonstrates MRT (Multiple Render Targets) setup
- Note: Actual multi-target rendering requires custom shaders

#### Mode 5: Depth Buffer Visualization
- FBO with depth buffer enabled
- `ofFboSettings::useDepth = true`
- `ofFboSettings::depthStencilAsTexture = true`
- Side-by-side color and depth texture display
- Objects at different depth values

#### Mode 6: Texture Readback & CPU Processing
- `fbo.readToPixels(pixels)` - GPU to CPU transfer
- CPU-side pixel manipulation (color inversion)
- Demonstrates full GPU→CPU→GPU pipeline
- Side-by-side original and processed display

### Interactive Controls

- **1-6**: Switch between FBO modes
- **SPACE**: Toggle animation on/off
- **M**: Toggle MSAA (4x multisampling) on/off
- **S**: Save FBO contents to PNG file
- **UP/DOWN**: Adjust FBO resolution (256-2048)

### Visual Elements

- **3D Scene**: Rotating sphere with orbiting cubes/cylinders
- **Lighting**: Point light with Phong shading
- **Materials**: Specular highlights (shininess 64)
- **Camera**: Perspective camera with orbit view
- **UI**: Real-time FPS, mode description, settings

## API Reference

### ofFbo - Core Methods

```cpp
// Allocation
void allocate(int width, int height, int internalformat);
void allocateWithSettings(const ofFboSettings& settings);

// Rendering
void begin();
void end();
void clear();

// Drawing
void draw(float x, float y);
void draw(float x, float y, float w, float h);

// Texture Access
ofTexture& getTexture();
ofTexture& getDepthTexture();
ofTexture& getTexture(int attachmentPoint);

// Properties
int getWidth();
int getHeight();
int getNumTextures();

// Data Transfer
void readToPixels(ofPixels& pixels);
void readToPixels(ofPixels& pixels, int attachmentPoint);

// Advanced
void setActiveDrawBuffer(int i);
```

### ofFboSettings - Configuration

```cpp
struct ofFboSettings {
    int width = 0;
    int height = 0;
    int internalformat = GL_RGBA;       // Color format
    int numSamples = 0;                  // MSAA samples (0 = off, 4/8/16 = MSAA)
    int numColorbuffers = 1;             // Multiple render targets
    bool useDepth = false;               // Depth buffer
    bool useStencil = false;             // Stencil buffer
    bool depthStencilAsTexture = false;  // Make depth readable as texture
    int textureTarget = GL_TEXTURE_2D;
    int wrapModeHorizontal = GL_CLAMP_TO_EDGE;
    int wrapModeVertical = GL_CLAMP_TO_EDGE;
    int minFilter = GL_LINEAR;
    int maxFilter = GL_LINEAR;
};
```

### Usage Examples

#### Basic Offscreen Rendering

```cpp
ofFbo fbo;
fbo.allocate(512, 512, GL_RGBA);

// Render to FBO
fbo.begin();
ofClear(0, 0, 0, 255);
ofDrawCircle(256, 256, 100);
fbo.end();

// Draw FBO to screen
ofSetColor(255);
fbo.draw(0, 0);
```

#### MSAA (Multisampling)

```cpp
ofFboSettings settings;
settings.width = 1024;
settings.height = 1024;
settings.internalformat = GL_RGBA;
settings.numSamples = 4;  // 4x MSAA
settings.useDepth = true;

ofFbo fbo;
fbo.allocateWithSettings(settings);

// Render with anti-aliasing
fbo.begin();
ofClear(0, 0, 0, 255);
ofDrawCircle(512, 512, 200);
fbo.end();

fbo.draw(0, 0);
```

#### Texture Readback

```cpp
ofFbo fbo;
fbo.allocate(512, 512, GL_RGBA);

// Render something
fbo.begin();
ofDrawCircle(256, 256, 100);
fbo.end();

// Read pixels back to CPU
ofPixels pixels;
fbo.readToPixels(pixels);

// Modify pixels
for (size_t i = 0; i < pixels.size(); i += 4) {
    pixels[i] = 255 - pixels[i];  // Invert red channel
}

// Save to disk
ofImage img;
img.setFromPixels(pixels);
img.save("output.png");
```

#### Multiple Color Attachments

```cpp
ofFboSettings settings;
settings.width = 1024;
settings.height = 1024;
settings.numColorbuffers = 2;  // Two render targets
settings.useDepth = true;

ofFbo fbo;
fbo.allocateWithSettings(settings);

// Render (requires custom shader for multi-target output)
fbo.begin();
// Fragment shader writes to gl_FragData[0] and gl_FragData[1]
fbo.end();

// Access individual attachments
ofTexture& colorTex0 = fbo.getTexture(0);
ofTexture& colorTex1 = fbo.getTexture(1);
```

#### Depth Buffer as Texture

```cpp
ofFboSettings settings;
settings.width = 1024;
settings.height = 1024;
settings.internalformat = GL_RGBA;
settings.useDepth = true;
settings.depthStencilAsTexture = true;  // Make depth readable

ofFbo fbo;
fbo.allocateWithSettings(settings);

// Render 3D scene
fbo.begin();
ofEnableDepthTest();
// ... render 3D objects ...
ofDisableDepthTest();
fbo.end();

// Draw color and depth side by side
fbo.getTexture().draw(0, 0, 512, 512);
fbo.getDepthTexture().draw(512, 0, 512, 512);
```

## Common Use Cases

### 1. Post-Processing Effects
- Render scene to FBO
- Apply shader effects (blur, bloom, color grading)
- Display processed result

### 2. Shadow Mapping
- Render scene from light's perspective to depth FBO
- Use depth texture for shadow calculations

### 3. Reflections
- Render mirrored view to FBO
- Apply as reflection texture

### 4. Deferred Rendering
- Multiple FBOs with different buffers (positions, normals, albedo)
- Final lighting pass combines all buffers

### 5. Screen Recording
- Render to FBO
- `readToPixels()` each frame
- Encode to video

### 6. High-Resolution Rendering
- Render to large FBO (4K, 8K)
- Save final image
- Display downscaled version in real-time

## Performance Considerations

### FBO Size
- Larger FBOs require more GPU memory
- Rendering time scales with pixel count
- Balance quality vs performance

### Multisampling (MSAA)
- 4x MSAA = 4x more samples per pixel
- Improves edge quality significantly
- Performance cost: 2-4x rendering time
- Most beneficial for geometric edges

### Texture Readback
- GPU→CPU transfer is slow
- Use sparingly (avoid per-frame readback)
- Async readback if possible
- Consider GPU-only processing

### Multiple Attachments
- Minimal performance cost if unused
- Efficient for deferred rendering
- Requires fragment shader support

### Depth Buffer
- Essential for 3D rendering
- Minimal performance impact
- Use `depthStencilAsTexture` only when needed

## Integration

### CMakeLists.txt

```cmake
add_executable(10_fbo
    examples/10_fbo/main.cpp
)

target_link_libraries(10_fbo
    oflike-metal
)

# Copy resources if needed
add_custom_command(TARGET 10_fbo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/examples/10_fbo/data
    $<TARGET_FILE_DIR:10_fbo>/data
)
```

### Build & Run

```bash
cd build
cmake ..
make 10_fbo
./10_fbo
```

## Learning Points

1. **Offscreen Rendering**: FBOs allow rendering to texture instead of screen
2. **Render Passes**: Chain multiple FBOs for multi-pass effects
3. **Multisampling**: MSAA provides hardware anti-aliasing
4. **Texture Readback**: Transfer GPU data to CPU for processing
5. **Depth Buffers**: Essential for 3D rendering with proper occlusion
6. **Multiple Attachments**: Efficient for deferred rendering pipelines
7. **Resolution Independence**: Render at different resolution than display

## Architecture Compliance

- ✅ **Pure C++ API**: No Metal/Swift in user code
- ✅ **Metal Backend**: FBO implemented using MTLTexture and render passes
- ✅ **openFrameworks Compatible**: Same API as oF's ofFbo
- ✅ **Zero-Copy**: Unified memory for efficient GPU access
- ✅ **MSAA Support**: Hardware multisampling via Metal
- ✅ **MRT Support**: Multiple render targets for advanced effects

## Expected Output

### Console

```
[notice ] FboApp: Setup complete - FBO resolution: 512x512
[notice ] FboApp: Press 1-6 to switch FBO modes
[notice ] FboApp: FBOs created:
[notice ] FboApp:   fbo1: 512x512
[notice ] FboApp:   multiSampleFbo: 512x512 (4x MSAA)
[notice ] FboApp:   multiAttachmentFbo: 512x512 (2 attachments)
[notice ] FboApp:   depthFbo: 512x512 (with depth)
[notice ] FboApp: Switched to mode 1
[notice ] FboApp: Animation: OFF
[notice ] FboApp: MSAA: ON
[notice ] FboApp: Saved FBO contents to: fbo_capture_2026-01-24-12-30-45.png
```

### Visual Output

- **Mode 1**: Rotating 3D scene (sphere + cubes) rendered to FBO, displayed on screen
- **Mode 2**: Split view showing original scene and post-processed version with glow
- **Mode 3**: Anti-aliased rendering with smooth edges (MSAA 4x)
- **Mode 4**: Complex 3D scene demonstrating MRT setup
- **Mode 5**: Split view showing color buffer and depth buffer side-by-side
- **Mode 6**: Split view showing original and color-inverted versions

### UI Overlay
- Mode description (e.g., "Mode 1: Basic Offscreen Rendering")
- FBO resolution (e.g., "512x512")
- MSAA status ("ON (4x)" or "OFF")
- Animation status ("ON" or "OFF")
- FPS counter
- Control hints

## Troubleshooting

### Black Screen
- Check FBO allocation succeeded
- Ensure `begin()/end()` are properly paired
- Verify camera is positioned correctly
- Check lighting is enabled for 3D scenes

### Poor Performance
- Reduce FBO resolution
- Disable MSAA
- Avoid per-frame texture readback
- Profile with Metal GPU Capture

### Depth Buffer Not Working
- Ensure `useDepth = true` in settings
- Call `ofEnableDepthTest()` before rendering
- Check near/far clip planes on camera

### MSAA Not Visible
- Compare with non-MSAA side-by-side
- Zoom in on geometric edges
- Ensure `numSamples = 4` or higher

## Related Examples

- **06_3d_primitives**: Basic 3D rendering
- **07_camera**: Camera setup and control
- **08_lighting**: Lighting and materials
- **09_mesh**: Custom mesh rendering

## References

- openFrameworks ofFbo documentation
- Metal Multi-Pass Rendering
- Deferred Rendering techniques
- Shadow Mapping algorithms
