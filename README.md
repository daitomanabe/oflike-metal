# oflike_metal_skeleton

macOS-only C++ creative-coding engine skeleton with an openFrameworks-like user API and Metal/MSL rendering.

## Requirements

- macOS 13+
- Xcode with Metal support
- CMake 3.24+

## Build

```bash
mkdir build && cd build
cmake .. -G Xcode
xcodebuild -project oflike_metal_skeleton.xcodeproj -scheme basic_app -configuration Debug build
```

Or open the generated Xcode project:

```bash
open oflike_metal_skeleton.xcodeproj
```

## Run

```bash
open Debug/basic_app.app
```

Or run directly:

```bash
./Debug/basic_app.app/Contents/MacOS/basic_app
```

## User-app shape

See `examples/basic_app.cpp` for a working example demonstrating:
- Background clear with `ofBackground()`
- Animated rectangle with `ofDrawRectangle()`
- Mouse drag to move the rectangle

## API

- `ofBackground(r, g, b)` - Set background clear color
- `ofSetColor(r, g, b, a)` - Set draw color
- `ofDrawRectangle(x, y, w, h)` - Draw a rectangle

Coordinate system: origin at top-left, +x right, +y down.

## Notes

- Public headers are pure C++. Objective-C++ should stay in `platform/macos` and renderer implementations.
- MSL shaders are embedded in `Renderer2D.mm` for v0.1 simplicity.
