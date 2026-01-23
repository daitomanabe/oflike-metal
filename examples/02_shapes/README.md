# Example 02: Shape Drawing

## Overview

This example demonstrates comprehensive 2D shape drawing capabilities in oflike-metal, showcasing the variety of primitive shapes and path-based drawing available through the openFrameworks-compatible API.

## What You'll See

A colorful grid layout displaying different types of shapes:

### Row 1: Basic Primitives
- **Circle**: Simple circular shape using `ofDrawCircle()`
- **Rectangle**: Rectangular shape using `ofDrawRectangle()`
- **Triangle**: Three-sided polygon using `ofDrawTriangle()`
- **Ellipse**: Oval shape using `ofDrawEllipse()`
- **Rounded Rectangle**: Rectangle with rounded corners using `ofDrawRectRounded()`

### Row 2: Lines and Polylines
- **Crossed Lines**: Simple line drawing with `ofDrawLine()`
- **Rotating Hexagon**: Multi-segment polyline forming an animated hexagon

### Row 3: ofPath Complex Shapes
- **Star**: Five-pointed star created with `ofPath` using alternating radii
- **Heart**: Smooth heart shape using bezier curves
- **Hexagon**: Six-sided polygon using `ofPath.lineTo()`

### Row 4: Advanced Paths
- **Bezier Curve**: Smooth cubic bezier demonstration
- **Arc**: Partial circle using `ofPath.arc()`

## Features Demonstrated

### Shape Drawing Functions
- `ofDrawCircle(x, y, radius)` - Circle at position with radius
- `ofDrawRectangle(x, y, width, height)` - Rectangle
- `ofDrawTriangle(x1, y1, x2, y2, x3, y3)` - Triangle from three points
- `ofDrawEllipse(x, y, width, height)` - Ellipse
- `ofDrawRectRounded(x, y, w, h, radius)` - Rounded rectangle
- `ofDrawLine(x1, y1, x2, y2)` - Line between two points

### Fill and Stroke Modes
- `ofFill()` - Enable filled shapes
- `ofNoFill()` - Draw outlines only
- `ofSetLineWidth(width)` - Set stroke width

### ofPath API
- `moveTo(x, y)` - Start new path or move without drawing
- `lineTo(x, y)` - Draw line to point
- `bezierTo(cx1, cy1, cx2, cy2, x, y)` - Cubic bezier curve
- `arc(x, y, radiusX, radiusY, angleBegin, angleEnd)` - Circular arc
- `close()` - Close the path
- `setFilled(bool)` - Set fill mode
- `setStrokeWidth(width)` - Set stroke width
- `setColor(color)` - Set path color
- `draw()` - Render the path

### Color Management
- `ofColor::fromHsb(hue, saturation, brightness)` - Create color from HSB
- `ofSetColor(r, g, b)` or `ofSetColor(color)` - Set draw color

### Interactive Controls
- **SPACE**: Toggle between fill and stroke mode
- **+/=**: Increase line width (stroke mode)
- **-/_**: Decrease line width (stroke mode)

### Animation
- Smooth color cycling through HSB spectrum
- Rotating hexagon polyline
- All shapes responsive to window resizing

## API Functions Used

### Graphics Functions (ofGraphics.h)
- `ofBackground(gray)` - Set background color
- `ofSetColor(r, g, b)` - Set current draw color
- `ofFill()` / `ofNoFill()` - Toggle fill mode
- `ofSetLineWidth(width)` - Set stroke width
- `ofDrawCircle(x, y, radius)` - Draw circle
- `ofDrawRectangle(x, y, w, h)` - Draw rectangle
- `ofDrawTriangle(x1, y1, x2, y2, x3, y3)` - Draw triangle
- `ofDrawEllipse(x, y, w, h)` - Draw ellipse
- `ofDrawRectRounded(x, y, w, h, radius)` - Draw rounded rectangle
- `ofDrawLine(x1, y1, x2, y2)` - Draw line

### Path Functions (ofPath.h)
- `ofPath::moveTo()` - Start path
- `ofPath::lineTo()` - Add line segment
- `ofPath::bezierTo()` - Add bezier curve
- `ofPath::arc()` - Add circular arc
- `ofPath::close()` - Close path
- `ofPath::setFilled()` - Set fill mode
- `ofPath::setStrokeWidth()` - Set stroke width
- `ofPath::setColor()` - Set path color
- `ofPath::draw()` - Render path

### Color Functions (ofColor.h)
- `ofColor::fromHsb(h, s, b)` - Create color from HSB values

### Utility Functions (ofUtils.h)
- `ofGetWidth()` - Get window width
- `ofGetHeight()` - Get window height
- `ofSetFrameRate(fps)` - Set target frame rate

### App Structure (AppBase.h)
- `ofBaseApp::setup()` - Initialization
- `ofBaseApp::update()` - Per-frame logic
- `ofBaseApp::draw()` - Rendering
- `ofBaseApp::keyPressed(key)` - Keyboard input
- `ofBaseApp::windowResized(w, h)` - Window resize handler

## How to Run

### Integration Method
1. Open `src/platform/bridge/SwiftBridge.mm`
2. Replace `TestApp` with `ShapeDrawingApp`
3. Include this file: `#include "../../examples/02_shapes/main.cpp"`
4. Build and run the Xcode project

### Standalone (if CMake support added)
```bash
cd build
cmake ..
make 02_shapes
./examples/02_shapes/02_shapes
```

## Expected Output

A 6-column grid layout displaying 10+ different shapes:
- All shapes cycle through rainbow colors (HSB animation)
- Shapes can be toggled between fill and stroke modes
- Line width adjustable in stroke mode
- Hexagon rotates continuously
- Shapes automatically reposition on window resize

## Learning Points

1. **Primitive Shapes**: oflike-metal provides immediate-mode drawing functions for common shapes
2. **Fill vs Stroke**: Shapes can be either filled or outlined (or both with manual drawing)
3. **ofPath Flexibility**: Complex shapes can be built using paths with lines, curves, and arcs
4. **Color Cycling**: HSB color space is useful for smooth color animations
5. **Interactive Parameters**: User input can control rendering parameters in real-time

## Architecture Notes

- Pure C++ code using openFrameworks-compatible API
- No direct Metal or SwiftUI code in the example
- All rendering handled by oflike-metal's Metal backend
- ofPath internally generates Metal vertex buffers for GPU rendering

## Next Steps

- Experiment with different path combinations
- Add your own custom shapes using `ofPath`
- Try combining multiple colors in a single shape
- Explore `ofPolyline` for complex line drawings (when implemented)
- Add texture mapping to shapes (Phase 7)

## Related Examples

- **01_basics**: Basic drawing fundamentals
- **03_color**: Advanced color manipulation (coming soon)
- **09_mesh**: Custom geometry with ofMesh (coming soon)
