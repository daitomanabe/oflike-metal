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
RUN_LOG="$ROOT_DIR/docs/run_app_debug.log"
LOG_ENABLED=1

mkdir -p "$(dirname "$RUN_LOG")" 2>/dev/null || LOG_ENABLED=0
if ! touch "$RUN_LOG" 2>/dev/null; then
    LOG_ENABLED=0
fi

log_echo() {
    if [ "$LOG_ENABLED" -eq 1 ]; then
        printf '%s\n' "$*" | tee -a "$RUN_LOG"
    else
        printf '%s\n' "$*"
    fi
}

log_append() {
    if [ "$LOG_ENABLED" -eq 1 ]; then
        printf '%s\n' "$*" >> "$RUN_LOG" 2>/dev/null || true
    fi
}

if [ "$LOG_ENABLED" -eq 1 ]; then
    log_append "----"
    log_append "$(date '+%Y-%m-%d %H:%M:%S') run_app app=$APP_NAME config=$CONFIGURATION metal_debug=$METAL_DEBUG build=$DO_BUILD"
else
    echo "Warning: cannot write log to $RUN_LOG" >&2
fi

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

# Capture non-zero exits from the app without tripping `set -e`.
set +e
if [ "$METAL_DEBUG" -eq 1 ]; then
    OFL_METAL_DEBUG=1 "$EXECUTABLE"
else
    "$EXECUTABLE"
fi
APP_EXIT_CODE=$?
set -e

if [ "$APP_EXIT_CODE" -ne 0 ]; then
    log_echo "Command failed with code $APP_EXIT_CODE"
    CRASH_DIR="$HOME/Library/Logs/DiagnosticReports"
    if [ -d "$CRASH_DIR" ]; then
        LATEST_CRASH="$(ls -t "$CRASH_DIR"/"$APP_NAME"* 2>/dev/null | head -n 1 || true)"
        if [ -n "${LATEST_CRASH:-}" ] && [ -f "$LATEST_CRASH" ]; then
            log_echo "Latest crash report: $LATEST_CRASH"
            if command -v rg >/dev/null 2>&1; then
                if [ "$LOG_ENABLED" -eq 1 ]; then
                    rg -n "Termination Reason|Exception Type|Application Specific Information|BUG IN CLIENT|Trace/BPT|Namespace SIGNAL|signal" \
                        "$LATEST_CRASH" | head -n 40 | tee -a "$RUN_LOG" || true
                else
                    rg -n "Termination Reason|Exception Type|Application Specific Information|BUG IN CLIENT|Trace/BPT|Namespace SIGNAL|signal" \
                        "$LATEST_CRASH" | head -n 40 || true
                fi
            else
                if [ "$LOG_ENABLED" -eq 1 ]; then
                    grep -nE "Termination Reason|Exception Type|Application Specific Information|BUG IN CLIENT|Trace/BPT|Namespace SIGNAL|signal" \
                        "$LATEST_CRASH" | head -n 40 | tee -a "$RUN_LOG" || true
                else
                    grep -nE "Termination Reason|Exception Type|Application Specific Information|BUG IN CLIENT|Trace/BPT|Namespace SIGNAL|signal" \
                        "$LATEST_CRASH" | head -n 40 || true
                fi
            fi
        else
            log_echo "No crash report found under $CRASH_DIR for $APP_NAME"
        fi
    fi
    exit "$APP_EXIT_CODE"
fi
