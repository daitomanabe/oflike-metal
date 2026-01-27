use crate::config::{get_author_from_git, load_global_config, Config};
use crate::error::{GeneratorError, Result};
use crate::utils::*;
use std::fs;
use std::path::{Path, PathBuf};
use std::process::Command;

pub fn execute(
    project_name: &str,
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
    validate_addon_mode(addon_mode)?;
    validate_template(template)?;

    // Load global config
    let config = load_global_config();

    // Determine project path (default to <oflike-root>/apps/<project-name>)
    let project_base = resolve_project_base_path(path, config.as_ref())?;
    let project_path = project_base.join(project_name);

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
        println!("  Template: {}", template);
        println!("  Bundle ID: {}", bundle_id);
        println!("  Author: {}", author_name);
    }

    // Create project structure
    create_project_structure(&project_path)?;

    // Generate files based on template
    generate_template_files(
        &project_path,
        project_name,
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
    generate_cmake_file(&project_path, project_name, &addon_list)?;
    generate_xcodegen_file(&project_path, project_name, &bundle_id, &addon_list)?;

    // Generate .gitignore
    generate_gitignore(&project_path)?;

    // Generate README
    if !no_readme {
        generate_readme(&project_path, project_name)?;
    }

    // Initialize git
    if !no_git {
        init_git(&project_path, verbose)?;
    }

    // Generate Xcode project via XcodeGen
    run_xcodegen(&project_path, verbose)?;

    println!("✅ Project '{}' created successfully!", project_name);
    println!("   Path: {}", project_path.display());
    println!("   Xcode: {}.xcodeproj", project_name);
    println!();
    println!("Next steps:");
    println!("  cd \"{}\"", project_path.display());
    println!("  open {}.xcodeproj", project_name);
    println!("  # Re-run `xcodegen generate` after editing project.yml");

    Ok(())
}

fn create_project_structure(project_path: &Path) -> Result<()> {
    fs::create_dir_all(project_path)?;
    fs::create_dir_all(project_path.join("src"))?;
    fs::create_dir_all(project_path.join("data"))?;
    fs::create_dir_all(project_path.join("resources"))?;
    fs::create_dir_all(project_path.join("resources/Assets.xcassets"))?;
    fs::write(project_path.join("data/.gitkeep"), "")?;
    fs::write(
        project_path.join("resources/Assets.xcassets/Contents.json"),
        r#"{
  "info": {
    "author": "xcode",
    "version": 1
  }
}
"#,
    )?;

    Ok(())
}

fn generate_template_files(
    project_path: &Path,
    project_name: &str,
    _template: &str,
    bundle_id: &str,
    _author: &str,
) -> Result<()> {
    let class_name = to_pascal_case(project_name);

    // Generate header file
    let header_content = format!(
        r#"#pragma once

#include <oflike/ofApp.h>

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
    let mut impl_content = format!(
        r#"#include "{}.h"

void {}::setup() {{
    // Initialization
    ofSetWindowTitle("{}");
}}

void {}::update() {{
    // Update logic
}}

void {}::draw() {{
    ofClear(50, 50, 50);

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
        class_name, class_name, class_name, class_name, class_name
    );
    impl_content.push_str(&format!(
        r#"
extern "C" ofBaseApp* ofCreateApp() {{
    return new {}();
}}
"#,
        class_name
    ));

    fs::write(
        project_path.join("src").join(format!("{}.cpp", class_name)),
        impl_content,
    )?;

    // Generate SwiftUI entry and Info.plist
    generate_swiftui_entry(project_path, &class_name)?;
    generate_info_plist(project_path, project_name, bundle_id)?;

    Ok(())
}

fn generate_swiftui_entry(project_path: &Path, class_name: &str) -> Result<()> {
    let swift_content = format!(
        r#"import SwiftUI

@main
struct {}App: App {{
    var body: some Scene {{
        WindowGroup {{
            MetalView()
                .frame(minWidth: 800, minHeight: 600)
        }}
        .windowStyle(.titleBar)
    }}
}}
"#,
        class_name
    );

    fs::write(project_path.join("src").join("App.swift"), swift_content)?;

    let oflike_root = find_oflike_root()?;
    let swiftui_dir = oflike_root.join("src").join("platform").join("swiftui");

    let metal_view_src = swiftui_dir.join("MetalView.swift");
    let performance_src = swiftui_dir.join("PerformanceMonitor.swift");

    if !metal_view_src.exists() {
        return Err(GeneratorError::Other(format!(
            "Missing SwiftUI template: {}",
            metal_view_src.display()
        )));
    }

    if !performance_src.exists() {
        return Err(GeneratorError::Other(format!(
            "Missing SwiftUI template: {}",
            performance_src.display()
        )));
    }

    fs::copy(&metal_view_src, project_path.join("src").join("MetalView.swift"))?;
    fs::copy(
        &performance_src,
        project_path.join("src").join("PerformanceMonitor.swift"),
    )?;

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

fn resolve_project_base_path(path: Option<&str>, config: Option<&Config>) -> Result<PathBuf> {
    if let Some(p) = path {
        return Ok(expand_tilde(p));
    }

    if let Some(root) = config.and_then(|c| c.paths.oflike_metal_root.as_deref()) {
        return Ok(expand_tilde(root).join("apps"));
    }

    Ok(find_oflike_root()?.join("apps"))
}

fn expand_tilde(path: &str) -> PathBuf {
    if path == "~" {
        if let Ok(home) = std::env::var("HOME") {
            return PathBuf::from(home);
        }
    }

    if let Some(stripped) = path.strip_prefix("~/") {
        if let Ok(home) = std::env::var("HOME") {
            return PathBuf::from(home).join(stripped);
        }
    }

    PathBuf::from(path)
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

fn generate_cmake_file(project_path: &Path, project_name: &str, addon_list: &[String]) -> Result<()> {
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

    cmake_content.push_str(
        r#"# Swift sources
file(GLOB SWIFT_SOURCES "src/*.swift")

"#,
    );

    cmake_content.push_str(&format!(
        r#"# Executable
add_executable({} MACOSX_BUNDLE ${{SOURCES}} ${{HEADERS}}"#,
        project_name
    ));

    cmake_content.push_str(" ${SWIFT_SOURCES}");

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
    cmake_content.push_str(&format!(
        r#"# Bundle settings
set_target_properties({} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST "${{CMAKE_CURRENT_SOURCE_DIR}}/resources/Info.plist"
    RESOURCE "${{CMAKE_CURRENT_SOURCE_DIR}}/resources;${{CMAKE_CURRENT_SOURCE_DIR}}/data"
)
"#,
        project_name
    ));

    fs::write(project_path.join("CMakeLists.txt"), cmake_content)?;

    Ok(())
}

fn generate_xcodegen_file(
    project_path: &Path,
    project_name: &str,
    bundle_id: &str,
    addon_list: &[String],
) -> Result<()> {
    let oflike_root = find_oflike_root().ok();
    let static_lib_dir = oflike_root
        .as_ref()
        .map(|root| root.join("build"))
        .filter(|path| path.join("liboflike-metal.a").exists());

    let library_settings_yaml = if static_lib_dir.is_some() {
        "      LIBRARY_SEARCH_PATHS:\n        - \"$(PROJECT_DIR)/../../build\"\n        - \"$(PROJECT_DIR)/../../build/third_party\"\n      OTHER_LDFLAGS:\n        - \"-loflike-metal\"\n        - \"-ltess2\"\n        - \"-loscpack\"\n        - \"-lpugixml\"\n"
            .to_string()
    } else {
        String::new()
    };

    let dependencies_yaml = if static_lib_dir.is_some() {
        "    dependencies:\n      - sdk: Cocoa.framework\n      - sdk: Metal.framework\n      - sdk: MetalKit.framework\n      - sdk: QuartzCore.framework\n      - sdk: CoreGraphics.framework\n      - sdk: CoreText.framework\n      - sdk: ImageIO.framework\n      - sdk: Accelerate.framework\n"
    } else {
        "    dependencies:\n      - framework: oflike-metal.framework\n        embed: true\n"
    };

    let mut sources_list = vec![
        "src".to_string(),
        "../../shaders/Basic2D.metal".to_string(),
        "../../shaders/Basic3D.metal".to_string(),
    ];
    if !addon_list.is_empty() {
        for addon in addon_list {
            sources_list.push(format!("addons/{}", addon));
        }
    }

    let mut sources_yaml_lines = sources_list
        .iter()
        .map(|s| format!("      - {}", s))
        .collect::<Vec<_>>();
    sources_yaml_lines.push("      - path: data".to_string());
    sources_yaml_lines.push("        type: folder".to_string());
    sources_yaml_lines.push("        buildPhase: resources".to_string());
    sources_yaml_lines.push("      - path: resources".to_string());
    sources_yaml_lines.push("        type: folder".to_string());
    sources_yaml_lines.push("        buildPhase: resources".to_string());
    let sources_yaml = sources_yaml_lines.join("\n");

    let xcodegen_content = format!(
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
      MTL_ENABLE_DEBUG_INFO: "NO"
      MTL_COMPILER_FLAGS: "-fmodules-cache-path=/tmp/oflike_metal_module_cache"
      SWIFT_OBJC_BRIDGING_HEADER: "$(PROJECT_DIR)/../../src/platform/bridge/oflike-metal-Bridging-Header.h"
      CLANG_CXX_LANGUAGE_STANDARD: "c++20"
      CLANG_CXX_LIBRARY: "libc++"
      ENABLE_HARDENED_RUNTIME: NO
      CODE_SIGNING_ALLOWED: NO
      CODE_SIGNING_REQUIRED: NO
      CODE_SIGN_IDENTITY: ""
      HEADER_SEARCH_PATHS:
        - "$(PROJECT_DIR)/../../src"
{}
{}
    scheme:
      testTargets: []
"#,
        project_name,
        bundle_id,
        project_name,
        sources_yaml,
        bundle_id,
        library_settings_yaml,
        dependencies_yaml
    );

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

fn generate_readme(project_path: &Path, project_name: &str) -> Result<()> {
    let readme_content = format!(
        r#"# {}

Generated with oflike-gen (oflike-metal Project Generator).

## Build

### With XcodeGen (Recommended)

```bash
open {}.xcodeproj
```

Re-generate the Xcode project after editing `project.yml`:

```bash
xcodegen generate
```

### With xcodebuild (CLI)

```bash
mkdir -p /tmp/oflike_metal_module_cache
xcodebuild -project {}.xcodeproj \
  -scheme {} \
  -configuration Debug \
  -destination "platform=macOS" \
  -derivedDataPath build/DerivedData/{} \
  MTL_COMPILER_FLAGS="-fmodules-cache-path=/tmp/oflike_metal_module_cache" \
  build
```

### Using repo scripts (from oflike-metal root)

```bash
./scripts/build_app.sh {}
./scripts/run_app.sh {}
# Metal debug
./scripts/run_app.sh {} --metal-debug
```

### Notes

- Xcode 26 requires the Metal Toolchain component (Xcode Settings > Components).
- Verify `xcrun metal -v` works before building.
- Shader debug info is disabled to avoid Metal cache warnings; set `MTL_ENABLE_DEBUG_INFO = INCLUDE_SOURCE` in `project.yml` if needed.
- Set `OFL_METAL_DEBUG=1` to log Metal library probing at app startup.

### With CMake

```bash
mkdir build && cd build
cmake .. -G Xcode
open {}.xcodeproj
```

## Dependencies

- oflike-metal framework

## License

MIT
"#,
        project_name,
        project_name,
        project_name,
        project_name,
        project_name,
        project_name,
        project_name,
        project_name,
        project_name,
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

fn run_xcodegen(project_path: &Path, verbose: bool) -> Result<()> {
    if verbose {
        println!("Running XcodeGen...");
    }

    let output = Command::new("xcodegen")
        .arg("generate")
        .current_dir(project_path)
        .output()
        .map_err(|err| {
            if err.kind() == std::io::ErrorKind::NotFound {
                GeneratorError::Other(
                    "XcodeGen not found. Install with: brew install xcodegen".to_string(),
                )
            } else {
                GeneratorError::Other(format!("Failed to run xcodegen: {}", err))
            }
        })?;

    if !output.status.success() {
        let stdout = String::from_utf8_lossy(&output.stdout);
        let stderr = String::from_utf8_lossy(&output.stderr);
        let detail = if verbose {
            format!("stdout:\n{}\nstderr:\n{}", stdout.trim(), stderr.trim())
        } else if !stderr.trim().is_empty() {
            stderr.trim().to_string()
        } else {
            stdout.trim().to_string()
        };
        let message = if detail.is_empty() {
            "XcodeGen failed with no output".to_string()
        } else {
            format!("XcodeGen failed: {}", detail)
        };
        return Err(GeneratorError::Other(message));
    }

    if verbose {
        let stdout = String::from_utf8_lossy(&output.stdout);
        if !stdout.trim().is_empty() {
            println!("{}", stdout.trim());
        }
        let stderr = String::from_utf8_lossy(&output.stderr);
        if !stderr.trim().is_empty() {
            eprintln!("{}", stderr.trim());
        }
    }

    Ok(())
}
