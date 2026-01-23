# oflike-metal Tests

This directory contains automated tests for the oflike-metal framework.

## Test Structure

### Math Library Tests (`math/`)
- **File**: `math_test.cpp`
- **Purpose**: Tests all mathematical components (Phase 3)
- **Coverage**:
  - ofVec2f, ofVec3f, ofVec4f
  - ofMatrix4x4
  - ofQuaternion
  - ofMath functions (random, noise, mapping, distance, angles)

### Rendering Tests (`rendering/`)
- **File**: `rendering_test.cpp`
- **Purpose**: Tests rendering API availability and callability (Phase 17.2)
- **Coverage**:
  - Color API (construction, HSB, lerp, invert)
  - Path API (moveTo, lineTo, curves, arcs, transformations)
  - Polyline API (vertices, queries, modifications, resampling)
  - Graphics State API (color, fill, blending, depth, lighting)
  - Drawing Functions API (2D/3D primitives, helpers)
  - Matrix Stack API (push/pop, transforms)
  - Shape Building API (ofBeginShape/ofEndShape)

### Performance Tests (`performance/`)
- **File**: `performance_test.cpp`
- **Purpose**: Benchmarks performance of various operations (Phase 17.2)
- **Coverage**:
  - Vector math operations (add, cross, dot, normalize)
  - Color operations (construction, HSB conversion, lerp)
  - Matrix stack operations (push/pop, transforms)
  - Path construction (lines, curves, beziers, arcs)
  - Polyline operations (vertices, queries, area, perimeter)
  - Shape building (ofBeginShape/ofEndShape)
  - 2D primitives rendering (circles, rectangles, ellipses, triangles)
  - 3D primitives rendering (boxes, spheres, cones, cylinders)

## Running Tests

### Using CMake

```bash
# Configure
mkdir build && cd build
cmake ..

# Build tests
make math_test
make rendering_test
make performance_test

# Run tests
./tests/math_test
./tests/rendering_test
./tests/performance_test

# Or use CTest
ctest
```

### Using Xcode

Tests are integrated into the Xcode project and can be run from the Test Navigator.

## Visual Verification

The rendering tests verify API availability but cannot generate visual output without a rendering context. For visual verification of rendering correctness, run the example applications:

- `examples/01_basics` - Basic drawing
- `examples/02_shapes` - 2D shapes
- `examples/03_color` - Color operations
- `examples/04_image` - Image loading and display
- `examples/05_typography` - Font rendering
- `examples/06_3d_primitives` - 3D shapes
- `examples/07_camera` - Camera controls
- `examples/08_lighting` - Phong lighting
- `examples/09_mesh` - Mesh operations
- `examples/10_fbo` - Framebuffer objects
- `examples/11_events` - Event handling
- `examples/12_easycam` - EasyCam controls

## Test Output

All tests use colored console output:
- ✓ Green: Test passed
- ✗ Red: Test failed
- Yellow: Section headers

Each test prints a summary with:
- Total tests run
- Passed tests
- Failed tests
- Success rate percentage

## Adding New Tests

To add a new test:

1. Create a new `.cpp` file in an appropriate subdirectory
2. Include necessary headers
3. Implement test functions with TEST_START(), TEST_PASS(), TEST_FAIL()
4. Add the test to `CMakeLists.txt`:
   ```cmake
   add_executable(my_test my/my_test.cpp)
   target_link_libraries(my_test PRIVATE oflike-metal)
   add_test(NAME MyTest COMMAND my_test)
   ```

## Test Philosophy

- **Math tests**: Verify numerical correctness with epsilon tolerance
- **Rendering tests**: Verify API availability and linkage
- **Examples**: Serve as visual regression tests

The combination of automated tests + manual visual verification provides comprehensive coverage of framework functionality.
