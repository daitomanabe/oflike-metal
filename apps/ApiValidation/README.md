# ApiValidation

Generated with oflike-gen (oflike-metal Project Generator).

## Build

### With XcodeGen (Recommended)

```bash
open ApiValidation.xcodeproj
```

Re-generate the Xcode project after editing `project.yml`:

```bash
xcodegen generate
```

### With xcodebuild (CLI)

```bash
mkdir -p /tmp/oflike_metal_module_cache
xcodebuild -project ApiValidation.xcodeproj \
  -scheme ApiValidation \
  -configuration Debug \
  -destination "platform=macOS" \
  -derivedDataPath build/DerivedData/ApiValidation \
  MTL_COMPILER_FLAGS="-fmodules-cache-path=/tmp/oflike_metal_module_cache" \
  build
```

### Using repo scripts (from oflike-metal root)

```bash
./scripts/build_app.sh ApiValidation
./scripts/run_app.sh ApiValidation
# Metal debug
./scripts/run_app.sh ApiValidation --metal-debug
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
open ApiValidation.xcodeproj
```

## Dependencies

- oflike-metal framework

## License

MIT
## Validation Controls

- Keys `1-4`: switch scenes
- Keys `n` / `p`: next / previous scene

## Optional Assets

Place assets in `apps/ApiValidation/data/`:
- `test.jpg` or `test.png` for image loading
- `font.ttf` (optional)

If assets are missing, the app falls back to a generated checker image and system fonts.
## Scenes

1. Overview
2. Foundation (time / FPS / window info)
3. 2D Primitives
4. Transforms
5. Image + Text

## Controls

- `1` - `5`: switch scenes
- `n` / `p`: next / previous scene
- `f`: toggle target FPS (30 / 60 / 120)

