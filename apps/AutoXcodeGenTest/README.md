# AutoXcodeGenTest

Generated with oflike-gen (oflike-metal Project Generator).

## Build

### With XcodeGen (Recommended)

```bash
open AutoXcodeGenTest.xcodeproj
```

Re-generate the Xcode project after editing `project.yml`:

```bash
xcodegen generate
```

### With CMake

```bash
mkdir build && cd build
cmake .. -G Xcode
open AutoXcodeGenTest.xcodeproj
```

## Entry Point

Entry mode: **swiftui**

SwiftUI-based macOS app with Metal rendering.

## Dependencies

- oflike-metal framework

## License

MIT
