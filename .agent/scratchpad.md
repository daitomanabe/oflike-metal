# oflike-metal Feature Development Scratchpad

## Current Status

**Phase**: Not Started
**Last Updated**: 2026-01-29

---

## TODO Tasks

### Phase 1: Foundation
- [ ] ofNode - Scene graph base class
- [ ] ofCamera - Camera base class (inherits ofNode)
- [ ] ofFbo - Framebuffer object
- [ ] ofShader - Custom shader management

### Phase 2: Enhancement
- [ ] ofVbo - GPU vertex buffers
- [ ] ofTexture extensions - Mipmap, wrap, filter
- [ ] Shader-based Lighting - Integrate ofLight/ofMaterial

### Phase 3: Media
- [ ] ofVideoPlayer - Video playback
- [ ] ofVideoGrabber - Camera input
- [ ] Image Filters - blur, sharpen, contrast, etc.

---

## Notes

### Dependencies
- ofNode → ofCamera → ofEasyCam (inheritance chain)
- ofShader → Shader Lighting (shader must exist first)
- MetalRenderer → ofFbo, ofVbo (need renderer access)

### Reference Files
- Existing ofEasyCam: `src/oflike/3d/ofEasyCam.h`
- Existing ofLight: `src/oflike/lighting/ofLight.h`
- Metal Renderer: `src/render/metal/MetalRenderer.mm`

---

## Progress Log

| Date | Phase | Task | Status |
|------|-------|------|--------|
| 2026-01-29 | - | Initial setup | Created Ralph config |

