# Lighting API

## Overview

Lighting system with multiple light types and material properties.

## Headers

```cpp
#include <oflike/lighting/ofLight.h>
#include <oflike/lighting/ofMaterial.h>
```

---

## ofLight - Light Source

```cpp
ofLight light;

// Enable/disable
light.enable();
light.disable();
bool enabled = light.getIsEnabled();

// Position (for point/spot lights)
light.setPosition(x, y, z);
light.setPosition(ofVec3f(x, y, z));
ofVec3f pos = light.getPosition();

// Direction (for directional/spot lights)
light.setOrientation(ofVec3f(dx, dy, dz));
ofVec3f dir = light.getOrientation();

// Light type
light.setPointLight();         // Omni-directional from point
light.setDirectionalLight();   // Parallel rays (sun-like)
light.setSpotLight();          // Cone of light

// Color
light.setDiffuseColor(ofColor(255, 255, 255));
light.setSpecularColor(ofColor(255, 255, 255));
light.setAmbientColor(ofColor(50, 50, 50));

ofColor diffuse = light.getDiffuseColor();
ofColor specular = light.getSpecularColor();
ofColor ambient = light.getAmbientColor();

// Attenuation (how light fades with distance)
light.setAttenuation(
    constant,   // Constant attenuation (default: 1.0)
    linear,     // Linear attenuation (default: 0.0)
    quadratic   // Quadratic attenuation (default: 0.0)
);

// Spot light parameters
light.setSpotlightCutOff(angle);      // Cone angle in degrees
light.setSpotConcentration(exponent); // Focus (0-128)
```

---

## ofMaterial - Surface Material

```cpp
ofMaterial material;

// Colors
material.setDiffuseColor(ofColor(200, 100, 100));   // Surface color
material.setSpecularColor(ofColor(255, 255, 255));  // Highlight color
material.setAmbientColor(ofColor(50, 50, 50));      // Ambient contribution
material.setEmissiveColor(ofColor(0, 0, 0));        // Self-illumination

// Shininess
material.setShininess(64.0f);  // 0-128, higher = tighter highlights

// Apply material
material.begin();
    // Draw objects with this material
    ofDrawSphere(100);
material.end();
```

---

## Example: Lit Scene

```cpp
class MyApp : public ofBaseApp {
    ofLight light1;
    ofLight light2;
    ofMaterial redMaterial;
    ofMaterial blueMaterial;
    ofEasyCam camera;

    void setup() override {
        // Setup lights
        light1.setPointLight();
        light1.setPosition(200, 200, 200);
        light1.setDiffuseColor(ofColor(255, 255, 200));
        light1.enable();

        light2.setDirectionalLight();
        light2.setOrientation(ofVec3f(-1, -1, -1));
        light2.setDiffuseColor(ofColor(100, 100, 255));
        light2.enable();

        // Setup materials
        redMaterial.setDiffuseColor(ofColor(200, 50, 50));
        redMaterial.setSpecularColor(ofColor(255, 255, 255));
        redMaterial.setShininess(64);

        blueMaterial.setDiffuseColor(ofColor(50, 50, 200));
        blueMaterial.setSpecularColor(ofColor(255, 255, 255));
        blueMaterial.setShininess(32);

        camera.setDistance(500);
    }

    void update() override {
        // Animate light
        float t = ofGetElapsedTimef();
        light1.setPosition(
            cos(t) * 300,
            200,
            sin(t) * 300
        );
    }

    void draw() override {
        ofBackground(20);
        ofEnableDepthTest();

        camera.begin();

            // Draw red sphere
            redMaterial.begin();
                ofDrawSphere(-150, 0, 0, 80);
            redMaterial.end();

            // Draw blue box
            blueMaterial.begin();
                ofDrawBox(150, 0, 0, 120);
            blueMaterial.end();

            // Draw floor (no material = default white)
            ofSetColor(150);
            ofDrawPlane(0, -100, 0, 1000, 1000);

        camera.end();

        ofDisableDepthTest();
    }
};
```

---

## Lighting System

### Global Control

```cpp
ofEnableLighting();          // Enable lighting globally
ofDisableLighting();         // Disable lighting globally

ofEnableDepthTest();         // Enable depth testing (3D)
ofDisableDepthTest();        // Disable depth testing
```

### Multiple Lights

Up to 8 lights can be active simultaneously (hardware limit). Lights are automatically assigned to light slots when enabled.

---

## Lighting Model

- **Phong shading**: Per-fragment lighting
- **Components**: Ambient + Diffuse + Specular
- **Normals**: Auto-generated for primitives, manual for meshes

---

## Performance Tips

- **Fewer lights**: Use 1-3 lights for best performance
- **Directional lights**: Fastest (no distance calculation)
- **Point lights**: More expensive (distance-based attenuation)
- **Spot lights**: Most expensive (angle + distance calculations)

---

## See Also

- [03_graphics_3d.md](03_graphics_3d.md) - 3D drawing
- [06_camera.md](06_camera.md) - Camera setup
- [04_color.md](04_color.md) - Color classes
