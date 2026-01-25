use crate::config::{get_author_from_git, load_global_config};
use crate::error::{GeneratorError, Result};
use crate::utils::*;
use std::fs;
use std::path::{Path, PathBuf};

pub fn execute(
    project_name: &str,
    entry: &str,
    addons: Option<&str>,
    addon_mode: &str,
    path: Option<&str>,
    template: &str,
    bundle_id: Option<&str>,
    author: Option<&str>,
    no_git: bool,
    no_readme: bool,
    verbose: bool,
) -> Result<()> {
    // Validate inputs
    validate_project_name(project_name)?;
    validate_entry_mode(entry)?;
    validate_addon_mode(addon_mode)?;
    validate_template(template)?;

    // Load global config
    let config = load_global_config();

    // Determine project path
    let project_path = if let Some(p) = path {
        PathBuf::from(p).join(project_name)
    } else {
        PathBuf::from(project_name)
    };

    // Check if project already exists
    if project_path.exists() {
        return Err(GeneratorError::ProjectExists(
            project_path.display().to_string(),
        ));
    }

    // Generate bundle ID
    let bundle_id = if let Some(id) = bundle_id {
        validate_bundle_id(id)?;
        id.to_string()
    } else {
        let prefix = config
            .as_ref()
            .map(|c| c.defaults.bundle_id_prefix.as_str())
            .unwrap_or("com.example");
        generate_bundle_id(project_name, prefix)
    };

    // Get author
    let author_name = author
        .map(String::from)
        .or_else(|| config.as_ref().map(|c| c.defaults.author.clone()))
        .or_else(get_author_from_git)
        .unwrap_or_else(|| "Unknown".to_string());

    if verbose {
        println!("Creating project: {}", project_name);
        println!("  Path: {}", project_path.display());
        println!("  Entry: {}", entry);
        println!("  Template: {}", template);
        println!("  Bundle ID: {}", bundle_id);
        println!("  Author: {}", author_name);
    }

    // Create project structure
    create_project_structure(&project_path, entry)?;

    // Generate files based on template
    generate_template_files(
        &project_path,
        project_name,
        entry,
        template,
        &bundle_id,
        &author_name,
    )?;

    // Parse and link/copy addons
    let addon_list = parse_addons(addons)?;
    if !addon_list.is_empty() {
        setup_addons(&project_path, &addon_list, addon_mode, verbose)?;
    }

    // Generate build files
    generate_cmake_file(&project_path, project_name, entry, &addon_list)?;
    generate_xcodegen_file(&project_path, project_name, entry, &bundle_id, &addon_list)?;

    // Generate .gitignore
    generate_gitignore(&project_path)?;

    // Generate README
    if !no_readme {
        generate_readme(&project_path, project_name, entry)?;
    }

    // Initialize git
    if !no_git {
        init_git(&project_path, verbose)?;
    }

    println!("✅ Project '{}' created successfully!", project_name);
    println!("   Path: {}", project_path.display());
    println!();
    println!("Next steps:");
    println!("  cd {}", project_name);
    println!("  xcodegen generate");
    println!("  open {}.xcodeproj", project_name);

    Ok(())
}

fn create_project_structure(project_path: &Path, entry: &str) -> Result<()> {
    fs::create_dir_all(project_path)?;
    fs::create_dir_all(project_path.join("src"))?;
    fs::create_dir_all(project_path.join("data"))?;

    if entry == "swiftui" {
        fs::create_dir_all(project_path.join("resources"))?;
        fs::create_dir_all(project_path.join("resources/Assets.xcassets"))?;
    }

    Ok(())
}

fn generate_template_files(
    project_path: &Path,
    project_name: &str,
    entry: &str,
    template: &str,
    bundle_id: &str,
    author: &str,
) -> Result<()> {
    let class_name = to_pascal_case(project_name);

    // Generate header file
    let header_content = format!(
        r#"#pragma once

#include <oflike/ofMain.h>

class {}: public ofBaseApp {{
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
}};
"#,
        class_name
    );

    fs::write(
        project_path.join("src").join(format!("{}.h", class_name)),
        header_content,
    )?;

    // Generate implementation file
    let impl_content = format!(
        r#"#include "{}.h"

void {}::setup() {{
    // Initialization
    ofSetWindowTitle("{}");
}}

void {}::update() {{
    // Update logic
}}

void {}::draw() {{
    ofClear(50);

    // Draw example
    ofSetColor(255);
    ofDrawCircle(ofGetWidth() / 2, ofGetHeight() / 2, 100);
}}

void {}::keyPressed(int key) {{
    // Handle key press
}}

void {}::keyReleased(int key) {{
    // Handle key release
}}

void {}::mouseMoved(int x, int y) {{
    // Handle mouse move
}}

void {}::mouseDragged(int x, int y, int button) {{
    // Handle mouse drag
}}

void {}::mousePressed(int x, int y, int button) {{
    // Handle mouse press
}}

void {}::mouseReleased(int x, int y, int button) {{
    // Handle mouse release
}}

void {}::windowResized(int w, int h) {{
    // Handle window resize
}}
"#,
        class_name, class_name, project_name, class_name, class_name, class_name, class_name,
        class_name, class_name, class_name, class_name, class_name, class_name
    );

    fs::write(
        project_path.join("src").join(format!("{}.cpp", class_name)),
        impl_content,
    )?;

    // Generate entry point based on mode
    if entry == "swiftui" {
        generate_swiftui_entry(project_path, &class_name)?;
    } else {
        generate_ofmain_entry(project_path, &class_name)?;
    }

    // Generate Info.plist for SwiftUI
    if entry == "swiftui" {
        generate_info_plist(project_path, project_name, bundle_id)?;
    }

    Ok(())
}

fn generate_swiftui_entry(project_path: &Path, class_name: &str) -> Result<()> {
    let swift_content = format!(
        r#"import SwiftUI
import MetalKit

@main
struct {}App: App {{
    @StateObject private var appState = AppState()

    var body: some Scene {{
        WindowGroup {{
            MetalView(appState: appState)
                .frame(minWidth: 800, minHeight: 600)
        }}
        .windowStyle(.titleBar)
    }}
}}

class AppState: ObservableObject {{
    @Published var isRunning = true
}}
"#,
        class_name
    );

    fs::write(project_path.join("src").join("App.swift"), swift_content)?;

    Ok(())
}

fn generate_ofmain_entry(project_path: &Path, class_name: &str) -> Result<()> {
    let main_content = format!(
        r#"#include <oflike/ofMain.h>
#include "{}.h"

int main() {{
    ofRunApp<{}>(1024, 768, "{}");
    return 0;
}}
"#,
        class_name, class_name, class_name
    );

    fs::write(project_path.join("src").join("main.mm"), main_content)?;

    Ok(())
}

fn generate_info_plist(project_path: &Path, project_name: &str, bundle_id: &str) -> Result<()> {
    let plist_content = format!(
        r#"<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleName</key>
    <string>{}</string>
    <key>CFBundleIdentifier</key>
    <string>{}</string>
    <key>CFBundleVersion</key>
    <string>1.0.0</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0.0</string>
    <key>CFBundleExecutable</key>
    <string>$(EXECUTABLE_NAME)</string>
    <key>NSPrincipalClass</key>
    <string>NSApplication</string>
    <key>LSMinimumSystemVersion</key>
    <string>13.0</string>
</dict>
</plist>
"#,
        project_name, bundle_id
    );

    fs::write(
        project_path.join("resources").join("Info.plist"),
        plist_content,
    )?;

    Ok(())
}

fn parse_addons(addons: Option<&str>) -> Result<Vec<String>> {
    if let Some(addon_str) = addons {
        let mut addon_list = Vec::new();
        for addon in addon_str.split(',') {
            let addon = addon.trim();
            if !addon.is_empty() {
                // Validate addon exists
                if !is_builtin_addon(addon) {
                    eprintln!("Warning: {} is not a builtin addon", addon);
                }
                addon_list.push(addon.to_string());
            }
        }
        Ok(addon_list)
    } else {
        Ok(Vec::new())
    }
}

fn setup_addons(
    project_path: &Path,
    addon_list: &[String],
    addon_mode: &str,
    verbose: bool,
) -> Result<()> {
    let addons_dir = project_path.join("addons");
    fs::create_dir_all(&addons_dir)?;

    // Find oflike-metal root (assume it's in parent directories)
    let oflike_root = find_oflike_root()?;

    for addon in addon_list {
        let addon_category = if core_addons().contains(&addon.as_str()) {
            "core"
        } else {
            "apple_native"
        };

        let source_addon_path = oflike_root
            .join("addons")
            .join(addon_category)
            .join(addon);

        if !source_addon_path.exists() {
            eprintln!(
                "Warning: Addon source not found: {}",
                source_addon_path.display()
            );
            continue;
        }

        let dest_addon_path = addons_dir.join(addon);

        match addon_mode {
            "reference" => {
                // Don't copy anything, just record the reference
                // (CMakeLists.txt will reference the oflike-metal installation)
                if verbose {
                    println!("  Referencing addon: {}", addon);
                }
            }
            "copy" => {
                // Copy addon files
                if verbose {
                    println!("  Copying addon: {}", addon);
                }
                copy_dir_recursive(&source_addon_path, &dest_addon_path)?;
            }
            "symlink" => {
                // Create symlink
                if verbose {
                    println!("  Symlinking addon: {}", addon);
                }
                #[cfg(unix)]
                {
                    std::os::unix::fs::symlink(&source_addon_path, &dest_addon_path)?;
                }
                #[cfg(not(unix))]
                {
                    return Err(GeneratorError::Other(
                        "Symlinks are only supported on Unix systems".to_string(),
                    ));
                }
            }
            _ => unreachable!(),
        }
    }

    Ok(())
}

fn find_oflike_root() -> Result<PathBuf> {
    // Try to find oflike-metal root by looking for addons/ directory
    let mut current = std::env::current_dir()?;

    loop {
        let addons_path = current.join("addons");
        if addons_path.exists() && addons_path.is_dir() {
            // Check if it has core/apple_native subdirectories
            if addons_path.join("core").exists() || addons_path.join("apple_native").exists() {
                return Ok(current);
            }
        }

        if !current.pop() {
            break;
        }
    }

    Err(GeneratorError::Other(
        "Could not find oflike-metal root directory. Run this command from within the oflike-metal project.".to_string(),
    ))
}

fn copy_dir_recursive(src: &Path, dst: &Path) -> Result<()> {
    fs::create_dir_all(dst)?;

    for entry in fs::read_dir(src)? {
        let entry = entry?;
        let file_type = entry.file_type()?;
        let src_path = entry.path();
        let dst_path = dst.join(entry.file_name());

        if file_type.is_dir() {
            copy_dir_recursive(&src_path, &dst_path)?;
        } else if file_type.is_file() {
            fs::copy(&src_path, &dst_path)?;
        }
    }

    Ok(())
}

fn generate_cmake_file(
    project_path: &Path,
    project_name: &str,
    entry: &str,
    addon_list: &[String],
) -> Result<()> {
    let class_name = to_pascal_case(project_name);

    let mut cmake_content = format!(
        r#"cmake_minimum_required(VERSION 3.20)
project({})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find oflike-metal framework
find_package(oflike-metal REQUIRED)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.mm")
file(GLOB_RECURSE HEADERS "src/*.h")

"#,
        project_name
    );

    if entry == "swiftui" {
        cmake_content.push_str(
            r#"# Swift sources
file(GLOB SWIFT_SOURCES "src/*.swift")

"#,
        );
    }

    cmake_content.push_str(&format!(
        r#"# Executable
add_executable({} MACOSX_BUNDLE ${{SOURCES}} ${{HEADERS}}"#,
        project_name
    ));

    if entry == "swiftui" {
        cmake_content.push_str(" ${SWIFT_SOURCES}");
    }

    cmake_content.push_str(")\n\n");

    // Link oflike-metal
    cmake_content.push_str(&format!(
        r#"# Link oflike-metal
target_link_libraries({} oflike-metal::oflike-metal)

"#,
        project_name
    ));

    // Add addons if specified
    if !addon_list.is_empty() {
        cmake_content.push_str("# Addons\n");
        for addon in addon_list {
            cmake_content.push_str(&format!(
                r#"file(GLOB_RECURSE {}_SOURCES "addons/{}/*.cpp" "addons/{}/*.mm")
file(GLOB_RECURSE {}_HEADERS "addons/{}/*.h")
target_sources({} PRIVATE ${{{}_SOURCES}} ${{{}_HEADERS}})
target_include_directories({} PRIVATE addons/{})

"#,
                addon.to_uppercase(),
                addon,
                addon,
                addon.to_uppercase(),
                addon,
                project_name,
                addon.to_uppercase(),
                addon.to_uppercase(),
                project_name,
                addon
            ));
        }
        cmake_content.push('\n');
    }

    // Bundle settings
    if entry == "swiftui" {
        cmake_content.push_str(&format!(
            r#"# Bundle settings
set_target_properties({} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST "${{CMAKE_CURRENT_SOURCE_DIR}}/resources/Info.plist"
    RESOURCE "${{CMAKE_CURRENT_SOURCE_DIR}}/resources"
)
"#,
            project_name
        ));
    }

    fs::write(project_path.join("CMakeLists.txt"), cmake_content)?;

    Ok(())
}

fn generate_xcodegen_file(
    project_path: &Path,
    project_name: &str,
    entry: &str,
    bundle_id: &str,
    addon_list: &[String],
) -> Result<()> {
    let mut sources_list = vec!["src".to_string()];
    if !addon_list.is_empty() {
        for addon in addon_list {
            sources_list.push(format!("addons/{}", addon));
        }
    }

    let sources_yaml = sources_list
        .iter()
        .map(|s| format!("      - {}", s))
        .collect::<Vec<_>>()
        .join("\n");

    let xcodegen_content = if entry == "swiftui" {
        format!(
            r#"name: {}
options:
  bundleIdPrefix: {}
  deploymentTarget:
    macOS: "13.0"

targets:
  {}:
    type: application
    platform: macOS
    sources:
{}
    settings:
      PRODUCT_BUNDLE_IDENTIFIER: {}
      INFOPLIST_FILE: resources/Info.plist
      SWIFT_VERSION: "5.9"
      ENABLE_HARDENED_RUNTIME: YES
    dependencies:
      - framework: oflike-metal.framework
        embed: true
    scheme:
      testTargets: []
"#,
            project_name, bundle_id, project_name, sources_yaml, bundle_id
        )
    } else {
        format!(
            r#"name: {}
options:
  bundleIdPrefix: {}
  deploymentTarget:
    macOS: "13.0"

targets:
  {}:
    type: application
    platform: macOS
    sources:
{}
    settings:
      PRODUCT_BUNDLE_IDENTIFIER: {}
      ENABLE_HARDENED_RUNTIME: YES
    dependencies:
      - framework: oflike-metal.framework
        embed: true
    scheme:
      testTargets: []
"#,
            project_name, bundle_id, project_name, sources_yaml, bundle_id
        )
    };

    fs::write(project_path.join("project.yml"), xcodegen_content)?;

    Ok(())
}

fn generate_gitignore(project_path: &Path) -> Result<()> {
    let gitignore_content = r#"# Xcode
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
"#;

    fs::write(project_path.join(".gitignore"), gitignore_content)?;

    Ok(())
}

fn generate_readme(project_path: &Path, project_name: &str, entry: &str) -> Result<()> {
    let readme_content = format!(
        r#"# {}

Generated with oflike-gen (oflike-metal Project Generator).

## Build

### With XcodeGen (Recommended)

```bash
xcodegen generate
open {}.xcodeproj
```

### With CMake

```bash
mkdir build && cd build
cmake .. -G Xcode
open {}.xcodeproj
```

## Entry Point

Entry mode: **{}**

{}

## Dependencies

- oflike-metal framework

## License

MIT
"#,
        project_name,
        project_name,
        project_name,
        entry,
        if entry == "swiftui" {
            "SwiftUI-based macOS app with Metal rendering."
        } else {
            "Legacy ofMain entry point (openFrameworks compatible)."
        }
    );

    fs::write(project_path.join("README.md"), readme_content)?;

    Ok(())
}

fn init_git(project_path: &Path, verbose: bool) -> Result<()> {
    if verbose {
        println!("Initializing git repository...");
    }

    let output = std::process::Command::new("git")
        .arg("init")
        .current_dir(project_path)
        .output()?;

    if !output.status.success() {
        return Err(GeneratorError::Other(
            "Failed to initialize git repository".to_string(),
        ));
    }

    Ok(())
}
