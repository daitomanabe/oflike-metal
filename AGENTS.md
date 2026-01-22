# AGENTS.md

This repo is a **macOS-only, C++-first, Metal/MSL-only** “openFrameworks-like” creative-coding engine skeleton.

Your job (local agent) is to implement the missing pieces and keep the scope tight.

## Non-negotiable constraints

- **User code must be C++** (C++20 OK; at minimum C++17).
- **Rendering must be Metal + MSL** (no OpenGL, no Vulkan).
- **macOS only** (no iOS in v0.1).
- **No openFrameworks compatibility layer** (API similarity only; no ofx add-ons).
- **No shader translation** (MSL only).
- Objective-C/Swift must be kept **out of public C++ headers**.
  - Use **Objective-C++ (.mm)** only inside `platform/macos/` and `src/render/*` implementations.
  - Public headers must remain pure C++.

## v0.1 acceptance criteria (must pass)

1. A minimal app written in C++ builds and runs:
   - `setup()` called once.
   - `update()` and `draw()` called every frame.
2. A rectangle renders and animates (e.g., moves horizontally).
3. Background clear works.
4. Basic input events are routed (at least mouse drag to move the rectangle).
5. App runs for 60 seconds without obvious leaks or runaway allocations.

## API goals (oF-like manners)

- App lifecycle: `setup()`, `update()`, `draw()`
- Facade functions (global style):
  - `ofBackground(r,g,b)`
  - `ofSetColor(r,g,b,a)`
  - `ofDrawRectangle(x,y,w,h)`
  - (optional next) `ofDrawCircle(x,y,r)`

Coordinate system target (v0.1):
- **Origin at top-left**, +x right, +y down (oF-style).
- API units are **points**; internal uses `MTKView.drawableSize` (pixels). Convert consistently.

## Architecture (must keep this separation)

### 1) platform/macos (Objective-C++)
Responsibilities:
- Create `NSApplication`, `NSWindow`, `MTKView`.
- Provide an `MTKViewDelegate` that:
  - computes `dt`
  - fetches `currentRenderPassDescriptor` and `currentDrawable`
  - calls `Engine::tick(dt, passDesc, drawable)`
- Translate Cocoa input events into `Engine`/`AppBase` callbacks.

### 2) src/core (pure C++)
Responsibilities:
- Own the app instance (`AppBase`).
- Run lifecycle: `setup/update/draw`.
- Store per-frame values: delta time, elapsed time.
- Keep an input state/event queue.

### 3) src/render (public headers pure C++; implementation can be .mm)
Responsibilities:
- Own Metal objects (device, queue, pipeline, buffers).
- Maintain a 2D pipeline for rectangles.
- Consume `DrawList2D` and issue Metal draw calls.

### 4) src/oflike (pure C++)
Responsibilities:
- Provide the oF-like facade functions that push into `DrawList2D`.
- Provide a single global-ish engine context accessor (without exposing ObjC types).

## Implementation plan (strict order)

### Phase A — Boot + loop
- [ ] `platform/macos/main.mm` starts the app.
- [ ] Create window + MTKView.
- [ ] Install view delegate; confirm `drawInMTKView` is called.

### Phase B — Engine + lifecycle
- [ ] Implement `Engine` setup/tick.
- [ ] Confirm `setup()` fires once.

### Phase C — Metal bring-up (triangle, then rectangle)
- [ ] Create Metal device/queue.
- [ ] Create `MTLRenderPipelineState` using `shaders/Basic2D.metal`.
- [ ] Implement vertex buffer upload for `DrawList2D`.
- [ ] Draw rectangle from `ofDrawRectangle`.

### Phase D — Input
- [ ] Mouse drag changes rectangle position.
- [ ] Key press logs or toggles a state.

## Coding rules

- Do not introduce “clever” abstractions. v0.1 should be minimal and debuggable.
- All Objective-C types must remain behind `void*` parameters or `pImpl`.
- Use RAII wrappers for Metal objects if possible; avoid manual retain/release where feasible.
- Avoid per-frame heap allocations in hot paths:
  - `DrawList2D` should reuse memory (`verts.reserve(...)`).

## Notes for ML integration later (do not implement now)

- The engine must eventually treat `CVPixelBuffer` as a first-class image type and support `CVMetalTextureCache` for zero-copy texture sharing.
- Do not bake ML assumptions into core; implement as modules.


## Build guidance (suggested)

This skeleton does not yet include a build system. Recommended approach:

- Use **CMake** to generate an Xcode project.
- Compile Objective-C++ sources with `.mm` extension.
- Ensure the target links:
  - `Metal.framework`
  - `MetalKit.framework`
  - `Cocoa.framework`
  - (later) `CoreVideo.framework`, `CoreML.framework`, `Vision.framework`

Minimum macOS version: pick something modern (e.g., 13+) to reduce compatibility burdens.

## Deliverables

When you are done, provide:

1. A runnable `examples/basic_app` that demonstrates background clear and moving rectangle.
2. Source code changes only (no binary outputs).
3. A short README describing how to build and run.
