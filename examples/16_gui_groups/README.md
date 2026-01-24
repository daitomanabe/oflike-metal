# Example 16: GUI Groups

## Overview

Demonstrates advanced **ofxGui** usage with parameter organization using **ofxGuiGroup**. Shows how to create a complex GUI with multiple parameter groups and tab-based navigation.

## Features

### Parameter Groups

1. **Shape Group**
   - Radius slider (20-200)
   - Resolution slider (3-128)
   - Count slider (1-16 shapes)
   - Randomize button

2. **Animation Group**
   - Rotation speed slider (0-5)
   - Orbit radius slider (50-300)
   - Auto-rotate toggle
   - Oscillate toggle
   - Reset animation button

3. **Rendering Group**
   - Wireframe toggle
   - Line width slider (1-10)
   - Alpha slider (0-255)
   - Show trails toggle

4. **Colors Group**
   - Background color picker
   - Main color picker
   - Accent color picker
   - Randomize colors button

## GUI Organization

- **Group Tabs**: Switch between "All", "Shape", "Animation", "Rendering", "Colors"
- **Collapsible Panel**: Click header chevron to expand/collapse
- **Filtered View**: Select a group tab to see only its parameters
- **"All" Tab**: Shows all parameters from all groups

## Controls

- **Space**: Toggle auto-rotation
- **W**: Toggle wireframe mode
- **T**: Toggle trails/orbit circle
- **O**: Toggle oscillation effect
- **R**: Randomize all parameters
- **+/-**: Increase/decrease rotation speed
- **Mouse**: Interact with GUI controls and group tabs

## Visual Elements

- Central animated shape (controlled by Shape group)
- Orbiting shapes (controlled by Animation group)
- Rendering styles (controlled by Rendering group)
- Color scheme (controlled by Colors group)
- Corner frame decoration
- Orbit circle trails (optional)

## Code Structure

```cpp
class GuiGroupsApp : public ofBaseApp {
    // Main panel and groups
    ofxPanel gui;
    ofxGuiGroup shapeGroup;
    ofxGuiGroup animationGroup;
    ofxGuiGroup renderingGroup;
    ofxGuiGroup colorsGroup;

    // Parameters organized by group
    float shapeRadius, orbitRadius;
    int shapeResolution, shapeCount;
    bool wireframe, autoRotate, oscillate, showTrails;
    ofColor backgroundColor, mainColor, accentColor;

    void setup() {
        gui.setup("Advanced Controls");

        // Setup groups
        shapeGroup.setName("Shape");
        shapeGroup.addSlider("Radius", shapeRadius, 20.0f, 200.0f);
        // ... more parameters

        // Add groups to panel
        gui.add(shapeGroup);
        gui.add(animationGroup);
        gui.add(renderingGroup);
        gui.add(colorsGroup);
    }
};
```

## Usage

1. Replace TestApp in `src/platform/bridge/SwiftBridge.mm`:
   ```cpp
   #include "../../examples/16_gui_groups/main.cpp"
   testApp_ = std::make_unique<GuiGroupsApp>();
   ```
2. Build and run in Xcode
3. GUI panel appears with group tabs at the top
4. Click tabs to switch between parameter groups
5. Adjust parameters and see coordinated effects

## Learning Points

- How to create **ofxGuiGroup** instances
- How to organize parameters logically
- How to add groups to an **ofxPanel**
- Group tabs for better usability
- Parameter relationships across groups
- Button callbacks for complex actions
- Color picker integration
- Real-time synchronized updates

## Advanced Techniques

### Parameter Organization
- Related parameters grouped together
- Clear naming hierarchy
- Logical separation of concerns

### Button Callbacks
```cpp
shapeGroup.addButton("Randomize", [this]() {
    shapeRadius = ofRandom(20.0f, 200.0f);
    shapeResolution = ofRandom(8, 64);
    shapeCount = ofRandom(3, 12);
});
```

### Color Management
```cpp
colorsGroup.addColor("Background", backgroundColor);
colorsGroup.addColor("Main Color", mainColor);
colorsGroup.addColor("Accent Color", accentColor);
```

### Parameter Interactions
- Animation affects all visual elements
- Rendering settings apply to all shapes
- Colors propagate through the scene
- Shape parameters control geometry

## Related Examples

- **15_gui_basic**: Basic GUI usage (start here first)
- **11_events**: Event handling
- **08_lighting**: Parameter control for 3D lighting
- **12_easycam**: Camera control with GUI

## Dependencies

- **ofxGui** (SwiftUI-based, Phase 20)
- **ofGraphics** for drawing
- **ofColor** for color management
- **ofUtils** for utility functions

## Architecture Notes

### SwiftUI Integration
- GUI rendered in SwiftUI overlay layer
- Group tabs implemented as horizontal ScrollView
- Selected group filtering in SwiftUI
- .ultraThinMaterial for macOS styling

### Parameter Binding
- C++ → Swift through `GuiParameterStore.shared`
- Objective-C++ bridge with NSInvocation
- Real-time bidirectional synchronization
- Type-safe parameter system

### Performance
- GUI updates don't impact draw performance
- Parameter changes are immediate
- No manual GUI rendering code needed
- SwiftUI handles all UI updates

## UI/UX Features

- **Horizontal scrolling tabs** for group navigation
- **"All" tab** to see all parameters at once
- **Selected group highlighting** with blue background
- **Collapsible panel** to maximize drawing area
- **Smooth animations** for expand/collapse
- **Native macOS styling** with system fonts
- **Dark mode support** automatically

## Tips

1. Start with basic groups (Shape, Animation)
2. Add rendering options later
3. Keep groups focused (5-7 parameters max)
4. Use descriptive names for clarity
5. Group-related parameters together
6. Add buttons for common actions
7. Use labels for dynamic information

## Next Steps

- Try adding your own parameter groups
- Experiment with different parameter ranges
- Add more button callbacks
- Create preset save/load functionality
- Integrate with other examples
