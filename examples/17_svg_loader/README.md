# Example 17: SVG Loader

Demonstrates how to use the **ofxSvg** addon to load and display SVG files.

## Features Demonstrated

- ✅ Loading SVG from string (`loadFromString()`)
- ✅ Loading SVG from file (`load()`)
- ✅ Accessing individual paths
- ✅ Drawing SVG content
- ✅ Querying SVG dimensions
- ✅ Interactive zoom and pan

## What This Example Shows

This example demonstrates:

1. **SVG Loading**: Load SVG content either from a string or file
2. **Path Access**: Query the number of paths and access them individually
3. **Drawing**: Render all paths in the SVG
4. **Dimensions**: Get the width and height of the SVG
5. **Interaction**: Use mouse to zoom and pan the SVG

## Controls

| Input | Action |
|-------|--------|
| Mouse Wheel | Zoom in/out |
| Mouse Drag | Pan/move the SVG |
| `R` key | Reset view to default |

## Code Overview

### Setup

```cpp
ofxSvg svg;

void setup() {
    // Load from string
    std::string svgContent = R"(
    <svg width="400" height="400">
        <circle cx="200" cy="200" r="100" fill="red"/>
    </svg>
    )";
    svg.loadFromString(svgContent);

    // Or load from file
    // svg.load("path/to/file.svg");

    // Query dimensions
    float w = svg.getWidth();
    float h = svg.getHeight();
    int numPaths = svg.getNumPaths();
}
```

### Drawing

```cpp
void draw() {
    // Draw all paths
    svg.draw();

    // Or access individual paths
    for (int i = 0; i < svg.getNumPaths(); ++i) {
        ofPath& path = svg.getPathAt(i);
        path.draw();
    }
}
```

### Transformations

```cpp
void draw() {
    ofPushMatrix();
    ofTranslate(x, y);
    ofScale(scale, scale);
    svg.draw();
    ofPopMatrix();
}
```

## Implementation Details

### Architecture

- Uses **nanosvg** for SVG parsing (single-header library, Zlib license)
- Converts SVG paths to `ofPath` objects with Bezier curves
- Preserves fill and stroke colors from the SVG
- Pure C++ implementation with pImpl pattern

### Supported SVG Features

✅ **Fully Supported**:
- Basic shapes (circle, rect, line, polyline, polygon)
- Path elements (M, L, C, Q, Z commands)
- Solid fill colors
- Solid stroke colors
- Stroke width
- Closed paths

🔄 **Partial Support**:
- Opacity (loaded but not fully applied yet)
- Transform matrices (TODO)

❌ **Not Yet Supported**:
- Gradients (linear, radial)
- Text elements (would need to be converted to paths first)
- Filters and effects
- Clipping paths
- Animations

### SVG → ofPath Conversion

The conversion process:

1. **Parse**: nanosvg parses the SVG file into shapes and paths
2. **Extract**: Each NSVGshape contains one or more NSVGpath
3. **Convert**: Each NSVGpath is converted to an ofPath:
   - Cubic Bezier points → `ofPath::bezierTo()` calls
   - Fill/stroke colors → `ofPath::setFillColor()`, `setStrokeColor()`
   - Closed flag → `ofPath::close()`

## SVG File Example

Create a file `test.svg`:

```xml
<svg width="200" height="200" xmlns="http://www.w3.org/2000/svg">
  <!-- Circle -->
  <circle cx="100" cy="100" r="50" fill="#ff0000" stroke="#000000" stroke-width="2"/>

  <!-- Rectangle -->
  <rect x="25" y="25" width="50" height="50" fill="#00ff00"/>

  <!-- Path (triangle) -->
  <path d="M 150 150 L 180 180 L 150 180 Z" fill="#0000ff"/>
</svg>
```

Then load it:

```cpp
svg.load("test.svg");
```

## Use Cases

This addon is useful for:

- Loading vector graphics and icons
- UI elements that need to scale cleanly
- Data visualization with pre-designed graphics
- Logo and branding display
- Importing designs from tools like Adobe Illustrator, Inkscape, Figma

## Related Examples

- **02_shapes**: Basic shape drawing
- **05_typography**: Text and fonts
- **04_image**: Raster image loading (PNG, JPG)

## Dependencies

- **ofxSvg** addon
- **nanosvg** library (included in third_party/)
- **ofPath** for rendering

## Building

Make sure the project is configured to include the ofxSvg addon:

```bash
# With CMake
cd build
cmake ..
make

# Or open the Xcode project
```

## Tips

1. **File Loading**: Make sure the SVG file path is correct (relative to executable)
2. **Coordinate System**: SVG uses top-left origin, same as oflike-metal 2D drawing
3. **Scale**: Use `ofScale()` to resize the SVG as needed
4. **Performance**: Complex SVGs with many paths may affect frame rate
5. **Debugging**: Check logs for parsing errors if SVG doesn't load

## Further Reading

- [SVG Specification](https://www.w3.org/TR/SVG/)
- [nanosvg GitHub](https://github.com/memononen/nanosvg)
- [ofxSvg README](../../src/addons/core/ofxSvg/README.md)
- [ofPath Documentation](../../src/oflike/graphics/ofPath.h)

## License

This example is part of oflike-metal and is licensed under the MIT License.
