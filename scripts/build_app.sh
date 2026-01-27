#!/bin/bash

set -euo pipefail

usage() {
    echo "Usage: $0 <AppName> [--project-dir <path>] [--configuration <Debug|Release>] [--skip-xcodegen]"
    echo ""
    echo "Examples:"
    echo "  $0 Test5"
    echo "  $0 Test5 --configuration Release"
    echo "  $0 Test5 --project-dir /path/to/app --skip-xcodegen"
}

APP_NAME=""
PROJECT_DIR=""
CONFIGURATION="Debug"
SKIP_XCODEGEN=0

while [ $# -gt 0 ]; do
    case "$1" in
        -c|--configuration)
            CONFIGURATION="$2"
            shift 2
            ;;
        -p|--project-dir)
            PROJECT_DIR="$2"
            shift 2
            ;;
        --skip-xcodegen)
            SKIP_XCODEGEN=1
            shift 1
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            if [ -z "$APP_NAME" ]; then
                APP_NAME="$1"
                shift 1
            else
                echo "Unknown argument: $1"
                usage
                exit 1
            fi
            ;;
    esac
done

if [ -z "$APP_NAME" ]; then
    usage
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

if [ -z "$PROJECT_DIR" ]; then
    PROJECT_DIR="$ROOT_DIR/apps/$APP_NAME"
fi

if [ ! -d "$PROJECT_DIR" ]; then
    echo "Error: Project directory not found: $PROJECT_DIR"
    exit 1
fi

if [ ! -f "$PROJECT_DIR/project.yml" ]; then
    echo "Error: project.yml not found in $PROJECT_DIR"
    exit 1
fi

if [ "$SKIP_XCODEGEN" -eq 0 ]; then
    if ! command -v xcodegen >/dev/null 2>&1; then
        echo "Error: xcodegen not found. Install with: brew install xcodegen"
        exit 1
    fi
    (cd "$PROJECT_DIR" && xcodegen generate)
fi

if [ ! -d "$PROJECT_DIR/$APP_NAME.xcodeproj" ]; then
    echo "Error: $APP_NAME.xcodeproj not found. Run xcodegen generate in $PROJECT_DIR"
    exit 1
fi

DERIVED_DATA_DIR="$ROOT_DIR/build/DerivedData/$APP_NAME"
mkdir -p "$DERIVED_DATA_DIR"

MTL_CACHE_DIR="/tmp/oflike_metal_module_cache"
mkdir -p "$MTL_CACHE_DIR"

xcodebuild \
    -project "$PROJECT_DIR/$APP_NAME.xcodeproj" \
    -scheme "$APP_NAME" \
    -configuration "$CONFIGURATION" \
    -destination "platform=macOS" \
    -derivedDataPath "$DERIVED_DATA_DIR" \
    MTL_COMPILER_FLAGS="-fmodules-cache-path=$MTL_CACHE_DIR" \
    build
