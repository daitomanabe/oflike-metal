# Project Checklist (Live)

This checklist reflects the **current** project state and what still needs validation.
Implementation milestones are marked separately from **manual validation**.

---

## Phase 0: Architecture & Policy

- [x] Define layer boundaries (oflike -> render abstraction only)
- [x] SwiftUI is the only entry point (ofMain removed)
- [x] Coordinate responsibilities clarified (2D top-left; renderer handles NDC/Y)
- [x] Generator-friendly folder layout documented
- [x] Addon linking strategy documented

## Phase 1: Build & Toolchain Stabilization

- [x] Swift sources gated in CMake
- [x] GLM header path removed
- [x] VisionKit optional link aligned
- [x] Swift PerformanceStats merged

## Phase 2: App Entry & Lifecycle

- [x] User app factory (`ofCreateApp`) wired
- [x] Legacy Engine path removed
- [x] SwiftUI mouse coordinate consistency verified
- [x] Window control scope clarified

## Phase 3: Renderer Ownership & Bridge

- [x] Context owns MetalRenderer
- [x] SwiftBridge exposes renderer init + render
- [x] Metal device exposed via context/renderer

## Phase 4: MetalRenderer Execution

- [x] executeDraw2D implemented
- [x] executeDraw3D implemented
- [x] FBO render target switching implemented
- [x] 2D projection + model-view handling implemented
- [x] Blend/cull pipeline states applied

## Phase 5: SwiftUI Integration

- [x] Swift-side pipeline/test triangle removed
- [x] C++ render entry called per frame
- [x] Resize/viewport propagation implemented

## Phase 6: oflike Graphics 2D + Matrix

- [x] Clear/background via DrawList
- [x] 2D primitives implemented
- [x] Matrix stack + transforms implemented
- [x] Text color integration implemented
- [x] Shape/curve TODOs resolved

## Phase 7: Resource Ownership + Texture/FBO

- [x] MTLCreateSystemDefaultDevice removed from oflike layer
- [x] ofTexture draw/bind with DrawList implemented
- [x] FBO begin/end restore + MRT implemented
- [x] 16-bit/float texture uploads implemented
- [x] GPU->CPU readback implemented

## Phase 8: 3D / Camera / Mesh / Lighting

- [x] ofCamera transforms implemented
- [x] ofMesh DrawCommand3D implemented
- [x] ofEasyCam auto distance implemented
- [x] Lighting/material uniforms integrated

## Phase 9: Utilities / Image I/O

- [x] 16-bit/float image I/O support
- [x] Renderer device/MTKTextureLoader path

## Phase 10: Project Generator + Tools

- [x] CLI generator implemented (`oflike-gen`)
- [x] Templates for generated apps
- [x] Addons selection/link logic
- [x] Build steps documented
- [ ] ProjectWizard GUI: generate + run end-to-end without external errors

---

## Phase 11: API Validation (Manual)

> See `docs/api/API_CHECKLIST.md` for detailed per-API coverage.

### Stage 0: Validation Harness (ApiValidation)
- [x] `apps/ApiValidation` created and builds
- [x] Scene switcher (1-4 / n / p)
- [x] Overlay (FPS, frame, size, scene)
- [x] Sample scenes: overview / primitives / transforms / image+text
- [ ] Optional assets placed in `apps/ApiValidation/data/` (test image, font)

### Stage 1+: API Coverage
- [ ] Foundation (frame rate, time, window size)
- [ ] Math (vectors, matrices)
- [ ] 2D graphics state/shape correctness
- [ ] 3D primitives + depth/cull correctness
- [ ] Image/texture correctness (load, draw, readback)
- [ ] FBO correctness (begin/end, restore)
- [ ] Font metrics + rendering correctness
- [ ] Events (mouse/keyboard/window)
- [ ] Utilities (logging, string/file helpers)
