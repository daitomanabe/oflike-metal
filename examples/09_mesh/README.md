# 09_mesh - ofMesh API Demonstration

Comprehensive demonstration of the ofMesh API in oflike-metal, showcasing vertices, normals, texcoords, colors, indices, primitive modes, procedural mesh generation, mesh manipulation, and lighting integration.

## Features

### 1. Custom Triangle Mesh (Mode 1)
- **Manual mesh construction**: Vertices, colors, normals added one by one
- **Indexed rendering**: Uses `addTriangle()` for efficient vertex reuse
- **Per-vertex colors**: Each vertex has its own color (red, green, blue, yellow, magenta)
- **Pyramid geometry**: 5 vertices, 12 indices (4 side faces + 2 base triangles)
- **Demonstrates**:
  - `addVertex()`, `addColor()`, `addNormal()`
  - `addTriangle(i1, i2, i3)` for indexed triangles
  - `setMode(OF_PRIMITIVE_TRIANGLES)`

### 2. Procedural Sphere (Mode 2)
- **UV Sphere**: Generated using `ofMesh::sphere(radius, resolution)`
- **Resolution control**: UP/DOWN keys adjust subdivision count (8-60 segments)
- **Smooth normals**: Automatically generated for proper lighting
- **Latitude/longitude topology**: Standard UV sphere parameterization
- **Demonstrates**:
  - Static mesh generator `ofMesh::sphere()`
  - Smooth shading with normals
  - Adjustable mesh resolution

### 3. Procedural Box (Mode 3)
- **Cube mesh**: Generated using `ofMesh::box(width, height, depth)`
- **6 faces**: Each face is two triangles (12 triangles total)
- **Proper normals**: Face normals for each quad
- **Demonstrates**:
  - Static mesh generator `ofMesh::box()`
  - Hard-edged geometry with per-face normals

### 4. Procedural Plane (Mode 4)
- **Subdivided plane**: Generated using `ofMesh::plane(width, height, columns, rows)`
- **Dynamic subdivision**: Resolution affects tessellation density
- **Grid topology**: Vertices arranged in rows and columns
- **Demonstrates**:
  - Static mesh generator `ofMesh::plane()`
  - Mesh subdivision for terrain/surface simulation
  - Indexed rendering with quads (as triangles)

### 5. Line Strip Mesh (Mode 5)
- **Spiral line**: Custom line strip with 100 vertices
- **Primitive mode**: `OF_PRIMITIVE_LINE_STRIP`
- **Gradient colors**: Smooth color transition from cyan to magenta
- **Animated thickness**: Line tapers as it spirals upward
- **Demonstrates**:
  - `setMode(OF_PRIMITIVE_LINE_STRIP)` for continuous lines
  - Per-vertex color gradients
  - Line-based rendering (no triangles)

### 6. Points Mesh (Mode 6)
- **Point cloud**: 1000 vertices rendered as points
- **Primitive mode**: `OF_PRIMITIVE_POINTS`
- **Spherical distribution**: Random points in a sphere volume
- **Color coding**: HSB colors based on spherical coordinates (phi, radius)
- **Demonstrates**:
  - `setMode(OF_PRIMITIVE_POINTS)` for point rendering
  - Point-based visualization
  - Large vertex counts for particle effects

### 7. Procedural Icosphere (Mode 7)
- **Geodesic sphere**: Generated using `ofMesh::icosphere(radius, subdivisions)`
- **Uniform triangles**: Better triangle distribution than UV sphere
- **Subdivision levels**: 2 subdivisions for balance of quality and performance
- **Demonstrates**:
  - Static mesh generator `ofMesh::icosphere()`
  - Alternative sphere topology with uniform triangles
  - Good for physics simulations and smooth surfaces

## Visual Elements

### Scene Layout
- **Center**: Current mesh with rotation animation
- **Camera**: Orbits around the mesh automatically
- **Lighting**: Single point light orbiting the scene
- **Material**: Blue-ish material with specular highlights
- **Background**: Dark gray (30, 30, 35)

### Animation
- **Mesh rotation**: Y-axis rotation (30°/sec) + X-axis oscillation (±20°)
- **Camera orbit**: Circular path around mesh (radius 10, height 4 + sine wave)
- **Light orbit**: Point light circles the scene (radius 8, height 6)
- **Toggle**: Press SPACE to pause/resume animation

### Rendering Modes
- **Solid mode** (default): Normal mesh rendering with lighting
- **Wireframe mode** (W key): Shows edge structure without faces
- **Normals display** (N key): Yellow lines showing normal vectors (0.3 units length)

## API Functions Used

### ofMesh - Vertex Data Management
- `addVertex(const ofVec3f& v)` - Add a vertex position
- `addNormal(const ofVec3f& n)` - Add a normal vector
- `addTexCoord(const ofVec2f& t)` - Add a texture coordinate
- `addColor(const ofColor& c)` - Add a vertex color

### ofMesh - Index Management
- `addIndex(uint32_t i)` - Add an index to the index buffer
- `addTriangle(uint32_t i1, uint32_t i2, uint32_t i3)` - Add a triangle (3 indices)

### ofMesh - Primitive Mode
- `setMode(ofPrimitiveMode mode)` - Set the primitive rendering mode
- `getMode()` - Get the current primitive mode

### ofMesh - Drawing
- `draw()` - Draw the mesh with current mode
- `drawWireframe()` - Draw only the edges (wireframe)
- `drawVertices()` - Draw only the vertices (as points)
- `drawFaces()` - Draw only the faces (triangles)

### ofMesh - Data Access
- `getVertices()` - Get reference to vertex vector
- `getNormals()` - Get reference to normal vector
- `getTexCoords()` - Get reference to texcoord vector
- `getColors()` - Get reference to color vector
- `getIndices()` - Get reference to index vector

### ofMesh - Utility
- `clear()` - Clear all mesh data
- `getNumVertices()` - Get vertex count
- `getNumIndices()` - Get index count
- `hasIndices()` - Check if mesh uses indexed rendering
- `hasNormals()` - Check if normals are present
- `hasColors()` - Check if colors are present

### ofMesh - Static Mesh Generators
- `ofMesh::plane(float width, float height, int columns, int rows)` - Create a plane
- `ofMesh::sphere(float radius, int resolution)` - Create a UV sphere
- `ofMesh::box(float width, float height, float depth)` - Create a box
- `ofMesh::cone(float radius, float height, int radiusSegments, int heightSegments)` - Create a cone
- `ofMesh::cylinder(float radius, float height, int radiusSegments, int heightSegments)` - Create a cylinder
- `ofMesh::icosphere(float radius, int subdivisions)` - Create a geodesic sphere

### Primitive Modes
- `OF_PRIMITIVE_TRIANGLES` - Independent triangles (3 vertices each)
- `OF_PRIMITIVE_TRIANGLE_STRIP` - Connected triangles sharing edges
- `OF_PRIMITIVE_TRIANGLE_FAN` - Triangles radiating from first vertex
- `OF_PRIMITIVE_LINES` - Independent line segments (2 vertices each)
- `OF_PRIMITIVE_LINE_STRIP` - Connected line segments
- `OF_PRIMITIVE_LINE_LOOP` - Connected line segments forming a loop
- `OF_PRIMITIVE_POINTS` - Individual points

### Other API Functions
- `ofCamera` - Camera setup, positioning, look-at, projection
- `ofLight::setPointLight()` - Point light source
- `ofMaterial` - Material properties (diffuse, specular, ambient, shininess)
- `ofEnableLighting()` / `ofDisableLighting()` - Toggle lighting system
- `ofEnableDepthTest()` - Enable depth buffer for 3D rendering
- `ofPushMatrix()` / `ofPopMatrix()` - Matrix stack management
- `ofRotateY()`, `ofRotateX()` - Rotation transformations
- `ofDrawLine()` - Draw lines (for normal visualization)
- `ofSetColor()` - Set drawing color
- `ofSetLineWidth()` - Set line width
- `ofLogNotice()` - Console logging

## Controls

| Key | Action |
|-----|--------|
| `1-7` | Switch mesh modes (1=Pyramid, 2=Sphere, 3=Box, 4=Plane, 5=Spiral, 6=Points, 7=Icosphere) |
| `SPACE` | Toggle animation (pause/resume) |
| `W` | Toggle wireframe mode (on/off) |
| `N` | Toggle normal display (show/hide yellow normal lines) |
| `L` | Toggle lighting system (on/off) |
| `R` | Reset camera and animation |
| `UP` | Increase mesh resolution (+4, max 60) |
| `DOWN` | Decrease mesh resolution (-4, min 8) |

## Usage

### Integration into Your Project
```cpp
#include "ofMain.h"
using namespace oflike;

class MyApp : public ofBaseApp {
public:
    void setup() override {
        // Create a procedural sphere
        myMesh = ofMesh::sphere(2.0f, 20);

        // Or build a custom mesh
        myMesh.clear();
        myMesh.setMode(OF_PRIMITIVE_TRIANGLES);
        myMesh.addVertex(ofVec3f(0, 1, 0));
        myMesh.addVertex(ofVec3f(-1, -1, 0));
        myMesh.addVertex(ofVec3f(1, -1, 0));
        myMesh.addColor(ofColor(255, 0, 0));
        myMesh.addColor(ofColor(0, 255, 0));
        myMesh.addColor(ofColor(0, 0, 255));
        myMesh.addTriangle(0, 1, 2);
    }

    void draw() override {
        camera.begin();
        myMesh.draw();
        camera.end();
    }

private:
    ofMesh myMesh;
    ofCamera camera;
};
```

### Build Instructions
```bash
# From project root
mkdir -p build && cd build
cmake ..
make 09_mesh

# Run the example
./examples/09_mesh/09_mesh
```

## Expected Output

### Console Log
```
[notice ] 09_mesh: Mesh demonstration started
[notice ] Controls: 1-7: Switch mesh modes, SPACE: Toggle animation
[notice ] Controls: W: Wireframe, N: Show normals, L: Lighting, UP/DOWN: Resolution
[notice ] buildCustomTriangleMesh: Pyramid: 5 vertices, 12 indices
[notice ] buildLineStripMesh: Spiral: 100 vertices
[notice ] buildPointsMesh: Point cloud: 1000 points
[notice ] buildMeshes: Built 7 mesh modes
[notice ] Mode 1: Custom Triangle Mesh (Pyramid with per-vertex colors)
[notice ] : Vertices: 5, Indices: 12, Primitive: TRIANGLES
```

### Visual Output
- Rotating 3D mesh in the center of the screen
- Smooth lighting with specular highlights
- Orbiting camera and point light
- On-screen UI showing mesh statistics:
  - Current mode description
  - Vertex count, index count
  - Primitive mode (TRIANGLES, LINE_STRIP, POINTS, etc.)
  - Normals and colors presence
  - Animation, wireframe, normals, lighting state
  - Current resolution
  - FPS counter

## Learning Points

1. **Mesh Construction**:
   - Manual vertex-by-vertex construction with `addVertex()`, `addColor()`, `addNormal()`
   - Indexed rendering with `addTriangle()` for vertex reuse and efficiency
   - Non-indexed rendering (direct vertex list)

2. **Procedural Generation**:
   - Static mesh generators: `sphere()`, `box()`, `plane()`, `icosphere()`
   - Resolution control for quality vs performance tradeoffs
   - Different topologies: UV sphere (latitude/longitude) vs icosphere (geodesic)

3. **Primitive Modes**:
   - `OF_PRIMITIVE_TRIANGLES`: Standard 3D geometry
   - `OF_PRIMITIVE_LINE_STRIP`: Continuous lines without triangles
   - `OF_PRIMITIVE_POINTS`: Point clouds and particle effects
   - Each mode interprets vertex data differently

4. **Mesh Data**:
   - Vertices: 3D positions (`ofVec3f`)
   - Normals: Surface orientation for lighting (`ofVec3f`, unit length)
   - Colors: Per-vertex colors (`ofColor`, RGBA)
   - Indices: Vertex reuse for memory efficiency (`uint32_t`)

5. **Lighting Integration**:
   - Normals are essential for lighting calculations
   - Materials control surface appearance (diffuse, specular, ambient, shininess)
   - Point lights with attenuation for realistic falloff

6. **Rendering Modes**:
   - `draw()`: Standard rendering with current primitive mode
   - `drawWireframe()`: Edge-only rendering for debugging
   - Normal visualization: Useful for debugging lighting issues

7. **Performance Considerations**:
   - Indexed meshes reduce memory usage and improve cache coherence
   - Resolution controls vertex count (higher = more detail, lower = faster rendering)
   - Point clouds can have thousands of vertices efficiently

## Architecture Compliance

- ✅ **Pure C++ API**: Uses `ofBaseApp` and oflike namespace
- ✅ **Metal Backend**: Mesh rendering via Metal (transparent to user)
- ✅ **No OpenGL**: No GL calls in user code
- ✅ **No prohibited libraries**: Uses only oflike-metal API
- ✅ **openFrameworks API compatible**: ofMesh matches oF API exactly

## Implementation Details

### Mesh Storage
- **Vertices**: `std::vector<ofVec3f>` - 3D positions
- **Normals**: `std::vector<ofVec3f>` - Surface normals (optional)
- **TexCoords**: `std::vector<ofVec2f>` - UV coordinates (optional, not used in this example)
- **Colors**: `std::vector<ofColor>` - Per-vertex colors (optional)
- **Indices**: `std::vector<uint32_t>` - Index buffer (optional)

### Rendering Pipeline
1. Mesh data uploaded to GPU buffers (vertex buffer, index buffer)
2. Metal shader processes vertices (transform, lighting)
3. Primitive assembly based on mode (triangles, lines, points)
4. Rasterization and fragment shading
5. Depth testing and blending
6. Output to framebuffer

### Normal Visualization
- Iterates through vertices and normals
- Draws a line from each vertex along its normal (0.3 units length)
- Yellow color for visibility
- Useful for debugging lighting issues and understanding mesh topology

## Console Commands

Pressing number keys or other controls will log to console:
- `[notice ] Mode 1: Custom Triangle Mesh (Pyramid with per-vertex colors)`
- `[notice ] : Vertices: 5, Indices: 12, Primitive: TRIANGLES`
- `[notice ] keyPressed: Animation: ON`
- `[notice ] keyPressed: Wireframe: ON`
- `[notice ] keyPressed: Resolution increased to 24`

## Troubleshooting

### Mesh appears black
- Check if normals are present (`hasNormals()`)
- Toggle lighting with `L` key to see if it's a lighting issue
- Use wireframe mode (`W` key) to verify geometry

### Normals are inverted (surface is dark)
- Normals may be pointing inward instead of outward
- Check winding order of triangles (counter-clockwise is front-facing)
- Use `N` key to visualize normal directions

### Low frame rate
- Decrease mesh resolution with `DOWN` key
- Reduce point cloud size (Mode 6)
- Disable normal visualization (`N` key)

### Mesh not visible
- Check camera position and orientation
- Verify vertices are within camera frustum (near: 0.1, far: 100)
- Try pressing `R` to reset camera

## Mesh Modes Summary

| Mode | Name | Primitive | Vertices | Indices | Colors | Normals | Features |
|------|------|-----------|----------|---------|--------|---------|----------|
| 1 | Pyramid | TRIANGLES | 5 | 12 | Yes | Yes | Custom indexed mesh, per-vertex colors |
| 2 | Sphere | TRIANGLES | ~800 | ~2400 | No | Yes | UV sphere, resolution adjustable |
| 3 | Box | TRIANGLES | 24 | 36 | No | Yes | 6 faces, hard edges |
| 4 | Plane | TRIANGLES | ~100 | ~200 | No | Yes | Subdivided, resolution adjustable |
| 5 | Spiral | LINE_STRIP | 100 | 0 | Yes | No | Continuous line, color gradient |
| 6 | Points | POINTS | 1000 | 0 | Yes | No | Point cloud, spherical distribution |
| 7 | Icosphere | TRIANGLES | ~162 | ~480 | No | Yes | Geodesic sphere, uniform triangles |

(Vertex/index counts are approximate and vary with resolution settings)
