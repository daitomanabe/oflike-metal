# Example 07: Camera

Demonstrates camera systems with oflike-metal, including manual camera control and interactive EasyCam.

## Features

This example showcases:

1. **Two Camera Types**:
   - **ofCamera**: Manual camera with full position and orientation control
   - **ofEasyCam**: Interactive camera with automatic mouse orbit/zoom

2. **Camera Switching**: Toggle between manual and EasyCam modes at runtime

3. **Manual Camera Controls**:
   - WASD movement (forward/back, left/right)
   - QE movement (up/down)
   - Arrow key rotation
   - Full 6-DOF (degrees of freedom) control

4. **EasyCam Controls**:
   - Mouse drag to orbit around target
   - Mouse wheel to zoom in/out
   - Smooth inertia for natural motion
   - Automatic target tracking

5. **Complex 3D Scene**:
   - Center animated sphere
   - Four orbiting boxes
   - Corner cylinders with position markers
   - Path of small spheres with rainbow colors
   - Reference grid and coordinate axes

6. **Lighting**:
   - Two point lights with different colors
   - Multiple materials with varying shininess
   - Proper depth testing

## Visual Elements

The scene contains:

- **Center**: Large red sphere that bobs up and down
- **Orbit**: Four green boxes rotating around the center
- **Corners**: Four blue cylinders at the corners with coordinate axes
- **Path**: 12 small rainbow-colored spheres following a 3D path
- **Grid**: Ground plane reference grid
- **Axes**: RGB coordinate axes (X=red, Y=green, Z=blue) at origin and corners

## Controls

### General
- **TAB**: Switch between Manual Camera and EasyCam modes
- **SPACE**: Toggle animation on/off
- **R**: Reset camera to default position

### Manual Camera Mode
- **W/S**: Move forward/backward
- **A/D**: Move left/right (strafe)
- **Q/E**: Move up/down
- **Arrow Keys**: Rotate camera view
  - Left/Right: Yaw rotation
  - Up/Down: Pitch rotation

### EasyCam Mode
- **Mouse Drag**: Orbit camera around target point
- **Mouse Wheel**: Zoom in/out
- **Inertia**: Camera continues moving after mouse release with smooth deceleration

## API Usage

### ofCamera API

```cpp
// Setup camera
ofCamera camera;
camera.setPosition(0, 200, 500);           // Set camera position
camera.lookAt(ofVec3f(0, 0, 0));          // Point camera at target
camera.setNearClip(1.0f);                 // Set near clipping plane
camera.setFarClip(10000.0f);              // Set far clipping plane
camera.setFov(60.0f);                     // Set field of view
camera.setAspectRatio(16.0f / 9.0f);      // Set aspect ratio

// Use camera for rendering
camera.begin();
// ... draw 3D objects ...
camera.end();

// Query camera state
ofVec3f pos = camera.getPosition();
ofQuaternion orientation = camera.getOrientation();
ofMatrix4x4 viewMatrix = camera.getModelViewMatrix();
ofMatrix4x4 projMatrix = camera.getProjectionMatrix();

// Manual orientation control
ofQuaternion pitch(angle, ofVec3f(1, 0, 0));
ofQuaternion yaw(angle, ofVec3f(0, 1, 0));
camera.setOrientation(yaw * pitch);
```

### ofEasyCam API

```cpp
// Setup EasyCam
ofEasyCam easyCam;
easyCam.setDistance(500);                 // Set distance from target
easyCam.setTarget(ofVec3f(0, 0, 0));     // Set orbit target
easyCam.enableMouseInput();               // Enable mouse controls
easyCam.enableInertia();                  // Enable smooth inertia
easyCam.setInertiaDamping(0.85f);        // Set damping factor

// Use EasyCam for rendering (handles mouse automatically)
easyCam.begin();
// ... draw 3D objects ...
easyCam.end();

// Update (call once per frame for inertia)
easyCam.update();

// Manual event forwarding (if not using ofBaseApp)
easyCam.onMousePressed(x, y, button);
easyCam.onMouseDragged(x, y, button);
easyCam.onMouseReleased(x, y, button);
easyCam.onMouseScrolled(scrollX, scrollY);

// Query state
float distance = easyCam.getDistance();
ofVec3f target = easyCam.getTarget();
bool mouseEnabled = easyCam.getMouseInputEnabled();
```

### Drawing Functions

```cpp
// 3D Primitives
ofDrawSphere(x, y, z, radius);
ofDrawBox(x, y, z, size);
ofDrawCylinder(x, y, z, radius, height);
ofDrawGridPlane(spacing, divisions);

// Coordinate axes helper
ofDrawLine(x1, y1, z1, x2, y2, z2);

// Matrix transformations
ofPushMatrix();
ofTranslate(x, y, z);
ofRotateX(degrees);
ofRotateY(degrees);
ofRotateZ(degrees);
ofPopMatrix();
```

### Lighting

```cpp
// Setup lights
ofLight light;
light.setPointLight();
light.setPosition(x, y, z);
light.setAmbientColor(ofColor(r, g, b));
light.setDiffuseColor(ofColor(r, g, b));
light.setSpecularColor(ofColor(r, g, b));

// Enable lighting
ofEnableLighting();
light.enable();

// Setup materials
ofMaterial material;
material.setAmbientColor(ofColor(r, g, b));
material.setDiffuseColor(ofColor(r, g, b));
material.setSpecularColor(ofColor(r, g, b));
material.setShininess(64.0f);

// Use material
material.begin();
// ... draw objects ...
material.end();
```

## Integration

To integrate this example into your oflike-metal project:

1. Copy `main.cpp` to your project
2. Ensure you have linked against the oflike-metal library
3. Register `CameraApp` with your `SwiftBridge.mm`:
   ```cpp
   auto app = std::make_unique<CameraApp>();
   ```

## Expected Output

You should see:
- A 3D scene with multiple animated objects
- Camera mode indicator (red box = Manual, green box = EasyCam)
- Console output showing:
  - Camera mode switches
  - Animation state
  - Camera position (in manual mode with verbose logging)

### Manual Camera Mode
- Camera moves freely through 3D space
- WASD controls feel like first-person game controls
- Arrow keys rotate view direction
- Full freedom to position camera anywhere

### EasyCam Mode
- Camera orbits smoothly around the center point
- Mouse drag feels natural with inertia
- Scroll wheel zooms in/out
- Camera always faces the target point

## Learning Points

1. **Camera Fundamentals**:
   - View matrix (position + orientation)
   - Projection matrix (FOV, aspect ratio, clipping planes)
   - begin()/end() pattern for camera rendering

2. **Manual Camera Control**:
   - Position control with velocity
   - Orientation with quaternions
   - First-person style movement
   - lookAt() for targeting

3. **EasyCam Features**:
   - Orbit camera pattern
   - Target-centric viewing
   - Mouse interaction handling
   - Inertia for smooth motion
   - Automatic event integration

4. **Camera Switching**:
   - Maintaining camera state between modes
   - Syncing positions when switching
   - Mode-specific control handling

5. **3D Scene Setup**:
   - Depth testing for proper occlusion
   - Lighting for 3D appearance
   - Multiple objects for spatial reference
   - Grid and axes for orientation

6. **Mouse Event Handling**:
   - Event forwarding to EasyCam
   - Mode-specific event processing
   - Integration with ofBaseApp event system

7. **Quaternion Rotation**:
   - Pitch and yaw with quaternions
   - Avoiding gimbal lock
   - Smooth rotation composition

## Architecture Notes

- **Pure C++ API**: User code is 100% C++, no Objective-C or Swift required
- **ofBaseApp Pattern**: Standard openFrameworks application structure
- **Camera Abstraction**: ofCamera provides view/projection matrices to renderer
- **EasyCam Integration**: Mouse events automatically handled when enabled
- **Metal Backend**: All rendering uses Metal via oflike-metal's DrawList system
- **Lighting System**: Up to 8 lights with Phong shading (Metal shader)
- **Performance**: Efficient camera matrix updates, minimal CPU overhead

## Console Output

```
[notice ] 07_camera: Setup complete
[notice ] Controls:
[notice ]   TAB: Switch camera mode (Manual/EasyCam)
[notice ]   W/A/S/D/Q/E: Move camera (Manual mode)
[notice ]   Arrow keys: Rotate camera (Manual mode)
[notice ]   Mouse drag: Orbit (EasyCam mode)
[notice ]   Mouse wheel: Zoom (EasyCam mode)
[notice ]   SPACE: Toggle animation
[notice ]   R: Reset camera
[notice ] Current mode: Manual Camera
[notice ] Switched to: EasyCam
[notice ] Animation: OFF
[notice ] Camera reset
[verbose] Camera position: (12.3, 234.5, 456.7)
```

## Performance Notes

- All geometry is generated on CPU and uploaded to GPU each frame
- Lighting calculations performed in Metal shader (GPU)
- Camera matrix updates are CPU-only (once per frame)
- EasyCam inertia updated once per frame (minimal overhead)
- Scene complexity: ~30 draw calls per frame

## Troubleshooting

1. **Camera not moving**: Check that you're in Manual Camera mode (TAB to switch)
2. **Mouse not working**: Ensure you're in EasyCam mode (TAB to switch)
3. **Objects not visible**: Try resetting camera (R key)
4. **No lighting**: Lights are enabled by default, check ofEnableLighting() call
5. **Jerky movement**: EasyCam inertia should be smooth; check update() is called

## Next Steps

- Try **Example 08: Lighting** for advanced lighting techniques
- Try **Example 12: EasyCam** for more EasyCam features and interactions
- Explore **Example 09: Mesh** for custom geometry with cameras

---

**oflike-metal** - Metal-based openFrameworks implementation for macOS
