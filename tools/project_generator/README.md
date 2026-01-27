# oflike-metal Project Generator

**Version**: 1.1.0
**Last Updated**: 2026-01-25
**Status**: Active (Phase 10.1)

> Command-line tool for generating oflike-metal projects with standardized structure and build configuration.

---

## Table of Contents

1. [Overview](#overview)
2. [Installation](#installation)
3. [CLI Reference](#cli-reference)
4. [Input Schema](#input-schema)
5. [Templates](#templates)
6. [Examples](#examples)
7. [Troubleshooting](#troubleshooting)

---

## Overview

The **oflike-gen** CLI tool generates fully-configured oflike-metal projects with:

✅ **Standardized Structure** - Consistent folder layout (see [PROJECT_STRUCTURE.md](../../docs/PROJECT_STRUCTURE.md))
✅ **SwiftUI Entry** - Modern macOS app by default
✅ **Addon Integration** - Reference, Copy, or Symlink strategies
✅ **Build Systems** - CMake and XcodeGen support
✅ **Immediate Build** - Generated projects build without manual setup

---

## Installation

### From Source (Development)

```bash
# Clone repository
git clone https://github.com/daito-lab/oflike-metal.git
cd oflike-metal/tools/project_generator

# Build and install
cargo build --release
sudo cp target/release/oflike-gen /usr/local/bin/
```

### Verify Installation

```bash
oflike-gen --version
# Output: oflike-gen 1.0.0
```

### Run From Source (Rust)

```bash
# From tools/project_generator
cargo run --release -- new myProject
```

---

## CLI Reference

### Global Options

```bash
oflike-gen [OPTIONS] <COMMAND>
```

| Option | Description | Default |
|--------|-------------|---------|
| `--version` | Print version information | - |
| `--help` | Print help information | - |
| `--verbose` | Enable verbose output | false |
| `--config <PATH>` | Custom config file path | `~/.oflike-gen.toml` |

---

### Command: `new`

Generate a new oflike-metal project.

#### Syntax

```bash
oflike-gen new <PROJECT_NAME> [OPTIONS]
```

#### Positional Arguments

| Argument | Type | Description | Required |
|----------|------|-------------|----------|
| `PROJECT_NAME` | string | Project name (PascalCase or kebab-case) | ✅ |

#### Options

| Option | Type | Description | Default |
|--------|------|-------------|---------|
| `--addons <LIST>` | comma-separated | Initial addons (e.g., `ofxOsc,ofxGui`) | none |
| `--addon-mode <MODE>` | `reference \| copy \| symlink` | Addon integration strategy | `reference` |
| `--path <DIR>` | path | Project creation directory | `<oflike-root>/apps/<PROJECT_NAME>` |
| `--template <NAME>` | `basic \| swiftui \| metal \| 3d` | Project template | `basic` |
| `--bundle-id <ID>` | string | macOS bundle identifier | `com.example.<project>` |
| `--author <NAME>` | string | Project author name | Git config user.name |
| `--no-git` | flag | Skip git initialization | false |
| `--no-readme` | flag | Skip README.md generation | false |

Note: If `--path` is omitted, the generator uses `<oflike-root>/apps` (auto-detected) or `paths.oflike_metal_root` from `~/.oflike-gen.toml`.
Note: `oflike-gen new` runs `xcodegen generate` after creating `project.yml` to produce `<project>.xcodeproj`.

#### Examples

**Basic SwiftUI Project** (Recommended):
```bash
oflike-gen new myProject
```

Generated structure:
```
myProject/
├── src/
│   ├── MyApp.h
│   ├── MyApp.cpp
│   ├── App.swift         # SwiftUI entry
│   ├── MetalView.swift   # MTKView + bridge
│   └── PerformanceMonitor.swift # FPS/stats overlay
├── data/
├── resources/
│   ├── Info.plist
│   └── Assets.xcassets/
├── CMakeLists.txt
├── project.yml
├── .gitignore
└── README.md
```

**With Core Addons**:
```bash
oflike-gen new musicApp --addons ofxOsc,ofxGui
```

`CMakeLists.txt` includes:
```cmake
# Core Addons (linked via oflike-metal framework)
# ofxOsc, ofxGui are available
```

**Custom Path**:
```bash
oflike-gen new demoApp --path ~/projects/demos
```

**3D Template**:
```bash
oflike-gen new modelViewer --template 3d --addons ofxSharp
```

Includes:
- 3D camera setup
- Mesh loading example
- Lighting configuration

**Full Options**:
```bash
oflike-gen new myStudio \
  --addons ofxOsc,ofxGui,ofxSharp \
  --template swiftui \
  --bundle-id com.mystudio.app \
  --author "John Doe" \
  --path ~/projects
```

---

### Command: `add-addon`

Add an addon to an existing project.

#### Syntax

```bash
oflike-gen add-addon <ADDON_NAME> [OPTIONS]
```

#### Positional Arguments

| Argument | Type | Description | Required |
|----------|------|-------------|----------|
| `ADDON_NAME` | string | Addon name (e.g., `ofxOsc`, `ofxMyAddon`) | ✅ |

#### Options

| Option | Type | Description | Default |
|--------|------|-------------|---------|
| `--mode <MODE>` | `reference \| copy \| symlink` | Integration strategy | `reference` |
| `--source <PATH>` | path | Addon source path (for custom addons) | auto-detect |
| `--project <PATH>` | path | Target project directory | `.` |
| `--update-build` | flag | Update build files (CMakeLists.txt, project.yml) | true |

#### Examples

**Add Core Addon (Reference)**:
```bash
cd myProject
oflike-gen add-addon ofxOsc
```

Result:
- `CMakeLists.txt` updated (if `--update-build`)
- `project.yml` updated (if `--update-build`)
- No files copied (addon is in oflike-metal framework)

**Add Custom Addon (Copy)**:
```bash
oflike-gen add-addon ofxMyAddon \
  --mode copy \
  --source ~/addons/ofxMyAddon
```

Result:
- `myProject/addons/ofxMyAddon/` created
- All addon files copied
- Build files updated

**Add Custom Addon (Symlink)**:
```bash
oflike-gen add-addon ofxThirdParty \
  --mode symlink \
  --source ~/addons/ofxThirdParty
```

Result:
- `myProject/addons/ofxThirdParty` -> `~/addons/ofxThirdParty` (symlink)
- Build files updated

**Multiple Addons**:
```bash
oflike-gen add-addon ofxOsc
oflike-gen add-addon ofxGui
oflike-gen add-addon ofxSharp
```

---

### Command: `remove-addon`

Remove an addon from a project.

#### Syntax

```bash
oflike-gen remove-addon <ADDON_NAME> [OPTIONS]
```

#### Positional Arguments

| Argument | Type | Description | Required |
|----------|------|-------------|----------|
| `ADDON_NAME` | string | Addon name to remove | ✅ |

#### Options

| Option | Type | Description | Default |
|--------|------|-------------|---------|
| `--project <PATH>` | path | Target project directory | `.` |
| `--keep-files` | flag | Keep addon files (only remove from build) | false |

#### Examples

**Remove Addon Completely**:
```bash
cd myProject
oflike-gen remove-addon ofxMyAddon
```

Result:
- `addons/ofxMyAddon/` deleted (if copy/symlink mode)
- Removed from `CMakeLists.txt`
- Removed from `project.yml`

**Remove from Build Only**:
```bash
oflike-gen remove-addon ofxMyAddon --keep-files
```

Result:
- `addons/ofxMyAddon/` remains
- Removed from build files

---

### Command: `list-addons`

List available or project addons.

#### Syntax

```bash
oflike-gen list-addons [OPTIONS]
```

#### Options

| Option | Type | Description | Default |
|--------|------|-------------|---------|
| `--available` | flag | List all available addons (Core + Native) | false |
| `--project <PATH>` | path | List addons in project | `.` |

#### Examples

**List Available Addons**:
```bash
oflike-gen list-addons --available
```

Output:
```
Core Addons:
  - ofxOsc          OSC communication
  - ofxGui          SwiftUI-based GUI
  - ofxXmlSettings  XML configuration
  - ofxSvg          SVG loading
  - ofxNetwork      TCP/UDP networking
  - ofxOpenCv       Computer vision

Apple Native Addons:
  - ofxSharp        3D Gaussian Splatting (Core ML)
  - ofxNeuralEngine ML inference (Core ML/Vision)
  - ofxMetalCompute GPU compute (Metal)
  - ofxMPS          Image processing (MPS)
  - ofxVideoToolbox Video encoding (VideoToolbox)
  - ofxSpatialAudio Spatial audio (PHASE)
  - ofxMetalFX      AI upscaling (MetalFX)
```

**List Project Addons**:
```bash
cd myProject
oflike-gen list-addons
```

Output:
```
Project: myProject

Core Addons (Reference):
  - ofxOsc
  - ofxGui

Custom Addons (Copy):
  - ofxMyAddon (addons/ofxMyAddon)
```

---

### Command: `init`

Initialize oflike-gen configuration.

#### Syntax

```bash
oflike-gen init [OPTIONS]
```

#### Options

| Option | Type | Description | Default |
|--------|------|-------------|---------|
| `--global` | flag | Create global config (`~/.oflike-gen.toml`) | false |
| `--local` | flag | Create local config (`./.oflike-gen.toml`) | true |

#### Example

```bash
oflike-gen init --global
```

Generated `~/.oflike-gen.toml`:
```toml
# oflike-gen configuration
[defaults]
addon_mode = "reference"
author = "John Doe"
bundle_id_prefix = "com.example"

[paths]
oflike_metal_root = "/usr/local/lib/oflike-metal"
addons_dir = "~/addons"

[templates]
default_template = "basic"
```

---

### Command: `validate`

Validate project structure and configuration.

#### Syntax

```bash
oflike-gen validate [OPTIONS]
```

#### Options

| Option | Type | Description | Default |
|--------|------|-------------|---------|
| `--project <PATH>` | path | Project directory to validate | `.` |
| `--fix` | flag | Attempt to fix issues automatically | false |

#### Example

```bash
cd myProject
oflike-gen validate
```

Output:
```
✅ Project structure valid
✅ CMakeLists.txt valid
✅ project.yml valid
⚠️  Missing Info.plist (run with --fix to generate)
✅ Addons configuration valid
```

**With Auto-Fix**:
```bash
oflike-gen validate --fix
```

---

## Input Schema

### Project Configuration Schema

Projects can be configured via `oflike-gen.toml` or `project.json`:

#### `oflike-gen.toml` (TOML Format)

```toml
[project]
name = "myProject"
version = "1.0.0"
author = "John Doe"
bundle_id = "com.example.myproject"

[addons]
core = ["ofxOsc", "ofxGui", "ofxSharp"]
custom = [
    { name = "ofxMyAddon", mode = "copy", source = "~/addons/ofxMyAddon" }
]

[build]
cmake = true
xcodegen = true
min_macos = "13.0"
swift_version = "5.9"
cpp_standard = "c++20"

[paths]
src = "src"
data = "data"
resources = "resources"
addons = "addons"
```

#### `project.json` (JSON Format)

```json
{
  "project": {
    "name": "myProject",
    "version": "1.0.0",
    "author": "John Doe",
    "bundle_id": "com.example.myproject"
  },
  "addons": {
    "core": ["ofxOsc", "ofxGui"],
    "custom": [
      {
        "name": "ofxMyAddon",
        "mode": "copy",
        "source": "~/addons/ofxMyAddon"
      }
    ]
  },
  "build": {
    "cmake": true,
    "xcodegen": true,
    "min_macos": "13.0",
    "swift_version": "5.9",
    "cpp_standard": "c++20"
  }
}
```

### Schema Validation

The generator validates:

| Field | Type | Required | Default | Constraints |
|-------|------|----------|---------|-------------|
| `project.name` | string | ✅ | - | PascalCase or kebab-case |
| `project.version` | string | ❌ | `1.0.0` | Semantic versioning |
| `project.bundle_id` | string | ❌ | `com.example.<name>` | Reverse DNS format |
| `addons.core` | array | ❌ | `[]` | Valid Core/Native addon names |
| `addons.custom` | array | ❌ | `[]` | Objects with `name`, `mode`, `source` |
| `build.min_macos` | string | ❌ | `13.0` | >= 13.0 |
| `build.cpp_standard` | string | ❌ | `c++20` | `c++17` \| `c++20` |

---

## Templates

### Available Templates

#### 1. `basic` (Default)

Minimal project with empty app class.

**Files Generated**:
- `src/MyApp.h`, `src/MyApp.cpp`
- `src/App.swift` (SwiftUI)
- Build configuration
- Empty data/resources folders

**Use Case**: General-purpose starting point

#### 2. `swiftui`

SwiftUI-focused project with advanced UI integration.

**Additional Files**:
- `src/AppState.swift` - SwiftUI state management
- `src/MetalView.swift` - Custom MTKView wrapper
- `src/PerformanceMonitor.swift` - FPS/stats overlay

**Use Case**: macOS apps with rich UI

#### 3. `metal`

Metal compute and shader-focused project.

**Additional Files**:
- `shaders/Compute.metal` - Example compute shader
- `shaders/Render.metal` - Example render shader
- `src/MetalPipeline.h/.cpp` - Pipeline setup

**Use Case**: GPU compute, shader development

#### 4. `3d`

3D graphics project with camera, lighting, models.

**Additional Files**:
- `src/Camera.h/.cpp` - Camera controller
- `data/models/` - Example models
- `src/Scene.h/.cpp` - Scene management

**Use Case**: 3D visualization, model viewers

---

## Examples

### Complete Workflow

**1. Create New Project**:
```bash
oflike-gen new particleSystem --template swiftui
cd particleSystem
```

**2. Add Addons**:
```bash
oflike-gen add-addon ofxGui
oflike-gen add-addon ofxSharp
```

**3. Validate**:
```bash
oflike-gen validate --fix
```

**4. Build with CMake**:
```bash
mkdir build && cd build
cmake .. -G Xcode
open particleSystem.xcodeproj
```

**5. Open Xcode Project**:
```bash
open particleSystem.xcodeproj
```

**Re-generate with XcodeGen (if project.yml changes)**:
```bash
xcodegen generate
```

### Custom Addon Workflow

**1. Create Project**:
```bash
oflike-gen new myApp
```

**2. Add Custom Addon (Copy)**:
```bash
oflike-gen add-addon ofxMyAddon --mode copy --source ~/dev/ofxMyAddon
```

**3. Verify Structure**:
```bash
tree myApp/addons
# myApp/addons/
# └── ofxMyAddon/
#     ├── src/
#     └── README.md
```

**4. Build**:
```bash
cd myApp/build
cmake ..
make
```

---

## Troubleshooting

### Issue: "Addon not found"

**Error**:
```
Error: Addon 'ofxMyAddon' not found in Core/Native addons
```

**Solution**:
- Specify `--source` for custom addons:
  ```bash
  oflike-gen add-addon ofxMyAddon --mode copy --source ~/addons/ofxMyAddon
  ```

### Issue: "Bundle ID invalid"

**Error**:
```
Error: Invalid bundle ID format
```

**Solution**:
- Use reverse DNS format:
  ```bash
  oflike-gen new myApp --bundle-id com.mystudio.myapp
  ```

### Issue: "CMakeLists.txt not updated"

**Error**:
```
Warning: Build files not updated
```

**Solution**:
- Ensure `--update-build` is not disabled:
  ```bash
  oflike-gen add-addon ofxOsc --update-build
  ```

### Issue: "Symlink creation failed"

**Error**:
```
Error: Permission denied creating symlink
```

**Solution**:
- Use `copy` mode instead:
  ```bash
  oflike-gen add-addon ofxMyAddon --mode copy --source ~/addons/ofxMyAddon
  ```

---

## Architecture Compliance

The generator ensures all projects comply with [ARCHITECTURE.md](../../docs/ARCHITECTURE.md):

✅ **SwiftUI Only**: single supported entry mode
✅ **Layer Boundaries**: No Metal imports in oflike layer
✅ **Coordinate System**: 2D top-left, 3D right-hand
✅ **Prohibited Libraries**: FreeType, OpenGL, SDL, stb_image excluded
✅ **macOS 13.0+**: Deployment target enforced

---

## Version History

| Date | Version | Changes |
|------|---------|---------|
| 2026-01-25 | 1.0.0 | Initial CLI spec and schema (Phase 10.1) |
| 2026-01-25 | 1.1.0 | Remove ofMain entry; SwiftUI-only generator |

---

## See Also

- [IMPLEMENTATION.md](../../docs/IMPLEMENTATION.md) - Implementation guidelines
- [PROJECT_STRUCTURE.md](../../docs/PROJECT_STRUCTURE.md) - Project structure
- [ARCHITECTURE.md](../../docs/ARCHITECTURE.md) - Architecture policies
