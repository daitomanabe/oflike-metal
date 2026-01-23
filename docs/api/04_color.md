# Color API

## Overview

Color representation and manipulation classes compatible with openFrameworks.

## Headers

```cpp
#include <oflike/types/ofColor.h>
```

---

## ofColor - 8-bit Color (0-255)

```cpp
// Construction
ofColor red(255, 0, 0);           // RGB
ofColor transparent(255, 0, 0, 128);  // RGBA
ofColor gray(128);                // Grayscale
ofColor::red                      // Predefined colors
ofColor::fromHex(0xFF0000)        // From hex

// Access components
uint8_t r = color.r;
uint8_t g = color.g;
uint8_t b = color.b;
uint8_t a = color.a;

// Arithmetic
ofColor mixed = color1 + color2;
ofColor scaled = color * 0.5f;

// Interpolation
ofColor lerped = color1.getLerped(color2, 0.5f);

// Conversion
uint32_t hex = color.getHex();
ofFloatColor floatColor = color;
```

---

## ofFloatColor - Floating Point Color (0.0-1.0)

```cpp
// Construction
ofFloatColor red(1.0f, 0.0f, 0.0f);
ofFloatColor transparent(1.0f, 0.0f, 0.0f, 0.5f);

// Access components
float r = color.r;
float g = color.g;
float b = color.b;
float a = color.a;

// Same operations as ofColor
ofFloatColor mixed = color1 + color2;
ofFloatColor scaled = color * 0.5f;
```

---

## Predefined Colors

```cpp
ofColor::white      // (255, 255, 255)
ofColor::black      // (0, 0, 0)
ofColor::red        // (255, 0, 0)
ofColor::green      // (0, 255, 0)
ofColor::blue       // (0, 0, 255)
ofColor::yellow     // (255, 255, 0)
ofColor::cyan       // (0, 255, 255)
ofColor::magenta    // (255, 0, 255)
```

---

## Example

```cpp
// Gradient
for (int i = 0; i < 256; i++) {
    ofColor c = ofColor::red.getLerped(ofColor::blue, i / 255.0f);
    ofSetColor(c);
    ofDrawLine(i, 0, i, height);
}

// Alpha blending
ofEnableAlphaBlending();
ofColor semi(255, 0, 0, 128);
ofSetColor(semi);
ofDrawCircle(x, y, 50);
```

---

## See Also

- [02_graphics_2d.md](02_graphics_2d.md) - Using colors in drawing
