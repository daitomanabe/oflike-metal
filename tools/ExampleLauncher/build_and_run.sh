#!/bin/bash

# Build and run the Example Launcher
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build/tools/ExampleLauncher"
APP_NAME="ExampleLauncher"

mkdir -p "$BUILD_DIR"

echo "Building ExampleLauncher..."
echo "Project dir: $PROJECT_DIR"
echo "Build dir: $BUILD_DIR"

# Compile the Swift app
swiftc \
    -o "$BUILD_DIR/$APP_NAME" \
    -framework AppKit \
    "$SCRIPT_DIR/ExampleLauncher.swift" \
    2>&1

if [ $? -eq 0 ]; then
    echo "Build successful. Launching..."
    "$BUILD_DIR/$APP_NAME" &
else
    echo "Build failed!"
    exit 1
fi
