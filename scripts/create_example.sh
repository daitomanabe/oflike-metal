#!/bin/bash

# Create a new oflike-metal example with Xcode project
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

# Create directories
mkdir -p "$EXAMPLE_DIR/$EXAMPLE_NAME.xcodeproj"

# Create main.mm template
cat > "$EXAMPLE_DIR/main.mm" << 'MAINEOF'
/// Example: __EXAMPLE_NAME__
/// Description: Add your description here

#include "core/AppBase.h"
#include "oflike/graphics/ofGraphics.h"
#include "oflike/types/ofColor.h"
#include "oflike/utils/ofUtils.h"

class ofApp : public ofBaseApp {
public:
    void setup() override {
        ofSetFrameRate(60);
    }

    void update() override {
    }

    void draw() override {
        ofBackground(40);

        // Draw something
        ofSetColor(255);
        ofDrawCircle(ofGetWidth() / 2, ofGetHeight() / 2, 100);
    }

    void keyPressed(int key) override {
    }

    void mousePressed(int x, int y, int button) override {
    }
};

#define OF_APP ofApp
#include "oflike/app/ofMain.h"
MAINEOF

# Replace placeholder
sed -i '' "s/__EXAMPLE_NAME__/$EXAMPLE_NAME/g" "$EXAMPLE_DIR/main.mm"

# Create project.pbxproj (copy from 01_basics and modify)
TEMPLATE_PROJECT="$PROJECT_DIR/examples/01_basics/01_basics.xcodeproj/project.pbxproj"

if [ -f "$TEMPLATE_PROJECT" ]; then
    cp "$TEMPLATE_PROJECT" "$EXAMPLE_DIR/$EXAMPLE_NAME.xcodeproj/project.pbxproj"

    # Replace 01_basics with new example name
    sed -i '' "s/01_basics/$EXAMPLE_NAME/g" "$EXAMPLE_DIR/$EXAMPLE_NAME.xcodeproj/project.pbxproj"
    sed -i '' "s/01-basics/${EXAMPLE_NAME//_/-}/g" "$EXAMPLE_DIR/$EXAMPLE_NAME.xcodeproj/project.pbxproj"
fi

echo ""
echo "Created example: $EXAMPLE_DIR"
echo ""
echo "To open in Xcode:"
echo "  open $EXAMPLE_DIR/$EXAMPLE_NAME.xcodeproj"
echo ""
echo "Before building, make sure the library is built:"
echo "  cd build && make -j8 oflike-metal"
