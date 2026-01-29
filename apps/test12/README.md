# test12

Generated with oflike-gen (oflike-metal Project Generator).

## Build

### With XcodeGen (Recommended)

```bash
open test12.xcodeproj
```

Re-generate the Xcode project after editing `project.yml`:

```bash
xcodegen generate
```

### With xcodebuild (CLI)

```bash
mkdir -p /tmp/oflike_metal_module_cache
xcodebuild -project test12.xcodeproj \
  -scheme test12 \
  -configuration Debug \
  -destination "platform=macOS" \
  -derivedDataPath build/DerivedData/test12 \
  MTL_COMPILER_FLAGS="-fmodules-cache-path=/tmp/oflike_metal_module_cache" \
  build
```

### Using repo scripts (from oflike-metal root)

```bash
./scripts/build_app.sh test12
./scripts/run_app.sh test12
# Metal debug
./scripts/run_app.sh test12 --metal-debug
```

### Notes

- Xcode 26 requires the Metal Toolchain component (Xcode Settings > Components).
- Verify `xcrun metal -v` works before building.
- Shader debug info is disabled to avoid Metal cache warnings; set `MTL_ENABLE_DEBUG_INFO = INCLUDE_SOURCE` in `project.yml` if needed.
- Set `OFL_METAL_DEBUG=1` to log Metal library probing at app startup.

### With CMake

```bash
mkdir build && cd build
cmake .. -G Xcode
open test12.xcodeproj
```

## Dependencies

- oflike-metal framework

## License

MIT
