#!/bin/bash

# Create a new SwiftUI example using oflike-gen
# Usage: ./scripts/create_example.sh <example_name>
# Example: ./scripts/create_example.sh 13_particles

if [ -z "$1" ]; then
    echo "Usage: $0 <example_name>"
    echo "Example: $0 13_particles"
    exit 1
fi

EXAMPLE_NAME="$1"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
EXAMPLE_DIR="$PROJECT_DIR/examples/$EXAMPLE_NAME"

if [ -d "$EXAMPLE_DIR" ]; then
    echo "Error: Example '$EXAMPLE_NAME' already exists"
    exit 1
fi

echo "Creating example: $EXAMPLE_NAME"

mkdir -p "$PROJECT_DIR/examples"

GENERATOR="$PROJECT_DIR/bin/oflike-gen"
if [ ! -x "$GENERATOR" ]; then
    GENERATOR="oflike-gen"
fi

"$GENERATOR" new "$EXAMPLE_NAME" --path "$PROJECT_DIR/examples" --no-git

echo ""
echo "Created example: $EXAMPLE_DIR"
echo ""
echo "To open in Xcode:"
echo "  open $EXAMPLE_DIR/$EXAMPLE_NAME.xcodeproj"
