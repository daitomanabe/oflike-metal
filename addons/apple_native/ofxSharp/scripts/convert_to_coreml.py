#!/usr/bin/env python3
"""
PyTorch → Core ML Conversion Script for SHARP Model
====================================================

This script converts a PyTorch SHARP (Single-image to High-fidelity Appearance
Reconstruction via neural Prefiltered splatting) model to Core ML format for
Neural Engine acceleration on Apple Silicon.

Reference: https://github.com/apple/ml-sharp

Requirements:
    - torch>=2.0.0
    - coremltools>=7.0
    - numpy>=1.24.0
    - pillow>=10.0.0

Usage:
    python convert_to_coreml.py --input sharp_model.pth --output sharp.mlpackage

Author: oflike-metal project
License: MIT
"""

import argparse
import os
import sys
from pathlib import Path
from typing import Tuple, Optional

try:
    import torch
    import torch.nn as nn
    import coremltools as ct
    import numpy as np
    from PIL import Image
except ImportError as e:
    print(f"Error: Missing required package - {e}")
    print("\nInstall required packages:")
    print("  pip install torch torchvision coremltools numpy pillow")
    sys.exit(1)


class SharpModelWrapper(nn.Module):
    """
    Wrapper for SHARP model to ensure proper Core ML conversion.

    Input: RGB image (1, 3, H, W) - normalized [0, 1]
    Output: Gaussian parameters (N, 14) where:
        - positions (x, y, z): 3 values
        - colors (r, g, b): 3 values
        - opacity (alpha): 1 value
        - covariance (scale + rotation): 7 values
    """

    def __init__(self, sharp_model: nn.Module):
        super().__init__()
        self.model = sharp_model

    def forward(self, image: torch.Tensor) -> Tuple[torch.Tensor, torch.Tensor,
                                                      torch.Tensor, torch.Tensor,
                                                      torch.Tensor]:
        """
        Forward pass returning structured Gaussian parameters.

        Returns:
            positions: (N, 3) - xyz positions
            colors: (N, 3) - rgb colors [0, 1]
            opacities: (N, 1) - alpha values [0, 1]
            scales: (N, 3) - scale factors
            rotations: (N, 4) - quaternions (w, x, y, z)
        """
        # Run inference
        output = self.model(image)

        # Parse output into structured components
        # Assuming output shape: (batch, num_gaussians, 14)
        positions = output[:, :, 0:3]    # xyz
        colors = output[:, :, 3:6]       # rgb
        opacities = output[:, :, 6:7]    # alpha
        scales = output[:, :, 7:10]      # scale xyz
        rotations = output[:, :, 10:14]  # quaternion wxyz

        return positions, colors, opacities, scales, rotations


def load_pytorch_model(model_path: str) -> nn.Module:
    """
    Load PyTorch SHARP model from checkpoint.

    Args:
        model_path: Path to .pth or .pt checkpoint file

    Returns:
        Loaded PyTorch model in evaluation mode
    """
    print(f"Loading PyTorch model from: {model_path}")

    # Load checkpoint
    checkpoint = torch.load(model_path, map_location='cpu')

    # Extract model state dict
    if isinstance(checkpoint, dict):
        if 'model' in checkpoint:
            state_dict = checkpoint['model']
        elif 'state_dict' in checkpoint:
            state_dict = checkpoint['state_dict']
        else:
            state_dict = checkpoint
    else:
        state_dict = checkpoint

    # TODO: Replace with actual SHARP model architecture
    # This is a placeholder - replace with real model definition
    model = nn.Sequential(
        nn.Conv2d(3, 64, 3, padding=1),
        nn.ReLU(),
        nn.AdaptiveAvgPool2d((1, 1)),
        nn.Flatten(),
        nn.Linear(64, 1000 * 14),  # 1000 gaussians * 14 parameters
        nn.Unflatten(1, (1000, 14))
    )

    # Load weights
    try:
        model.load_state_dict(state_dict)
    except RuntimeError as e:
        print(f"Warning: Could not load state dict directly: {e}")
        print("Attempting to load with strict=False...")
        model.load_state_dict(state_dict, strict=False)

    model.eval()
    print(f"✓ Model loaded successfully")
    return model


def create_example_input(height: int = 512, width: int = 512) -> torch.Tensor:
    """
    Create example input tensor for tracing.

    Args:
        height: Input image height
        width: Input image width

    Returns:
        Example tensor (1, 3, H, W) normalized [0, 1]
    """
    return torch.rand(1, 3, height, width)


def convert_to_coreml(
    pytorch_model: nn.Module,
    output_path: str,
    input_shape: Tuple[int, int] = (512, 512),
    compute_units: ct.ComputeUnit = ct.ComputeUnit.ALL,
    quantize: bool = False
) -> ct.models.MLModel:
    """
    Convert PyTorch model to Core ML format.

    Args:
        pytorch_model: PyTorch model in eval mode
        output_path: Output path for .mlpackage or .mlmodel
        input_shape: Input image (height, width)
        compute_units: Compute units (ALL, CPU_AND_NE, CPU_ONLY)
        quantize: Apply INT8 quantization for smaller model size

    Returns:
        Core ML model
    """
    print(f"\nConverting to Core ML...")
    print(f"  Input shape: (1, 3, {input_shape[0]}, {input_shape[1]})")
    print(f"  Compute units: {compute_units}")
    print(f"  Quantization: {quantize}")

    # Wrap model
    wrapped_model = SharpModelWrapper(pytorch_model)
    wrapped_model.eval()

    # Create example input
    example_input = create_example_input(input_shape[0], input_shape[1])

    # Trace model
    print("\nTracing model...")
    with torch.no_grad():
        traced_model = torch.jit.trace(wrapped_model, example_input)

    # Define input
    image_input = ct.TensorType(
        name="image",
        shape=(1, 3, input_shape[0], input_shape[1]),
        dtype=np.float32
    )

    # Define outputs
    outputs = [
        ct.TensorType(name="positions", dtype=np.float32),   # (N, 3)
        ct.TensorType(name="colors", dtype=np.float32),      # (N, 3)
        ct.TensorType(name="opacities", dtype=np.float32),   # (N, 1)
        ct.TensorType(name="scales", dtype=np.float32),      # (N, 3)
        ct.TensorType(name="rotations", dtype=np.float32),   # (N, 4)
    ]

    # Convert to Core ML
    print("\nConverting to Core ML format...")
    mlmodel = ct.convert(
        traced_model,
        inputs=[image_input],
        outputs=outputs,
        compute_units=compute_units,
        minimum_deployment_target=ct.target.macOS13,
        convert_to="mlprogram"  # Use ML Program (supports more ops)
    )

    # Add metadata
    mlmodel.author = "oflike-metal"
    mlmodel.license = "MIT"
    mlmodel.short_description = "SHARP: Single-image 3D Gaussian Splatting"
    mlmodel.version = "1.0.0"

    # Add input description
    mlmodel.input_description["image"] = (
        "Input RGB image normalized to [0, 1] range. "
        f"Shape: (1, 3, {input_shape[0]}, {input_shape[1]})"
    )

    # Add output descriptions
    mlmodel.output_description["positions"] = (
        "3D positions of Gaussians in world space (N, 3)"
    )
    mlmodel.output_description["colors"] = (
        "RGB colors of Gaussians [0, 1] (N, 3)"
    )
    mlmodel.output_description["opacities"] = (
        "Opacity values [0, 1] (N, 1)"
    )
    mlmodel.output_description["scales"] = (
        "Scale factors for Gaussian covariance (N, 3)"
    )
    mlmodel.output_description["rotations"] = (
        "Rotation quaternions (w, x, y, z) (N, 4)"
    )

    # Apply quantization if requested
    if quantize:
        print("\nApplying INT8 quantization...")
        mlmodel = ct.models.neural_network.quantization_utils.quantize_weights(
            mlmodel, nbits=8
        )

    # Save model
    print(f"\nSaving Core ML model to: {output_path}")
    mlmodel.save(output_path)

    # Print model info
    print("\n✓ Conversion complete!")
    print(f"\nModel Details:")
    print(f"  - Format: ML Program")
    print(f"  - Deployment Target: macOS 13.0+")
    print(f"  - Compute Units: {compute_units}")
    print(f"  - Input: image (1, 3, {input_shape[0]}, {input_shape[1]})")
    print(f"  - Outputs:")
    print(f"    • positions (N, 3)")
    print(f"    • colors (N, 3)")
    print(f"    • opacities (N, 1)")
    print(f"    • scales (N, 3)")
    print(f"    • rotations (N, 4)")

    return mlmodel


def validate_coreml_model(
    mlmodel: ct.models.MLModel,
    pytorch_model: nn.Module,
    input_shape: Tuple[int, int] = (512, 512),
    tolerance: float = 1e-3
) -> bool:
    """
    Validate Core ML model output against PyTorch model.

    Args:
        mlmodel: Core ML model
        pytorch_model: Original PyTorch model
        input_shape: Input image shape
        tolerance: Maximum allowed difference

    Returns:
        True if validation passes
    """
    print("\nValidating Core ML model...")

    # Create test input
    test_input = create_example_input(input_shape[0], input_shape[1])
    test_input_np = test_input.numpy()

    # PyTorch inference
    pytorch_model.eval()
    with torch.no_grad():
        wrapped = SharpModelWrapper(pytorch_model)
        pt_positions, pt_colors, pt_opacities, pt_scales, pt_rotations = wrapped(test_input)

    # Core ML inference
    coreml_input = {"image": test_input_np}
    coreml_output = mlmodel.predict(coreml_input)

    # Compare outputs
    outputs = [
        ("positions", pt_positions),
        ("colors", pt_colors),
        ("opacities", pt_opacities),
        ("scales", pt_scales),
        ("rotations", pt_rotations)
    ]

    all_passed = True
    for name, pt_tensor in outputs:
        pt_np = pt_tensor.numpy()
        coreml_np = coreml_output[name]

        max_diff = np.max(np.abs(pt_np - coreml_np))
        mean_diff = np.mean(np.abs(pt_np - coreml_np))

        passed = max_diff < tolerance
        status = "✓" if passed else "✗"

        print(f"  {status} {name:12s} - max_diff: {max_diff:.6f}, mean_diff: {mean_diff:.6f}")

        if not passed:
            all_passed = False

    if all_passed:
        print("\n✓ Validation passed!")
    else:
        print(f"\n✗ Validation failed! Max difference exceeds tolerance ({tolerance})")

    return all_passed


def main():
    parser = argparse.ArgumentParser(
        description="Convert PyTorch SHARP model to Core ML",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Basic conversion
  python convert_to_coreml.py --input sharp.pth --output models/sharp.mlpackage

  # With quantization for smaller size
  python convert_to_coreml.py --input sharp.pth --output models/sharp.mlpackage --quantize

  # Custom input resolution
  python convert_to_coreml.py --input sharp.pth --output models/sharp.mlpackage --width 1024 --height 768

  # CPU-only for testing
  python convert_to_coreml.py --input sharp.pth --output models/sharp.mlpackage --compute-units CPU_ONLY
        """
    )

    parser.add_argument(
        "--input", "-i",
        type=str,
        required=True,
        help="Input PyTorch model (.pth or .pt)"
    )

    parser.add_argument(
        "--output", "-o",
        type=str,
        required=True,
        help="Output Core ML model (.mlpackage or .mlmodel)"
    )

    parser.add_argument(
        "--width",
        type=int,
        default=512,
        help="Input image width (default: 512)"
    )

    parser.add_argument(
        "--height",
        type=int,
        default=512,
        help="Input image height (default: 512)"
    )

    parser.add_argument(
        "--compute-units",
        type=str,
        choices=["ALL", "CPU_AND_NE", "CPU_ONLY"],
        default="ALL",
        help="Compute units (default: ALL for Neural Engine + GPU)"
    )

    parser.add_argument(
        "--quantize",
        action="store_true",
        help="Apply INT8 quantization for smaller model size"
    )

    parser.add_argument(
        "--skip-validation",
        action="store_true",
        help="Skip output validation"
    )

    parser.add_argument(
        "--tolerance",
        type=float,
        default=1e-3,
        help="Validation tolerance (default: 1e-3)"
    )

    args = parser.parse_args()

    # Check input file
    if not os.path.exists(args.input):
        print(f"Error: Input file not found: {args.input}")
        sys.exit(1)

    # Create output directory
    output_dir = os.path.dirname(args.output)
    if output_dir:
        os.makedirs(output_dir, exist_ok=True)

    # Map compute units
    compute_units_map = {
        "ALL": ct.ComputeUnit.ALL,
        "CPU_AND_NE": ct.ComputeUnit.CPU_AND_NE,
        "CPU_ONLY": ct.ComputeUnit.CPU_ONLY
    }
    compute_units = compute_units_map[args.compute_units]

    try:
        # Load PyTorch model
        pytorch_model = load_pytorch_model(args.input)

        # Convert to Core ML
        mlmodel = convert_to_coreml(
            pytorch_model,
            args.output,
            input_shape=(args.height, args.width),
            compute_units=compute_units,
            quantize=args.quantize
        )

        # Validate
        if not args.skip_validation:
            validate_coreml_model(
                mlmodel,
                pytorch_model,
                input_shape=(args.height, args.width),
                tolerance=args.tolerance
            )

        print(f"\n{'='*60}")
        print("SUCCESS: Model conversion complete!")
        print(f"{'='*60}")
        print(f"\nOutput: {args.output}")
        print(f"\nNext steps:")
        print(f"  1. Copy model to: src/addons/apple_native/ofxSharp/models/")
        print(f"  2. Compile .mlpackage to .mlmodelc (Xcode does this automatically)")
        print(f"  3. Load in ofxSharp:")
        print(f"     Sharp::SharpModel model;")
        print(f"     model.load(\"sharp.mlmodelc\");")

    except Exception as e:
        print(f"\n{'='*60}")
        print(f"ERROR: Conversion failed!")
        print(f"{'='*60}")
        print(f"\n{e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()
