# Example 06: 3D Primitives

A comprehensive demonstration of 3D primitive rendering capabilities in oflike-metal.

## Overview

This example showcases the core 3D rendering features of oflike-metal, including primitive shapes, camera control, lighting, and materials. It demonstrates how to build interactive 3D scenes using the openFrameworks-compatible API.

## Features

### 3D Primitives Demonstrated

The example displays six different 3D primitives arranged in a 3x2 grid:

**Row 1:**
1. **Box** (Red) - A cube with rotation on Y and X axes
2. **Sphere** (Green) - A smooth sphere with faster rotation
3. **Cylinder** (Blue) - A cylinder oriented horizontally

**Row 2:**
4. **Cone** (Yellow) - An inverted cone with double-speed rotation
5. **Plane** (Cyan) - A flat square plane with combined rotation
6. **IcoSphere** (Magenta) - An icosahedral sphere with slower rotation

### Rendering Features

- **3D Camera**: Fully controllable perspective camera with mouse interaction
- **Lighting System**: Single point light with ambient, diffuse, and specular components
- **Materials**: Each primitive has a unique material with different shininess values
- **Depth Testing**: Proper 3D depth sorting
- **Wireframe Mode**: Toggle between solid and wireframe rendering
- **Grid and Axes**: Visual reference with coordinate axes (RGB = XYZ) and ground grid
- **Smooth Animation**: 60 FPS with continuous rotation

### Interactive Controls

- **SPACE**: Toggle rotation animation on/off
- **L**: Toggle lighting on/off (compare flat shading vs Phong shading)
- **W**: Toggle wireframe mode (see mesh structure)
- **Mouse Drag**: Rotate camera around scene (orbiting)
- **Mouse Scroll**: Zoom in/out (camera distance)

## Visual Layout

```
        Y (green)
        |
        |
   [-150,-100]    [0,-100]    [150,-100]
      BOX         SPHERE      CYLINDER
       🟥           🟩           🟦

   [-150,100]     [0,100]     [150,100]
      CONE         PLANE      ICOSPHERE
       🟨           🟦           🟪

        +-------- X (red)
       /
      /
     Z (blue)
```

## Technical Details

### 3D Primitives API

All primitive functions follow the openFrameworks pattern:

```cpp
// Box
ofDrawBox(x, y, z, size);
ofDrawBox(x, y, z, width, height, depth);

// Sphere
ofDrawSphere(x, y, z, radius);

// Cylinder
ofDrawCylinder(x, y, z, radius, height);

// Cone
ofDrawCone(x, y, z, radius, height);

// Plane
ofDrawPlane(x, y, z, width, height);

// IcoSphere (icosahedral sphere)
ofDrawIcoSphere(x, y, z, radius);
```

### Camera System

```cpp
// Camera setup
ofCamera camera;
camera.setPosition(x, y, z);
camera.lookAt(ofVec3f(0, 0, 0));
camera.setNearClip(1.0f);
camera.setFarClip(10000.0f);
camera.setFov(60.0f);
camera.setAspectRatio(aspect);

// Render with camera
camera.begin();
// ... draw 3D content ...
camera.end();
```

### Lighting System

```cpp
// Point light setup
ofLight light;
light.setPointLight();
light.setPosition(200, 200, 200);
light.setAmbientColor(ofColor(50, 50, 50));
light.setDiffuseColor(ofColor(200, 200, 200));
light.setSpecularColor(ofColor(255, 255, 255));

// Enable lighting
ofEnableLighting();
light.enable();

// Disable lighting
ofDisableLighting();
```

### Material System

```cpp
// Material setup
ofMaterial material;
material.setAmbientColor(ofColor(100, 20, 20));
material.setDiffuseColor(ofColor(255, 50, 50));
material.setSpecularColor(ofColor(255, 255, 255));
material.setShininess(64.0f);

// Apply material
material.begin();
ofDrawSphere(0, 0, 0, 50);
material.end();
```

### Matrix Transformations

```cpp
ofPushMatrix();
ofTranslate(x, y, z);
ofRotateY(angle);
ofRotateX(angle);
ofScale(scale);
// ... draw primitives ...
ofPopMatrix();
```

### Depth and Rendering State

```cpp
// Enable depth testing (required for 3D)
ofEnableDepthTest();

// Fill mode
ofFill();          // Solid rendering
ofNoFill();        // Wireframe rendering

// Line width for wireframe
ofSetLineWidth(2.0f);

// Disable depth test (for UI overlay)
ofDisableDepthTest();
```

## API Functions Used

### 3D Primitives (ofGraphics.h / of3dPrimitives.h)
- `ofDrawBox()` - Draw a cube or rectangular box
- `ofDrawSphere()` - Draw a UV sphere
- `ofDrawCylinder()` - Draw a cylinder
- `ofDrawCone()` - Draw a cone
- `ofDrawPlane()` - Draw a flat plane
- `ofDrawIcoSphere()` - Draw an icosahedral sphere
- `ofDrawGridPlane()` - Draw a reference grid
- `ofDrawLine()` - Draw coordinate axes

### Camera (ofCamera.h)
- `ofCamera::setPosition()` - Set camera position
- `ofCamera::lookAt()` - Point camera at target
- `ofCamera::setNearClip()` / `setFarClip()` - Clipping planes
- `ofCamera::setFov()` - Field of view
- `ofCamera::setAspectRatio()` - Aspect ratio
- `ofCamera::begin()` / `end()` - Activate camera

### Lighting (ofLight.h)
- `ofLight::setPointLight()` - Configure as point light
- `ofLight::setPosition()` - Position light
- `ofLight::setAmbientColor()` - Ambient component
- `ofLight::setDiffuseColor()` - Diffuse component
- `ofLight::setSpecularColor()` - Specular component
- `ofLight::enable()` / `disable()` - Toggle light
- `ofEnableLighting()` / `ofDisableLighting()` - Global lighting toggle

### Material (ofMaterial.h)
- `ofMaterial::setAmbientColor()` - Ambient material color
- `ofMaterial::setDiffuseColor()` - Diffuse material color
- `ofMaterial::setSpecularColor()` - Specular material color
- `ofMaterial::setShininess()` - Specular exponent
- `ofMaterial::begin()` / `end()` - Apply material

### Transformations (ofGraphics.h)
- `ofPushMatrix()` / `ofPopMatrix()` - Save/restore transform
- `ofTranslate()` - Translation
- `ofRotateX()` / `ofRotateY()` / `ofRotateZ()` - Rotation
- `ofScale()` - Scaling

### Rendering State (ofGraphics.h)
- `ofEnableDepthTest()` / `ofDisableDepthTest()` - Depth testing
- `ofFill()` / `ofNoFill()` - Fill mode
- `ofSetLineWidth()` - Line width for wireframes
- `ofBackground()` - Clear color
- `ofSetColor()` - Current color

### Utility (ofUtils.h)
- `ofSetFrameRate()` - Target frame rate
- `ofGetFrameRate()` - Current FPS
- `ofToString()` - Number to string conversion
- `ofSetWindowTitle()` - Window title
- `ofLogNotice()` - Console logging
- `ofDrawBitmapString()` - Text overlay

## Expected Output

When running this example, you should see:

1. **Six 3D primitives** arranged in a grid, each with a unique color and material
2. **Smooth rotation animation** of all primitives (toggleable with SPACE)
3. **Phong lighting** with visible specular highlights (toggle with L)
4. **Coordinate axes** (RGB = XYZ) at the origin
5. **Ground grid** for spatial reference
6. **Interactive camera** that rotates around the scene with mouse drag
7. **Zoom control** with mouse scroll wheel
8. **Wireframe mode** showing mesh topology (toggle with W)
9. **UI overlay** showing FPS, camera info, and controls

### Console Output

```
06_3d_primitives: Setup complete
Controls:
  SPACE: Toggle rotation
  L: Toggle lighting
  W: Toggle wireframe
  Mouse drag: Rotate camera
```

## Integration Instructions

To integrate this example into your oflike-metal project:

1. Copy `main.cpp` to your examples directory
2. Update `SwiftBridge.mm` to use `Primitives3DApp`:
   ```objc
   static Primitives3DApp app;
   ```
3. Ensure the following headers are available:
   - `oflike/graphics/ofGraphics.h`
   - `oflike/3d/of3dPrimitives.h`
   - `oflike/3d/ofCamera.h`
   - `oflike/lighting/ofLight.h`
   - `oflike/lighting/ofMaterial.h`
4. Add to `examples/CMakeLists.txt`:
   ```cmake
   add_oflike_example(06_3d_primitives)
   ```

## Learning Points

1. **3D Rendering Pipeline**: Understanding the camera → transform → lighting → draw pipeline
2. **Camera Control**: Implementing orbiting camera with mouse interaction
3. **Lighting and Materials**: Using Phong shading for realistic surfaces
4. **Matrix Stack**: Using push/pop matrix for hierarchical transformations
5. **Depth Testing**: Proper 3D occlusion with depth buffer
6. **Wireframe Rendering**: Visualizing mesh topology
7. **Interactive Controls**: Keyboard and mouse event handling
8. **Performance**: Maintaining 60 FPS with multiple lit primitives

## Architecture Compliance

✅ **Pure C++ Implementation**: Uses only openFrameworks-style API
✅ **Metal Backend**: All rendering goes through Metal (transparent to user)
✅ **No OpenGL**: No GL calls, all Metal under the hood
✅ **No Prohibited Libraries**: Uses only native frameworks
✅ **ofBaseApp Pattern**: Standard setup/update/draw lifecycle
✅ **SIMD Math**: Camera and lighting calculations use Metal SIMD types internally

## Performance Notes

- **Primitive Generation**: Meshes are generated once and cached
- **Phong Lighting**: Calculated per-fragment in Metal shader
- **Depth Testing**: Hardware depth buffer on GPU
- **Matrix Operations**: Use SIMD acceleration via Metal
- **Material Changes**: Efficient uniform buffer updates
- **Target FPS**: 60 FPS with all 6 primitives + lighting

## Related Examples

- **01_basics**: Basic 2D drawing concepts
- **02_shapes**: 2D shape primitives
- **07_camera**: Advanced camera controls (EasyCam)
- **08_lighting**: Multiple lights and advanced materials
- **09_mesh**: Custom mesh creation and manipulation

## Next Steps

After mastering this example, explore:
- Creating custom meshes with `ofMesh`
- Using multiple lights simultaneously
- Loading 3D models from files (.obj, .ply)
- Implementing custom shaders
- Creating complex 3D scenes with hierarchies
- Adding textures to 3D primitives
