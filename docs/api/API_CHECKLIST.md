# API Development Checklist (Validation First)

This checklist drives API verification and implementation order.
Each item should be validated with the API validation app before moving on.

## Stage 0: Validation Harness (Required)
- [ ] Create `apps/ApiValidation` with SwiftUI entry + C++ `ofBaseApp`.
      DoD: `./scripts/run_app.sh ApiValidation --build` launches and exits cleanly.
- [ ] Add scene switcher (keyboard 1-9) to toggle test cases.
      DoD: overlay shows current scene; switching does not crash.
- [ ] Add on-screen overlay (FPS, frame count, app name, scene name).
      DoD: overlay visible on all scenes; no layout glitches.
- [ ] Add test assets (font + image) under `apps/ApiValidation/data/`.
      DoD: assets load without errors on first run.

## Stage 1: Foundation / Core
- [ ] `ofGetWidth/Height`, `ofGetWindowSize`, `ofGetFrameRate`.
      DoD: overlay values match window size; FPS stable.
- [ ] `ofSetFrameRate`, `ofSetVerticalSync`.
      DoD: measured FPS changes as expected.
- [ ] `ofGetElapsedTimef`, `ofGetElapsedTimeMillis`.
      DoD: monotonically increasing and consistent.
- [ ] App lifecycle callbacks (`setup/update/draw/exit`, `windowResized`).
      DoD: all callbacks fire at expected times.

## Stage 2: Math
- [ ] `ofVec2f/3f/4f` arithmetic and normalization.
      DoD: unit tests in validation app match expected values.
- [ ] `ofMatrix4x4` transforms (translate/rotate/scale, inverse).
      DoD: scene renders predictable transforms.
- [ ] `ofQuaternion` basic rotation usage (if exposed).
      DoD: camera rotation scene behaves correctly.

## Stage 3: 2D Graphics
- [ ] Primitives: line, rectangle, circle, triangle.
      DoD: rendered with correct positions, sizes, colors.
- [ ] State: `ofSetColor`, `ofSetLineWidth`, fill/noFill.
      DoD: state changes take effect and restore.
- [ ] Matrix stack: `ofPush/PopMatrix`, `ofTranslate/Rotate/Scale`.
      DoD: nested transforms work as expected.
- [ ] `ofPath` / polyline / curves.
      DoD: curves render smoothly; tessellation stable.

## Stage 4: 3D Graphics
- [ ] `ofDrawBox`, `ofDrawSphere`, `ofDrawPlane`.
      DoD: objects render with correct depth ordering.
- [ ] `ofMesh` draw modes (points/lines/triangles).
      DoD: indices and normals behave correctly.
- [ ] Depth test, cull state, wireframe.
      DoD: visual correctness toggles as expected.

## Stage 5: Color
- [ ] `ofColor`, `ofFloatColor` conversions/clamp.
      DoD: gradients match expected values.
- [ ] Alpha blending modes.
      DoD: transparent overlays render correctly.

## Stage 6: Image / Texture
- [ ] `ofImage` load (PNG/JPG), draw, resize.
      DoD: image draws with correct size and color.
- [ ] `ofTexture` allocate/upload (8/16/float).
      DoD: pixel formats render correctly.
- [ ] Readback (`ofImage`/`ofPixels`).
      DoD: readback equals source within tolerance.

## Stage 7: FBO
- [ ] `ofFbo` allocate/begin/end.
      DoD: render-to-texture works and restores state.
- [ ] Depth/stencil attachments.
      DoD: 3D scene in FBO has correct depth.
- [ ] Multi-render target (if supported).
      DoD: multiple outputs verified.

## Stage 8: Font / Text
- [ ] `ofTrueTypeFont` load/draw.
      DoD: text renders at correct size and position.
- [ ] Metrics: `stringWidth`, `stringHeight`, `getLineHeight`.
      DoD: metrics match visual bounds.
- [ ] Multiline and UTF-8 (if supported).
      DoD: layout stable; no crashes.

## Stage 9: Events
- [ ] Mouse move/press/drag/release.
      DoD: coordinates and button states are correct.
- [ ] Keyboard key down/up.
      DoD: modifier keys and repeat behave correctly.
- [ ] Window resize / focus / fullscreen.
      DoD: callbacks fire; renderer updates size.

## Stage 10: Utils
- [ ] Logging (`ofLogNotice/Warning/Error`).
      DoD: logs appear with correct categories.
- [ ] File utils (`ofToDataPath`, `ofLoadURL` if supported).
      DoD: paths resolve correctly; IO errors handled.
- [ ] String utils (`ofToString`, `ofSplitString`).
      DoD: results match expected values.
