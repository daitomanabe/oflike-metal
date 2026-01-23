# Image & Texture API

## Overview

Image loading, manipulation, and texture rendering using ImageIO and Metal.

## Headers

```cpp
#include <oflike/image/ofImage.h>
#include <oflike/image/ofTexture.h>
#include <oflike/image/ofPixels.h>
```

---

## ofImage - Image Loading & Drawing

```cpp
// Loading
ofImage img;
img.load("photo.jpg");           // Load from file

// Drawing
img.draw(x, y);                  // Draw at position
img.draw(x, y, w, h);            // Draw with size

// Properties
int width = img.getWidth();
int height = img.getHeight();

// Pixel access
ofPixels& pixels = img.getPixels();
ofColor color = pixels.getColor(x, y);

// Modification
img.resize(newW, newH);
img.crop(x, y, w, h);
img.rotate90(rotations);

// Texture access
ofTexture& tex = img.getTexture();
```

---

## ofTexture - GPU Texture

```cpp
ofTexture tex;

// From image
tex.loadData(pixels);

// Drawing
tex.draw(x, y);
tex.draw(x, y, w, h);

// Binding (for custom rendering)
tex.bind();
// ... custom rendering ...
tex.unbind();

// Properties
int w = tex.getWidth();
int h = tex.getHeight();
bool allocated = tex.isAllocated();

// Settings
tex.setTextureWrap(GL_REPEAT, GL_REPEAT);
tex.setTextureMinMagFilter(GL_LINEAR, GL_LINEAR);
```

---

## ofPixels - Pixel Buffer

```cpp
ofPixels pixels;
pixels.allocate(width, height, OF_PIXELS_RGBA);

// Access
ofColor c = pixels.getColor(x, y);
pixels.setColor(x, y, ofColor::red);

// Properties
int w = pixels.getWidth();
int h = pixels.getHeight();
size_t channels = pixels.getNumChannels();
unsigned char* data = pixels.getData();

// Operations
pixels.resize(newW, newH);
pixels.crop(x, y, w, h);
pixels.mirror(vertical, horizontal);
pixels.rotate90(rotations);
```

---

## Supported Formats

- **JPEG** (.jpg, .jpeg)
- **PNG** (.png)
- **TIFF** (.tif, .tiff)
- **BMP** (.bmp)
- **GIF** (.gif)

All formats handled by macOS ImageIO framework.

---

## Example

```cpp
class MyApp : public ofBaseApp {
    ofImage image;
    ofTexture texture;

    void setup() override {
        // Load image
        image.load("photo.jpg");

        // Create custom texture
        ofPixels pix;
        pix.allocate(256, 256, OF_PIXELS_RGBA);
        for (int y = 0; y < 256; y++) {
            for (int x = 0; x < 256; x++) {
                pix.setColor(x, y, ofColor(x, y, 128));
            }
        }
        texture.loadData(pix);
    }

    void draw() override {
        ofBackground(0);

        // Draw loaded image
        image.draw(0, 0);

        // Draw custom texture
        texture.draw(300, 0, 256, 256);
    }
};
```

---

## See Also

- [02_graphics_2d.md](02_graphics_2d.md) - Drawing functions
- [04_color.md](04_color.md) - Color classes
