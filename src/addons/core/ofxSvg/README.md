# ofxSvg

SVG loader for oflike-metal using nanosvg.

## Overview

ofxSvg provides a simple interface for loading SVG files and converting them to `ofPath` objects for rendering. It uses [nanosvg](https://github.com/memononen/nanosvg) for parsing SVG files.

## Features

- ✅ Load SVG files from disk or string
- ✅ Convert SVG paths to `ofPath` objects
- ✅ Preserve fill and stroke colors
- ✅ Support for Bezier curves and closed paths
- ✅ Unit conversion (px, pt, pc, mm, cm, in)
- 🔄 TODO: Transform matrix support
- 🔄 TODO: Gradient support

## Dependencies

- **nanosvg** (Zlib license) - Single-header SVG parser
- Pure C++17 implementation
- No external dependencies

## Usage

### Basic Loading

```cpp
#include "addons/core/ofxSvg/ofxSvg.h"

ofxSvg svg;
svg.load("logo.svg");

// Draw all paths
svg.draw();
```

### Path Access

```cpp
// Get individual paths
for (int i = 0; i < svg.getNumPaths(); ++i) {
    ofPath& path = svg.getPathAt(i);
    path.draw();
}

// Or get all paths
std::vector<ofPath>& paths = svg.getPaths();
```

### Dimensions

```cpp
float width = svg.getWidth();
float height = svg.getHeight();
```

### Loading from String

```cpp
std::string svgContent = R"(
<svg width="100" height="100">
    <circle cx="50" cy="50" r="40" fill="red" />
</svg>
)";

svg.loadFromString(svgContent);
```

### Unit Conversion

```cpp
// Load with specific units and DPI
svg.load("drawing.svg", "mm", 96.0f);  // millimeters at 96 DPI
svg.load("drawing.svg", "in", 72.0f); // inches at 72 DPI
```

## API Reference

### Core Methods

```cpp
bool load(const std::string& path, const std::string& units = "px", float dpi = 96.0f);
bool loadFromString(const std::string& svgStr, const std::string& units = "px", float dpi = 96.0f);
bool isLoaded() const;
void clear();
```

### Path Access

```cpp
int getNumPaths() const;
ofPath& getPathAt(int index);
std::vector<ofPath>& getPaths();
```

### Dimensions

```cpp
float getWidth() const;
float getHeight() const;
```

### Drawing

```cpp
void draw();
void draw(float x, float y);                    // TODO: Translation
void draw(float x, float y, float w, float h);  // TODO: Scaling
```

## Implementation Details

### Architecture

- **pImpl pattern**: Public header is pure C++, implementation uses nanosvg
- **Header-only library**: nanosvg is included directly (single file)
- **No Objective-C**: Pure C++ implementation
- **Path conversion**: SVG cubic Bezier curves → `ofPath::bezierTo()`

### SVG → ofPath Conversion

nanosvg parses SVG and provides:
- **NSVGshape**: Each shape in the SVG
- **NSVGpath**: Each path within a shape (can have multiple contours)
- **Points**: Cubic Bezier control points as `[x0,y0, cpx1,cpy1,cpx2,cpy2,x1,y1, ...]`

ofxSvg converts these to `ofPath` commands:
1. Extract shape properties (fill, stroke, opacity)
2. Iterate through paths in each shape
3. Convert Bezier points to `bezierTo()` calls
4. Apply colors and styles

### Supported SVG Features

✅ Supported:
- Paths (line, curve, bezier, arc)
- Solid fill colors
- Solid stroke colors
- Stroke width
- Closed paths

🔄 TODO:
- Gradients (linear, radial)
- Opacity (partial support)
- Transform matrices
- Text elements
- Clipping paths

### Limitations

- Gradients are not yet supported (will be ignored)
- Text is not converted to paths automatically
- Some advanced SVG features may not be supported

## Examples

See `examples/svg_loader/` for a complete example.

## License

- **ofxSvg**: MIT License (same as oflike-metal)
- **nanosvg**: Zlib License (permissive)

## References

- [nanosvg on GitHub](https://github.com/memononen/nanosvg)
- [SVG Specification](https://www.w3.org/TR/SVG/)
- [ofPath Documentation](../../../oflike/graphics/ofPath.h)
