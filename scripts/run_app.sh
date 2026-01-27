#!/bin/bash

set -euo pipefail

usage() {
    echo "Usage: $0 <AppName> [--project-dir <path>] [--configuration <Debug|Release>] [--metal-debug] [--build]"
    echo ""
    echo "Examples:"
    echo "  $0 Test5"
    echo "  $0 Test5 --metal-debug"
    echo "  $0 Test5 --build"
}

APP_NAME=""
PROJECT_DIR=""
CONFIGURATION="Debug"
METAL_DEBUG=0
DO_BUILD=0

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
        --metal-debug)
            METAL_DEBUG=1
            shift 1
            ;;
        --build)
            DO_BUILD=1
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

if [ "$DO_BUILD" -eq 1 ]; then
    "$ROOT_DIR/scripts/build_app.sh" "$APP_NAME" --project-dir "$PROJECT_DIR" --configuration "$CONFIGURATION"
fi

APP_PATH="$ROOT_DIR/build/DerivedData/$APP_NAME/Build/Products/$CONFIGURATION/$APP_NAME.app"
EXECUTABLE="$APP_PATH/Contents/MacOS/$APP_NAME"

if [ ! -x "$EXECUTABLE" ]; then
    echo "Error: Built app not found: $EXECUTABLE"
    echo "Build first with: $ROOT_DIR/scripts/build_app.sh $APP_NAME"
    exit 1
fi

if [ "$METAL_DEBUG" -eq 1 ]; then
    OFL_METAL_DEBUG=1 "$EXECUTABLE"
else
    "$EXECUTABLE"
fi
