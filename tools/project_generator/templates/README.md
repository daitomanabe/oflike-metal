# oflike-metal Project Templates

This directory contains template files used by the `oflike-gen` CLI tool to generate new projects.

## Template Files

### Core Templates

- **ofApp.h.template** - Application class header
- **ofApp.cpp.template** - Application class implementation
- **main.mm.template** - ofMain entry point (legacy)
- **App.swift.template** - SwiftUI entry point (default)

### Build Configuration Templates

- **CMakeLists.txt.template** - CMake build configuration
- **project.yml.template** - XcodeGen project definition
- **Info.plist.template** - macOS application info
- **Entitlements.plist.template** - Sandbox and capability settings

### Project Files

- **README.md.template** - Generated project README
- **.gitignore.template** - Git ignore rules

## Template Syntax

Templates use a simple substitution syntax with `{{VARIABLE}}` placeholders:

### Available Variables

- `{{PROJECT_NAME}}` - Project name (e.g., "myProject")
- `{{CLASS_NAME}}` - PascalCase class name (e.g., "MyProject")
- `{{BUNDLE_ID}}` - macOS bundle identifier (e.g., "com.example.myProject")
- `{{BUNDLE_ID_PREFIX}}` - Bundle ID prefix (e.g., "com.example")
- `{{AUTHOR}}` - Author name

### Conditional Blocks

Templates support conditional rendering:

```
{{#if_swiftui}}
This content appears for SwiftUI projects
{{/if_swiftui}}

{{#if_ofmain}}
This content appears for ofMain projects
{{/if_ofmain}}
```

## Usage

Templates are automatically loaded by the CLI generator:

```bash
# SwiftUI entry (uses App.swift.template)
oflike-gen new myProject

# ofMain entry (uses main.mm.template)
oflike-gen new myProject --entry ofmain
```

## Adding New Templates

To add a new template:

1. Create a `.template` file in this directory
2. Use `{{VARIABLE}}` syntax for substitutions
3. Update the generator code to process the new template
4. Add conditional blocks if needed with `{{#if_*}}` / `{{/if_*}}`

## Template Processing

The CLI generator performs the following steps:

1. Read template file
2. Replace all `{{VARIABLE}}` placeholders
3. Process conditional blocks based on entry mode
4. Write output to project directory

## Notes

- All templates use UTF-8 encoding
- Line endings should be LF (Unix-style)
- Templates should follow the project's code style
- SwiftUI is the default entry mode (ARCHITECTURE.md compliant)
- ofMain entry is legacy/compatibility mode
