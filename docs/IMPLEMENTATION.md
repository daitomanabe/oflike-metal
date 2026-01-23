# oflike-metal Implementation Guide

**Version**: 2.0.0  
**Last Updated**: 2026-01-23  
**Status**: Living Document

> This document provides practical guidance for implementing oflike-metal components.

---

## 1. Development Workflow

### 1.1 Phase-by-Phase Implementation

Follow the phases defined in `CHECKLIST.md`:
- Phase 0-18: Core framework (existing)
- Phase 19-24: Core Addons migration
- Phase 25-30: Apple Native Addons

Each phase builds on the previous one. Complete all tasks in a phase before moving to the next.

### 1.2 Commit Message Format

Use Conventional Commits:
```
<type>(<scope>): <description>

<body>

🤖 Auto-committed by ralph-orchestrator
```

**Types:**
- `feat`: New feature implementation
- `fix`: Bug fix
- `refactor`: Code refactoring
- `docs`: Documentation updates
- `chore`: Other changes

**Scope:** Phase number or component name

---

## 2. Architecture Compliance

### 2.1 Forbidden Libraries

❌ **FreeType** → Use Core Text  
❌ **OpenGL** → Use Metal  
❌ **SDL/GLFW** → Use SwiftUI + MTKView  
❌ **stb_image** → Use ImageIO / MTKTextureLoader  
❌ **AppKit direct usage** → Use SwiftUI  
❌ **GLM (in new code)** → Use simd  
❌ **Poco** → Use Foundation / Network.framework  

### 2.2 Required Patterns

✅ **pImpl Pattern**: Keep Objective-C types out of public headers  
✅ **Pure C++ headers (.h)**: Public API must be pure C++  
✅ **Objective-C++ implementation (.mm)**: Use .mm for Apple frameworks  
✅ **@autoreleasepool**: Wrap Objective-C object creation  
✅ **simd**: Use simd types for math operations  

---

## 3. Core Addons Implementation

### 3.1 ofxOsc

**Dependencies**: oscpack (MIT)

```cpp
// ofxOsc.h - Pure C++ header
#pragma once
#include <string>
#include <functional>
#include <vector>

class ofxOscMessage {
public:
    void setAddress(const std::string& address);
    void addIntArg(int arg);
    void addFloatArg(float arg);
    void addStringArg(const std::string& arg);
    // ...
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

class ofxOscSender {
public:
    void setup(const std::string& host, int port);
    void sendMessage(const ofxOscMessage& msg);
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

class ofxOscReceiver {
public:
    void setup(int port);
    bool hasWaitingMessages();
    bool getNextMessage(ofxOscMessage& msg);
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
```

### 3.2 ofxNetwork

**Dependencies**: Network.framework (Apple)

```cpp
// ofxTcpClient.h
#pragma once
#include <string>
#include <functional>

class ofxTcpClient {
public:
    bool connect(const std::string& host, int port);
    void disconnect();
    bool isConnected() const;
    
    bool send(const std::string& data);
    std::string receive();
    
    std::function<void(const std::string&)> onReceive;
    std::function<void()> onDisconnect;
    
private:
    class Impl;  // Uses nw_connection_t internally
    std::unique_ptr<Impl> impl_;
};
```

### 3.3 ofxOpenCv

**Dependencies**: Vision.framework (primary), OpenCV (optional)

**Strategy**:
- Basic detection (faces, bodies, text) → Vision.framework
- Advanced processing (optical flow, contours) → OpenCV

```cpp
// ofxCv.h
#pragma once
#include "ofPixels.h"
#include "ofRectangle.h"

namespace ofxCv {
    // Vision.framework based
    std::vector<ofRectangle> detectFaces(const ofPixels& image);
    std::vector<ofRectangle> detectBodies(const ofPixels& image);
    std::vector<std::string> detectText(const ofPixels& image);
    
    // OpenCV based (optional)
    class ContourFinder {
    public:
        void findContours(const ofPixels& input);
        std::vector<ofPolyline> getPolylines() const;
    };
}
```

### 3.4 ofxSvg

**Dependencies**: nanosvg (Zlib), Core Graphics

```cpp
// ofxSvg.h
#pragma once
#include "ofPath.h"

class ofxSvg {
public:
    bool load(const std::string& path);
    
    int getNumPaths() const;
    ofPath getPathAt(int index) const;
    
    float getWidth() const;
    float getHeight() const;
    
    void draw() const;
    
private:
    std::vector<ofPath> paths_;
    float width_, height_;
};
```

### 3.5 ofxXmlSettings

**Dependencies**: pugixml (MIT)

```cpp
// ofxXmlSettings.h
#pragma once
#include <string>

class ofxXmlSettings {
public:
    bool load(const std::string& path);
    bool save(const std::string& path);
    
    int getValue(const std::string& tag, int defaultValue);
    float getValue(const std::string& tag, float defaultValue);
    std::string getValue(const std::string& tag, const std::string& defaultValue);
    
    void setValue(const std::string& tag, int value);
    void setValue(const std::string& tag, float value);
    void setValue(const std::string& tag, const std::string& value);
    
    bool pushTag(const std::string& tag, int which = 0);
    void popTag();
    
private:
    class Impl;  // Uses pugi::xml_document internally
    std::unique_ptr<Impl> impl_;
};
```

### 3.6 ofxGui

**Primary**: SwiftUI  
**Debug**: Dear ImGui (optional)

```cpp
// ofxGui.h - C++ interface
#pragma once
#include <string>
#include <functional>

class ofxGuiGroup {
public:
    void add(const std::string& name, float& value, float min, float max);
    void add(const std::string& name, bool& value);
    void add(const std::string& name, int& value, int min, int max);
};

// Bridge to SwiftUI
class ofxPanel {
public:
    void setup(const std::string& name);
    ofxGuiGroup& getGroup();
    void draw();  // No-op (SwiftUI handles rendering)
    
private:
    class Impl;  // Bridges to SwiftUI
    std::unique_ptr<Impl> impl_;
};
```

---

## 4. Apple Native Addons Implementation

### 4.1 ofxSharp (Apple ml-sharp)

**Purpose**: Single image → 3D Gaussian Splatting in < 1 second

**Dependencies**: Core ML, Metal, Neural Engine

**Architecture**:
```
Input Image → Core ML Model → Gaussian Parameters → Metal Renderer → Output
              (Neural Engine)   (positions, scales,   (GPU sorted,
                                rotations, colors)    alpha blended)
```

**Key Classes**:
```cpp
namespace Sharp {
    class GaussianCloud;      // 3DGS data container
    class SharpModel;         // Core ML inference
    class GaussianRenderer;   // Metal rendering
    class SharpScene;         // Scene management
    class CameraPath;         // Animation paths
    class VideoExporter;      // Video output
}

class ofxSharp {
public:
    void setup(int width, int height);
    Sharp::GaussianCloud generateFromImage(const ofPixels& image);
    void drawCloud(const Sharp::GaussianCloud& cloud, const ofCamera& camera);
};
```

**Model Conversion**:
```
PyTorch (.pt) → TorchScript → Core ML (.mlmodelc)
                              ↓
                        Neural Engine optimized
```

### 4.2 ofxNeuralEngine

**Purpose**: ML inference using Neural Engine

**Dependencies**: Core ML, Vision

```cpp
namespace ofxNeuralEngine {
    // Image classification
    class ImageClassifier {
        std::vector<Result> classify(const ofPixels& image, int topK = 5);
    };
    
    // Pose estimation (VNDetectHumanBodyPoseRequest)
    class PoseEstimator {
        std::vector<Body> detect(const ofPixels& image);
        void draw();  // Skeleton visualization
    };
    
    // Depth estimation
    class DepthEstimator {
        ofFloatPixels estimate(const ofPixels& image);
    };
    
    // Style transfer
    class StyleTransfer {
        ofPixels transfer(const ofPixels& content);
    };
    
    // Person segmentation
    class PersonSegmentation {
        ofPixels segment(const ofPixels& image);
    };
}
```

### 4.3 ofxMetalCompute

**Purpose**: Easy-to-use Metal Compute Shaders

**Dependencies**: Metal

```cpp
class ofxMetalCompute {
public:
    bool load(const std::string& shaderPath);
    bool loadFromSource(const std::string& source, const std::string& functionName);
    
    void setBuffer(int index, ofBufferObject& buffer);
    void setTexture(int index, ofTexture& texture, bool write = false);
    void setUniform(const std::string& name, float value);
    
    void dispatch(int threadsX, int threadsY = 1, int threadsZ = 1);
    void dispatch1D(int count);
    void dispatch2D(int width, int height);
    
    void waitUntilCompleted();
    
    template<typename T>
    std::vector<T> readBuffer(int index);
};
```

### 4.4 ofxMPS

**Purpose**: Metal Performance Shaders for optimized image processing

**Dependencies**: MetalPerformanceShaders.framework

```cpp
namespace ofxMPS {
    class ImageFilter {
    public:
        // Blur
        static void gaussianBlur(ofTexture& src, ofTexture& dst, float sigma);
        static void boxBlur(ofTexture& src, ofTexture& dst, int kernelSize);
        
        // Edge detection
        static void sobel(ofTexture& src, ofTexture& dst);
        static void laplacian(ofTexture& src, ofTexture& dst);
        
        // Morphology
        static void erode(ofTexture& src, ofTexture& dst, int kernelSize);
        static void dilate(ofTexture& src, ofTexture& dst, int kernelSize);
        
        // Transform
        static void scale(ofTexture& src, ofTexture& dst, int newWidth, int newHeight);
        static void lanczosScale(ofTexture& src, ofTexture& dst, int w, int h);
    };
}
```

### 4.5 ofxVideoToolbox

**Purpose**: Hardware-accelerated video encoding (4K/8K/ProRes)

**Dependencies**: VideoToolbox, AVFoundation

```cpp
class ofxVideoEncoder {
public:
    enum class Codec { H264, H265, ProRes422, ProRes4444, ProResRAW };
    
    struct Settings {
        Codec codec = Codec::H265;
        int width = 1920;
        int height = 1080;
        float fps = 60.0f;
        int bitrate = 50000000;
        bool useHardware = true;  // Media Engine
    };
    
    bool setup(const std::string& outputPath, const Settings& settings);
    void addFrame(const ofTexture& texture);  // Zero-copy GPU
    void finish();
};
```

### 4.6 ofxSpatialAudio

**Purpose**: 3D spatial audio with PHASE framework

**Dependencies**: PHASE.framework

```cpp
class ofxSpatialAudio {
public:
    void setup();
    
    // Listener
    void setListenerPosition(const glm::vec3& pos);
    void setListenerOrientation(const glm::vec3& forward, const glm::vec3& up);
    
    // Sources
    class Source {
    public:
        void load(const std::string& path);
        void play();
        void setPosition(const glm::vec3& pos);
        void setReverb(float amount);
    };
    
    Source& createSource();
    
    // Environment
    void setReverb(const std::string& preset);
    
    // Head tracking (AirPods Pro/Max)
    void enableHeadTracking(bool enable);
};
```

### 4.7 ofxMetalFX

**Purpose**: AI-based upscaling (e.g., 720p → 4K)

**Dependencies**: MetalFX.framework

```cpp
class ofxMetalFX {
public:
    enum class Mode { Spatial, Temporal };
    
    void setup(int inputW, int inputH, int outputW, int outputH, Mode mode);
    
    // For Temporal mode
    void setMotionVectors(const ofTexture& motionTex);
    void setDepth(const ofTexture& depthTex);
    
    void upscale(const ofTexture& input, ofTexture& output);
};
```

### 4.8 ofxUnifiedMemory

**Purpose**: Zero-copy CPU/GPU buffers on Apple Silicon

**Dependencies**: Metal (Unified Memory)

```cpp
template<typename T>
class ofxUnifiedBuffer {
public:
    void allocate(size_t count);
    
    // CPU access
    T* data();
    T& operator[](size_t index);
    
    // GPU access (no copy needed)
    id<MTLBuffer> getMetalBuffer();
    
    // Iterators
    T* begin();
    T* end();
};
```

### 4.9 ofxLiveText

**Purpose**: OCR and text recognition

**Dependencies**: VisionKit (macOS 14+)

```cpp
class ofxLiveText {
public:
    struct TextBlock {
        std::string text;
        ofRectangle bounds;
        float confidence;
    };
    
    std::vector<TextBlock> recognize(const ofPixels& image);
    std::vector<std::string> recognizeBarcodes(const ofPixels& image);
    
    void setLanguages(const std::vector<std::string>& langs);
};
```

### 4.10 ofxObjectCapture

**Purpose**: Photos to 3D model

**Dependencies**: Object Capture API (macOS 12+)

```cpp
class ofxObjectCapture {
public:
    enum class Quality { Preview, Reduced, Medium, Full, Raw };
    
    void processImages(const std::string& folderPath);
    
    float getProgress() const;
    bool isComplete() const;
    
    ofMesh getMesh() const;
    ofTexture getTexture() const;
    
    void exportUSDZ(const std::string& path);
};
```

---

## 5. Implementation Patterns

### 5.1 Core ML Integration Pattern

```objc
// ofxNeuralEngine.mm
#import <CoreML/CoreML.h>
#import <Vision/Vision.h>

class ClassName::Impl {
    MLModel* model_;
    
public:
    bool load(const std::string& path) {
        @autoreleasepool {
            NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
            NSURL* url = [NSURL fileURLWithPath:nsPath];
            
            MLModelConfiguration* config = [[MLModelConfiguration alloc] init];
            config.computeUnits = MLComputeUnitsAll;  // Prefer Neural Engine
            
            NSError* error = nil;
            model_ = [MLModel modelWithContentsOfURL:url 
                                       configuration:config 
                                               error:&error];
            return (error == nil);
        }
    }
};
```

### 5.2 Metal Compute Pattern

```objc
// ofxMetalCompute.mm
#import <Metal/Metal.h>

class ofxMetalCompute::Impl {
    id<MTLDevice> device_;
    id<MTLCommandQueue> queue_;
    id<MTLComputePipelineState> pipeline_;
    
public:
    void dispatch(int x, int y, int z) {
        @autoreleasepool {
            id<MTLCommandBuffer> buffer = [queue_ commandBuffer];
            id<MTLComputeCommandEncoder> encoder = [buffer computeCommandEncoder];
            
            [encoder setComputePipelineState:pipeline_];
            // Set buffers, textures...
            
            MTLSize threadgroups = MTLSizeMake(x, y, z);
            MTLSize threadsPerGroup = MTLSizeMake(256, 1, 1);
            [encoder dispatchThreadgroups:threadgroups 
                    threadsPerThreadgroup:threadsPerGroup];
            
            [encoder endEncoding];
            [buffer commit];
        }
    }
};
```

### 5.3 Async Operation Pattern

```cpp
// Async with callback
void processAsync(const ofPixels& input, std::function<void(Result)> callback) {
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        Result result = process(input);  // Heavy computation
        
        dispatch_async(dispatch_get_main_queue(), ^{
            callback(result);  // Callback on main thread
        });
    });
}
```

---

## 6. Testing Strategy

### 6.1 Addon Testing

For each addon:
1. **Unit tests**: Core functionality
2. **Integration tests**: With oflike core
3. **Visual tests**: Example programs
4. **Performance tests**: Benchmarks

### 6.2 Example Structure

```
examples/
├── addons/
│   ├── osc_sender/
│   ├── osc_receiver/
│   ├── sharp_basic/
│   ├── sharp_scene/
│   ├── neural_pose/
│   ├── compute_particles/
│   ├── mps_filters/
│   ├── video_export/
│   ├── spatial_audio/
│   └── live_text/
```

---

## 7. Documentation Standards

### 7.1 API Documentation

```cpp
/// Generates 3D Gaussian Splatting from a single image
/// 
/// Uses Apple's SHARP model running on Neural Engine for fast inference.
/// Typical inference time is < 1 second on Apple Silicon.
///
/// @param image Input image (RGB, any resolution)
/// @return GaussianCloud containing 3D gaussian primitives
/// @note Requires macOS 13.0+ and Apple Silicon for Neural Engine acceleration
Sharp::GaussianCloud ofxSharp::generateFromImage(const ofPixels& image);
```

### 7.2 Migration Notes

For each addon, document:
- Original oF addon API
- Changes in Mac Native version
- Migration steps
- Known limitations

---

## 8. Priority Order

### 8.1 Core Addons (Phase 19-24)

1. **ofxOsc** - Essential for creative coding
2. **ofxGui** - SwiftUI integration
3. **ofxXmlSettings** - Settings persistence
4. **ofxSvg** - Vector graphics
5. **ofxNetwork** - TCP/UDP
6. **ofxOpenCv** - Computer vision

### 8.2 Apple Native Addons (Phase 25-30)

1. **ofxSharp** - Flagship feature (3DGS)
2. **ofxNeuralEngine** - ML inference
3. **ofxMetalCompute** - GPU compute
4. **ofxMPS** - Image processing
5. **ofxVideoToolbox** - Video export
6. **ofxSpatialAudio** - 3D audio
7. **ofxMetalFX** - Upscaling
8. **ofxUnifiedMemory** - Performance
9. **ofxLiveText** - OCR
10. **ofxObjectCapture** - 3D scanning

---

## References

- [ARCHITECTURE.md](ARCHITECTURE.md) - Project architecture
- [CHECKLIST.md](CHECKLIST.md) - Development checklist
- [THIRDPARTY.md](THIRDPARTY.md) - Dependencies
- [Apple Metal Documentation](https://developer.apple.com/metal/)
- [Apple Core ML Documentation](https://developer.apple.com/documentation/coreml)
- [Apple Vision Documentation](https://developer.apple.com/documentation/vision)
- [Apple SHARP Repository](https://github.com/apple/ml-sharp)
