# 08_lighting - Comprehensive Lighting System

Demonstrates the full lighting system in oflike-metal with multiple light types, materials, and Phong shading.

## Visual Features

This example showcases:

### 1. **Three Light Types**
   - **Point Light (Red-Orange)**: Orbits around the scene with vertical bobbing motion
   - **Directional Light (Blue)**: Simulates sunlight from above with rotating direction
   - **Spotlight (Green)**: Follows a circular path, always pointing at the center

### 2. **Three Material Presets**
   - **Metal (Shiny)**: High specular reflection, high shininess (128)
   - **Plastic (Matte)**: Low specular reflection, low shininess (32)
   - **Paint (Glossy)**: Medium specular reflection, medium shininess (64)

### 3. **Scene Elements**
   - Center sphere with current material
   - Ground plane showing lighting across a large surface
   - 4 corner boxes, each with different material presets
   - 8 cylinders arranged in a circle
   - Light position indicators (small colored spheres)
   - Spotlight direction visualization (green line)

### 4. **Lighting Effects**
   - **Ambient**: Base illumination from all directions
   - **Diffuse**: Direction-dependent surface color
   - **Specular**: Shiny highlights based on view angle
   - **Attenuation**: Light falloff for point/spot lights
   - **Phong Shading**: Smooth per-pixel lighting calculation

## Controls

### Light Toggles
- **1**: Toggle Point Light (Red-Orange, orbiting)
- **2**: Toggle Directional Light (Blue, from above)
- **3**: Toggle Spotlight (Green, from front)

### Light Intensities
- **Q/W**: Decrease/Increase Light 1 intensity (0.0 - 2.0)
- **A/S**: Decrease/Increase Light 2 intensity (0.0 - 2.0)
- **Z/X**: Decrease/Increase Light 3 intensity (0.0 - 2.0)

### Material Control
- **4**: Metal preset (shiny, high shininess)
- **5**: Plastic preset (matte, low shininess)
- **6**: Paint preset (glossy, medium shininess)
- **UP/DOWN**: Adjust shininess (1 - 256)

### System Control
- **SPACE**: Toggle animation on/off
- **L**: Toggle lighting system on/off
- **R**: Reset all settings to defaults

## API Usage

This example demonstrates the following oflike-metal APIs:

### ofLight
```cpp
// Three types of lights
ofLight light;
light.setPointLight();       // Emits in all directions from a position
light.setDirectional();       // Parallel rays like sunlight
light.setSpotlight();        // Cone of light with direction

// Position and direction
light.setPosition(x, y, z);
ofVec3f pos = light.getPosition();

// Colors (Phong model)
light.setAmbientColor(color);    // Base illumination
light.setDiffuseColor(color);    // Main surface color
light.setSpecularColor(color);   // Shiny highlights

// Attenuation (point/spot lights only)
light.setAttenuation(constant, linear, quadratic);
// falloff = 1.0 / (constant + linear*d + quadratic*d²)

// Spotlight parameters
light.setSpotlightCutOff(degrees);      // Cone angle (0-90)
light.setSpotConcentration(exponent);   // Focus (0-128)

// Enable/disable
light.enable();
light.disable();
```

### ofMaterial
```cpp
ofMaterial material;

// Phong material properties
material.setAmbientColor(color);     // Ambient reflection
material.setDiffuseColor(color);     // Diffuse reflection
material.setSpecularColor(color);    // Specular reflection
material.setEmissiveColor(color);    // Self-illumination (not in this example)
material.setShininess(value);        // Specular exponent (1-256)

// Apply material to geometry
material.begin();
ofDrawSphere(x, y, z, radius);
material.end();
```

### Global Lighting System
```cpp
// Enable/disable lighting calculations
ofEnableLighting();
ofDisableLighting();

// Smooth shading (Phong) - always enabled in oflike-metal
ofSetSmoothLighting(true);
```

### 3D Rendering
```cpp
// Depth testing required for correct 3D rendering
ofEnableDepthTest();
ofDisableDepthTest();

// Draw 3D primitives with lighting
ofDrawSphere(x, y, z, radius);
ofDrawBox(x, y, z, size);
ofDrawCylinder(x, y, z, radius, height);
ofDrawPlane(x, y, width, height);
```

### Camera
```cpp
ofCamera camera;
camera.setPosition(x, y, z);
camera.lookAt(target);
camera.begin();
// ... draw 3D scene ...
camera.end();
```

## Expected Output

When you run this example, you should see:

1. A 3D scene with:
   - Center sphere with selected material
   - Ground plane showing light distribution
   - 4 rotating boxes with different materials
   - 8 cylinders in a circle

2. Three colored lights:
   - Red-orange point light orbiting
   - Blue directional light from above
   - Green spotlight following a path

3. Light indicators:
   - Small colored spheres at light positions
   - Green line showing spotlight direction

4. On-screen UI showing:
   - Current light states and intensities
   - Material selection and shininess
   - Animation and lighting status
   - FPS counter

5. Console output:
   - Control confirmations
   - Light/material state changes

## Integration

To integrate this example into the oflike-metal build:

```cmake
# In examples/CMakeLists.txt
add_oflike_example(08_lighting)
```

Or manually:
```bash
cd examples/08_lighting
# Compile with oflike-metal library
clang++ -std=c++20 main.cpp -o 08_lighting \
  -I../../src \
  -L../../build \
  -loflike-metal \
  -framework Metal -framework MetalKit -framework SwiftUI
```

## Learning Points

### 1. **Light Types**
   - **Point lights**: Illuminate from a single point in all directions (e.g., light bulb)
   - **Directional lights**: Parallel rays, position doesn't matter (e.g., sun)
   - **Spotlights**: Cone of light with position and direction (e.g., flashlight)

### 2. **Phong Lighting Model**
   ```
   Final Color = Ambient + Diffuse + Specular

   Ambient   = Ka × Light_Ambient
   Diffuse   = Kd × Light_Diffuse × max(0, N·L)
   Specular  = Ks × Light_Specular × max(0, R·V)^shininess

   Where:
   Ka, Kd, Ks = material colors
   N = surface normal
   L = light direction
   R = reflection vector
   V = view direction
   ```

### 3. **Material Properties**
   - **Shininess**: Higher = smaller, brighter highlights (1-256)
   - **Specular color**: Determines highlight color (often white for physical materials)
   - **Diffuse color**: Main surface color under direct light
   - **Ambient color**: Surface color in shadow (often darker version of diffuse)

### 4. **Light Attenuation**
   ```
   Attenuation = 1.0 / (constant + linear×distance + quadratic×distance²)

   Typical values:
   - No falloff: (1.0, 0.0, 0.0)
   - Linear: (1.0, 0.1, 0.0)
   - Realistic: (1.0, 0.07, 0.017) for ~50 unit range
   ```

### 5. **Spotlight Parameters**
   - **CutOff angle**: Cone aperture (25° is typical for focused light)
   - **Concentration**: Focus/falloff (higher = more focused beam)

### 6. **Performance Considerations**
   - oflike-metal supports up to 8 active lights simultaneously
   - Phong shading is per-pixel (high quality, moderate cost)
   - Light calculations are done in Metal shaders on GPU
   - Disabling unused lights improves performance

### 7. **Multiple Light Interaction**
   - Lights are additive (combined in the shader)
   - Each light contributes ambient, diffuse, and specular
   - Material properties are applied to each light
   - Total illumination = sum of all enabled lights

## Architecture

### Metal Backend
- Lighting calculations in `Lighting.metal` shader
- Phong shading per-pixel
- Up to 8 lights in uniform buffer
- Material properties as shader parameters

### ARCHITECTURE.md Compliance
- ✅ No OpenGL (Metal only)
- ✅ No prohibited libraries
- ✅ Pure C++ application code
- ✅ openFrameworks API compatibility (Level A)
- ✅ Metal shaders for GPU lighting
- ✅ Phong shading model

### Coordinate System
- Right-handed 3D coordinate system
- Camera looks down -Z axis
- Y is up
- Light positions in world space

## Troubleshooting

### Lights not visible
- Ensure `ofEnableLighting()` is called before drawing
- Check that lights are enabled with `light.enable()`
- Verify `ofEnableDepthTest()` is enabled
- Make sure camera is inside the scene

### Dark or black objects
- Check material diffuse color is not black
- Verify at least one light is enabled
- Ensure light intensities are > 0
- Check ambient color is not zero on all lights

### No specular highlights
- Increase material shininess (try 64-128)
- Set specular color to white or light color
- Check camera view angle (highlights are view-dependent)
- Verify light specular color is not black

### Performance issues
- Reduce number of active lights (disable unused)
- Lower scene complexity
- Check FPS counter in UI

## Console Output

Expected log messages:
```
[notice ] 08_lighting: === Controls ===
[notice ] 08_lighting: 1-3: Toggle lights (point, directional, spot)
[notice ] 08_lighting: Q/W: Light 1 intensity
...
[notice ] 08_lighting: Light 1 (Point): ON
[notice ] 08_lighting: Material: Metal (Shiny)
[notice ] 08_lighting: Shininess: 128.0
```

## References

- Phong reflection model: https://en.wikipedia.org/wiki/Phong_reflection_model
- Light attenuation: https://learnopengl.com/Lighting/Light-casters
- Metal shading language: https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf
- openFrameworks lighting: https://openframeworks.cc/documentation/3d/ofLight/
