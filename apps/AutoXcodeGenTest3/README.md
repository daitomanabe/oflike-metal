# AutoXcodeGenTest3

Generated with oflike-gen (oflike-metal Project Generator).

## Build

### With XcodeGen (Recommended)

```bash
open AutoXcodeGenTest3.xcodeproj
```

Re-generate the Xcode project after editing `project.yml`:

```bash
xcodegen generate
```

### With CMake

```bash
mkdir build && cd build
cmake .. -G Xcode
open AutoXcodeGenTest3.xcodeproj
```

## Dependencies

- oflike-metal framework

## License

MIT
