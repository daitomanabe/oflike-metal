# oflike-metal Third-Party Dependencies

**Version**: 2.0.0  
**Last Updated**: 2026-01-23  
**Status**: Active

> This document lists all third-party libraries used in oflike-metal, their licenses, and integration guidelines.

---

## 1. Overview

oflike-metal minimizes external dependencies by leveraging Apple's native frameworks. Only essential libraries are included.

### 1.1 Dependency Philosophy

- **Prefer Apple frameworks**: Use Metal, Core ML, Vision, Core Text, etc.
- **Minimize bloat**: Only add libraries when necessary
- **License compatibility**: All dependencies must be permissive (MIT, BSD, Apache, Zlib)
- **Header-only preferred**: Simplifies integration

---

## 2. Core Dependencies

### 2.1 tess2 (Tessellation)

**Purpose**: Polygon tessellation for path filling

| 項目 | 値 |
|------|-----|
| License | SGI Free Software License B (permissive) |
| Source | https://github.com/memononen/tess2 |
| Version | Latest stable |
| Status | ✅ Active |

**Usage**:
- `ofPath::getTessellation()` - Convert paths to triangles
- Shape filling with holes
- Complex polygon rendering

---

### 2.2 utf8-cpp (UTF-8 String Handling)

**Purpose**: UTF-8 string processing for text rendering

| 項目 | 値 |
|------|-----|
| License | Boost Software License 1.0 (permissive) |
| Source | https://github.com/nemtrif/utfcpp |
| Version | v4.x or later |
| Status | ✅ Active |
| Type | Header-only |

**Usage**:
- String length calculation for fonts
- Character iteration for text layout
- UTF-8 validation
- Unicode code point extraction

---

## 3. Addon Dependencies

### 3.1 oscpack (OSC Protocol)

**Purpose**: OSC protocol implementation for ofxOsc

| 項目 | 値 |
|------|-----|
| License | MIT |
| Source | https://github.com/RossBencina/oscpack |
| Status | ✅ Active |
| Used by | ofxOsc |

**Usage**:
- OSC message sending/receiving
- Parameter synchronization with external apps (TouchOSC, Max, SuperCollider)

---

### 3.2 nanosvg (SVG Parsing)

**Purpose**: SVG parsing for ofxSvg

| 項目 | 値 |
|------|-----|
| License | Zlib |
| Source | https://github.com/memononen/nanosvg |
| Status | ✅ Active |
| Type | Header-only (single file) |
| Used by | ofxSvg |

**Usage**:
- SVG file loading
- Path extraction
- Color/style parsing

---

### 3.3 pugixml (XML Parsing)

**Purpose**: XML parsing for ofxXmlSettings

| 項目 | 値 |
|------|-----|
| License | MIT |
| Source | https://github.com/zeux/pugixml |
| Status | ✅ Active |
| Type | Header-only option available |
| Used by | ofxXmlSettings |

**Usage**:
- XML file reading/writing
- Settings persistence
- Data serialization

---

### 3.4 Dear ImGui (Debug GUI) - Optional

**Purpose**: Debug overlay for development

| 項目 | 値 |
|------|-----|
| License | MIT |
| Source | https://github.com/ocornut/imgui |
| Status | ✅ Optional |
| Used by | ofxGui (debug mode) |
| Note | Metal backend included |

**Usage**:
- Debug parameter adjustment
- Performance monitoring
- Development-time UI

---

### 3.5 OpenCV (Computer Vision) - Optional

**Purpose**: Advanced computer vision for ofxOpenCv

| 項目 | 値 |
|------|-----|
| License | Apache 2.0 / BSD |
| Source | https://opencv.org |
| Status | ✅ Optional |
| Used by | ofxOpenCv (advanced features) |
| Note | Use Vision.framework for basic detection |

**Usage**:
- Optical flow
- Contour detection
- Advanced image processing

---

## 4. Apple Native Frameworks (Required)

These are part of the Apple SDK and do not require separate attribution:

### 4.1 Core Frameworks

| Framework | Purpose | Min Version |
|-----------|---------|-------------|
| **Metal** | GPU rendering, compute shaders | macOS 13.0 |
| **MetalKit** | MTKView, texture loading | macOS 13.0 |
| **MetalFX** | Upscaling, super resolution | macOS 13.0 |
| **Metal Performance Shaders** | Optimized image processing | macOS 13.0 |
| **Core ML** | Machine learning inference | macOS 13.0 |
| **Vision** | Image analysis, detection | macOS 13.0 |
| **VisionKit** | OCR, Live Text | macOS 13.0 |

### 4.2 Media Frameworks

| Framework | Purpose | Min Version |
|-----------|---------|-------------|
| **AVFoundation** | Video playback/capture | macOS 13.0 |
| **VideoToolbox** | Hardware video encoding | macOS 13.0 |
| **PHASE** | Spatial audio | macOS 13.0 |
| **AVAudioEngine** | Audio processing | macOS 13.0 |

### 4.3 Graphics & UI Frameworks

| Framework | Purpose | Min Version |
|-----------|---------|-------------|
| **SwiftUI** | Window management, UI | macOS 13.0 |
| **Core Graphics** | 2D rendering, PDF | macOS 13.0 |
| **Core Text** | Font rendering | macOS 13.0 |
| **ImageIO** | Image file I/O | macOS 13.0 |
| **QuartzCore** | Animation, CAMetalLayer | macOS 13.0 |

### 4.4 System Frameworks

| Framework | Purpose | Min Version |
|-----------|---------|-------------|
| **Accelerate** | simd, vDSP, BNNS | macOS 13.0 |
| **Foundation** | String, File I/O | macOS 13.0 |
| **Network** | TCP/UDP networking | macOS 13.0 |
| **Model I/O** | 3D model loading | macOS 13.0 |

### 4.5 Apple Silicon Specific

| Framework | Purpose | Hardware |
|-----------|---------|----------|
| **Neural Engine** | ML acceleration | Apple Silicon |
| **Media Engine** | Video encode/decode | Apple Silicon |
| **Unified Memory** | Zero-copy buffers | Apple Silicon |

---

## 5. Addon → Framework Mapping

### 5.1 Core Addons Migration

| oF Addon | Original Dep | Mac Native Replacement |
|----------|--------------|------------------------|
| ofxGui | Custom | SwiftUI / Dear ImGui |
| ofxNetwork | Poco | Network.framework |
| ofxOsc | oscpack | oscpack (継続) |
| ofxOpenCv | OpenCV | Vision.framework + OpenCV |
| ofxSvg | svgTiny | Core Graphics + nanosvg |
| ofxXmlSettings | Poco XML | pugixml |
| ofxAssimpModelLoader | Assimp | Model I/O |
| ofxThreadedImageLoader | FreeImage | GCD + ImageIO |
| ofxVectorGraphics | Cairo | Core Graphics (PDF) |

### 5.2 Apple Native Addons (New)

| Addon | Apple Framework | Purpose |
|-------|-----------------|---------|
| **ofxSharp** | Core ML + Metal | Single image → 3D Gaussian Splatting |
| **ofxNeuralEngine** | Core ML / Vision | ML inference, pose estimation |
| **ofxMetalCompute** | Metal Compute | GPU compute shaders |
| **ofxMPS** | Metal Performance Shaders | Optimized image processing |
| **ofxVideoToolbox** | VideoToolbox | 4K/8K/ProRes encoding |
| **ofxSpatialAudio** | PHASE | 3D spatial audio |
| **ofxMetalFX** | MetalFX | AI upscaling |
| **ofxUnifiedMemory** | Metal Shared Memory | Zero-copy CPU/GPU buffers |
| **ofxLiveText** | VisionKit | OCR, text recognition |
| **ofxObjectCapture** | Object Capture API | Photo → 3D model |

---

## 6. Explicitly Forbidden Libraries

These libraries are **NOT** allowed in oflike-metal:

| Library | Reason | Alternative |
|---------|--------|-------------|
| ❌ FreeType | Use native | Core Text |
| ❌ OpenGL / GLEW / GLFW | Use Metal | Metal, MetalKit |
| ❌ SDL | Use SwiftUI | SwiftUI + MTKView |
| ❌ stb_image | Use native | ImageIO / MTKTextureLoader |
| ❌ stb_image_write | Use native | CGImageDestination |
| ❌ Cairo | Use native | Core Graphics |
| ❌ Poco | Use native | Foundation / Network.framework |
| ❌ Boost | Use std/native | C++17 std / Foundation |
| ❌ FBX SDK / Assimp | Use native | Model I/O |

**Exception**: stb_image_write may be used for EXR/HDR formats only.

---

## 7. License Compliance

### 7.1 License Summary

| Library | License | Commercial Use |
|---------|---------|----------------|
| tess2 | SGI FSL B | ✅ OK |
| utf8-cpp | Boost | ✅ OK |
| oscpack | MIT | ✅ OK |
| nanosvg | Zlib | ✅ OK |
| pugixml | MIT | ✅ OK |
| Dear ImGui | MIT | ✅ OK |
| OpenCV | Apache 2.0/BSD | ✅ OK |

### 7.2 Forbidden Licenses

- ❌ GPL (incompatible with App Store)
- ❌ LGPL (linking restrictions)
- ❌ AGPL (network restrictions)
- ❌ Any copyleft license

---

## 8. Adding New Dependencies

### 8.1 Approval Checklist

Before adding a new dependency:

- [ ] Check if Apple framework can solve this
- [ ] License is permissive (MIT, BSD, Apache, Zlib, Boost)
- [ ] No GPL/LGPL/AGPL
- [ ] Header-only preferred
- [ ] Compatible with CMake
- [ ] Works on macOS 13+
- [ ] Documented in THIRDPARTY.md
- [ ] Updated CMakeLists.txt
- [ ] License file copied

### 8.2 Decision Framework

```
Need functionality?
    │
    ├─ Apple framework available? ──► Use Apple framework
    │
    ├─ Can implement in < 500 lines? ──► Implement ourselves
    │
    └─ Must use third-party? ──► Check license ──► Add with approval
```

---

## 9. Directory Structure

```
thirdparty/
├── tess2/
│   ├── Include/
│   ├── Source/
│   └── LICENSE.txt
├── utf8cpp/
│   ├── source/
│   └── LICENSE
├── glm/                    # Transitional
│   ├── glm/
│   └── copying.txt
├── oscpack/
│   ├── ip/
│   ├── osc/
│   └── LICENSE
├── nanosvg/
│   ├── src/
│   └── LICENSE.txt
├── pugixml/
│   ├── src/
│   └── LICENSE.md
└── imgui/                  # Optional
    ├── *.cpp
    ├── *.h
    ├── backends/
    └── LICENSE.txt
```

---

## 10. Summary Table

| Library | Purpose | License | Status |
|---------|---------|---------|--------|
| tess2 | Tessellation | SGI FSL B | ✅ Active |
| utf8-cpp | UTF-8 handling | Boost | ✅ Active |
| oscpack | OSC protocol | MIT | ✅ Active |
| nanosvg | SVG parsing | Zlib | ✅ Active |
| pugixml | XML parsing | MIT | ✅ Active |
| Dear ImGui | Debug UI | MIT | ✅ Optional |
| OpenCV | Computer vision | Apache/BSD | ✅ Optional |

**Primary Strategy**: Apple Frameworks First

**Fallback**: Permissive-licensed, header-only libraries

---

## Appendix: Framework Availability by macOS Version

| Framework | macOS 13 | macOS 14 | macOS 15 |
|-----------|----------|----------|----------|
| Metal 3 | ✅ | ✅ | ✅ |
| MetalFX | ✅ | ✅ | ✅ |
| Core ML 4+ | ✅ | ✅ | ✅ |
| Vision | ✅ | ✅ | ✅ |
| PHASE | ✅ | ✅ | ✅ |
| Object Capture | ✅ | ✅ | ✅ |
| VisionKit (macOS) | - | ✅ | ✅ |
| Apple Intelligence | - | - | ✅ |
