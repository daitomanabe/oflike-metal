# Example 26: Neural Image Classification

Image classification using Core ML and Neural Engine via `ofxNeuralEngine::ImageClassifier`.

## What This Example Demonstrates

- Loading Core ML image classification models (`.mlmodelc`)
- Running inference with Neural Engine acceleration
- Getting top-K classification results with confidence scores
- Handling model loading errors gracefully
- Understanding Core ML preprocessing requirements

## API Usage

```cpp
#include "ofxNeuralEngine.h"

NeuralEngine::ImageClassifier classifier;

// Load model
bool success = classifier.load("MobileNetV3.mlmodelc");

// Classify image (top 5 results)
auto results = classifier.classify(myPixels, 5);

// Process results
for (const auto& result : results) {
    std::cout << result.label << ": "
              << (result.confidence * 100) << "%" << std::endl;
}

// Check Neural Engine support
bool hasNE = classifier.isNeuralEngineSupported();
```

## Using Real Models

This example demonstrates the API with synthetic test data. To use it with real models:

### 1. Download a Core ML Model

Visit [Apple's Core ML Models](https://developer.apple.com/machine-learning/models/) and download an image classification model. Popular choices:

- **MobileNetV3** - Fast, good for real-time (224×224 input)
- **ResNet50** - Accurate, larger model (224×224 input)
- **EfficientNet** - Balanced accuracy/speed (varies)

### 2. Compile the Model

Core ML models come in two formats:
- `.mlmodel` - Source format (needs compilation)
- `.mlmodelc` - Compiled format (ready to use)

If you have a `.mlmodel`, compile it:

```bash
xcrun coremlcompiler compile MyModel.mlmodel ./
```

This creates a `MyModel.mlmodelc` directory.

### 3. Update the Example

In `main.mm`, update the model path:

```cpp
std::string modelPath = "path/to/MobileNetV3.mlmodelc";
```

Also update image dimensions to match model requirements:

```cpp
int imageWidth = 224;   // Match your model's input size
int imageHeight = 224;
```

### 4. Provide Test Images

Replace the synthetic test image with real images:

```cpp
ofImage testImage;
testImage.load("path/to/test_image.jpg");
ofPixels& pixels = testImage.getPixels();
auto results = classifier.classify(pixels, 5);
```

## Model Requirements

Core ML image classification models typically expect:

- **Input**: RGB image (3 channels)
- **Common sizes**: 224×224, 299×299, 384×384
- **Pixel format**: Normalized or raw (handled automatically by ImageClassifier)
- **Output**: Probability distribution over classes

## Neural Engine Acceleration

The ImageClassifier automatically uses Neural Engine when:
- Running on Apple Silicon (M1, M2, M3, etc.)
- Model supports Neural Engine operations
- Model compiled with Neural Engine optimization

Check availability:
```cpp
if (classifier.isNeuralEngineSupported()) {
    std::cout << "Using Neural Engine!" << std::endl;
}
```

## Performance Notes

Typical classification times on Apple Silicon:

| Model | Input Size | Neural Engine | Time |
|-------|-----------|---------------|------|
| MobileNetV3 | 224×224 | Yes | 1-3 ms |
| ResNet50 | 224×224 | Yes | 3-8 ms |
| EfficientNet-B0 | 224×224 | Yes | 2-5 ms |

Without Neural Engine (CPU/GPU only), times are typically 10-50x slower.

## Controls

- **SPACE** - Load model and run classification
- **1-9** - Set number of top results to display (1-9)
- **R** - Re-run classification on current image

## Example Output

```
=== Running Classification ===
✓ Classification complete in 2.34 ms

Top 5 predictions:
  1. Egyptian cat (45.2%)
  2. tabby (23.1%)
  3. tiger cat (15.7%)
  4. lynx (8.3%)
  5. Persian cat (4.2%)
```

## Error Handling

The example demonstrates proper error handling:

```cpp
bool loaded = classifier.load(modelPath);
if (!loaded) {
    std::string error = classifier.getLastError();
    ofLog() << "Failed to load model: " << error;
}
```

Common errors:
- Model file not found
- Invalid model format
- Incompatible model architecture
- Memory allocation failure

## Integration with Other Components

### With ofxGui

```cpp
ofxGuiPanel gui;
gui.addSlider("Top K", topK, 1, 10);
gui.addButton("Classify").addListener([&]() {
    results = classifier.classify(pixels, topK);
});
```

### With Camera Input

```cpp
// In ofApp::update()
if (camera.isFrameNew()) {
    results = classifier.classify(camera.getPixels(), 5);
}
```

### With File Browser

```cpp
void ofApp::dragEvent(ofDragInfo dragInfo) {
    for (const auto& file : dragInfo.files) {
        ofImage img;
        img.load(file);
        results = classifier.classify(img.getPixels(), 5);
    }
}
```

## See Also

- `ImageClassifier.h` - Full API documentation
- Example 27: Neural Pose Estimation
- Example 28: Neural Depth Estimation
- Example 29: Neural Style Transfer
- [Core ML Documentation](https://developer.apple.com/documentation/coreml)
- [Neural Engine Guide](https://github.com/hollance/neural-engine)

## Notes

- Neural Engine is only available on Apple Silicon devices
- Intel Macs will use CPU/GPU compute (slower)
- Model size affects memory usage but not inference speed significantly
- Batch processing is not demonstrated but supported by the API
- Image preprocessing (resize, crop) is handled automatically
