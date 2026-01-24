#!/bin/bash

# Quick launcher for oflike-metal examples
# Usage: ./launcher.sh

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
LAUNCHER_DIR="$SCRIPT_DIR/tools/ExampleLauncher"
BUILD_DIR="$SCRIPT_DIR/build/tools/ExampleLauncher"
APP="$BUILD_DIR/ExampleLauncher"

# Build if not exists or source is newer
if [ ! -f "$APP" ] || [ "$LAUNCHER_DIR/ExampleLauncher.swift" -nt "$APP" ]; then
    echo "Building ExampleLauncher..."
    mkdir -p "$BUILD_DIR"
    swiftc -o "$APP" -framework AppKit "$LAUNCHER_DIR/ExampleLauncher.swift"
    if [ $? -ne 0 ]; then
        echo "Build failed!"
        exit 1
    fi
fi

# Launch
"$APP" &
