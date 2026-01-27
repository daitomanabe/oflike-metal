# oflike-metal prompt

## Project summary
- oflike-metal is a macOS-native reimplementation of openFrameworks.
- User code is C++ and targets the oflike API.
- Rendering is Metal via a render abstraction, with SwiftUI for windows and events.
- SwiftUI is the only supported entry point.

## Pillars
1) SwiftUI for windowing, events, UI.
2) Metal for GPU rendering and compute.
3) C++ for app logic.
4) openFrameworks API compatibility.
5) Apple native frameworks (Core ML, Vision, PHASE, MetalFX, etc.).

## Absolute rules
- No AppKit direct use. SwiftUI only, with MTKView or low-level bridging when necessary.
- No OpenGL or GLSL.
- No FreeType. Use Core Text.
- No SDL or GLFW.
- No stb_image. Use ImageIO or MTKTextureLoader.
- No GLM or external SIMD. Use simd/simd.h or Accelerate.
- No Poco, Cairo, or Assimp. Use Foundation or Network, Core Graphics, and Model I/O.
- Avoid cross-platform libraries unless explicitly approved.

## Platform
- Minimum macOS 13.0.
- macOS 14+ frameworks (VisionKit) are optional or weak-linked.

## Coordinate system
- 2D origin is top-left (+X right, +Y down).
- 3D is right-handed (oF compatible).
- Texture origin is top-left.
- Metal NDC conversion, Y flip, and Z range (0..1) are handled in the renderer.

## Implementation patterns
- Public headers are pure C++ (.h).
- Use pImpl to hide Objective-C++.
- Use .mm for Apple framework implementations.
- Wrap Objective-C object creation in @autoreleasepool.

## Source layout (canonical)
- src/core, src/math, src/oflike, src/render, src/platform.
- Metal implementation lives in src/render/metal.
- SwiftUI implementation lives in src/platform/swiftui.
- Addons live under src/addons/core and src/addons/native.

## Workflow
- Read docs/ARCHITECTURE.md first, then docs/CHECKLIST.md.
- Follow CHECKLIST phases in order. Addons are last (Phase 12).
- Update docs/CHECKLIST.md from [ ] to [x] when a task is completed.
- Only perform actions required by the current task or explicit request.
