# oflike-metal Third-Party Dependencies

**Version**: 1.0.0
**Last Updated**: 2026-01-23
**Status**: Active

> This document lists all third-party libraries used in oflike-metal, their licenses, and integration guidelines.

---

## 1. Overview

oflike-metal minimizes external dependencies by leveraging Apple's native frameworks. Only essential libraries are included.

### 1.1 Dependency Philosophy

- **Prefer Apple frameworks**: Use Metal, Core Graphics, Core Text, etc.
- **Minimize bloat**: Only add libraries when necessary
- **License compatibility**: All dependencies must be permissive (MIT, BSD, Apache)
- **Header-only preferred**: Simplifies integration

---

## 2. Required Dependencies

### 2.1 tess2 (Tessellation)

**Purpose**: Polygon tessellation for path filling

**License**: SGI Free Software License B (permissive)

**Source**: https://github.com/memononen/tess2

**Version**: Latest stable

**Usage**:
- `ofPath::getTessellation()` - Convert paths to triangles
- Shape filling with holes
- Complex polygon rendering

**Integration**:
```cmake
# CMakeLists.txt
add_subdirectory(thirdparty/tess2)
target_link_libraries(oflike-metal PRIVATE tess2)
```

**Files**:
```
thirdparty/tess2/
├── Include/
│   └── tesselator.h
└── Source/
    ├── bucketalloc.c
    ├── dict.c
    ├── geom.c
    ├── mesh.c
    ├── priorityq.c
    ├── sweep.c
    └── tess.c
```

**Example**:
```cpp
#include <tesselator.h>

void tessellatePath(const ofPath& path) {
    TESStesselator* tess = tessNewTess(nullptr);
    // Add contours from path
    tessTesselate(tess, TESS_WINDING_ODD, TESS_POLYGONS, 3, 2, nullptr);
    // Extract triangles
    tessDeleteTess(tess);
}
```

---

### 2.2 utf8-cpp (UTF-8 String Handling)

**Purpose**: UTF-8 string processing for text rendering

**License**: Boost Software License 1.0 (permissive)

**Source**: https://github.com/nemtrif/utfcpp

**Version**: v4.x or later

**Usage**:
- String length calculation for fonts
- Character iteration for text layout
- UTF-8 validation
- Unicode code point extraction

**Integration**:
```cmake
# Header-only library
target_include_directories(oflike-metal PRIVATE
    thirdparty/utf8cpp/source)
```

**Files**:
```
thirdparty/utf8cpp/
└── source/
    └── utf8.h
    └── utf8/
        ├── checked.h
        ├── core.h
        └── unchecked.h
```

**Example**:
```cpp
#include <utf8.h>

int getUTF8Length(const std::string& str) {
    return utf8::distance(str.begin(), str.end());
}

std::vector<uint32_t> getCodePoints(const std::string& str) {
    std::vector<uint32_t> result;
    utf8::utf8to32(str.begin(), str.end(), std::back_inserter(result));
    return result;
}
```

---

### 2.3 GLM (Transitional - To Be Removed in Phase 18)

**Purpose**: Math library (TEMPORARY until simd migration complete)

**License**: MIT License

**Source**: https://github.com/g-truc/glm

**Version**: 0.9.9.x

**Status**: ⚠️ **TRANSITIONAL** - Will be removed in Phase 18

**Usage** (LIMITED):
- Only used in legacy code during migration
- New code MUST use simd instead
- Should only appear in `.cpp` files, never in `.h` headers

**Migration Timeline**:
- Phase 0-17: GLM allowed in implementation files only
- Phase 18: Complete removal, full simd migration

**Integration**:
```cmake
# Header-only library
target_include_directories(oflike-metal PRIVATE
    thirdparty/glm)
```

**Files**:
```
thirdparty/glm/
├── glm/
│   ├── glm.hpp
│   ├── gtc/
│   └── gtx/
```

**Migration Example**:
```cpp
// OLD (GLM - to be replaced)
#include <glm/glm.hpp>
glm::vec3 cross = glm::cross(a, b);

// NEW (simd - use in all new code)
#include <simd/simd.h>
simd_float3 cross = simd_cross(a, b);
```

---

## 3. Apple Native Frameworks (Not Third-Party)

These are part of the Apple SDK and do not require separate attribution:

### 3.1 Metal Framework
- GPU rendering
- Shaders
- Compute

### 3.2 MetalKit
- MTKView for rendering surface
- MTKTextureLoader for image loading
- MTKMesh for model loading (future)

### 3.3 QuartzCore
- CAMetalLayer
- Animation timing

### 3.4 Core Graphics
- Image decoding
- Color spaces
- PDF rendering (future)

### 3.5 Core Text
- Font rendering
- Text layout
- Glyph extraction

### 3.6 ImageIO
- Image file format support
- PNG, JPEG, TIFF, etc.

### 3.7 Accelerate
- simd vector math
- vDSP for audio (future)
- BNNS for ML (future)

### 3.8 Foundation
- String handling
- File I/O
- Data structures

### 3.9 AppKit
- MTKView base class (NSView)
- Limited usage only

### 3.10 SwiftUI
- Window management
- Event handling
- UI layout

---

## 4. Explicitly Forbidden Libraries

These libraries are **NOT** allowed in oflike-metal:

### 4.1 ❌ FreeType
**Why**: Use Core Text instead
**Alternative**: CTFont, CTLine, CTFrame

### 4.2 ❌ OpenGL / GLEW / GLFW
**Why**: Use Metal instead
**Alternative**: Metal, MetalKit, MTKView

### 4.3 ❌ SDL
**Why**: Use SwiftUI + MTKView instead
**Alternative**: SwiftUI, NSViewRepresentable, MTKView

### 4.4 ❌ stb_image / stb_image_write
**Why**: Use ImageIO / MTKTextureLoader instead
**Alternative**: CGImageSource, CGImageDestination, MTKTextureLoader

### 4.5 ❌ GLFW
**Why**: Use SwiftUI instead
**Alternative**: SwiftUI WindowGroup, MTKView

### 4.6 ❌ Boost (except Boost.Filesystem if needed)
**Why**: Use C++17 standard library or Apple frameworks
**Alternative**: std::filesystem, Foundation

### 4.7 ❌ FBX SDK / Assimp (for now)
**Why**: Phase 0-12 focus on 2D/3D primitives only
**Note**: May be reconsidered for advanced 3D model support

---

## 5. License Compliance

### 5.1 License Texts

Full license texts for all dependencies must be included:
```
thirdparty/
├── tess2/
│   └── LICENSE.txt
├── utf8cpp/
│   └── LICENSE
└── glm/
    └── copying.txt
```

### 5.2 Attribution

Include attribution in:
- `README.md` - Credits section
- `LICENSE` file - Combined license file
- About dialog (if GUI is added)

### 5.3 Example Attribution

```markdown
## Third-Party Libraries

oflike-metal uses the following open-source libraries:

- **tess2** by Mikko Mononen - SGI Free Software License B
- **utf8-cpp** by Nemanja Trifunovic - Boost Software License 1.0
- **GLM** by G-Truc Creation - MIT License (transitional, will be removed)
```

---

## 6. Adding New Dependencies

### 6.1 Approval Process

Before adding a new dependency:

1. **Check if necessary**
   - Can Apple frameworks solve this?
   - Can we implement it ourselves (if small)?
   - Is it really needed now, or can it wait?

2. **License check**
   - Must be permissive (MIT, BSD, Apache, Boost, etc.)
   - No GPL/LGPL (incompatible with App Store)
   - No copyleft restrictions

3. **Integration check**
   - Header-only preferred
   - No complex build systems
   - Compatible with CMake
   - Works on macOS 11+

4. **Documentation**
   - Add to this file (THIRDPARTY.md)
   - Document in IMPLEMENTATION.md
   - Update CMakeLists.txt
   - Copy license file

### 6.2 Dependency Checklist

- [ ] Checked Apple frameworks first
- [ ] License is permissive
- [ ] Added to `thirdparty/` directory
- [ ] Updated THIRDPARTY.md
- [ ] Updated CMakeLists.txt
- [ ] Copied LICENSE file
- [ ] Tested build
- [ ] Documented usage

---

## 7. Dependency Management

### 7.1 Version Pinning

Pin dependency versions for stability:
```cmake
# CMakeLists.txt
FetchContent_Declare(
  tess2
  GIT_REPOSITORY https://github.com/memononen/tess2.git
  GIT_TAG        master  # TODO: Pin to specific commit
)
```

### 7.2 Updating Dependencies

When updating:
1. Test thoroughly
2. Check for API changes
3. Update version in this document
4. Commit with clear message

### 7.3 Removing Dependencies

When removing (like GLM in Phase 18):
1. Identify all usage
2. Replace with alternative
3. Remove from CMakeLists.txt
4. Remove from thirdparty/
5. Update this document
6. Test all examples

---

## 8. Build Integration

### 8.1 CMake Configuration

```cmake
# thirdparty/CMakeLists.txt
add_subdirectory(tess2)

# utf8cpp is header-only
add_library(utf8cpp INTERFACE)
target_include_directories(utf8cpp INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/utf8cpp/source)

# GLM is header-only (transitional)
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/glm)
```

### 8.2 Linking

```cmake
# Main CMakeLists.txt
target_link_libraries(oflike-metal
    PRIVATE
        tess2
        utf8cpp
        glm  # Transitional
)
```

---

## 9. Future Considerations

### 9.1 Potential Additions

Libraries being considered for future phases:

- **stb_truetype** (if Core Text proves insufficient)
  - License: Public Domain / MIT
  - Purpose: Alternative font rendering

- **nanovg** (reference implementation)
  - License: Zlib
  - Purpose: Study vector graphics implementation

- **Dear ImGui** (debug UI)
  - License: MIT
  - Purpose: Phase 16 debug overlay alternative

### 9.2 Not Planned

Libraries we explicitly decided against:

- **Poco** - Too heavy, use Foundation
- **Qt** - Wrong UI paradigm, use SwiftUI
- **SFML** - Wrong graphics backend, use Metal
- **Cinder** - Similar project, but we want oF API

---

## 10. Support and Updates

### 10.1 Maintenance

Each dependency should be reviewed annually for:
- Security updates
- API improvements
- Breaking changes
- Continued maintenance

### 10.2 Contact

For questions about dependencies:
- Check ARCHITECTURE.md first
- Consult IMPLEMENTATION.md
- Create GitHub issue if unsure

---

## Summary

| Library | Purpose | License | Status |
|---------|---------|---------|--------|
| tess2 | Tessellation | SGI FSL B | Active |
| utf8-cpp | UTF-8 handling | Boost | Active |
| GLM | Math (temp) | MIT | Transitional (remove Phase 18) |

**Apple Frameworks**: Metal, MetalKit, Core Text, ImageIO, simd (preferred)

**Forbidden**: FreeType, OpenGL, SDL, GLFW, stb_image
