# SwiftUI Renderer Integration Plan

## Scope

Unify the SwiftUI MTKView path with the C++ DrawList and MetalRenderer path so
the SwiftUI app uses the same renderer and command execution flow as the
C++ render path.

## Current State (Observed)

- There are two rendering paths:
  - SwiftUI path builds its own Metal pipeline in Swift and draws a test
    triangle.
  - C++ path builds DrawList commands and has a MetalRenderer implementation,
    but it is not wired into SwiftUI.
- Context::renderer() returns nullptr; oflike draw calls populate DrawList but
  are not executed in the SwiftUI app.
- Event flow is already bridged (Swift -> SwiftBridge -> EventDispatcher).

## Goals

- Single render path: SwiftUI uses render::metal::MetalRenderer and executes
  C++ DrawList every frame.
- Keep SwiftUI for window/event lifecycle; keep MetalRenderer for GPU work.
- Ensure performance stats flow (draw calls, vertices, GPU time).

## Non-goals (For This Pass)

- Full feature completeness of all oflike draw APIs.
- Entry-point changes (SwiftUI is already the only entry).

## Proposed Flow (Target)

SwiftUI MTKView -> SwiftBridge -> Context -> DrawList -> MetalRenderer
-> MTKView drawable.

## Plan (Phases)

### Phase 0: Baseline and API Definition

- Document the desired ownership and lifetime of MetalRenderer:
  Context owns the renderer; Swift provides device and MTKView.
- Define new C++ entry points:
  - Context::initializeRenderer(device, view)
  - Context::renderer() returns MetalRenderer*
- Define frame calls expected from Swift:
  - beginFrame -> executeDrawList -> endFrame

### Phase 1: Context-Owned Renderer

- Implement renderer ownership in `src/core/Context.h` and
  `src/core/Context.mm`.
- Ensure renderer is created once, reused across frames, and shutdown cleanly.
- Update Context::renderer() to return a valid pointer.

### Phase 2: SwiftBridge Wiring

- Extend `src/platform/bridge/SwiftBridge.h` / `.mm` to:
  - Initialize renderer with device + MTKView.
  - Provide a render entry that calls beginFrame/executeDrawList/endFrame.
- Ensure DrawList is reset after execution.

### Phase 3: SwiftUI Frame Loop Integration

- Update `src/platform/swiftui/MetalView.swift`:
  - Remove the Swift-side pipeline draw (test triangle).
  - Call the new bridge render function each frame.
  - Keep event callbacks as-is.
- Align viewport updates on resize to the C++ renderer.

### Phase 4: DrawList Coverage and Clear Path

- Add minimal DrawList commands for clear/background so the first frame is
  visually valid.
- Verify ofGraphics/ofImage/ofTrueTypeFont paths are emitting DrawList commands
  that the renderer can execute.

### Phase 5: Validation and Cleanup

- Confirm performance stats are non-zero in SwiftUI.
- Verify examples that use SwiftUI path render expected output.
- Validate SwiftUI-only path and remove legacy assumptions.

## Risks and Mitigations

- Dual ownership of command buffers (Swift vs C++): ensure C++ owns the Metal
  command buffer for DrawList execution; Swift should not encode its own draw
  calls in the same frame.
- View/descriptor mismatch: MetalRenderer should use MTKView-provided drawable
  and render pass descriptor.
- Threading: all rendering must stay on the MTKView delegate thread (main).

## Open Questions

- None (Engine path removed; SwiftUI-only lifecycle).

## Acceptance Criteria

- SwiftUI app renders via C++ DrawList (no Swift test triangle).
- Context::renderer() is non-null and used by performance stats.
- oflike drawing calls produce visible output in SwiftUI.
