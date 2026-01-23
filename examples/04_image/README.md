# 04_image - Image Loading and Manipulation

## Overview

This example demonstrates the **ofImage** class capabilities in oflike-metal, including:
- Loading and creating images
- Pixel manipulation (tinting, grayscale conversion)
- Image operations (cropping, resizing)
- Drawing with transformations
- Saving images to disk

## Visual Elements

### Mode 1: Original Image (Procedural)
- **512x512** procedurally generated test image
- **Radial gradient** from center
- **RGB channels** create colorful pattern
- Shows basic image creation using **ofPixels**

### Mode 2: Tinted Image
- **Pixel manipulation** example
- Applies **magenta tint** by modifying RGB channels
- R channel: `×1.2` (boost red)
- G channel: `×0.7` (reduce green)
- B channel: `×1.1` (boost blue)

### Mode 3: Grayscale Conversion
- **Luminance-based** conversion
- Formula: `0.299*R + 0.587*G + 0.114*B`
- Demonstrates per-pixel color processing
- **ofPixels** iteration and manipulation

### Mode 4: Cropped Image
- **256x256** center crop from original
- Shows **region extraction** from larger image
- Uses `getColor()` and `setColor()` for pixel copying
- Demonstrates **ofPixels** coordinate access

### Mode 5: Resized Image
- **256x256** downscaled version
- Uses `ofImage::resize()` method
- Bilinear interpolation (platform default)
- Shows **image scaling** capabilities

### Mode 6: Image with Effects
- **Animated rotation** using `ofRotateZ()`
- **Pulsing scale** with sine wave
- **Color tinting** with animated RGB
- Demonstrates **matrix transformations** with images

## Interactive Controls

| Key | Action |
|-----|--------|
| **SPACE** | Cycle through 6 different modes |
| **S** | Save current image to disk (PNG format) |

## API Functions Used

### ofImage Methods
```cpp
// Allocation and setup
ofPixels pixels;
pixels.allocate(width, height, OF_IMAGE_COLOR);
ofImage image;
image.setFromPixels(pixels);

// Drawing
image.draw(x, y);

// Operations
image.resize(newWidth, newHeight);
image.save("filename.png");

// Pixel access
ofPixels pixels = image.getPixels();
ofColor color = pixels.getColor(x, y);
pixels.setColor(x, y, ofColor(r, g, b));

// Status check
bool allocated = image.isAllocated();
```

### ofPixels Methods
```cpp
// Allocation
ofPixels pixels;
pixels.allocate(width, height, OF_IMAGE_COLOR);  // RGB
pixels.allocate(width, height, OF_IMAGE_GRAYSCALE);  // Grayscale
pixels.allocate(width, height, OF_IMAGE_COLOR_ALPHA);  // RGBA

// Pixel access
pixels.setColor(x, y, ofColor(r, g, b));
ofColor color = pixels.getColor(x, y);

// Raw data access
unsigned char* data = pixels.getData();
size_t totalBytes = pixels.size();

// Properties
int width = pixels.getWidth();
int height = pixels.getHeight();
int channels = pixels.getNumChannels();
```

### Graphics Functions (with images)
```cpp
// Matrix transformations
ofPushMatrix();
ofTranslate(x, y, z);
ofRotateZ(angle);
ofScale(sx, sy, sz);
// ... draw image ...
ofPopMatrix();

// Color tinting
ofSetColor(r, g, b);  // Tint color
image.draw(x, y);
ofSetColor(255);  // Reset to white (no tint)
```

## Image Formats Supported

oflike-metal uses **ImageIO** (Apple) and **MTKTextureLoader** for image I/O:

### Supported Input Formats
- **PNG** - Lossless, transparency support
- **JPEG** - Lossy compression
- **TIFF** - High-quality, multi-page
- **BMP** - Windows bitmap
- **GIF** - Animation support
- **HEIC** - Modern Apple format
- **WebP** - Modern web format (macOS 11+)

### Supported Output Formats (via save())
- **PNG** (default)
- **JPEG**
- **TIFF**

## Architecture Notes

### ImageIO Integration (ARCHITECTURE.md Section 7)
- ✅ **No stb_image** - Uses native **ImageIO** framework
- ✅ **MTKTextureLoader** - Direct GPU texture loading
- ✅ **CGImageDestination** - Image saving
- ✅ **Zero-copy** where possible (Unified Memory)

### Implementation Details
- **ofImage** wraps **ofPixels** (CPU) and **ofTexture** (GPU)
- **Lazy texture upload** - Texture created on first draw()
- **Pixel modifications** trigger texture re-upload
- **ofPixels** uses `std::vector<unsigned char>` for storage

## Expected Output

The example should display:
1. **Original Image**: Colorful radial gradient (512x512)
2. **Tinted Image**: Magenta-tinted version
3. **Grayscale**: Black and white conversion
4. **Cropped**: 256x256 center region
5. **Resized**: 256x256 scaled version
6. **Effects**: Animated rotation, scaling, and tint

Press **SPACE** to cycle through modes.
Press **S** to save the current image to disk.

## Learning Points

### 1. Image Creation
```cpp
// Method 1: From pixels
ofPixels pixels;
pixels.allocate(512, 512, OF_IMAGE_COLOR);
// ... fill pixels ...
ofImage image;
image.setFromPixels(pixels);

// Method 2: Load from file
ofImage image;
image.load("path/to/image.png");
```

### 2. Pixel Manipulation
```cpp
// Get pixels, modify, set back
ofPixels pixels = image.getPixels();
for (int y = 0; y < pixels.getHeight(); y++) {
    for (int x = 0; x < pixels.getWidth(); x++) {
        ofColor color = pixels.getColor(x, y);
        // Modify color...
        pixels.setColor(x, y, color);
    }
}
image.setFromPixels(pixels);
```

### 3. Raw Pixel Access (Fast)
```cpp
ofPixels pixels = image.getPixels();
unsigned char* data = pixels.getData();
int channels = pixels.getNumChannels();

for (size_t i = 0; i < pixels.size(); i += channels) {
    data[i + 0] = ...; // R
    data[i + 1] = ...; // G
    data[i + 2] = ...; // B
    // data[i + 3] for alpha if RGBA
}
image.setFromPixels(pixels);
```

### 4. Image Transformations
```cpp
// Drawing with transformations
ofPushMatrix();
ofTranslate(x, y);
ofRotateZ(angle);
ofScale(scale);
ofSetColor(tintR, tintG, tintB);  // Optional tint
image.draw(0, 0);
ofPopMatrix();
```

## Integration with Your Project

```cpp
#include "ofImage.h"

class MyApp : public ofBaseApp {
public:
    void setup() override {
        // Load image from file
        myImage.load("images/photo.jpg");

        // Or create procedurally
        ofPixels pixels;
        pixels.allocate(256, 256, OF_IMAGE_COLOR);
        // ... generate pixels ...
        myImage.setFromPixels(pixels);
    }

    void draw() override {
        // Draw image
        myImage.draw(100, 100);

        // Or with size
        myImage.draw(100, 100, 400, 300);

        // Or with tint
        ofSetColor(255, 200, 200);
        myImage.draw(100, 100);
    }

private:
    ofImage myImage;
};
```

## Performance Notes

- **Texture caching**: Textures are cached until pixels change
- **GPU upload**: Happens automatically on first draw after pixel modification
- **Unified Memory** (Apple Silicon): Zero-copy CPU↔GPU transfer where possible
- **ImageIO**: Hardware-accelerated decoding on Apple platforms
- **Batch drawing**: Multiple `draw()` calls of same image are efficient (shared texture)

## Common Use Cases

### Photo Viewer
```cpp
ofImage photo;
photo.load("photo.jpg");
photo.draw(0, 0, ofGetWidth(), ofGetHeight());
```

### Image Filter
```cpp
ofPixels pixels = inputImage.getPixels();
// Apply filter to pixels...
outputImage.setFromPixels(pixels);
```

### Texture Mapping
```cpp
// ofImage provides ofTexture via getTexture()
ofTexture& tex = myImage.getTexture();
tex.bind();
// Draw 3D geometry with texture...
tex.unbind();
```

## Next Steps

- **05_typography**: Text rendering with **ofTrueTypeFont**
- **10_fbo**: Render to texture with **ofFbo**
- **Image filters**: Use **ofxMPS** addon for GPU-accelerated filters
- **Video**: **ofVideoPlayer** for video file playback
- **Camera**: **ofVideoGrabber** for webcam input

## References

- [openFrameworks ofImage documentation](https://openframeworks.cc/documentation/graphics/ofImage/)
- [Apple ImageIO framework](https://developer.apple.com/documentation/imageio)
- [Metal Texture Loading](https://developer.apple.com/documentation/metalkit/mtkttextureloader)
