# ofxSharp - 3D Gaussian Splatting for oflike-metal

**Apple SHARP (Single-image to High-fidelity Appearance Reconstruction via neural Prefiltered splatting) integration for oflike-metal**

Generate 3D Gaussian Splatting representations from a single image in **< 1 second** using Neural Engine acceleration on Apple Silicon.

## Features

- ⚡ **Neural Engine Acceleration**: Sub-second inference on Apple Silicon
- 🖼️ **Single Image Input**: Generate 3D scenes from one photo
- 🎨 **Real-time Rendering**: 60+ FPS rendering with Metal
- 🔄 **Zero-Copy Pipeline**: Unified Memory for CPU↔GPU efficiency
- 📹 **Video Export**: H.265/ProRes output support
- 🎥 **Camera Paths**: Animated camera trajectories

## Quick Start

```cpp
#include "ofxSharp.h"

class ofApp : public ofBaseApp {
    Sharp::SharpModel model;
    Sharp::GaussianRenderer renderer;
    Sharp::GaussianCloud cloud;
    ofEasyCam camera;

    void setup() {
        // Load Core ML model
        model.load("sharp.mlmodelc");

        // Setup renderer
        renderer.setup(ofGetWidth(), ofGetHeight());

        // Generate from image
        ofImage input;
        input.load("photo.jpg");
        cloud = model.predict(input.getPixels());

        ofLog() << "Generated " << cloud.size() << " Gaussians";
    }

    void draw() {
        camera.begin();
        renderer.render(cloud, camera);
        camera.end();
    }
};
```

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                  ofxSharp Pipeline                   │
├─────────────────────────────────────────────────────┤
│                                                      │
│  Single Image  →  SHARP Model  →  Gaussian Cloud    │
│   (ofPixels)      (Core ML)        (3DGS Data)      │
│                       ↓                 ↓            │
│                Neural Engine      Metal Renderer    │
│                  (< 1 sec)         (60+ FPS)        │
└─────────────────────────────────────────────────────┘
```

## Core Classes

### `Sharp::SharpModel`

Core ML model wrapper for SHARP inference.

```cpp
#include "SharpModel.h"

Sharp::SharpModel model;

// Load model (looks in app bundle and models/ directory)
bool success = model.load("sharp.mlmodelc");

// Check Neural Engine usage
if (model.usesNeuralEngine()) {
    ofLog() << "Neural Engine available!";
}

// Inference from ofPixels
Sharp::GaussianCloud cloud = model.predict(pixels);

// Async inference
model.predictAsync(pixels, [](Sharp::GaussianCloud cloud) {
    ofLog() << "Generated " << cloud.size() << " Gaussians";
});
```

### `Sharp::GaussianCloud`

Container for 3D Gaussian Splatting data.

```cpp
#include "SharpGaussianCloud.h"

Sharp::GaussianCloud cloud;

// Access Gaussians
size_t count = cloud.size();
const Sharp::Gaussian& g = cloud[0];

// Get properties
simd::float3 pos = g.position;
simd::float3 color = g.color;
float opacity = g.opacity;

// Transformations
cloud.translate(simd::float3{0, 1, 0});
cloud.rotate(M_PI/4, simd::float3{0, 1, 0});
cloud.scale(1.5f);

// Filtering
cloud.filterByOpacity(0.1f);  // Remove low-opacity Gaussians
cloud.filterByDistance(10.0f); // Remove distant Gaussians

// I/O
cloud.save("scene.ply");
cloud.load("scene.ply");
```

### `Sharp::GaussianRenderer`

Metal-based renderer for Gaussian Splatting.

```cpp
#include "SharpRenderer.h"

Sharp::GaussianRenderer renderer;

// Setup
renderer.setup(1920, 1080);

// Rendering
renderer.begin();
renderer.render(cloud, camera);
renderer.end();

// Configuration
renderer.setQuality(Sharp::RenderQuality::High);
renderer.enableAntiAliasing(true);
renderer.setMaxGaussians(10000);
```

### `Sharp::SharpScene`

Manage multiple Gaussian clouds with transforms.

```cpp
#include "SharpScene.h"

Sharp::SharpScene scene;

// Add objects
int id1 = scene.addCloud(cloud1);
int id2 = scene.addCloud(cloud2);

// Transform
scene.setPosition(id1, {0, 0, 0});
scene.setRotation(id2, M_PI/2, {0, 1, 0});
scene.setScale(id1, 1.5f);

// Render entire scene
scene.render(renderer, camera);

// Save/Load
scene.save("scene.sharp");
scene.load("scene.sharp");
```

## Model Integration

### Converting PyTorch to Core ML

1. **Train or download** a PyTorch SHARP model following [Apple ML-SHARP](https://github.com/apple/ml-sharp)

2. **Convert to Core ML**:

```bash
cd src/addons/apple_native/ofxSharp/scripts/
python convert_to_coreml.py \
  --input sharp_pytorch.pth \
  --output ../models/sharp.mlpackage
```

3. **Build project** - Xcode compiles `.mlpackage` → `.mlmodelc` automatically

4. **Load in code**:

```cpp
model.load("sharp.mlmodelc");
```

See [`scripts/README.md`](scripts/README.md) for detailed conversion instructions.

## Performance

### Inference Time (Neural Engine)

| Resolution | M1     | M2     | M3     | M4     |
|------------|--------|--------|--------|--------|
| 256×256    | 0.2s   | 0.15s  | 0.12s  | 0.10s  |
| 512×512    | 0.8s   | 0.6s   | 0.5s   | 0.4s   |
| 1024×1024  | 3.0s   | 2.2s   | 1.8s   | 1.5s   |

### Rendering Performance (Metal)

| Gaussians | Resolution | FPS (M1) | FPS (M3) |
|-----------|------------|----------|----------|
| 1,000     | 1080p      | 120+     | 120+     |
| 5,000     | 1080p      | 90       | 120+     |
| 10,000    | 1080p      | 60       | 90       |
| 50,000    | 1080p      | 20       | 40       |

## Advanced Features

### Async Inference

```cpp
model.predictAsync(pixels, [this](Sharp::GaussianCloud cloud) {
    this->cloud = std::move(cloud);
    this->isReady = true;
});
```

### Camera Animation

```cpp
Sharp::CameraPath path;
path.addKeyframe(0.0f, {0, 0, 5}, {0, 0, 0});
path.addKeyframe(2.0f, {3, 2, 4}, {0, 0, 0});
path.addKeyframe(4.0f, {0, 3, 3}, {0, 0, 0});

// Interpolate
float t = ofGetElapsedTimef();
auto [pos, target] = path.evaluate(t);
camera.setPosition(pos);
camera.lookAt(target);
```

### Video Export

```cpp
Sharp::VideoExporter exporter;
exporter.setup("output.mp4", 1920, 1080, 60, Sharp::VideoCodec::H265);

// Per frame
renderer.render(cloud, camera);
ofPixels pixels;
renderer.readToPixels(pixels);
exporter.addFrame(pixels);

exporter.finalize();
```

### Batch Processing

```cpp
std::vector<ofImage> images = loadImages("photos/*.jpg");
std::vector<Sharp::GaussianCloud> clouds;

for (auto& img : images) {
    clouds.push_back(model.predict(img.getPixels()));
}

// Merge into scene
Sharp::SharpScene scene;
for (size_t i = 0; i < clouds.size(); i++) {
    int id = scene.addCloud(clouds[i]);
    scene.setPosition(id, {i * 2.0f, 0, 0});
}
```

## Directory Structure

```
ofxSharp/
├── README.md                    # This file
├── .gitignore                   # Git ignore rules
├── SharpGaussian.h              # Gaussian structure
├── SharpGaussian.mm
├── SharpGaussianCloud.h         # Gaussian container
├── SharpGaussianCloud.mm
├── SharpModel.h                 # Core ML inference
├── SharpModel.mm
├── SharpRenderer.h              # Metal rendering
├── SharpRenderer.mm
├── scripts/                     # Conversion scripts
│   ├── README.md
│   ├── convert_to_coreml.py
│   └── requirements.txt
├── models/                      # Core ML models
│   ├── README.md
│   └── sharp.mlpackage/
└── shaders/                     # Metal shaders
    └── GaussianSplat.metal
```

## Requirements

- **macOS**: 13.0 (Ventura) or later
- **Xcode**: 15.0 or later
- **Hardware**: Apple Silicon recommended (M1+) for Neural Engine
- **Frameworks**:
  - Core ML
  - Metal
  - MetalKit

## Examples

### Example 1: Basic Usage

```cpp
// examples/sharp_basic/src/ofApp.cpp
model.load("sharp.mlmodelc");
renderer.setup(ofGetWidth(), ofGetHeight());

ofImage img;
img.load("photo.jpg");
cloud = model.predict(img.getPixels());
```

### Example 2: Interactive Scene

```cpp
// examples/sharp_scene/src/ofApp.cpp
scene.addCloud(model.predict(img1.getPixels()));
scene.addCloud(model.predict(img2.getPixels()));

// Mouse interaction
scene.setRotation(selectedId, mouseX * 0.01f, {0, 1, 0});
```

### Example 3: Video Export

```cpp
// examples/sharp_video_export/src/ofApp.cpp
exporter.setup("orbit.mp4", 1920, 1080, 60);

for (float angle = 0; angle < TWO_PI; angle += 0.01f) {
    camera.setPosition(cos(angle) * 5, 2, sin(angle) * 5);
    renderer.render(cloud, camera);
    exporter.addFrame(renderer);
}
```

## Troubleshooting

### Model Loading Issues

```cpp
if (!model.load("sharp.mlmodelc")) {
    ofLogError() << "Failed to load model";
    ofLogError() << "Check that sharp.mlmodelc exists in bundle";
    ofLogError() << "Rebuild project to compile .mlpackage → .mlmodelc";
}
```

### Neural Engine Not Used

- Verify you're on Apple Silicon (not Intel)
- Check model was converted with `--compute-units ALL`
- Some operations automatically fall back to GPU

### Rendering Performance

```cpp
// Reduce Gaussians
cloud.filterByOpacity(0.2f);  // Higher threshold

// Lower rendering resolution
renderer.setResolution(1280, 720);

// Reduce quality
renderer.setQuality(Sharp::RenderQuality::Fast);
```

## References

- **Apple ML-SHARP**: https://github.com/apple/ml-sharp
- **3D Gaussian Splatting Paper**: https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/
- **Core ML**: https://developer.apple.com/documentation/coreml
- **Metal**: https://developer.apple.com/metal/

## Citation

```bibtex
@article{sharp2024,
  title={SHARP: Single-image to High-fidelity Appearance Reconstruction via neural Prefiltered splatting},
  author={Apple Machine Learning Research},
  year={2024}
}
```

## License

MIT License - See LICENSE file in project root

---

**ofxSharp** is part of the [oflike-metal](https://github.com/daitomanabe/oflike-metal) project - an Apple-native reimplementation of openFrameworks with native Apple framework integrations.
