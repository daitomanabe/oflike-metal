# Example 21: Face Detection with Vision.framework

This example demonstrates face detection using the `ofxOpenCv` addon, which uses Apple's Vision.framework for high-performance computer vision on macOS and iOS.

## Features Demonstrated

- **Face Detection**: Detect faces in images using `VisionDetector`
- **Bounding Boxes**: Draw detection results with normalized coordinates
- **Confidence Scores**: Display detection confidence (0.0 - 1.0)
- **Drag & Drop**: Load images by dragging them into the window
- **Real-time Processing**: Fast detection using Neural Engine acceleration

## How to Use

1. **Run the example**:
   ```bash
   cd examples/21_face_detection
   # Build and run (see project build instructions)
   ```

2. **Load an image**:
   - The example will look for `test_faces.jpg` in the data folder
   - If not found, a placeholder image will be shown
   - **Drag and drop** any image file with faces into the window

3. **View results**:
   - Green boxes: High confidence (>80%)
   - Yellow boxes: Medium confidence (60-80%)
   - Orange boxes: Low confidence (50-60%)
   - Detection time is shown in milliseconds

4. **Re-run detection**:
   - Press **SPACE** to re-run detection on the current image

## Code Walkthrough

### Setup
```cpp
ofxCv::VisionDetector detector;
detector.setMinimumConfidence(0.5f);  // 50% confidence threshold
```

### Detection
```cpp
const ofPixels& pixels = sourceImage.getPixels();
std::vector<ofxCv::FaceDetection> faces;
bool success = detector.detectFaces(pixels, faces);
```

### Drawing Results
```cpp
for (const auto& face : faces) {
    // Coordinates are normalized [0,1]
    float x = face.boundingBox.x * imageWidth;
    float y = face.boundingBox.y * imageHeight;
    float w = face.boundingBox.width * imageWidth;
    float h = face.boundingBox.height * imageHeight;

    ofDrawRectangle(x, y, w, h);
}
```

## Vision.framework vs OpenCV

This addon uses **Vision.framework** instead of traditional OpenCV:

| Feature | Vision.framework | OpenCV |
|---------|-----------------|--------|
| Performance | Neural Engine acceleration | CPU/GPU |
| Integration | Native macOS/iOS | Cross-platform |
| Face Detection | VNDetectFaceRectanglesRequest | Haar Cascades |
| Memory | Zero-copy with Metal | Requires data conversion |
| Dependencies | Built-in (macOS 10.13+) | External library |

## Architecture

- **ofxCv namespace**: Contains all computer vision classes
- **VisionDetector**: Main detection class with pImpl pattern
- **FaceDetection**: Result structure with bounding box and confidence
- **Normalized coordinates**: All positions are in [0,1] range (top-left origin)

## Performance Notes

- Typical detection time: **5-15ms** for 1920x1080 images on Apple Silicon
- Neural Engine acceleration is automatic on supported hardware
- Multiple faces can be detected in a single image
- Detection is synchronous (blocking) but very fast

## Testing Tips

1. Use images with clear, front-facing faces for best results
2. Good lighting improves detection accuracy
3. Face size should be at least 32x32 pixels
4. Side profiles and occluded faces may not be detected
5. Try different confidence thresholds (0.3 - 0.8) for your use case

## Next Steps

- See `ofxCvVisionDetector.h` for more detection types:
  - Human detection (full body)
  - Text recognition (OCR)
  - Barcode detection (QR codes, etc.)
- Check `ofxCvImageConversion.h` for pixel format conversions
- Explore real-time video detection using `ofVideoGrabber`

## Related Examples

- **04_image**: Basic image loading and display
- **10_fbo**: Framebuffer objects for image processing
- **07_camera**: Camera capture (for real-time face detection)
