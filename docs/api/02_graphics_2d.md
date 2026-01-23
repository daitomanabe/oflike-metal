# 2D Graphics API

## Overview

The 2D graphics API provides openFrameworks-compatible drawing functions for creating 2D visuals. All drawing is hardware-accelerated using Metal.

## Header

```cpp
#include <oflike/graphics/ofGraphics.h>
```

---

## Color Management

### Set Drawing Color

```cpp
ofSetColor(255, 0, 0);           // Red
ofSetColor(128, 128, 255, 128);  // Semi-transparent blue
ofSetColor(200);                 // Grayscale
ofSetHexColor(0xFF0000);         // Red from hex
```

### Set Background Color

```cpp
ofSetBackgroundColor(0, 0, 0);           // Black background
ofSetBackgroundColor(128);               // Gray background
ofSetBackgroundHexColor(0x336699);       // Blue-gray from hex
```

### Clear Screen

```cpp
ofBackground(0);                 // Clear with current bg color
ofBackground(255, 255, 255);     // Clear with white
ofClear();                       // Clear all buffers
ofClearDepth();                  // Clear depth only
ofClearAlpha();                  // Clear alpha only
```

---

## Fill & Stroke Mode

```cpp
ofFill();                        // Draw filled shapes (default)
ofNoFill();                      // Draw outlines only

if (ofGetFill()) {
    // Currently in fill mode
}
```

---

## Line & Shape Settings

```cpp
ofSetLineWidth(3.0f);            // Set line thickness
float w = ofGetLineWidth();

ofSetCircleResolution(64);       // Smoother circles (default: 32)
uint32_t res = ofGetCircleResolution();

ofSetCurveResolution(30);        // Curve smoothness (default: 20)
```

---

## Basic 2D Shapes

### Line

```cpp
ofDrawLine(0, 0, 100, 100);      // Draw line from (0,0) to (100,100)
```

### Rectangle

```cpp
ofDrawRectangle(10, 10, 200, 100);      // x, y, width, height
ofDrawRectRounded(10, 10, 200, 100, 10); // Rounded corners (radius: 10)
```

### Circle & Ellipse

```cpp
ofDrawCircle(400, 300, 50);      // x, y, radius
ofDrawEllipse(400, 300, 100, 50); // x, y, width, height
```

### Triangle

```cpp
ofDrawTriangle(100, 100, 200, 100, 150, 200);  // x1,y1, x2,y2, x3,y3
```

---

## Curves

### Catmull-Rom Curve

```cpp
// 4 control points: p0, p1, p2, p3
// Curve goes through p1 and p2
ofDrawCurve(
    0, 100,       // p0: influences start
    100, 100,     // p1: start point
    200, 200,     // p2: end point
    300, 200      // p3: influences end
);
```

### Bezier Curve

```cpp
// Cubic bezier: start, control1, control2, end
ofDrawBezier(
    0, 100,       // start
    100, 0,       // control point 1
    200, 0,       // control point 2
    300, 100      // end
);
```

---

## Custom Shapes (ofPath-style)

Build complex shapes vertex-by-vertex:

```cpp
// Simple polygon
ofBeginShape();
    ofVertex(100, 100);
    ofVertex(200, 100);
    ofVertex(200, 200);
    ofVertex(100, 200);
ofEndShape(true);  // true = close shape

// Curve-based shape
ofBeginShape();
    ofVertex(0, 100);
    ofCurveVertex(100, 100);
    ofCurveVertex(200, 50);
    ofCurveVertex(300, 100);
    ofVertex(400, 100);
ofEndShape();

// Bezier-based shape
ofBeginShape();
    ofVertex(0, 100);
    ofBezierVertex(50, 0, 150, 0, 200, 100);
    ofBezierVertex(250, 200, 350, 200, 400, 100);
ofEndShape();

// Multiple contours (holes)
ofBeginShape();
    // Outer contour
    ofVertex(0, 0);
    ofVertex(200, 0);
    ofVertex(200, 200);
    ofVertex(0, 200);

    ofNextContour();  // Start inner contour (hole)

    ofVertex(50, 50);
    ofVertex(150, 50);
    ofVertex(150, 150);
    ofVertex(50, 150);
ofEndShape(true);
```

---

## Transformation Matrix Stack

Transform the coordinate system for easier drawing:

```cpp
ofPushMatrix();                  // Save current transformation

    ofTranslate(400, 300);       // Move origin to center
    ofRotate(45);                // Rotate 45° around Z axis
    ofScale(2.0f);               // Scale 2x

    ofDrawCircle(0, 0, 50);      // Draw at new origin

ofPopMatrix();                   // Restore transformation

// Query stack depth
int depth = ofGetMatrixStackDepth();
```

### Transformation Functions

```cpp
ofTranslate(x, y);               // Move 2D
ofTranslate(x, y, z);            // Move 3D

ofRotate(angle);                 // Rotate around Z (2D rotation)
ofRotateZ(angle);                // Same as above
ofRotateX(angle);                // Rotate around X
ofRotateY(angle);                // Rotate around Y
ofRotate(angle, x, y, z);        // Rotate around arbitrary axis

ofScale(s);                      // Uniform scale
ofScale(sx, sy);                 // Non-uniform 2D scale
ofScale(sx, sy, sz);             // Non-uniform 3D scale

// Matrix operations
ofLoadIdentityMatrix();          // Reset to identity
ofLoadMatrix(mat);               // Load custom matrix
ofMultMatrix(mat);               // Multiply by matrix
```

---

## Blending Modes

```cpp
ofEnableAlphaBlending();         // Standard alpha blending (default)
ofDisableAlphaBlending();        // No blending

// Advanced blend modes
ofEnableBlendMode(OF_BLENDMODE_ALPHA);      // Standard
ofEnableBlendMode(OF_BLENDMODE_ADD);        // Additive
ofEnableBlendMode(OF_BLENDMODE_SUBTRACT);   // Subtractive
ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);   // Multiply
ofEnableBlendMode(OF_BLENDMODE_SCREEN);     // Screen
ofEnableBlendMode(OF_BLENDMODE_DISABLED);   // No blending
```

---

## Complete Example

```cpp
#include <oflike/graphics/ofGraphics.h>

class MyApp : public ofBaseApp {
    float rotation = 0;

    void setup() override {
        ofSetCircleResolution(64);
        ofSetLineWidth(2);
    }

    void update() override {
        rotation += 1;
    }

    void draw() override {
        // Clear background
        ofBackground(20, 20, 30);

        // Draw filled circle
        ofFill();
        ofSetColor(255, 100, 100);
        ofDrawCircle(200, 200, 80);

        // Draw outlined rectangle
        ofNoFill();
        ofSetColor(100, 255, 100);
        ofDrawRectangle(300, 150, 200, 100);

        // Rotating shape at center
        ofPushMatrix();
            ofTranslate(ctx().getWindowWidth()/2,
                        ctx().getWindowHeight()/2);
            ofRotate(rotation);

            ofFill();
            ofSetColor(100, 100, 255, 200);
            ofDrawRectangle(-50, -50, 100, 100);
        ofPopMatrix();

        // Curve
        ofSetColor(255, 255, 0);
        ofSetLineWidth(3);
        ofDrawBezier(100, 400, 200, 300, 400, 300, 500, 400);

        // Custom shape
        ofFill();
        ofSetColor(255, 0, 255, 150);
        ofBeginShape();
            ofVertex(600, 300);
            ofVertex(700, 300);
            ofVertex(700, 400);
            ofVertex(650, 450);
            ofVertex(600, 400);
        ofEndShape(true);
    }
};
```

---

## Coordinate System

- **Origin**: Top-left (0, 0)
- **X-axis**: Right is positive
- **Y-axis**: Down is positive
- **Rotation**: Clockwise is positive (around Z axis)

---

## Performance Tips

- **Batching**: Multiple calls are batched automatically
- **Resolution**: Lower circle/curve resolution = better performance
- **Fill mode**: Filled shapes are faster than stroked shapes with wide lines
- **Matrix stack**: Push/Pop is fast, but avoid deep nesting

---

## See Also

- [00_foundation.md](00_foundation.md) - Application structure
- [01_math.md](01_math.md) - Math types and operations
- [03_graphics_3d.md](03_graphics_3d.md) - 3D drawing
- [04_color.md](04_color.md) - Color classes
