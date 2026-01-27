# Test5

Generated with oflike-gen (oflike-metal Project Generator).

## Build

### With XcodeGen (Recommended)

```bash
open Test5.xcodeproj
```

Re-generate the Xcode project after editing `project.yml`:

```bash
xcodegen generate
```

### With xcodebuild (CLI)

```bash
mkdir -p /tmp/oflike_metal_module_cache
xcodebuild -project Test5.xcodeproj \
  -scheme Test5 \
  -configuration Debug \
  -destination "platform=macOS" \
  -derivedDataPath build/DerivedData/Test5 \
  MTL_COMPILER_FLAGS="-fmodules-cache-path=/tmp/oflike_metal_module_cache" \
  build
```

### Notes

- Xcode 26 requires the Metal Toolchain component (Xcode Settings > Components).
- Verify `xcrun metal -v` works before building.

### With CMake

```bash
mkdir build && cd build
cmake .. -G Xcode
open Test5.xcodeproj
```

## Dependencies

- oflike-metal framework

## License

MIT
