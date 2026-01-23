# oflike-metal Implementation Guide

**Version**: 1.0.0
**Last Updated**: 2026-01-23
**Status**: Living Document

> This document provides practical guidance for implementing oflike-metal components.

---

## 1. Development Workflow

### 1.1 Phase-by-Phase Implementation

Follow the phases defined in `CHECKLIST.md`:
- Phase 0: Project foundation
- Phase 1: SwiftUI + Metal foundation
- Phase 2-18: Core systems, rendering, 2D/3D APIs, etc.

Each phase builds on the previous one. Complete all tasks in a phase before moving to the next.

### 1.2 Task Completion Workflow

For each task:
1. Implement the feature according to ARCHITECTURE.md
2. Test the implementation
3. Update CHECKLIST.md ([x] the completed task)
4. Commit with conventional commit format
5. Push to GitHub

### 1.3 Commit Message Format

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
- Examples: `phase-0`, `phase-1`, `metal`, `swiftui`, `oflike`

---

## 2. Architecture Compliance

### 2.1 Forbidden Libraries (MUST NOT USE)

❌ **FreeType** → Use Core Text
❌ **OpenGL** → Use Metal
❌ **SDL/GLFW** → Use SwiftUI + MTKView
❌ **stb_image** → Use ImageIO / MTKTextureLoader
❌ **AppKit direct usage** → Use SwiftUI
❌ **GLM (in new code)** → Use simd

### 2.2 Required Patterns

✅ **pImpl Pattern**: Keep Objective-C types out of public headers
✅ **Pure C++ headers (.h)**: Public API must be pure C++
✅ **Objective-C++ implementation (.mm)**: Use .mm for Apple frameworks
✅ **@autoreleasepool**: Wrap Objective-C object creation
✅ **simd**: Use simd types for math operations

### 2.3 API Compatibility

All public APIs must match openFrameworks signatures:
- Same function names (of-prefix)
- Same parameter types and order
- Same coordinate systems (2D: top-left origin, 3D: right-handed)
- Same default values

---

## 3. Code Organization

### 3.1 Directory Structure

```
src/
├── core/           # Context, Engine, Time, AppBase
├── render/         # MetalRenderer, DrawList, Buffers, Textures
├── oflike/         # openFrameworks-compatible API layer
│   ├── ofGraphics.h/mm
│   ├── ofMath.h/mm
│   ├── ofImage.h/mm
│   └── ...
└── bridge/         # Swift-C++ interop
```

### 3.2 Header Organization

Public headers (.h):
```cpp
#pragma once
#include <simd/simd.h>
// Pure C++ only - no Objective-C types

namespace oflike {
    class ClassName {
    public:
        // Public API
    private:
        class Impl;
        std::unique_ptr<Impl> impl_;  // pImpl pattern
    };
}
```

Implementation files (.mm):
```objc++
#import "ClassName.h"
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

// Objective-C++ implementation
class ClassName::Impl {
    id<MTLDevice> device_;
    // Use Objective-C types freely
};
```

---

## 4. Testing Strategy

### 4.1 Visual Tests

Create example programs in `examples/` to verify:
- Basic rendering works
- Animations are smooth
- API behaves like openFrameworks
- No visual artifacts

### 4.2 Build Tests

Ensure:
- CMake configuration works
- All examples build successfully
- No compiler warnings
- No linker errors

### 4.3 Runtime Tests

Verify:
- No crashes during execution
- No memory leaks
- Smooth 60fps performance
- Proper resource cleanup

---

## 5. Common Patterns

### 5.1 Resource Management

Use RAII for all resources:
```cpp
class Texture {
    id<MTLTexture> texture_;

public:
    Texture() : texture_(nil) {}
    ~Texture() {
        texture_ = nil;  // ARC handles cleanup
    }

    // Disable copying
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // Allow moving
    Texture(Texture&&) = default;
    Texture& operator=(Texture&&) = default;
};
```

### 5.2 Error Handling

Check for errors and provide fallbacks:
```cpp
bool loadTexture(const std::string& path) {
    @autoreleasepool {
        NSError* error = nil;
        id<MTLTexture> tex = [loader newTextureWithPath:...
                                                 error:&error];
        if (error) {
            NSLog(@"Failed to load texture: %@", error);
            return false;
        }
        texture_ = tex;
        return true;
    }
}
```

### 5.3 Swift-C++ Bridge

Use Objective-C++ as bridge:
```swift
// Swift side
class MetalView: NSViewRepresentable {
    func makeNSView(context: Context) -> MTKView {
        let view = MTKView()
        OFLBridge.shared.setup(view)
        return view
    }
}
```

```objc++
// Bridge implementation
@implementation OFLBridge
- (void)setup:(MTKView*)view {
    // Call C++ code
    oflike::Context::shared().initialize(view);
}
@end
```

---

## 6. Performance Guidelines

### 6.1 Triple Buffering

Use Metal's triple buffering for smooth rendering:
```cpp
static const int kMaxBuffersInFlight = 3;
dispatch_semaphore_t inFlightSemaphore_ =
    dispatch_semaphore_create(kMaxBuffersInFlight);

void draw() {
    dispatch_semaphore_wait(inFlightSemaphore_, DISPATCH_TIME_FOREVER);

    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
        dispatch_semaphore_signal(inFlightSemaphore_);
    }];
}
```

### 6.2 Buffer Pooling

Reuse buffers instead of recreating:
```cpp
class BufferPool {
    std::vector<id<MTLBuffer>> available_;
    std::vector<id<MTLBuffer>> inUse_;

    id<MTLBuffer> acquire(size_t size);
    void release(id<MTLBuffer> buffer);
};
```

### 6.3 Batch Rendering

Group similar draw calls together:
```cpp
// Instead of:
for (auto& rect : rects) {
    drawRectangle(rect);  // Many draw calls
}

// Do:
beginBatch();
for (auto& rect : rects) {
    addRectangle(rect);  // Add to batch
}
endBatch();  // One draw call
```

---

## 7. Debugging Tips

### 7.1 Metal Debugging

Enable Metal validation layer:
```cpp
// In debug builds
#ifdef DEBUG
    device = MTLCreateSystemDefaultDevice();
    // Enable Metal API validation
#endif
```

### 7.2 Logging

Use os_log for debugging:
```objc++
#import <os/log.h>

os_log_debug(OS_LOG_DEFAULT, "Texture loaded: %{public}s", path.c_str());
os_log_error(OS_LOG_DEFAULT, "Failed to create pipeline state");
```

### 7.3 Xcode Instruments

Profile with:
- Time Profiler: CPU performance
- Metal System Trace: GPU performance
- Leaks: Memory leaks
- Allocations: Memory usage

---

## 8. Documentation Standards

### 8.1 API Documentation

Document all public APIs:
```cpp
/// Draws a circle at the specified position
/// @param x X-coordinate of the center (in points)
/// @param y Y-coordinate of the center (in points)
/// @param radius Radius of the circle (in points)
void ofDrawCircle(float x, float y, float radius);
```

### 8.2 Implementation Comments

Comment complex logic:
```cpp
// Convert from oF coordinate system (top-left origin)
// to Metal normalized device coordinates (center origin, y-up)
float ndcX = (x / width) * 2.0f - 1.0f;
float ndcY = 1.0f - (y / height) * 2.0f;
```

---

## 9. Migration Path

### 9.1 From GLM to simd

Replace GLM types with simd:
```cpp
// Old (GLM)
glm::vec3 pos(1.0f, 2.0f, 3.0f);
glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos);

// New (simd)
simd_float3 pos = {1.0f, 2.0f, 3.0f};
simd_float4x4 transform = matrix_identity_float4x4;
transform = matrix_multiply(transform,
                           matrix_translation(pos));
```

### 9.2 Phase 18 GLM Removal

In Phase 18:
1. Identify all GLM usage with grep
2. Replace with simd equivalents
3. Remove GLM from dependencies
4. Verify all builds succeed

---

## 10. Release Checklist

Before releasing:
- [ ] All CHECKLIST.md tasks complete
- [ ] All examples build and run
- [ ] No ARCHITECTURE.md violations
- [ ] Documentation is complete
- [ ] Performance meets 60fps target
- [ ] No memory leaks
- [ ] Version tagged in git
- [ ] Release notes written

---

## References

- [ARCHITECTURE.md](ARCHITECTURE.md) - Project architecture
- [CHECKLIST.md](CHECKLIST.md) - Development checklist
- [Apple Metal Documentation](https://developer.apple.com/metal/)
- [Apple simd Documentation](https://developer.apple.com/documentation/accelerate/simd)
- [openFrameworks Documentation](https://openframeworks.cc/documentation/)
