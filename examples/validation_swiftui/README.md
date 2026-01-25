# Validation Test: SwiftUI Rendering

This example validates that the SwiftUI entry point correctly renders:
- **ofDrawCircle()** - 2D circle drawing
- **ofImage** - Image loading and rendering
- **ofTrueTypeFont** - Font loading and text rendering

## Build Instructions

### Step 1: Build oflike-metal Framework

From the repository root:
```bash
cd /path/to/oflike-metal
xcodegen generate
xcodebuild -scheme oflike-metal -configuration Release
```

### Step 2: Generate Xcode Project

```bash
cd examples/validation_swiftui
xcodegen generate
```

### Step 3: Build and Run

```bash
open validation_swiftui.xcodeproj
```

In Xcode:
1. Select **validation_swiftui** scheme
2. Press **⌘R** to build and run

## Expected Results

When the app runs successfully, you should see:

1. **Window Opens**
   - SwiftUI window appears (800x600 minimum)
   - No crashes or errors

2. **ofDrawCircle Rendering**
   - Large pink circle at center (radius 100)
   - Smaller green circle orbiting around center
   - Smooth animation at 60 FPS

3. **ofImage Rendering** (if test.jpg exists in data/)
   - Image displayed at top-left (50, 50)
   - Scaled to 200x150
   - Correct colors and aspect ratio

4. **ofTrueTypeFont Rendering**
   - "Hello, oflike-metal!" text at bottom
   - Yellow color (255, 255, 100)
   - 32pt font size, readable
   - FPS counter at top-right corner

## Troubleshooting

### Framework Not Found

If you see "framework not found" error:
- Verify oflike-metal.framework was built in Release configuration
- Check FRAMEWORK_SEARCH_PATHS in project.yml
- Ensure the framework is in `build/Release/`

### Black Screen

If the window opens but shows a black screen:
- Add debug logging to ValidationApp::draw()
- Check console for initialization errors
- Verify Metal device is available

### Image/Font Not Loading

Images and fonts are optional for this test:
- The app should run without test.jpg or Arial.ttf
- Only the circles will render if resources are missing
- Check data/ directory if you want to test image/font loading

## Test Procedure

See `docs/VALIDATION_TESTS.md` (Test 11.1) for complete validation test procedure and result documentation guidelines.

## Success Criteria

This validation test **PASSES** if:
- ✅ Window opens successfully
- ✅ Two circles render (pink center, green orbiting)
- ✅ Animation is smooth (60 FPS)
- ✅ No crashes or errors
- ✅ Window resize works correctly

Image and font rendering are **optional** - the test can pass without them as long as the circles render correctly.
