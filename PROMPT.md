# oflike-metal Feature Development

## Project Summary
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

## Absolute Rules
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

---

## Current Development: Feature Enhancement

### Phases Overview

| Phase | Focus | Features |
|-------|-------|----------|
| 1 | 基盤強化 | ofNode, ofCamera, ofFbo, ofShader |
| 2 | 表現力向上 | ofVbo, ofTexture拡張, Shader Lighting |
| 3 | メディア対応 | ofVideoPlayer, ofVideoGrabber, Image Filters |

### Phase 1: Foundation (基盤強化)

| Feature | Description | Dependencies |
|---------|-------------|--------------|
| ofNode | Scene graph base node with parent/child transforms | None |
| ofCamera | General camera class (base for ofEasyCam) | ofNode |
| ofFbo | Off-screen rendering / framebuffer object | MetalRenderer |
| ofShader | Custom Metal shader management | MetalRenderer |

### Phase 2: Enhancement (表現力向上)

| Feature | Description | Dependencies |
|---------|-------------|--------------|
| ofVbo | GPU vertex buffer management | MetalRenderer |
| ofTexture Extension | Mipmap, Wrap, Filter settings | ofTexture |
| Shader Lighting | Integrate ofLight/ofMaterial with shaders | ofShader, ofLight |

### Phase 3: Media (メディア対応)

| Feature | Description | Dependencies |
|---------|-------------|--------------|
| ofVideoPlayer | Video playback with AVFoundation | AVPlayer |
| ofVideoGrabber | Camera input with AVCaptureSession | AVFoundation |
| Image Filters | blur, sharpen, contrast, etc. | Accelerate/Metal |

---

## Critical Rules for Ralph

### Every Iteration Must Do:

1. **Log to `.agent/iteration.log`**
   ```
   [{ISO8601}] iteration #{n} | {Hat名} | {状態} | {概要}
   ```

2. **Git commit & push**
   ```bash
   git add -A
   git commit -m "[Ralph] {Hat名}: {完了内容}"
   git push origin master
   ```

### On LOOP_COMPLETE:

1. Generate `COMPLETION_REPORT.md`
2. Final git push

---

## Directory Structure

```
src/oflike/
├── 3d/
│   ├── ofNode.h/.cpp      # Phase 1
│   ├── ofCamera.h/.cpp    # Phase 1 (inherits ofNode)
│   ├── ofEasyCam.h/.cpp   # Existing (inherits ofCamera)
│   └── ofMesh.h/.cpp      # Existing
├── graphics/
│   ├── ofFbo.h/.mm        # Phase 1
│   ├── ofShader.h/.mm     # Phase 1
│   └── ofVbo.h/.mm        # Phase 2
├── image/
│   ├── ofTexture.h/.mm    # Phase 2 (extensions)
│   └── ofImage.h/.mm      # Phase 3 (filters)
├── video/                  # Phase 3
│   ├── ofVideoPlayer.h/.mm
│   └── ofVideoGrabber.h/.mm
└── lighting/
    ├── ofLight.h/.cpp     # Phase 2 (shader integration)
    └── ofMaterial.h/.cpp  # Phase 2 (shader integration)
```

---

## Success Criteria

### Phase 1 Complete When:
- [x] ofNode: Parent/child hierarchy with global transforms working (2026-01-30)
- [x] ofCamera: Perspective/ortho projection, coordinate conversion working (2026-01-29)
- [x] ofFbo: Off-screen rendering working (2026-01-29)
- [x] ofShader: Custom shader loading and application working (2026-01-30)
- [x] test_phase1 app builds and runs successfully (2026-01-30)

**Phase 1 COMPLETE** (2026-01-30)

### Phase 2 Complete When:
- [x] ofVbo: GPU vertex data management working (2026-01-30)
- [x] ofTexture: Mipmap, wrap, filter settings working (2026-01-30)
- [x] Shader Lighting: ofLight/ofMaterial already implemented (prior)
- [x] test_phase2 app builds and runs successfully (2026-01-30)

**Phase 2 COMPLETE** (2026-01-30)

### Phase 3 Complete When:
- [ ] ofVideoPlayer: Video playback working
- [ ] ofVideoGrabber: Camera input working
- [ ] Image Filters: blur, sharpen, etc. working
- [ ] test_phase3 app builds and runs successfully

### Final Criteria:
- [ ] All changes pushed to remote
- [ ] .agent/iteration.log up to date
- [ ] COMPLETION_REPORT.md generated
- [ ] LOOP_COMPLETE issued

---

## Reference Documents

- `docs/FEATURE_IDEAS.md` - Detailed feature ideas
- `docs/ARCHITECTURE.md` - Architecture guidelines
- Existing code: `src/oflike/`, `src/render/metal/`
