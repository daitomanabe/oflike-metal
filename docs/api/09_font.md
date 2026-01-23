# Font API - Text Rendering

## Overview

TrueType font rendering using Core Text and Metal.

## Header

```cpp
#include <oflike/graphics/ofTrueTypeFont.h>
```

---

## ofTrueTypeFont

```cpp
ofTrueTypeFont font;

// Load font
font.load("Arial.ttf", 24);                // Font file, size
font.load("Helvetica", 32, true, true);    // System font, size, antialiased, full charset

// Draw text
font.drawString("Hello World", x, y);

// Text properties
ofRectangle bbox = font.getStringBoundingBox("Hello", x, y);
float width = bbox.width;
float height = bbox.height;

// Font metrics
float lineHeight = font.getLineHeight();
float ascender = font.getAscenderHeight();
float descender = font.getDescenderHeight();

// Character info
bool hasChar = font.hasCharacter('A');
```

---

## Example

```cpp
class MyApp : public ofBaseApp {
    ofTrueTypeFont titleFont;
    ofTrueTypeFont bodyFont;

    void setup() override {
        titleFont.load("Arial.ttf", 48);
        bodyFont.load("Helvetica", 18);
    }

    void draw() override {
        ofBackground(0);

        ofSetColor(255);
        titleFont.drawString("oflike-metal", 100, 100);

        ofSetColor(200);
        bodyFont.drawString("Hardware-accelerated graphics", 100, 150);
    }
};
```

---

## Supported Fonts

- **TrueType** (.ttf)
- **OpenType** (.otf)
- **System fonts**: "Helvetica", "Arial", "Monaco", etc.
- **Japanese/Unicode**: Full Unicode support via Core Text

---

## See Also

- [02_graphics_2d.md](02_graphics_2d.md) - Drawing functions
- [04_color.md](04_color.md) - Text color
