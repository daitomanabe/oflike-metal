#!/bin/bash

# Open oflike-metal in Xcode
# Usage: ./open_xcode.sh

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
XCODE_DIR="$SCRIPT_DIR/xcode"
XCODE_PROJECT="$XCODE_DIR/oflike-metal.xcodeproj"

# Generate Xcode project if it doesn't exist
if [ ! -d "$XCODE_PROJECT" ]; then
    echo "Generating Xcode project..."
    mkdir -p "$XCODE_DIR"
    cd "$XCODE_DIR"
    cmake -G Xcode ..
    if [ $? -ne 0 ]; then
        echo "Failed to generate Xcode project"
        exit 1
    fi
fi

# Open in Xcode
echo "Opening Xcode..."
open "$XCODE_PROJECT"
