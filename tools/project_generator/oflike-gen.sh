#!/bin/bash
# oflike-gen - Project generator for oflike-metal
# Temporary shell implementation until Rust build is available
# Full Rust implementation in src/

VERSION="1.0.0"

show_help() {
    cat <<EOF
oflike-gen ${VERSION}
Project generator for oflike-metal

USAGE:
    oflike-gen [OPTIONS] <COMMAND>

OPTIONS:
    --version              Print version information
    --help                 Print help information
    --verbose              Enable verbose output
    --config <PATH>        Custom config file path

COMMANDS:
    new          Generate a new oflike-metal project
    add-addon    Add an addon to an existing project
    remove-addon Remove an addon from a project
    list-addons  List available or project addons
    init         Initialize oflike-gen configuration
    validate     Validate project structure and configuration

Run 'oflike-gen <COMMAND> --help' for more information on a command.

NOTE: This is a shell wrapper. Full Rust implementation requires:
      cargo build --release
      sudo cp target/release/oflike-gen /usr/local/bin/
EOF
}

show_new_help() {
    cat <<EOF
oflike-gen-new
Generate a new oflike-metal project

USAGE:
    oflike-gen new <PROJECT_NAME> [OPTIONS]

ARGS:
    <PROJECT_NAME>    Project name (PascalCase or kebab-case)

OPTIONS:
    --entry <MODE>           Entry point mode [default: swiftui] [possible values: swiftui, ofmain]
    --addons <LIST>          Initial addons (comma-separated)
    --addon-mode <MODE>      Addon integration strategy [default: reference] [possible values: reference, copy, symlink]
    --path <DIR>             Project creation directory [default: ./<PROJECT_NAME>]
    --template <NAME>        Project template [default: basic] [possible values: basic, swiftui, metal, 3d]
    --bundle-id <ID>         macOS bundle identifier [default: com.example.<project>]
    --author <NAME>          Project author name [default: Git config user.name]
    --no-git                 Skip git initialization
    --no-readme              Skip README.md generation
    --help                   Print help information

EXAMPLES:
    oflike-gen new myProject
    oflike-gen new myProject --entry swiftui --addons ofxOsc,ofxGui
    oflike-gen new modelViewer --template 3d --addons ofxSharp
EOF
}

create_project() {
    local project_name="$1"
    local entry="${2:-swiftui}"
    local template="${3:-basic}"

    if [ -z "$project_name" ]; then
        echo "Error: PROJECT_NAME is required"
        echo "Run 'oflike-gen new --help' for usage"
        exit 1
    fi

    if [ -d "$project_name" ]; then
        echo "Error: Project '$project_name' already exists"
        exit 1
    fi

    echo "Creating project: $project_name"
    echo "  Entry: $entry"
    echo "  Template: $template"

    # Create directory structure
    mkdir -p "$project_name/src"
    mkdir -p "$project_name/data"

    if [ "$entry" = "swiftui" ]; then
        mkdir -p "$project_name/resources"
    fi

    # Convert to PascalCase (simple version)
    class_name=$(echo "$project_name" | sed -e 's/-/ /g' -e 's/\b\(.\)/\u\1/g' -e 's/ //g')

    # Generate header file
    cat > "$project_name/src/${class_name}.h" <<EOF
#pragma once

#include <oflike/ofMain.h>

class ${class_name}: public ofBaseApp {
public:
    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
};
EOF

    # Generate implementation file
    cat > "$project_name/src/${class_name}.cpp" <<EOF
#include "${class_name}.h"

void ${class_name}::setup() {
    // Initialization
    ofSetWindowTitle("${project_name}");
}

void ${class_name}::update() {
    // Update logic
}

void ${class_name}::draw() {
    ofClear(50);

    // Draw example
    ofSetColor(255);
    ofDrawCircle(ofGetWidth() / 2, ofGetHeight() / 2, 100);
}

void ${class_name}::keyPressed(int key) {
    // Handle key press
}

void ${class_name}::keyReleased(int key) {
    // Handle key release
}

void ${class_name}::mouseMoved(int x, int y) {
    // Handle mouse move
}

void ${class_name}::mouseDragged(int x, int y, int button) {
    // Handle mouse drag
}

void ${class_name}::mousePressed(int x, int y, int button) {
    // Handle mouse press
}

void ${class_name}::mouseReleased(int x, int y, int button) {
    // Handle mouse release
}

void ${class_name}::windowResized(int w, int h) {
    // Handle window resize
}
EOF

    # Generate entry point
    if [ "$entry" = "swiftui" ]; then
        cat > "$project_name/src/App.swift" <<EOF
import SwiftUI
import MetalKit

@main
struct ${class_name}App: App {
    @StateObject private var appState = AppState()

    var body: some Scene {
        WindowGroup {
            MetalView(appState: appState)
                .frame(minWidth: 800, minHeight: 600)
        }
        .windowStyle(.titleBar)
    }
}

class AppState: ObservableObject {
    @Published var isRunning = true
}
EOF
    else
        cat > "$project_name/src/main.mm" <<EOF
#include <oflike/ofMain.h>
#include "${class_name}.h"

int main() {
    ofRunApp<${class_name}>(1024, 768, "${class_name}");
    return 0;
}
EOF
    fi

    # Generate CMakeLists.txt
    cat > "$project_name/CMakeLists.txt" <<EOF
cmake_minimum_required(VERSION 3.20)
project(${project_name})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find oflike-metal framework
find_package(oflike-metal REQUIRED)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.mm")
file(GLOB_RECURSE HEADERS "src/*.h")

EOF

    if [ "$entry" = "swiftui" ]; then
        echo "# Swift sources" >> "$project_name/CMakeLists.txt"
        echo 'file(GLOB SWIFT_SOURCES "src/*.swift")' >> "$project_name/CMakeLists.txt"
        echo "" >> "$project_name/CMakeLists.txt"
    fi

    cat >> "$project_name/CMakeLists.txt" <<EOF
# Executable
add_executable(${project_name} MACOSX_BUNDLE \${SOURCES} \${HEADERS})

# Link oflike-metal
target_link_libraries(${project_name} oflike-metal::oflike-metal)
EOF

    # Generate project.yml
    cat > "$project_name/project.yml" <<EOF
name: ${project_name}
options:
  bundleIdPrefix: com.example
  deploymentTarget:
    macOS: "13.0"

targets:
  ${project_name}:
    type: application
    platform: macOS
    sources:
      - src
    settings:
      PRODUCT_BUNDLE_IDENTIFIER: com.example.${project_name}
      ENABLE_HARDENED_RUNTIME: YES
    dependencies:
      - framework: oflike-metal.framework
        embed: true
    scheme:
      testTargets: []
EOF

    # Generate .gitignore
    cat > "$project_name/.gitignore" <<'EOF'
# Xcode
*.xcodeproj
*.xcworkspace
!default.xcworkspace
*.pbxuser
*.mode1v3
*.mode2v3
*.perspectivev3
xcuserdata/
DerivedData/
*.xccheckout
*.moved-aside
*.hmap
*.ipa

# CMake
build/
CMakeCache.txt
CMakeFiles/
cmake_install.cmake
*.cmake

# macOS
.DS_Store
.AppleDouble
.LSOverride
Icon
._*

# Temporary
*.swp
*.swo
*~

# Data
data/*
!data/.gitkeep
EOF

    # Generate README.md
    cat > "$project_name/README.md" <<EOF
# ${project_name}

Generated with oflike-gen (oflike-metal Project Generator).

## Build

### With XcodeGen (Recommended)

\`\`\`bash
xcodegen generate
open ${project_name}.xcodeproj
\`\`\`

### With CMake

\`\`\`bash
mkdir build && cd build
cmake .. -G Xcode
open ${project_name}.xcodeproj
\`\`\`

## Entry Point

Entry mode: **${entry}**

$([ "$entry" = "swiftui" ] && echo "SwiftUI-based macOS app with Metal rendering." || echo "Legacy ofMain entry point (openFrameworks compatible).")

## Dependencies

- oflike-metal framework

## License

MIT
EOF

    # Initialize git
    cd "$project_name"
    git init > /dev/null 2>&1
    cd ..

    echo ""
    echo "✅ Project '${project_name}' created successfully!"
    echo "   Path: $(pwd)/${project_name}"
    echo ""
    echo "Next steps:"
    echo "  cd ${project_name}"
    echo "  xcodegen generate"
    echo "  open ${project_name}.xcodeproj"
}

# Main command dispatch
case "${1:-}" in
    --version)
        echo "oflike-gen ${VERSION}"
        ;;
    --help|-h|help)
        show_help
        ;;
    new)
        shift
        if [ "${1:-}" = "--help" ] || [ "${1:-}" = "-h" ]; then
            show_new_help
        else
            create_project "$@"
        fi
        ;;
    add-addon|remove-addon|list-addons|init|validate)
        echo "Command '$1' not yet implemented in shell version"
        echo "Full implementation available in Rust: cargo build --release"
        ;;
    *)
        if [ -z "${1:-}" ]; then
            show_help
        else
            echo "Error: Unknown command '$1'"
            echo "Run 'oflike-gen --help' for usage"
            exit 1
        fi
        ;;
esac
