# ofxSharp Models Directory

This directory contains Core ML models for SHARP (3D Gaussian Splatting) inference.

## Model Files

### Expected Structure

```
models/
├── README.md              # This file
├── sharp.mlpackage/       # Core ML model (source, version controlled)
│   ├── Data/
│   │   └── ...
│   └── Manifest.json
└── sharp.mlmodelc/        # Compiled model (auto-generated, .gitignored)
    ├── model.mil
    ├── weights/
    └── ...
```

## File Types

### `.mlpackage` (Source Model)

- **Purpose**: Human-readable Core ML model format
- **Version Control**: ✅ Should be committed to Git
- **Size**: Larger (includes metadata and uncompressed weights)
- **Usage**: Source format, compiled at build time

### `.mlmodelc` (Compiled Model)

- **Purpose**: Optimized runtime format
- **Version Control**: ❌ Should NOT be committed (auto-generated)
- **Size**: Smaller (optimized and compressed)
- **Usage**: Loaded by Core ML at runtime

## Getting Models

### Option 1: Convert from PyTorch (Recommended)

Use the conversion script:

```bash
cd ../scripts/
python convert_to_coreml.py \
  --input /path/to/sharp_pytorch.pth \
  --output ../models/sharp.mlpackage
```

See `../scripts/README.md` for detailed instructions.

### Option 2: Download Pretrained Models

If pretrained Core ML models become available:

```bash
# Download from releases or external source
curl -L -o sharp.mlpackage.zip https://example.com/sharp.mlpackage.zip
unzip sharp.mlpackage.zip -d ./
```

### Option 3: Train Your Own

Follow the [Apple ML-SHARP repository](https://github.com/apple/ml-sharp) to train a custom model.

## Model Compilation

### Automatic Compilation (Xcode)

When building the project, Xcode automatically compiles `.mlpackage` to `.mlmodelc`:

```bash
# Build triggers compilation
xcodebuild -scheme oflike-metal build
```

### Manual Compilation

```bash
# Compile .mlpackage to .mlmodelc
xcrun coremlcompiler compile sharp.mlpackage ./

# Result: ./sharp.mlmodelc/
```

## Model Specifications

### Input Requirements

| Parameter   | Value              |
|-------------|-------------------|
| Format      | RGB Image         |
| Shape       | (1, 3, H, W)      |
| Data Type   | float32           |
| Range       | [0.0, 1.0]        |
| Default Res | 512 × 512         |

### Output Format

| Output      | Shape    | Data Type | Description                    |
|-------------|----------|-----------|--------------------------------|
| positions   | (N, 3)   | float32   | 3D positions (x, y, z)        |
| colors      | (N, 3)   | float32   | RGB colors [0, 1]             |
| opacities   | (N, 1)   | float32   | Alpha values [0, 1]           |
| scales      | (N, 3)   | float32   | Covariance scale factors      |
| rotations   | (N, 4)   | float32   | Quaternions (w, x, y, z)      |

Where `N` = number of Gaussians (model-dependent, typically 1000-10000)

## Model Variants

You can have multiple model variants for different use cases:

```
models/
├── sharp_512.mlpackage       # 512×512 input (balanced)
├── sharp_1024.mlpackage      # 1024×1024 input (high quality)
├── sharp_256.mlpackage       # 256×256 input (fast)
├── sharp_quantized.mlpackage # INT8 quantized (smallest)
└── README.md
```

Load the appropriate model:

```cpp
Sharp::SharpModel model;

// Choose based on performance needs
model.load("sharp_512.mlmodelc");      // Balanced
model.load("sharp_1024.mlmodelc");     // High quality, slower
model.load("sharp_quantized.mlmodelc"); // Fast, smaller
```

## Performance Expectations

### Apple Silicon (M1/M2/M3/M4)

| Resolution | Inference Time | Memory Usage | Target Device |
|------------|---------------|--------------|---------------|
| 256×256    | < 0.3s        | ~200 MB      | All M-series  |
| 512×512    | < 1.0s        | ~500 MB      | M1 Pro+       |
| 1024×1024  | < 3.0s        | ~2 GB        | M1 Max+       |

**Note**: Actual performance depends on model architecture and Neural Engine availability.

## Verification

Test that a model loads correctly:

```cpp
#include "ofxSharp.h"

Sharp::SharpModel model;
if (model.load("sharp.mlmodelc")) {
    ofLog() << "✓ Model loaded successfully";
    ofLog() << "  Input: " << model.getInputWidth() << "×" << model.getInputHeight();
    ofLog() << "  Neural Engine: " << (model.usesNeuralEngine() ? "YES" : "NO");
} else {
    ofLogError() << "✗ Model loading failed";
}
```

## Troubleshooting

### "Model not found"

- Verify the `.mlmodelc` directory exists (not `.mlpackage`)
- Check the path is relative to the app bundle
- Ensure the model was compiled (build the project)

### "Model loading failed"

- Check that you're loading `.mlmodelc`, not `.mlpackage`
- Verify Core ML version compatibility (macOS 13.0+)
- Check for error messages in the console

### "Poor inference performance"

- Verify Neural Engine is being used (check logs)
- Ensure compute units are set to `ALL` in conversion
- Try quantized model for faster inference
- Reduce input resolution

### "Model too large"

- Apply INT8 quantization during conversion
- Use smaller input resolution (e.g., 256×256)
- Reduce number of output Gaussians in training

## Git Configuration

Add to `.gitignore`:

```gitignore
# Compiled Core ML models (auto-generated)
*.mlmodelc/

# Temporary files
*.mlpackage.zip
*.tmp
```

Keep in version control:

```gitignore
# Source Core ML models (human-readable)
!*.mlpackage/
```

## References

- **Core ML Model Format**: https://apple.github.io/coremltools/
- **Neural Engine Guide**: https://machinelearning.apple.com/
- **Apple ML-SHARP**: https://github.com/apple/ml-sharp

## License

Models should include appropriate license information. Check the source before distribution.
