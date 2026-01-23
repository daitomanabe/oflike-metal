# Camera API

## Overview

Camera classes for 3D rendering with view and projection matrices.

## Headers

```cpp
#include <oflike/3d/ofCamera.h>
#include <oflike/3d/ofEasyCam.h>
```

---

## ofCamera - 3D Camera

```cpp
ofCamera camera;

// Position & orientation
camera.setPosition(0, 0, 500);
camera.lookAt(ofVec3f(0, 0, 0));
camera.setUpAxis(ofVec3f(0, 1, 0));

// Projection
camera.setupPerspective();               // Default FOV, aspect
camera.setupPerspective(
    true,        // vertical FOV
    60.0f,       // FOV in degrees
    0.1f,        // near plane
    1000.0f      // far plane
);

// Orthographic
camera.enableOrtho();
camera.disableOrtho();

// Apply camera (setup view/proj matrices)
camera.begin();
    // Draw 3D scene here
camera.end();

// Matrices
ofMatrix4x4 view = camera.getModelViewMatrix();
ofMatrix4x4 proj = camera.getProjectionMatrix();

// Properties
ofVec3f pos = camera.getPosition();
ofVec3f side = camera.getSideDir();
ofVec3f up = camera.getUpDir();
ofVec3f look = camera.getLookAtDir();
float fov = camera.getFov();
float aspect = camera.getAspectRatio();
```

---

## ofEasyCam - Interactive Camera

Mouse-controlled camera with orbit, pan, and zoom:

```cpp
ofEasyCam easyCam;

void setup() {
    easyCam.setDistance(500);         // Distance from target
    easyCam.setTarget(ofVec3f(0, 0, 0));
    easyCam.setNearClip(0.1f);
    easyCam.setFarClip(10000.0f);
}

void draw() {
    easyCam.begin();
        // Draw 3D scene
        ofDrawBox(100);
    easyCam.end();
}

// Mouse controls are automatic:
// - Left drag: rotate (orbit)
// - Right drag: pan
// - Scroll: zoom
// - Middle drag: pan

// Manual control
easyCam.rotate(angle, axis);
easyCam.pan(dx, dy);
easyCam.dolly(distance);
easyCam.reset();

// Enable/disable mouse
easyCam.enableMouseInput();
easyCam.disableMouseInput();
```

---

## Example: Multiple Cameras

```cpp
class MyApp : public ofBaseApp {
    ofCamera mainCam;
    ofCamera topCam;
    bool useTopView = false;

    void setup() override {
        mainCam.setPosition(0, 100, 500);
        mainCam.lookAt(ofVec3f(0, 0, 0));

        topCam.setPosition(0, 1000, 0);
        topCam.lookAt(ofVec3f(0, 0, 0));
    }

    void draw() override {
        ofBackground(0);

        ofCamera& cam = useTopView ? topCam : mainCam;

        cam.begin();
            // Draw grid
            for (int i = -10; i <= 10; i++) {
                ofDrawLine(-1000, 0, i * 100,
                            1000, 0, i * 100);
                ofDrawLine(i * 100, 0, -1000,
                           i * 100, 0, 1000);
            }

            // Draw objects
            ofSetColor(255, 100, 100);
            ofDrawBox(0, 50, 0, 100, 100, 100);

            ofSetColor(100, 255, 100);
            ofDrawSphere(200, 50, 0, 50);
        cam.end();
    }

    void keyPressed(int key) override {
        if (key == ' ') {
            useTopView = !useTopView;
        }
    }
};
```

---

## Coordinate System

- **Right-handed**: X right, Y up, Z toward viewer
- **Camera default**: Looks down -Z axis
- **World coordinates**: Transformed by view matrix

---

## See Also

- [01_math.md](01_math.md) - Matrix operations
- [03_graphics_3d.md](03_graphics_3d.md) - 3D drawing
- [07_lighting.md](07_lighting.md) - Lighting
