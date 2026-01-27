# fff

Generated with oflike-gen (oflike-metal Project Generator).

## Build

### With XcodeGen (Recommended)

```bash
open fff.xcodeproj
```

Re-generate the Xcode project after editing `project.yml`:

```bash
xcodegen generate
```

### With xcodebuild (CLI)

```bash
mkdir -p /tmp/oflike_metal_module_cache
xcodebuild -project fff.xcodeproj \
  -scheme fff \
  -configuration Debug \
  -destination "platform=macOS" \
  -derivedDataPath build/DerivedData/fff \
  MTL_COMPILER_FLAGS="-fmodules-cache-path=/tmp/oflike_metal_module_cache" \
  build
```

### Using repo scripts (from oflike-metal root)

```bash
./scripts/build_app.sh fff
./scripts/run_app.sh fff
# Metal debug
./scripts/run_app.sh fff --metal-debug
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
open fff.xcodeproj
```

## Dependencies

- oflike-metal framework

## License

MIT
