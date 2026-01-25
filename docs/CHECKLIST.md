# Task List (Detailed, Ordered)

This is the full, ordered task list to align implementation with the target
architecture (oF-compatible API, SwiftUI + Metal + Apple Native, C++).
Addons are explicitly last.

---

## Phase 0: Structure & Policy (Must Decide First)

- [x] **Define layer boundaries (oflike -> render abstraction only).**
  Files: `docs/ARCHITECTURE.md`
  DoD: oflike layer cannot include `render/metal/*`; Metal code is restricted to `src/render/metal/`.

- [x] **Declare SwiftUI as default, ofMain as legacy/compat.**
  Files: `docs/ARCHITECTURE.md`, `docs/MIGRATION.md`
  DoD: docs explicitly state SwiftUI is the default entry; ofMain is legacy.

- [x] **Clarify coordinate responsibilities (2D top-left; renderer handles NDC/Y).**
  Files: `docs/ARCHITECTURE.md`
  DoD: renderer responsible for Metal NDC/Y conversion; oflike API remains oF-compatible.

- [x] **Finalize generator-friendly folder layout.**
  Files: `docs/PROJECT_STRUCTURE.md`
  DoD: standard generated project layout is documented (src/addons/data/resources/shaders/tools).

- [x] **Document addon linking strategy (copy/symlink/reference).**
  Files: `docs/IMPLEMENTATION.md`
  DoD: generator strategy + constraints are explicit.

---

## Phase 1: Build Stabilization (Before Deeper Refactors)

- [x] **Gate Swift sources in CMake (only when Swift compiler is available).**
  Files: `CMakeLists.txt`
  DoD: non-Xcode CMake builds do not fail due to Swift sources.

- [x] **Remove GLM header search path.**
  Files: `project.yml`
  DoD: `third_party/glm` is not referenced unless explicitly justified.

- [x] **Align VisionKit optional link in XcodeGen.**
  Files: `project.yml`
  DoD: VisionKit is weak-linked or conditionally linked, consistent with CMake.

- [x] **Merge duplicate Swift PerformanceStats.**
  Files: `src/platform/swiftui/MetalView.swift`, `src/platform/swiftui/PerformanceMonitor.swift`
  DoD: only one `PerformanceStats` type remains and is used.

---

## Phase 2: App Entry & Lifecycle

- [x] **Replace TestApp with user app factory.**
  Files: `src/platform/bridge/SwiftBridge.h`, `src/platform/bridge/SwiftBridge.mm`, `src/core/TestApp.h`
  DoD: SwiftUI path constructs user app via factory (e.g., `ofCreateApp()`); TestApp no longer required.

- [x] **Decide and implement Engine usage.**
  Files: `src/core/Engine.h`, `src/core/Engine.mm`, `src/platform/bridge/SwiftBridge.mm`, `docs/ARCHITECTURE.md`
  DoD: either Engine drives setup/update/draw, or Engine is declared non-participating.

- [x] **Unify Y-axis handling between SwiftUI and ofMain.**
  Files: `src/platform/swiftui/MetalView.swift`, `src/oflike/app/ofMain.h`
  DoD: mouse coordinates are consistent with oF expectations.

- [x] **Clarify window control scope (SwiftUI vs AppKit).**
  Files: `docs/ARCHITECTURE.md`, `src/platform/swiftui/MetalView.swift`, `src/core/Context.mm`
  DoD: allowed window operations are defined; AppKit usage is scoped or removed.

---

## Phase 3: Renderer Ownership & Bridge

- [x] **Context owns MetalRenderer.**
  Files: `src/core/Context.h`, `src/core/Context.mm`
  DoD: `Context::renderer()` returns a valid pointer after init.

- [x] **SwiftBridge exposes renderer init + render entry.**
  Files: `src/platform/bridge/SwiftBridge.h`, `src/platform/bridge/SwiftBridge.mm`
  DoD: Swift can call `initializeRenderer(device, view)` and `renderFrame()`.

- [x] **Expose Metal device via renderer or context.**
  Files: `src/render/metal/MetalRenderer.h`, `src/render/metal/MetalRenderer.mm`, `src/core/Context.h`
  DoD: oflike layer can access the correct `MTLDevice` without creating a new one.

---

## Phase 4: MetalRenderer Execution (Drawing Must Work)

- [x] **Implement `executeDraw2D` (vertex upload, pipeline bind, draw).**
  Files: `src/render/metal/MetalRenderer.mm`
  DoD: DrawList 2D draws produce visible output.

- [x] **Implement `executeDraw3D` (vertex/index buffers, depth state).**
  Files: `src/render/metal/MetalRenderer.mm`
  DoD: DrawList 3D draws render correctly.

- [x] **Implement SetRenderTarget (FBO).**
  Files: `src/render/metal/MetalRenderer.mm`
  DoD: render target switching works with `ofFbo`.

- [x] **Add 2D projection + model-view handling.**
  Files: `src/render/metal/MetalRenderer.mm`
  DoD: 2D positions map to top-left origin space.

- [x] **Apply blend/cull states in pipelines.**
  Files: `src/render/metal/MetalRenderer.mm`
  DoD: blend/culling TODOs are removed and states are honored.

---

## Phase 5: SwiftUI Integration (DrawList Path)

- [x] **Remove Swift-side pipeline and test triangle.**
  Files: `src/platform/swiftui/MetalView.swift`
  DoD: Swift does not issue any Metal draw commands directly.

- [x] **Call C++ render entry per frame.**
  Files: `src/platform/swiftui/MetalView.swift`
  DoD: `renderFrame()` is called each draw cycle.

- [x] **Resize/viewport updates propagate to renderer.**
  Files: `src/platform/swiftui/MetalView.swift`, `src/render/metal/MetalRenderer.mm`
  DoD: viewport changes reflect new MTKView sizes.

---

## Phase 6: oflike Graphics 2D + Matrix

- [x] **Emit clear/background commands via DrawList.**
  Files: `src/oflike/graphics/ofGraphics.cpp`, `src/render/DrawCommand.h`
  DoD: `ofBackground` and `ofClear` trigger Clear commands.

- [x] **Implement 2D primitives with DrawList.**
  Files: `src/oflike/graphics/ofGraphics.cpp`
  DoD: line/rect/circle/triangle draw commands execute.

- [x] **Implement matrix stack and transforms.**
  Files: `src/oflike/graphics/ofGraphics.cpp`, `src/core/Context.mm`
  DoD: `ofPushMatrix/ofPopMatrix/ofTranslate/ofRotate/ofScale` affect rendering.

- [x] **Integrate graphics state color into text rendering.**
  Files: `src/oflike/graphics/ofTrueTypeFont.mm`
  DoD: TODOs for color/transform are removed.

- [x] **Fill in shape/curve TODOs.**
  Files: `src/oflike/graphics/ofGraphics.cpp`
  DoD: rounded rect, arc, curve, bezier placeholders are implemented or removed.

---

## Phase 7: Device/Resource Ownership + Texture/FBO

- [x] **Remove `MTLCreateSystemDefaultDevice` from oflike layer.**
  Files: `src/oflike/image/ofTexture.mm`, `src/oflike/graphics/ofFbo.mm`
  DoD: all devices come from Context/renderer.

- [x] **Implement ofTexture draw/bind with DrawList.**
  Files: `src/oflike/image/ofTexture.mm`, `src/render/DrawList.h`
  DoD: texture draws and binds are tracked via DrawList.

- [x] **Implement FBO begin/end state restore and MRT.**
  Files: `src/oflike/graphics/ofFbo.mm`
  DoD: render target/viewport restore works; MRT selection is implemented.

- [x] **Implement 16-bit/float texture uploads.**
  Files: `src/oflike/image/ofTexture.mm`
  DoD: 16-bit and float pixel uploads work without TODOs.

- [x] **Implement GPU->CPU readback path (ofImage).**
  Files: `src/oflike/image/ofImage.mm`
  DoD: readback TODO removed or clearly deferred with alternate path.

---

## Phase 8: 3D / Camera / Mesh / Lighting

- [x] **Implement ofCamera view/proj stack + screen/world conversions.**
  Files: `src/oflike/3d/ofCamera.cpp`
  DoD: all placeholders removed; correct viewport-based transforms.

- [x] **Implement ofMesh DrawCommand3D path.**
  Files: `src/oflike/3d/ofMesh.cpp`
  DoD: DrawCommand3D is fully populated with state/matrices.

- [x] **Implement ofEasyCam auto distance.**
  Files: `src/oflike/3d/ofEasyCam.cpp`
  DoD: TODO removed with functional logic.

- [x] **Integrate lighting/material uniforms.**
  Files: `src/oflike/lighting/ofMaterial.cpp`
  DoD: material state is uploaded and restored.

---

## Phase 9: Utilities / Image I/O

- [x] **Add 16-bit/float image I/O support.**
  Files: `src/oflike/utils/ofUtilsImage.mm`
  DoD: 16-bit/float load/save TODOs removed.

- [x] **Use renderer device/MTKTextureLoader path.**
  Files: `src/oflike/utils/ofUtilsImage.mm`, `src/render/IRenderer.h`
  DoD: device access is through renderer, not `MTLCreateSystemDefaultDevice`.

---

## Phase 10: Project Generator (CLI First)

- [x] **Define CLI spec and input schema.**
  Files: `docs/IMPLEMENTATION.md`, `tools/project_generator/README.md`
  DoD: CLI usage and example config are documented.

- [ ] **Implement CLI generator.**  
  Files: `tools/project_generator/`  
  DoD: `oflike-gen --help` works; generates project skeleton.

- [ ] **Add templates for generated projects.**  
  Files: `tools/project_generator/templates/`  
  DoD: includes `ofApp.*`, `main.mm`, `project.yml`, `Info.plist`.

- [ ] **Add addons selection + link/copy logic.**  
  Files: `tools/project_generator/`  
  DoD: addons are reflected in include/link/resources.

- [ ] **Document build steps for generated projects.**  
  Files: `docs/MIGRATION.md`  
  DoD: post-generation build steps are listed.

---

## Phase 11: Validation

- [ ] **SwiftUI path renders ofDrawCircle/ofImage/ofTrueTypeFont.**  
  Files: `examples/` (or new SwiftUI example)  
  DoD: visual confirmation + no runtime errors.

- [ ] **PerformanceStats returns non-zero values.**  
  Files: `src/platform/swiftui/MetalView.swift`  
  DoD: drawCalls/vertices/gpuTime update during render.

---

## Phase 12: Addons (Last)

### Core Addons
- [ ] **ofxSvg opacity/transform/scale TODOs.**  
  Files: `src/addons/core/ofxSvg/ofxSvg.cpp`, `src/addons/core/ofxSvg/README.md`  
  DoD: TODOs removed or explicitly deferred.

- [ ] **ofxGui parameter binding TODO.**  
  Files: `src/addons/core/ofxGui/ofxGui.mm`  
  DoD: UI bindings persist and update.

### Apple Native Addons
- [ ] **ofxNeuralEngine GPU path placeholders.**  
  Files: `src/addons/apple_native/ofxNeuralEngine/*.mm`  
  DoD: placeholder comments replaced with implementation or explicit deferral.

- [ ] **ofxSharp scene/model/renderer placeholders.**  
  Files: `src/addons/apple_native/ofxSharp/*.mm`  
  DoD: TODOs removed; scene rendering path exists.

- [ ] **CoreML conversion script placeholders.**  
  Files: `src/addons/apple_native/ofxSharp/scripts/convert_to_coreml.py`  
  DoD: script reflects actual model architecture.

---

## Notes

- All TODO/placeholder items found in `src/` are assigned above.
- Addons are last as requested.
- This file should be treated as the canonical implementation roadmap.

