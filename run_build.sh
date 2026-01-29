#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

echo "Working in: $(pwd)"

# Create build directory
mkdir -p build
cd build

echo "Configuring CMake..."
cmake .. -G "Unix Makefiles" -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Debug

echo "Building..."
cmake --build . --target oflike-metal tess2 oscpack pugixml -j8

echo ""
echo "=== Build Complete ==="
ls -la liboflike-metal.a
echo ""
echo "Third party libraries:"
ls -la third_party/*.a 2>/dev/null || ls -la third_party/
