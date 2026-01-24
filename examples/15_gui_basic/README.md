# Example 15: GUI Basic

## Overview

Demonstrates basic **ofxGui** usage with SwiftUI integration. Shows how to create a simple parameter panel with common GUI controls.

## Features

- **ofxPanel** setup and initialization
- **Float slider** for radius control (10-300 range)
- **Int slider** for circle resolution (3-128 range)
- **Boolean toggle** for wireframe/fill mode
- **Color picker** for shape color selection
- **Button** with callback for resetting values
- **Label** for displaying dynamic text (FPS)

## GUI Integration

The GUI uses SwiftUI for native macOS styling:
- `.ultraThinMaterial` translucent background
- Automatic dark mode support
- macOS-native controls (Slider, Toggle, ColorPicker, Button)
- Real-time parameter binding to C++ variables

## Controls

- **Space**: Toggle wireframe mode
- **R**: Randomize shape color
- **+/-**: Increase/decrease radius
- **Mouse**: Interact with GUI controls

## Visual Elements

- Animated central circle (controlled by GUI parameters)
- 8 orbiting smaller circles
- Rotating animation
- Corner frame indicators
- Center crosshair

## Code Structure

```cpp
class GuiBasicApp : public ofBaseApp {
    ofxPanel gui;               // Main GUI panel
    float radius;               // Parameter bound to slider
    int resolution;             // Parameter bound to slider
    bool wireframe;             // Parameter bound to toggle
    ofColor shapeColor;         // Parameter bound to color picker
    std::string infoText;       // Parameter bound to label

    void setup() {
        gui.setup("Controls");
        gui.addSlider("Radius", radius, 10.0f, 300.0f);
        gui.addSlider("Resolution", resolution, 3, 128);
        gui.addToggle("Wireframe", wireframe);
        gui.addColor("Shape Color", shapeColor);
        gui.addButton("Reset", []{ /* callback */ });
        gui.addLabel("Info", infoText);
    }
};
```

## Usage

1. Replace TestApp in `src/platform/bridge/SwiftBridge.mm`:
   ```cpp
   #include "../../examples/15_gui_basic/main.cpp"
   testApp_ = std::make_unique<GuiBasicApp>();
   ```
2. Build and run in Xcode
3. GUI panel appears on the right side of window
4. Adjust parameters and see real-time updates

## Learning Points

- How to create an **ofxPanel**
- How to add different widget types
- How to bind C++ variables to GUI controls
- How to create button callbacks
- SwiftUI automatically handles rendering and interaction
- Parameters are synchronized in real-time

## Related Examples

- **16_gui_groups**: Advanced example with parameter groups and tabs
- **11_events**: Mouse and keyboard event handling

## Dependencies

- **ofxGui** (SwiftUI-based, included in Phase 20)
- **ofGraphics** for drawing
- **ofColor** for color management
- **ofUtils** for utility functions

## Architecture Notes

- GUI rendering is handled by SwiftUI (not drawn in `draw()`)
- Parameters are bound through `GuiParameterStore.shared`
- C++ → Swift bridge uses Objective-C++ interop
- GUI panel uses pImpl pattern for platform abstraction
