#!/bin/bash

set -euo pipefail

usage() {
    echo "Usage: $0 [--configuration <Debug|Release>] [--skip-build]"
}

CONFIGURATION="Debug"
SKIP_BUILD=0

while [ $# -gt 0 ]; do
    case "$1" in
        -c|--configuration)
            CONFIGURATION="$2"
            shift 2
            ;;
        --skip-build)
            SKIP_BUILD=1
            shift 1
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown argument: $1"
            usage
            exit 1
            ;;
    esac
done

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
PROJECT_DIR="$ROOT_DIR/tools/ProjectWizard"
BUILD_DIR="$ROOT_DIR/build/DerivedData/ProjectWizard"

if [ ! -f "$PROJECT_DIR/project.yml" ]; then
    echo "Error: ProjectWizard project.yml not found at $PROJECT_DIR"
    exit 1
fi

if [ "$SKIP_BUILD" -eq 0 ]; then
    if ! command -v xcodegen >/dev/null 2>&1; then
        echo "Error: xcodegen not found. Install with: brew install xcodegen"
        exit 1
    fi

    (cd "$PROJECT_DIR" && xcodegen generate)

    mkdir -p "$BUILD_DIR"
    xcodebuild \
        -project "$PROJECT_DIR/ProjectWizard.xcodeproj" \
        -scheme ProjectWizard \
        -configuration "$CONFIGURATION" \
        -destination "platform=macOS" \
        -derivedDataPath "$BUILD_DIR" \
        build
fi

APP_PATH="$BUILD_DIR/Build/Products/$CONFIGURATION/ProjectWizard.app"
EXECUTABLE="$APP_PATH/Contents/MacOS/ProjectWizard"

if [ ! -x "$EXECUTABLE" ]; then
    echo "Error: ProjectWizard app not found at $EXECUTABLE"
    echo "Run with --skip-build only if already built."
    exit 1
fi

cd "$ROOT_DIR"
OFLIKE_ROOT="$ROOT_DIR" "$EXECUTABLE"
