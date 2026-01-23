# FBO API - Framebuffer Objects

## Overview

Render to texture using Framebuffer Objects (FBO) for post-processing, multi-pass rendering, and offscreen rendering.

## Header

```cpp
#include <oflike/graphics/ofFbo.h>
```

---

## ofFbo - Framebuffer Object

```cpp
ofFbo fbo;

// Allocation
fbo.allocate(width, height);
fbo.allocate(width, height, internalFormat);

// Render to FBO
fbo.begin();
    ofClear(0, 0, 0, 255);
    // Draw content here
    ofDrawCircle(width/2, height/2, 100);
fbo.end();

// Draw FBO contents
fbo.draw(x, y);
fbo.draw(x, y, w, h);

// Access texture
ofTexture& tex = fbo.getTexture();

// Properties
int w = fbo.getWidth();
int h = fbo.getHeight();
bool allocated = fbo.isAllocated();

// Cleanup
fbo.clear();
```

---

## Example: Post-Processing

```cpp
class MyApp : public ofBaseApp {
    ofFbo fbo;

    void setup() override {
        fbo.allocate(1024, 768);
    }

    void draw() override {
        // Render scene to FBO
        fbo.begin();
            ofClear(0);
            ofSetColor(255, 100, 100);
            ofDrawCircle(512, 384, 200);
        fbo.end();

        // Draw FBO with effects
        ofBackground(0);
        ofSetColor(255);
        fbo.draw(0, 0);
    }
};
```

---

## Use Cases

- **Post-processing**: Blur, bloom, etc.
- **Multi-pass rendering**: Reflections, shadows
- **Texture generation**: Procedural textures
- **Offscreen rendering**: Hidden buffer rendering

---

## See Also

- [05_image.md](05_image.md) - Texture operations
- [02_graphics_2d.md](02_graphics_2d.md) - Drawing functions
