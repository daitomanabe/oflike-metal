# Validation Tests

**Version**: 1.0.0
**Last Updated**: 2026-01-25

This document describes manual validation tests for oflike-metal implementation. These tests verify that the core functionality works correctly with visual confirmation.

---

## Phase 11: Validation Tests

### Test 11.1: SwiftUI Path Rendering

**Objective**: Verify that the SwiftUI entry point can render basic oF drawing primitives, images, and text.

**Components to Test**:
- `ofDrawCircle()` - 2D circle drawing
- `ofImage` - Image loading and rendering
- `ofTrueTypeFont` - Font loading and text rendering

**Prerequisites**:
- oflike-metal framework built
- Xcode 15.0+ installed
- macOS 13.0+ with Metal support

**Test Procedure**:

#### Step 1: Create Test Project

```bash
cd /path/to/oflike-metal
mkdir -p examples/validation_swiftui
cd examples/validation_swiftui
```

#### Step 2: Create SwiftUI Test App

Create `src/ValidationApp.h`:
```cpp
#pragma once
#include <core/AppBase.h>
#include <oflike/ofMain.h>

class ValidationApp : public ofBaseApp {
public:
    void setup() override;
    void update() override;
    void draw() override;

private:
    ofImage testImage;
    ofTrueTypeFont testFont;
    float angle = 0.0f;
};
```

Create `src/ValidationApp.cpp`:
```cpp
#include "ValidationApp.h"

void ValidationApp::setup() {
    ofSetFrameRate(60);

    // Load test image
    testImage.load("test.jpg");

    // Load test font
    testFont.load("Arial.ttf", 32);
}

void ValidationApp::update() {
    angle += 0.02f;
}

void ValidationApp::draw() {
    ofBackground(50);

    // Test 1: ofDrawCircle
    ofSetColor(255, 100, 100);
    float centerX = ofGetWidth() / 2.0f;
    float centerY = ofGetHeight() / 2.0f;
    ofDrawCircle(centerX, centerY, 100);

    // Animated circle
    float orbitX = centerX + cos(angle) * 150.0f;
    float orbitY = centerY + sin(angle) * 150.0f;
    ofSetColor(100, 255, 100);
    ofDrawCircle(orbitX, orbitY, 30);

    // Test 2: ofImage (if loaded)
    if (testImage.isAllocated()) {
        ofSetColor(255);
        testImage.draw(50, 50, 200, 150);
    }

    // Test 3: ofTrueTypeFont
    ofSetColor(255, 255, 100);
    testFont.drawString("Hello, oflike-metal!", 50, ofGetHeight() - 50);

    // Info text
    ofSetColor(255);
    testFont.drawString("FPS: " + ofToString(ofGetFrameRate(), 1),
                       ofGetWidth() - 150, 50);
}
```

Create `src/App.swift`:
```swift
import SwiftUI

@main
struct ValidationApp: App {
    var body: some Scene {
        WindowGroup {
            MetalView()
                .frame(minWidth: 800, minHeight: 600)
        }
        .windowStyle(.titleBar)
    }
}
```

Create `project.yml`:
```yaml
name: validation_swiftui
options:
  bundleIdPrefix: com.oflike
  deploymentTarget:
    macOS: "13.0"

targets:
  validation_swiftui:
    type: application
    platform: macOS
    sources:
      - src
    settings:
      PRODUCT_BUNDLE_IDENTIFIER: com.oflike.validation_swiftui
      SWIFT_VERSION: "5.9"
      ENABLE_HARDENED_RUNTIME: YES
      FRAMEWORK_SEARCH_PATHS: $(SRCROOT)/../../build/Release
    dependencies:
      - framework: oflike-metal.framework
        embed: true
```

#### Step 3: Add Test Data

```bash
mkdir -p data
# Add a test image to data/test.jpg
# Add a font file to data/Arial.ttf (or copy from system fonts)
```

#### Step 4: Build and Run

```bash
xcodegen generate
open validation_swiftui.xcodeproj
```

In Xcode:
1. Select validation_swiftui scheme
2. Press ⌘R to build and run

#### Step 5: Visual Verification

**Expected Results** ✅:

1. **Window Opens**:
   - SwiftUI window appears
   - Size: 800x600 minimum
   - No crashes or errors

2. **ofDrawCircle Rendering**:
   - Large pink circle at center (radius 100)
   - Smaller green circle orbiting around center
   - Smooth animation at 60 FPS

3. **ofImage Rendering** (if test.jpg exists):
   - Image displayed at top-left (50, 50)
   - Scaled to 200x150
   - Correct colors and aspect ratio

4. **ofTrueTypeFont Rendering**:
   - "Hello, oflike-metal!" text at bottom
   - Yellow color (255, 255, 100)
   - 32pt font size, readable
   - FPS counter at top-right corner

5. **Performance**:
   - Consistent 60 FPS
   - No flickering or tearing
   - No memory warnings

**Failure Indicators** ❌:

- Window doesn't appear
- Black screen (no rendering)
- Circles not visible or distorted
- Image not loaded or incorrectly rendered
- Text not visible or garbled
- Crashes or exceptions
- FPS below 50 on modern Mac

#### Step 6: Test Variants

Test with different configurations:

1. **Without Image**:
   - Remove test.jpg
   - App should run without crashing
   - Other elements should still render

2. **Without Font**:
   - Remove Arial.ttf
   - App should run (font may fallback to default)
   - Other elements should still render

3. **Window Resize**:
   - Drag window to resize
   - All elements should remain correctly positioned
   - No rendering artifacts

4. **Different Machines**:
   - Test on Apple Silicon (M1/M2/M3/M4)
   - Test on Intel Mac with Metal support
   - Verify consistent behavior

#### Step 7: Document Results

Create `validation_results.md`:
```markdown
# Validation Test 11.1 Results

**Date**: 2026-01-25
**Tester**: [Your Name]
**System**: [macOS version, Mac model, chip]

## Test Results

- [ ] Window opens successfully
- [ ] ofDrawCircle renders correctly
- [ ] ofImage loads and displays (if available)
- [ ] ofTrueTypeFont renders text correctly
- [ ] Animation is smooth (60 FPS)
- [ ] No crashes or errors
- [ ] Window resize works correctly

## Issues Found

(List any issues discovered during testing)

## Screenshots

(Attach screenshots showing successful rendering)

## Conclusion

- [ ] Test PASSED - All functionality works as expected
- [ ] Test FAILED - See issues above
```

### Test 11.2: PerformanceStats Validation

**Objective**: Verify that performance statistics (draw calls, vertices, GPU time) are correctly tracked and reported.

**Components to Test**:
- `src/platform/swiftui/MetalView.swift` - PerformanceStats update
- `src/render/metal/MetalRenderer.mm` - Statistics tracking

**Test Procedure**:

#### Step 1: Check PerformanceStats Display

Using the validation app from Test 11.1:

1. Build and run the app
2. Observe the FPS overlay (top-left corner)
3. Verify the following values update:
   - FPS (should be ~60)
   - Frame count (should increment)
   - Elapsed time (should increment)

#### Step 2: Add Performance Overlay

Modify `src/App.swift` to display detailed stats:

```swift
import SwiftUI

struct StatsOverlay: View {
    let drawCalls: UInt32
    let vertices: UInt32
    let gpuTime: Double

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            Text("Draw Calls: \(drawCalls)")
                .font(.system(size: 12, design: .monospaced))
            Text("Vertices: \(vertices)")
                .font(.system(size: 12, design: .monospaced))
            Text("GPU Time: \(String(format: "%.2f", gpuTime))ms")
                .font(.system(size: 12, design: .monospaced))
        }
        .foregroundColor(.white)
        .padding(8)
        .background(Color.black.opacity(0.6))
        .cornerRadius(6)
    }
}
```

#### Step 3: Visual Verification

**Expected Results** ✅:

1. **Non-Zero Values**:
   - Draw calls > 0 (should be 2-5 for validation app)
   - Vertices > 0 (varies based on geometry)
   - GPU time > 0.0 (should be < 16.67ms for 60 FPS)

2. **Values Update Every Frame**:
   - Frame count increments by 1 each frame
   - Draw calls may vary based on rendering
   - GPU time fluctuates slightly

3. **Reasonable Performance**:
   - GPU time < 16.67ms (for 60 FPS target)
   - CPU time not excessive
   - No stuttering or lag

**Failure Indicators** ❌:

- All values are zero
- Values don't change between frames
- GPU time > 16.67ms consistently (performance issue)
- Frame drops or stuttering

#### Step 4: Document Results

Update `validation_results.md`:
```markdown
## Test 11.2: PerformanceStats

- [ ] Draw calls reported (non-zero)
- [ ] Vertices count reported (non-zero)
- [ ] GPU time measured (non-zero, < 16.67ms)
- [ ] Stats update every frame
- [ ] Performance is acceptable

### Measured Values
- Draw Calls: [e.g., 4]
- Vertices: [e.g., 240]
- GPU Time: [e.g., 2.3ms]
- FPS: [e.g., 60]
```

---

## Validation Checklist

After completing all validation tests, verify:

- [ ] **Test 11.1 PASSED**: SwiftUI path renders ofDrawCircle/ofImage/ofTrueTypeFont
- [ ] **Test 11.2 PASSED**: PerformanceStats returns non-zero values
- [ ] Screenshots captured and saved
- [ ] Results documented in validation_results.md
- [ ] Issues reported (if any)
- [ ] `docs/CHECKLIST.md` updated with test results

---

## Next Steps

Once validation tests pass:

1. Update `docs/CHECKLIST.md` - mark validation items as [x]
2. Commit validation results and screenshots
3. Proceed to Phase 12: Addons implementation
4. Report any issues found during validation

---

## Troubleshooting

### Issue: Window doesn't open

**Solutions**:
- Check that oflike-metal.framework is built
- Verify framework search paths in Xcode
- Check console for error messages

### Issue: Black screen (no rendering)

**Solutions**:
- Verify Metal device is available
- Check that setup() is being called
- Add debug logging to draw() method
- Verify SwiftBridge is initialized correctly

### Issue: Image/Font not loading

**Solutions**:
- Check file paths (relative to data/ directory)
- Verify files exist in built app bundle
- Add logging to load() methods
- Check for file permission issues

### Issue: Poor performance (< 60 FPS)

**Solutions**:
- Build in Release mode (not Debug)
- Check for excessive draw calls
- Profile with Xcode Instruments (Metal System Trace)
- Verify no synchronous readbacks or stalls

---

## Automated Testing (Future)

These validation tests are currently manual. Future enhancements:

- [ ] Automated screenshot comparison
- [ ] Headless rendering tests
- [ ] Performance regression testing
- [ ] CI/CD integration with visual diff tools
