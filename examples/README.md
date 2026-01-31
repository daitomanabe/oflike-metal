# oflike-metal Examples

Comprehensive collection of examples demonstrating all major features of the oflike-metal framework.
Note: Legacy ofMain-based examples are being migrated to the SwiftUI-only entry path and may not build yet.

## Quick Start

- Use `examples/validation_swiftui` for the current SwiftUI validation sample.

---

## Example Overview

### Beginner Level

#### [01_basics](01_basics/)
**Foundation**: Application structure, setup/update/draw pattern
- ofBaseApp lifecycle
- Background colors
- Basic 2D shapes (circle, rectangle, line, triangle)
- Simple animation
- Color setting

**Concepts**: ofApp pattern, drawing API basics, animation loop

---

#### [02_shapes](02_shapes/)
**2D Drawing**: Comprehensive shape drawing capabilities
- Lines, rectangles, circles, ellipses, triangles
- Rounded rectangles
- Filled vs stroked shapes
- Line width and resolution
- ofPath and ofPolyline

**Concepts**: Shape API, fill/stroke modes, path building, tessellation

---

#### [03_color](03_color/)
**Color System**: Color representation and manipulation
- ofColor, ofFloatColor, ofShortColor
- RGB, HSB color spaces
- Color interpolation (lerp)
- Hex colors
- Color operations (invert, clamp, normalize)

**Concepts**: Color spaces, color math, interpolation

---

### Intermediate Level

#### [04_image](04_image/)
**Image Handling**: Loading, displaying, and manipulating images
- ofImage loading (PNG, JPG, JPEG 2000, HEIF)
- ofTexture drawing
- ofPixels manipulation
- Image transformations (resize, crop, rotate, mirror)
- Native ImageIO integration

**Concepts**: Image I/O, texture mapping, pixel access, Metal texture loading

---

#### [05_typography](05_typography/)
**Text Rendering**: Font loading and text display
- ofTrueTypeFont with Core Text
- Font loading (TrueType, OpenType)
- Text rendering with positioning
- String bounding boxes
- Line height and letter spacing
- UTF-8 support (Japanese, Chinese, etc.)

**Concepts**: Font rendering, text layout, Unicode, Core Text integration

---

#### [06_3d_primitives](06_3d_primitives/)
**3D Graphics**: Built-in 3D shape drawing
- ofDrawBox, ofDrawSphere, ofDrawCone
- ofDrawCylinder, ofDrawPlane, ofDrawIcoSphere
- 3D transformations (translate, rotate, scale)
- Depth testing
- Wireframe vs solid rendering

**Concepts**: 3D coordinate system, transformations, depth buffer

---

### Advanced Level

#### [07_camera](07_camera/)
**Camera System**: 3D viewport control
- ofCamera setup and configuration
- View and projection matrices
- Camera positioning and orientation
- Look-at functionality
- World-to-screen / screen-to-world conversion
- FOV, near/far clipping planes

**Concepts**: Perspective projection, view matrices, camera transforms

---

#### [08_lighting](08_lighting/)
**Lighting System**: Realistic 3D lighting
- ofLight types (point, directional, spot)
- ofMaterial properties (ambient, diffuse, specular)
- Multiple lights (up to 8)
- Phong shading model
- Light attenuation
- Smooth lighting toggle

**Concepts**: Phong lighting, material properties, Metal shader lighting

---

#### [09_mesh](09_mesh/)
**Custom 3D Geometry**: Building and manipulating meshes
- ofMesh creation and editing
- Vertices, normals, texCoords, colors, indices
- Draw modes (triangles, lines, points)
- Normal calculation (smooth, flat)
- Mesh generators (sphere, box, plane, icosphere)
- PLY/OBJ loading and saving

**Concepts**: Vertex buffers, index buffers, mesh topology, file formats

---

#### [10_fbo](10_fbo/)
**Offscreen Rendering**: Render-to-texture techniques
- ofFbo allocation and configuration
- Multi-pass rendering
- Post-processing effects
- Ping-pong buffers
- Depth textures
- Multiple color attachments

**Concepts**: Framebuffer objects, render targets, post-processing pipeline

---

### Expert Level

#### [11_events](11_events/)
**Event System**: Complete input handling
- Mouse events (move, drag, press, release, scroll, enter, exit)
- Keyboard events (press, release, state query)
- Window events (resize, drag & drop)
- Event logging and visualization
- Mouse trail rendering
- Interactive drawing

**Concepts**: Event callbacks, input state, interactive applications

---

#### [12_easycam](12_easycam/)
**Interactive Camera**: Advanced 3D navigation
- ofEasyCam orbit controls
- Mouse-driven camera (drag to rotate, scroll to zoom)
- Camera inertia and momentum
- Target tracking and animation
- Auto distance adjustment
- Preset viewpoints
- Camera state management

**Concepts**: 3D interaction, orbital camera, quaternion rotation, camera UX

---

### Feature Test Examples

#### [28_vbo_mesh](28_vbo_mesh/)
**VBO Mesh Rendering**: Modern Metal GPU mesh with VboMesh
- VboMesh creation (sphere, box, cone primitives)
- Dynamic mesh updates (vertex animation)
- Static vs dynamic usage hints
- Transform matrix integration (ofPushMatrix/ofTranslate)
- Color tinting with ofSetColor
- Wireframe mode support
- Lighting integration

**Concepts**: GPU buffers, vertex buffer objects, dynamic mesh updates, Metal storage modes

---

#### [29_graphics_primitives](29_graphics_primitives/)
**Graphics API Test**: Comprehensive 2D/3D primitive rendering
- 2D shapes (circle, rectangle, ellipse, triangle)
- 3D primitives (box, sphere, cone, cylinder)
- Toggle between 2D and 3D modes

**Concepts**: ofGraphics API, primitive rendering, coordinate systems

---

#### [30_image_handling](30_image_handling/)
**Image API Test**: Image loading, manipulation, and rendering
- ofImage loading and display
- Pixel manipulation
- Image generation
- Mode switching

**Concepts**: ofImage, ofPixels, texture mapping

---

#### [31_math_operations](31_math_operations/)
**Math API Test**: Vector, matrix, and quaternion operations
- ofVec3f operations
- ofMatrix4x4 transformations
- ofQuaternion rotations
- Mathematical visualization

**Concepts**: Linear algebra, transformations, quaternions

---

#### [32_lighting_systems](32_lighting_systems/)
**Lighting API Test**: Multiple light types and properties
- Point lights
- Directional lights
- Spot lights
- Material properties

**Concepts**: Phong lighting model, light attenuation, material system

---

#### [33_phase1_features](33_phase1_features/)
**Phase 1 Features**: Core framework feature demonstration
- ofNode hierarchy
- ofCamera setup
- ofFbo usage
- ofShader integration

**Concepts**: Scene graph, render targets, shader pipeline

---

#### [34_phase2_rendering](34_phase2_rendering/)
**Phase 2 Rendering**: Advanced rendering pipeline
- VBO rendering
- Texture mapping
- Multiple lights
- Mipmap generation

**Concepts**: GPU pipeline, texture filtering, multi-pass rendering

---

#### [35_phase3_media](35_phase3_media/)
**Phase 3 Media**: Video and image processing
- Video playback
- Camera input
- Image filters
- Pixel effects

**Concepts**: Media handling, real-time video, image processing

---

#### [36_phase4_advanced](36_phase4_advanced/)
**Phase 4 Advanced**: Complete 3D scene with audio
- 3D primitives
- Material properties
- Sound loading and playback
- Wireframe toggle

**Concepts**: Audio integration, complete 3D scenes, material system

---

## Learning Path

### Path 1: 2D Creative Coding
```
01_basics → 02_shapes → 03_color → 05_typography → 04_image → 11_events
```
Build interactive 2D applications with drawing, text, images, and user input.

### Path 2: 3D Graphics
```
01_basics → 06_3d_primitives → 07_camera → 08_lighting → 09_mesh → 12_easycam
```
Learn 3D rendering from primitives to advanced lighting and camera control.

### Path 3: Generative Art
```
01_basics → 02_shapes → 03_color → 09_mesh → 10_fbo
```
Create algorithmic art with shapes, colors, custom geometry, and post-processing.

### Path 4: Complete Framework Tour
```
01 → 02 → 03 → 04 → 05 → 06 → 07 → 08 → 09 → 10 → 11 → 12
```
Comprehensive exploration of all framework features in order.

---

## Building Examples

### All Examples (CMake)
```bash
# Configure
mkdir build && cd build
cmake .. -G Xcode

# Build all
cmake --build . --config Release

# Build specific example
cmake --build . --target 01_basics --config Release

# Run
cd examples/01_basics
./01_basics
```

### Individual Example (Xcode)
```bash
cd examples/01_basics
open 01_basics.xcodeproj
# Press Cmd+R to build and run
```

### From Terminal (Xcode Command Line)
```bash
cd examples/01_basics
xcodebuild -scheme 01_basics build
# Find binary in build/Release/
```

---

## Example Structure

Each example follows this structure:
```
XX_example_name/
├── README.md          # Detailed documentation
├── main.cpp           # Application entry point
├── MyApp.h            # Application class header (if separate)
├── MyApp.cpp          # Application implementation (if separate)
├── XX_example_name.xcodeproj  # Xcode project (optional)
└── CMakeLists.txt     # CMake configuration (optional)
```

Most examples use inline classes in `main.cpp` for simplicity.

---

## Requirements

- **macOS**: 13.0 Ventura or later
- **Xcode**: 15.0 or later
- **Hardware**: Apple Silicon (M1/M2/M3/M4) recommended, Intel with Metal support also works
- **CMake**: 3.20 or later (for CMake builds)

---

## Key Features Demonstrated

| Feature | Examples |
|---------|----------|
| **2D Drawing** | 01, 02, 03, 29 |
| **3D Graphics** | 06, 07, 08, 09, 28, 29, 36 |
| **Camera Control** | 07, 12, 33 |
| **Lighting** | 08, 32, 34 |
| **Textures** | 04, 10, 30, 34 |
| **Text Rendering** | 05 |
| **Custom Geometry** | 09, 28 |
| **Render-to-Texture** | 10, 33 |
| **User Input** | 11, 12 |
| **VBO Mesh** | 28, 34 |
| **Media/Video** | 35 |
| **Audio** | 36 |
| **Shaders** | 33 |
| **Transformations** | All |
| **Animation** | All |

---

## API Coverage

Each example demonstrates specific API categories:

- **Core**: setup/update/draw pattern (all examples)
- **Graphics 2D**: ofDrawCircle, ofDrawRectangle, ofPath, ofPolyline (01, 02)
- **Graphics 3D**: ofDrawBox, ofDrawSphere, ofMesh (06, 08, 09)
- **Color**: ofColor, ofSetColor, ofBackground (01, 02, 03)
- **Image**: ofImage, ofTexture, ofPixels (04, 10)
- **Typography**: ofTrueTypeFont, drawString (05)
- **Camera**: ofCamera, ofEasyCam (07, 12)
- **Lighting**: ofLight, ofMaterial (08)
- **FBO**: ofFbo, render targets (10)
- **Events**: mouse, keyboard, window events (11)
- **Math**: ofVec3f, ofMatrix4x4, ofQuaternion (all 3D examples)
- **Utils**: ofLog, ofGetElapsedTimef, ofRandom (all examples)

---

## Native Technology Integration

oflike-metal uses Apple-native frameworks throughout:

| Framework | Used In | Purpose |
|-----------|---------|---------|
| **Metal** | All | GPU rendering |
| **MetalKit** | All | Texture loading, MTKView |
| **SwiftUI** | All | Window, UI, events |
| **Core Text** | 05 | Font rendering |
| **Core Graphics** | 02, 09 | Path tessellation, image I/O |
| **ImageIO** | 04 | Image loading/saving |
| **simd** | All | Math library (Apple Silicon optimized) |
| **Accelerate** | 03, 09 | SIMD operations |

---

## Performance Notes

All examples are optimized for Apple Silicon:
- **Metal Rendering**: Hardware-accelerated GPU drawing
- **SIMD Math**: Vectorized math operations
- **Unified Memory**: Zero-copy CPU↔GPU transfers
- **Metal Shaders**: Optimized for Apple GPU architecture

Typical performance (M1 Max, 4K display):
- 2D examples: 200-300 FPS
- 3D examples: 60 FPS (vsync limited)
- Complex lighting: 60 FPS
- Large meshes (100K vertices): 60 FPS

---

## Common Patterns

### Basic Application (SwiftUI)
```cpp
// MyApp.cpp
#include <oflike/ofApp.h>

class MyApp : public ofBaseApp {
    void setup() override {
        ofSetFrameRate(60);
        ofBackground(0);
    }

    void update() override {
        // Update logic
    }

    void draw() override {
        // Drawing code
    }
};

extern "C" ofBaseApp* ofCreateApp() {
    return new MyApp();
}
```

```swift
// App.swift
import SwiftUI

@main
struct MyApp: App {
    var body: some Scene {
        WindowGroup {
            MetalView()
        }
    }
}
```

### 3D Scene Setup
```cpp
void setup() override {
    ofEnableDepthTest();
    ofEnableLighting();

    light.setDirectional();
    light.setPosition(100, 200, 100);

    camera.setPosition(0, 100, 300);
    camera.lookAt(ofVec3f(0, 0, 0));
}

void draw() override {
    camera.begin();
    light.enable();
    // Draw 3D content
    light.disable();
    camera.end();
}
```

---

## Troubleshooting

### "Cannot find oflike/ofApp.h"
Update include paths:
```cpp
#include <oflike/ofApp.h>  // Not "ofMain.h"
```

### Black screen / Nothing renders
Check:
- `ofBackground()` is called in `draw()` or `setup()`
- Drawing commands are in `draw()` method
- Window size is appropriate (not 0x0)

### Build errors
Ensure:
- Xcode 15.0+ installed
- macOS 13.0+ deployment target
- C++20 language standard set
- oflike-metal framework linked

### Performance issues
Build in **Release** mode, not Debug:
```bash
cmake --build . --config Release  # Not Debug
```

---

## Next Steps

After completing the examples:

1. **Read the API Documentation**: `docs/api/` for complete reference
2. **Check MIGRATION.md**: If coming from openFrameworks
3. **Read ARCHITECTURE.md**: Understand the technical implementation
4. **Explore the Source**: `src/oflike/` for API implementation
5. **Build Your Project**: Start creating!

---

## Contributing

Found an issue or want to improve an example?
- Open an issue on GitHub
- Submit a pull request
- Ask questions in discussions

---

## License

These examples are part of the oflike-metal project.
See LICENSE file for details.

---

**Happy Creative Coding!** 🎨✨
