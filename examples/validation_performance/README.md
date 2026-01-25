# Performance Statistics Validation Test (Phase 11.2)

This example validates that `PerformanceStats` correctly tracks and reports non-zero values for:
- **drawCalls**: Number of draw commands issued per frame
- **vertices**: Number of vertices rendered per frame
- **gpuTime**: GPU execution time in milliseconds

## What This Test Does

The app renders:
- 20 animated circles orbiting around the center (20 draw calls)
- 5 rectangles at the top (5 draw calls)
- 1 center marker (1 draw call)

Total: **26 draw calls per frame** with hundreds of vertices.

A **PerformanceMonitor** overlay is displayed in the top-left corner showing real-time statistics.

## Build Instructions

### Step 1: Build oflike-metal Framework

```bash
cd /path/to/oflike-metal
xcodebuild -project oflike-metal.xcodeproj -scheme oflike-metal -configuration Release
```

### Step 2: Generate Xcode Project

```bash
cd examples/validation_performance
xcodegen generate
```

### Step 3: Build and Run

```bash
open validation_performance.xcodeproj
```

In Xcode:
1. Select `validation_performance` scheme
2. Press ⌘R to build and run

## Expected Results ✅

The PerformanceMonitor overlay should display:

1. **FPS**: ~59-60 FPS (green)
2. **Frame Time**: ~16-17ms (green)
3. **Draw Calls**: **26** (should be non-zero, green if < 300)
4. **Vertices**: **Several thousand** (should be non-zero)
5. **GPU Time**: **< 10ms** (should be non-zero)

## Success Criteria

- ✅ `drawCalls` is **26** (not zero)
- ✅ `vertexCount` is **> 0** (several thousand vertices)
- ✅ `gpuTime` is **> 0.0** (measured in milliseconds)
- ✅ Values update every frame
- ✅ No crashes or errors

## Failure Indicators ❌

- ❌ All statistics show **zero**
- ❌ Statistics never update
- ❌ App crashes on launch
- ❌ Performance overlay not visible

## Troubleshooting

### Statistics Show Zero

If statistics are zero:

1. **Check MetalView.swift**: Verify `updatePerformanceStats()` is being called
2. **Check SwiftBridge.mm**: Verify `getPerformanceStats()` is implemented
3. **Check MetalRenderer**: Verify `getStatistics()` and `getLastGPUTime()` return values
4. **Build Configuration**: Ensure you're using Release build with optimizations

### Overlay Not Visible

The PerformanceMonitor is only visible in DEBUG builds. If using Release:
- Switch to Debug configuration
- Or modify App.swift to always show the overlay

## Reference

See `docs/VALIDATION_TESTS.md` (Test 11.2) for complete validation procedure.

## Manual Verification

When the app is running, verify:

1. **Visual Check**: Multiple colored circles orbiting + rectangles at top
2. **Overlay Check**: PerformanceMonitor shows non-zero values
3. **Update Check**: Draw calls/vertices/GPU time change as circles animate
4. **Performance Check**: Smooth 60 FPS with no stuttering

## Documentation

After validation, document results in `validation_results.md`:

```markdown
# Phase 11.2 Validation Results

**Date**: 2026-01-25
**System**: [macOS version, Mac model]

## Test Results

- [ ] PerformanceStats.drawCalls > 0
- [ ] PerformanceStats.vertexCount > 0
- [ ] PerformanceStats.gpuTime > 0.0
- [ ] Values update every frame
- [ ] Performance overlay displays correctly
- [ ] No crashes or errors

## Observed Values

- Draw Calls: ___
- Vertex Count: ___
- GPU Time: ___ms
- FPS: ___

## Conclusion

- [ ] Test PASSED - All statistics are non-zero
- [ ] Test FAILED - See issues above
```
