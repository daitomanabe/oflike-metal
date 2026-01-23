# 3D Graphics API

## Overview

3D drawing functions for creating 3D primitives, meshes, and scenes. Uses Metal for hardware-accelerated rendering with right-handed coordinate system.

## Header

```cpp
#include <oflike/graphics/ofGraphics.h>
#include <oflike/3d/ofMesh.h>
```

---

## 3D Primitives

### Box

```cpp
ofDrawBox(100);                      // Cube at origin, size 100
ofDrawBox(x, y, z, 100);             // Cube at position, size 100
ofDrawBox(x, y, z, w, h, d);         // Box with dimensions
```

### Sphere

```cpp
ofDrawSphere(50);                    // Sphere at origin, radius 50
ofDrawSphere(x, y, z, 50);           // Sphere at position, radius 50

ofSetSphereResolution(32);           // Set tessellation (default: 20)
```

### Cone

```cpp
ofDrawCone(radius, height);          // Cone at origin
ofDrawCone(x, y, z, radius, height); // Cone at position
```

### Cylinder

```cpp
ofDrawCylinder(radius, height);          // Cylinder at origin
ofDrawCylinder(x, y, z, radius, height); // Cylinder at position
```

### Plane

```cpp
ofDrawPlane(width, height);          // Plane at origin
ofDrawPlane(x, y, z, width, height); // Plane at position
```

### Icosphere

```cpp
ofDrawIcoSphere(radius);             // Icosphere at origin
ofDrawIcoSphere(x, y, z, radius);    // Icosphere at position
ofDrawIcoSphere(radius, 3);          // Higher subdivision (smoother)
```

### Axis (Debug Helper)

```cpp
ofDrawAxis(100);                     // Draw XYZ axes, length 100
```

---

## ofMesh - Custom 3D Geometry

Build custom 3D geometry vertex-by-vertex:

```cpp
ofMesh mesh;

// Add vertices
mesh.addVertex(ofVec3f(0, 0, 0));
mesh.addVertex(ofVec3f(100, 0, 0));
mesh.addVertex(ofVec3f(100, 100, 0));

// Add colors (per-vertex)
mesh.addColor(ofColor(255, 0, 0));
mesh.addColor(ofColor(0, 255, 0));
mesh.addColor(ofColor(0, 0, 255));

// Add normals (for lighting)
mesh.addNormal(ofVec3f(0, 0, 1));
mesh.addNormal(ofVec3f(0, 0, 1));
mesh.addNormal(ofVec3f(0, 0, 1));

// Add texture coordinates
mesh.addTexCoord(ofVec2f(0, 0));
mesh.addTexCoord(ofVec2f(1, 0));
mesh.addTexCoord(ofVec2f(1, 1));

// Add indices (for indexed drawing)
mesh.addIndex(0);
mesh.addIndex(1);
mesh.addIndex(2);

// Set draw mode
mesh.setMode(OF_PRIMITIVE_TRIANGLES);  // or LINES, POINTS, TRIANGLE_STRIP, etc.

// Draw the mesh
mesh.draw();
```

### Mesh Properties

```cpp
size_t numVerts = mesh.getNumVertices();
size_t numIndices = mesh.getNumIndices();

ofVec3f v = mesh.getVertex(0);
ofColor c = mesh.getColor(0);
ofVec3f n = mesh.getNormal(0);

mesh.clearVertices();
mesh.clearColors();
mesh.clearNormals();
mesh.clearTexCoords();
mesh.clearIndices();
mesh.clear();  // Clear everything
```

---

## 3D Transformations

Use the same matrix stack as 2D:

```cpp
ofPushMatrix();
    ofTranslate(0, 0, -500);     // Move away from camera
    ofRotateX(30);                // Tilt down
    ofRotateY(angle);             // Spin around Y
    ofDrawBox(100);
ofPopMatrix();
```

---

## Example: 3D Scene

```cpp
class MyApp : public ofBaseApp {
    float angle = 0;

    void setup() override {
        ofSetSphereResolution(32);
    }

    void update() override {
        angle += 0.5f;
    }

    void draw() override {
        ofBackground(0);

        // Setup 3D camera
        ofPushMatrix();
        ofTranslate(ctx().getWindowWidth()/2,
                    ctx().getWindowHeight()/2,
                    -500);

        // Rotating cube
        ofPushMatrix();
            ofRotateY(angle);
            ofRotateX(angle * 0.3f);
            ofSetColor(255, 100, 100);
            ofDrawBox(150);
        ofPopMatrix();

        // Sphere
        ofPushMatrix();
            ofTranslate(300, 0, 0);
            ofSetColor(100, 255, 100);
            ofDrawSphere(80);
        ofPopMatrix();

        // Cone
        ofPushMatrix();
            ofTranslate(-300, 0, 0);
            ofSetColor(100, 100, 255);
            ofDrawCone(60, 150);
        ofPopMatrix();

        ofPopMatrix();
    }
};
```

---

## Coordinate System

- **Right-handed**: X right, Y up, Z toward viewer
- **Camera**: Looks down -Z axis by default
- **Rotation**: Counter-clockwise is positive (right-hand rule)

---

## See Also

- [01_math.md](01_math.md) - Vectors and matrices
- [06_camera.md](06_camera.md) - Camera classes
- [07_lighting.md](07_lighting.md) - Lighting and materials
- [02_graphics_2d.md](02_graphics_2d.md) - 2D drawing
