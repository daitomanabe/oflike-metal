# ofxGui

SwiftUI-based GUI system for oflike-metal.

## Overview

ofxGui provides a modern, native macOS GUI for parameter adjustment and real-time control. Built on SwiftUI with `.ultraThinMaterial` background, it seamlessly integrates with the Metal rendering pipeline.

## Features

- **SwiftUI-based**: Native macOS look and feel
- **Parameter binding**: C++ variables automatically sync with GUI
- **Type-safe**: Templated parameter types
- **Grouped parameters**: Organize parameters into collapsible groups
- **Dark mode support**: Automatic system appearance adaptation
- **Minimal overhead**: Efficient rendering on separate UI thread

## Widgets

- **Slider**: Float and Int sliders with min/max range
- **Toggle**: Boolean switches
- **Button**: Action buttons with callbacks
- **ColorPicker**: RGB color selection
- **Label/TextField**: Text input and display
- **Dropdown**: Coming soon

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                   C++ Application                        │
│                   (ofBaseApp)                            │
└─────────────────┬───────────────────────────────────────┘
                  │
                  │ ofxPanel / ofxGuiGroup
                  │ (Pure C++ API)
                  │
┌─────────────────▼───────────────────────────────────────┐
│               ofxGui.h / ofxGui.mm                       │
│               (pImpl Pattern)                            │
│               (Objective-C++ Bridge)                     │
└─────────────────┬───────────────────────────────────────┘
                  │
                  │ NSInvocation / Objective-C Runtime
                  │
┌─────────────────▼───────────────────────────────────────┐
│            OFLGuiPanel.swift                             │
│            GuiParameterStore (Swift Singleton)           │
└─────────────────┬───────────────────────────────────────┘
                  │
                  │ @Published / @ObservedObject
                  │
┌─────────────────▼───────────────────────────────────────┐
│                  SwiftUI Views                           │
│          (Slider, Toggle, ColorPicker, etc.)             │
└──────────────────────────────────────────────────────────┘
```

## Usage

### Basic Example

```cpp
#include "ofxGui.h"

class MyApp : public ofBaseApp {
    ofxPanel gui;
    float radius = 50.0f;
    bool wireframe = false;
    ofColor bgColor;

    void setup() {
        gui.setup("My GUI");
        gui.addSlider("Radius", radius, 10.0f, 200.0f);
        gui.addToggle("Wireframe", wireframe);
        gui.addColor("Background", bgColor);
    }

    void draw() {
        ofBackground(bgColor);

        if (wireframe) {
            ofNoFill();
        } else {
            ofFill();
        }

        ofDrawCircle(ofGetWidth()/2, ofGetHeight()/2, radius);
    }
};
```

### With Groups

```cpp
ofxPanel gui;
ofxGuiGroup shapes;
ofxGuiGroup colors;

void setup() {
    gui.setup("My GUI");

    shapes.setName("Shapes");
    shapes.addSlider("Radius", radius, 10.0f, 200.0f);
    shapes.addSlider("Sides", sides, 3, 12);

    colors.setName("Colors");
    colors.addColor("Fill", fillColor);
    colors.addColor("Stroke", strokeColor);

    gui.add(shapes);
    gui.add(colors);
}
```

### Convenience Types

```cpp
// Using convenience wrappers
ofxFloatSlider radius;
ofxToggle wireframe;
ofxColorPicker bgColor;
ofxButton resetBtn;

void setup() {
    radius.setup("Radius", 50.0f, 10.0f, 200.0f, "Shapes");
    wireframe.setup("Wireframe", false, "Rendering");
    bgColor.setup("Background", ofColor(0, 0, 0), "Colors");
    resetBtn.setup("Reset", []() {
        // Reset button callback
        radius = 50.0f;
        wireframe = false;
    }, "Actions");
}

void draw() {
    // Use parameters directly
    ofBackground(bgColor);

    if (wireframe) {
        ofNoFill();
    }

    ofDrawCircle(ofGetWidth()/2, ofGetHeight()/2, radius);
}
```

## Implementation Notes

### Parameter Binding

Parameters are bound using reference semantics. When you call `gui.addSlider("Name", value, min, max)`, the GUI maintains a pointer to your `value` variable. Changes in the SwiftUI interface will update your C++ variable automatically through the Objective-C++ bridge.

### Thread Safety

SwiftUI rendering happens on the main UI thread, separate from the Metal rendering thread. Parameter updates are thread-safe through SwiftUI's `@Published` property wrappers.

### Performance

The GUI panel is rendered independently of the Metal viewport. Parameter updates have minimal overhead thanks to SwiftUI's efficient diffing and the pImpl pattern used in the C++ bridge.

## SwiftUI Integration

The GUI panel is automatically added to the app's SwiftUI view hierarchy. It appears as an overlay with `.ultraThinMaterial` background, providing a native macOS appearance with automatic dark mode support.

## Limitations

- Parameter names must be unique within a group
- Maximum recommended parameters per panel: 100 (for performance)
- String parameters are single-line only (multiline text fields not yet supported)

## Dependencies

- SwiftUI (macOS 13.0+)
- Foundation
- AppKit (for color conversion)

## License

Part of oflike-metal framework. Same license as the main project.
