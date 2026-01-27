#!/bin/bash

set -euo pipefail

usage() {
    echo "Usage: $0 <AppName> [options]"
    echo ""
    echo "Options:"
    echo "  --template <name>       Template name (basic/swiftui/metal/3d)"
    echo "  --addons <list>         Comma-separated addons"
    echo "  --addon-mode <mode>     reference|copy|symlink"
    echo "  --path <dir>            Project base directory"
    echo "  --bundle-id <id>        Bundle identifier"
    echo "  --author <name>         Author name"
    echo "  --no-git                Skip git init"
    echo "  --no-readme             Skip README generation"
    echo "  -c, --configuration     Debug or Release (default: Debug)"
    echo "  --metal-debug           Run with OFL_METAL_DEBUG=1"
    echo "  --skip-new              Skip oflike-gen new"
    echo "  --skip-build            Skip build step"
    echo "  --skip-run              Skip run step"
    echo "  --skip-xcodegen          Skip xcodegen during build"
    echo "  --verbose               Enable generator verbose output"
}

expand_tilde() {
    case "$1" in
        "~") echo "$HOME" ;;
        "~/"*) echo "$HOME/${1#~/}" ;;
        *) echo "$1" ;;
    esac
}

APP_NAME=""
TEMPLATE=""
ADDONS=""
ADDON_MODE=""
PROJECT_BASE=""
BUNDLE_ID=""
AUTHOR=""
CONFIGURATION="Debug"
METAL_DEBUG=0
NO_GIT=0
NO_README=0
SKIP_NEW=0
SKIP_BUILD=0
SKIP_RUN=0
SKIP_XCODEGEN=0
VERBOSE=0

while [ $# -gt 0 ]; do
    case "$1" in
        --template)
            TEMPLATE="$2"
            shift 2
            ;;
        --addons)
            ADDONS="$2"
            shift 2
            ;;
        --addon-mode)
            ADDON_MODE="$2"
            shift 2
            ;;
        --path)
            PROJECT_BASE="$2"
            shift 2
            ;;
        --bundle-id)
            BUNDLE_ID="$2"
            shift 2
            ;;
        --author)
            AUTHOR="$2"
            shift 2
            ;;
        --no-git)
            NO_GIT=1
            shift 1
            ;;
        --no-readme)
            NO_README=1
            shift 1
            ;;
        -c|--configuration)
            CONFIGURATION="$2"
            shift 2
            ;;
        --metal-debug)
            METAL_DEBUG=1
            shift 1
            ;;
        --skip-new)
            SKIP_NEW=1
            shift 1
            ;;
        --skip-build)
            SKIP_BUILD=1
            shift 1
            ;;
        --skip-run)
            SKIP_RUN=1
            shift 1
            ;;
        --skip-xcodegen)
            SKIP_XCODEGEN=1
            shift 1
            ;;
        --verbose)
            VERBOSE=1
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

GENERATOR="$ROOT_DIR/bin/oflike-gen"
if [ ! -x "$GENERATOR" ]; then
    if command -v oflike-gen >/dev/null 2>&1; then
        GENERATOR="oflike-gen"
    else
        echo "Error: oflike-gen not found. Build it in tools/project_generator or install it."
        exit 1
    fi
fi

if [ -n "$PROJECT_BASE" ]; then
    PROJECT_BASE="$(expand_tilde "$PROJECT_BASE")"
    if [ "${PROJECT_BASE:0:1}" != "/" ]; then
        PROJECT_BASE="$ROOT_DIR/$PROJECT_BASE"
    fi
else
    PROJECT_BASE="$ROOT_DIR/apps"
fi

PROJECT_DIR="$PROJECT_BASE/$APP_NAME"

if [ "$SKIP_NEW" -eq 0 ]; then
    GEN_ARGS=()
    if [ -n "$ADDONS" ]; then
        GEN_ARGS+=(--addons "$ADDONS")
    fi
    if [ -n "$ADDON_MODE" ]; then
        GEN_ARGS+=(--addon-mode "$ADDON_MODE")
    fi
    if [ -n "$PROJECT_BASE" ]; then
        GEN_ARGS+=(--path "$PROJECT_BASE")
    fi
    if [ -n "$TEMPLATE" ]; then
        GEN_ARGS+=(--template "$TEMPLATE")
    fi
    if [ -n "$BUNDLE_ID" ]; then
        GEN_ARGS+=(--bundle-id "$BUNDLE_ID")
    fi
    if [ -n "$AUTHOR" ]; then
        GEN_ARGS+=(--author "$AUTHOR")
    fi
    if [ "$NO_GIT" -eq 1 ]; then
        GEN_ARGS+=(--no-git)
    fi
    if [ "$NO_README" -eq 1 ]; then
        GEN_ARGS+=(--no-readme)
    fi
    if [ "$VERBOSE" -eq 1 ]; then
        GEN_ARGS+=(--verbose)
    fi
    "$GENERATOR" new "$APP_NAME" "${GEN_ARGS[@]}"
fi

if [ ! -d "$PROJECT_DIR" ]; then
    echo "Error: Project directory not found: $PROJECT_DIR"
    exit 1
fi

if [ "$SKIP_BUILD" -eq 0 ]; then
    BUILD_ARGS=(--project-dir "$PROJECT_DIR" --configuration "$CONFIGURATION")
    if [ "$SKIP_XCODEGEN" -eq 1 ]; then
        BUILD_ARGS+=(--skip-xcodegen)
    fi
    "$ROOT_DIR/scripts/build_app.sh" "$APP_NAME" "${BUILD_ARGS[@]}"
fi

if [ "$SKIP_RUN" -eq 0 ]; then
    RUN_ARGS=(--project-dir "$PROJECT_DIR" --configuration "$CONFIGURATION")
    if [ "$METAL_DEBUG" -eq 1 ]; then
        RUN_ARGS+=(--metal-debug)
    fi
    "$ROOT_DIR/scripts/run_app.sh" "$APP_NAME" "${RUN_ARGS[@]}"
fi
