# Example 12: ofEasyCam - Interactive 3D Camera

Comprehensive demonstration of ofEasyCam, showcasing intuitive 3D navigation with mouse control, inertia, and advanced camera features.

## Features

### Camera Controls
- **Orbit Control**: Left-drag to rotate around target point
- **Zoom Control**: Right-drag or scroll wheel to zoom in/out
- **Pan Control**: Middle-drag to pan camera (shift target)
- **Smooth Inertia**: Optional momentum-based camera movement
- **Auto Distance**: Automatic distance adjustment to fit scene

### Camera Properties
- **Target Setting**: Define the point the camera looks at
- **Distance Control**: Adjust distance from target
- **FOV Control**: Field of view adjustment
- **Near/Far Clipping**: Control rendering depth range
- **Reset Functionality**: Return to default view instantly

### Scene Features
- **Multiple Objects**: Spheres, boxes, and ground plane
- **Lighting**: Directional light with shadows
- **Grid Display**: Reference grid for orientation
- **Axis Display**: XYZ axes visualization
- **Status Overlay**: Real-time camera information
- **Interactive Targets**: Switch between preset viewpoints

## Controls

| Key/Action | Description |
|------------|-------------|
| **Mouse Left Drag** | Orbit camera around target |
| **Mouse Right Drag** | Zoom in/out (dolly) |
| **Mouse Wheel** | Zoom in/out alternative |
| **Mouse Middle Drag** | Pan camera (move target) |
| **1-5** | Jump to preset target positions |
| **+ / -** | Increase/decrease distance |
| **I** | Toggle inertia (momentum) |
| **M** | Enable/disable mouse input |
| **A** | Toggle auto distance mode |
| **R** | Reset to default view |
| **T** | Animate target position |
| **SPACE** | Reset orientation only |
| **G** | Toggle grid display |
| **X** | Toggle axis display |
| **F** | Adjust FOV |
| **ESC** | Quit application |

## Code Structure

```cpp
class EasyCamApp : public ofBaseApp {
    ofEasyCam camera;
    ofLight light;
    ofMaterial material;

    void setup() override {
        // Configure camera
        camera.setDistance(300);
        camera.setTarget(ofVec3f(0, 0, 0));
        camera.enableMouseInput();
        camera.enableInertia();

        // Setup scene
        light.setDirectional();
        light.setPosition(100, 200, 100);
        material.setShininess(64);
    }

    void draw() override {
        camera.begin();
        // Draw 3D scene
        camera.end();
    }
};
```

## What You'll Learn

1. **3D Camera Navigation**: Understanding orbit, zoom, and pan controls
2. **Camera Configuration**: Setting up FOV, clipping planes, target
3. **Interactive Controls**: Implementing intuitive 3D interaction
4. **Camera Inertia**: Adding momentum for smooth movement
5. **View Presets**: Saving and restoring camera positions
6. **Auto Distance**: Automatic framing of scene content
7. **Camera Debugging**: Visualizing camera state and parameters

## Technical Details

### ofEasyCam Features

**Mouse Interaction**:
- Left drag: Rotation around target (orbit)
- Right drag: Distance adjustment (dolly)
- Middle drag: Target translation (pan)
- Scroll wheel: Distance adjustment
- Automatic constraint to prevent flipping

**Inertia System**:
- Smooth deceleration after mouse release
- Configurable damping factor
- Independent for rotation and translation
- Can be enabled/disabled at runtime

**Target Tracking**:
- Camera always looks at target point
- Target can be static or animated
- Smooth target transitions
- Distance maintained during orbit

**Auto Distance**:
- Automatically adjusts distance to frame content
- Based on scene bounds or specified objects
- Smooth transitions to new distances

### Camera Matrix Pipeline

```cpp
// ofEasyCam internally manages:
1. Position (spherical coordinates around target)
2. Orientation (quaternion for rotation)
3. Target (look-at point)
4. Distance (radius from target)

// Generates:
- View Matrix (modelViewMatrix)
- Projection Matrix (projectionMatrix)
- MVP Matrix (combined transformation)
```

### Performance Considerations
- Efficient quaternion-based rotation
- Lazy matrix update (only when changed)
- Optional inertia update (can be disabled)
- Minimal overhead for mouse tracking

## Building

### Using Xcode
```bash
open examples/12_easycam/12_easycam.xcodeproj
# Build and Run (Cmd+R)
```

### Using CMake
```bash
mkdir build && cd build
cmake .. -G Xcode
cmake --build . --target 12_easycam --config Release
./12_easycam
```

## Related Examples

- **06_3d_primitives**: 3D shapes (used in scene)
- **07_camera**: Basic ofCamera usage
- **08_lighting**: Light and material setup
- **09_mesh**: Custom 3D geometry

## API Reference

### ofEasyCam Methods

**Setup**:
- `setDistance(float distance)` - Set camera distance from target
- `setTarget(ofVec3f target)` - Set look-at target point
- `setFov(float fov)` - Set field of view (degrees)
- `setNearClip(float near)` - Set near clipping plane
- `setFarClip(float far)` - Set far clipping plane

**Mouse Control**:
- `enableMouseInput()` - Enable mouse control
- `disableMouseInput()` - Disable mouse control
- `getMouseInputEnabled()` - Query mouse input state

**Inertia**:
- `enableInertia()` - Enable momentum-based movement
- `disableInertia()` - Disable inertia
- `getInertiaEnabled()` - Query inertia state

**Auto Distance**:
- `setAutoDistance(bool auto)` - Enable/disable auto distance
- `getAutoDistance()` - Query auto distance state

**Query State**:
- `getDistance()` - Get current distance from target
- `getTarget()` - Get current target point
- `getFov()` - Get field of view

**Reset**:
- `reset()` - Reset to default view

**Camera Base**:
- Inherits all `ofCamera` methods (see Example 07)

## Expected Output

When you run this example, you should see:
- A 3D scene with multiple colored spheres and boxes
- A ground plane with grid
- XYZ axes for orientation
- Directional lighting with shading
- Smooth camera movement when dragging
- Momentum-based deceleration (if inertia enabled)
- Status overlay showing:
  - Current camera position
  - Target position
  - Distance from target
  - FOV value
  - Mouse input state (enabled/disabled)
  - Inertia state (enabled/disabled)
  - Auto distance state

## Tips

1. **Orbit Practice**: Left-drag slowly to see smooth orbital rotation
2. **Zoom Control**: Use scroll wheel for fine zoom control
3. **Inertia Feel**: Flick mouse quickly with inertia on for momentum
4. **Preset Views**: Use 1-5 keys to jump to interesting angles
5. **Target Animation**: Press 'T' to see animated target tracking
6. **Reset Often**: Use 'R' to return to default view when lost
7. **Grid Reference**: Keep grid on ('G') for orientation help
8. **Distance Adjustment**: Use +/- for precise distance control

## Common Use Cases

### Architectural Visualization
```cpp
camera.setTarget(buildingCenter);
camera.setDistance(buildingHeight * 2);
camera.setFov(45);  // Natural perspective
```

### Product Inspection
```cpp
camera.enableInertia();
camera.setAutoDistance(true);
camera.setTarget(productCenter);
// User can freely orbit and inspect
```

### Game-Style Camera
```cpp
camera.disableInertia();  // Immediate response
camera.setTarget(playerPosition);
camera.setDistance(100);
// Follow player with immediate control
```

### Cinematic Flythrough
```cpp
// Animate target over time
ofVec3f target = path.getPointAt(time);
camera.setTarget(target);
// Camera smoothly follows path
```

## Advanced Techniques

### Custom Constraints
```cpp
// Limit rotation range
void update() override {
    ofVec3f pos = camera.getPosition();
    if (pos.y < 0) {
        // Prevent camera going below ground
        pos.y = 0;
        camera.setPosition(pos);
    }
}
```

### Preset Views
```cpp
struct CameraPreset {
    ofVec3f position;
    ofVec3f target;
    float distance;
};

void loadPreset(CameraPreset preset) {
    camera.setTarget(preset.target);
    camera.setDistance(preset.distance);
}
```

### Smooth Transitions
```cpp
void transitionToTarget(ofVec3f newTarget, float duration) {
    ofVec3f currentTarget = camera.getTarget();
    // Lerp over time in update()
    float t = ofGetElapsedTimef() / duration;
    ofVec3f target = currentTarget.interpolate(newTarget, t);
    camera.setTarget(target);
}
```

## Differences from openFrameworks

oflike-metal's ofEasyCam is fully compatible with openFrameworks:
- ✅ Same API and behavior
- ✅ Same mouse controls
- ✅ Same inertia system
- ✅ Same coordinate system handling

Implementation differences:
- Uses Metal for rendering (not OpenGL)
- Uses `simd` types internally (for Apple Silicon optimization)
- Integrated with SwiftUI event system

## Troubleshooting

**Camera flips unexpectedly**:
- This is a gimbal lock issue; use reset ('R') and avoid extreme angles

**Inertia feels too fast/slow**:
- Inertia speed depends on mouse drag velocity
- Try toggling inertia off ('I') if you prefer immediate control

**Can't see any objects**:
- Camera might be inside or too far from scene
- Press 'R' to reset to default view
- Check near/far clipping planes

**Mouse not responding**:
- Press 'M' to ensure mouse input is enabled
- Check if another UI element captured mouse

## Further Exploration

- Implement camera path recording/playback
- Add first-person camera mode
- Create a camera animation system
- Implement multi-camera switching
- Add collision detection for camera
- Create VR-style dual camera rendering
- Implement depth-of-field effects based on distance

## Integration Notes

To use ofEasyCam in your own project:
1. Include `<oflike/3d/ofEasyCam.h>`
2. Create an `ofEasyCam` member variable
3. Configure in `setup()`: target, distance, FOV
4. Call `camera.begin()` before 3D drawing
5. Draw your 3D scene
6. Call `camera.end()` after 3D drawing
7. Enable mouse input with `camera.enableMouseInput()`
8. No need to handle mouse events manually!
