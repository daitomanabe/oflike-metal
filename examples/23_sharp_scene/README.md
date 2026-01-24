# Example 23: Sharp Scene

**Multi-Object 3D Gaussian Splatting Scene Management**

This example demonstrates the `Sharp::SharpScene` class, which provides a high-level scene graph for managing multiple 3D Gaussian Splatting clouds with independent transformations and rendering.

## Features Demonstrated

### Scene Management
- **Multiple Object Management**: Add, remove, and manage multiple Gaussian clouds in a single scene
- **Unique Object IDs**: Each object receives a unique ID for efficient access
- **Object Naming**: Assign meaningful names to objects for easier identification
- **Scene Clearing**: Remove all objects at once with `clear()`

### Per-Object Transformations
- **Position**: Independent 3D position for each object (`setPosition`, `getPosition`)
- **Rotation**: Quaternion or axis-angle rotation (`setRotation`, `getRotation`)
- **Scale**: Uniform or non-uniform scaling (`setScale`, `getScale`)
- **Transform Matrix**: Direct matrix manipulation for advanced use cases

### Visibility Control
- **Individual Visibility**: Show/hide specific objects (`setVisible`, `isVisible`)
- **Batch Operations**: Show or hide all objects at once (`showAll`, `hideAll`)
- **Efficient Rendering**: Hidden objects are skipped during rendering

### Scene Serialization
- **Save Scene**: Export entire scene to `.sharp` file format
- **Load Scene**: Import scene from file with all objects and transformations
- **File Format**: Custom binary format with JSON metadata

### Batch Rendering
- **Single Draw Call**: Render all visible objects with `scene.render(renderer, camera)`
- **Individual Rendering**: Render specific objects with `renderObject(id, ...)`
- **Automatic Transforms**: Per-object transformations applied automatically

### Scene Queries
- **Statistics**: Total Gaussian count, memory usage, object count
- **Bounding Box**: Compute scene bounds, center, and size
- **Object Lookup**: Find objects by ID or name

## Example Scene Structure

This example creates a scene with 4 different Gaussian clouds:

```
Scene Layout:
     [Top Object]  ← Yellow cloud, bouncing up/down
          │
          │
[Left] ───┼─── [Right]  ← Orbiting around center
    Green │ Blue
          │
     [Center]  ← Red cloud, rotating in place
```

Each object has:
- Independent position and rotation
- Different color (red, green, blue, yellow)
- Animated transformations (rotation, orbit, bounce)
- Different Gaussian counts (100, 80, 80, 60)

## Controls

### Scene Management
- **SPACE**: Create scene with 4 procedurally generated clouds
- **C**: Clear entire scene
- **S**: Save scene to timestamped `.sharp` file
- **L**: Load scene from file (shows placeholder message)

### Animation
- **A**: Toggle automatic animation on/off

### Object Selection
- **1**: Select center object (red)
- **2**: Select left object (green)
- **3**: Select right object (blue)
- **4**: Select top object (yellow)

### Object Manipulation
- **H**: Hide/show selected object
- **UP**: Increase scale of selected object (1.1x)
- **DOWN**: Decrease scale of selected object (0.9x)

### Camera
- **Mouse Drag**: Rotate camera around scene
- **Mouse Wheel**: Zoom in/out

## Key Code Patterns

### Creating and Adding Objects

```cpp
// Create Gaussian cloud (from image, PLY, or procedural)
Sharp::GaussianCloud cloud = createProceduralCloud(100, ofColor(255, 100, 100));

// Add to scene with optional name
Sharp::ObjectID id = scene.addCloud(std::move(cloud), "myObject");

// Set transformation
scene.setPosition(id, {0, 0, 0});
scene.setRotation(id, M_PI/2, {0, 1, 0});
scene.setScale(id, 1.5f);
```

### Rendering the Scene

```cpp
void draw() {
    camera.begin();

    // Render all visible objects with automatic transformations
    scene.render(renderer, camera);

    camera.end();
}
```

### Animating Objects

```cpp
void update() {
    float time = ofGetElapsedTimef();

    // Rotate object
    scene.setRotation(objectId, time * 0.5f, {0, 1, 0});

    // Move object in circle
    float x = cos(time) * 2.0f;
    float z = sin(time) * 2.0f;
    scene.setPosition(objectId, {x, 0, z});
}
```

### Scene Persistence

```cpp
// Save entire scene
if (scene.save("my_scene.sharp")) {
    ofLog() << "Saved successfully";
}

// Load scene
if (scene.load("my_scene.sharp")) {
    // Get object IDs from loaded scene
    auto ids = scene.getObjectIDs();
    for (auto id : ids) {
        ofLog() << "Loaded: " << scene.getName(id);
    }
}
```

### Scene Queries

```cpp
// Get scene statistics
size_t objectCount = scene.getObjectCount();
size_t totalGaussians = scene.getTotalGaussianCount();
size_t memoryMB = scene.getTotalMemoryUsage() / 1024 / 1024;

// Get scene bounds
ofVec3f boundsMin = scene.getBoundsMin();
ofVec3f boundsMax = scene.getBoundsMax();
ofVec3f center = scene.getCenter();
ofVec3f size = scene.getSize();

// Check if object exists
if (scene.hasObject(id)) {
    const Sharp::SceneObject* obj = scene.getObject(id);
    ofLog() << "Position: " << obj->position;
}
```

## API Overview

### Object Management
```cpp
ObjectID addCloud(GaussianCloud&& cloud, const string& name = "")
bool removeObject(ObjectID id)
void clear()
size_t getObjectCount() const
bool hasObject(ObjectID id) const
vector<ObjectID> getObjectIDs() const
```

### Transformations
```cpp
void setPosition(ObjectID id, const float3& position)
float3 getPosition(ObjectID id) const
void setRotation(ObjectID id, const quatf& rotation)
void setRotation(ObjectID id, float angle, const float3& axis)
quatf getRotation(ObjectID id) const
void setScale(ObjectID id, float scale)
void setScale(ObjectID id, const float3& scale)
float3 getScale(ObjectID id) const
```

### Visibility
```cpp
void setVisible(ObjectID id, bool visible)
bool isVisible(ObjectID id) const
void showAll()
void hideAll()
```

### Naming
```cpp
void setName(ObjectID id, const string& name)
string getName(ObjectID id) const
ObjectID findByName(const string& name) const
```

### Rendering
```cpp
void render(SharpRenderer& renderer, const ofCamera& camera) const
void render(SharpRenderer& renderer,
            const float4x4& viewMatrix,
            const float4x4& projectionMatrix) const
void renderObject(ObjectID id, SharpRenderer& renderer,
                  const ofCamera& camera) const
```

### Serialization
```cpp
bool save(const string& filepath) const
bool load(const string& filepath)
```

## Performance Considerations

### Memory Usage
- Each Gaussian cloud maintains separate GPU buffers
- Total scene memory = sum of all cloud memory
- Use `getTotalMemoryUsage()` to monitor memory consumption

### Rendering Performance
- All objects rendered in single frame
- Per-object transformations applied via GPU uniforms
- Hidden objects are skipped (no GPU processing)
- Depth sorting performed per-object if enabled

### Scene Complexity
- Recommended: 4-10 objects per scene
- Each object can have 10K-100K Gaussians
- Total scene complexity: ~100K-1M Gaussians for 60 FPS

## File Format: .sharp

The `.sharp` file format stores complete scene data:

```
Structure:
- Header (magic number, version)
- Scene metadata (JSON):
  - Object count
  - Object names
  - Transformations (position, rotation, scale)
  - Visibility flags
- Per-object data:
  - Embedded PLY format Gaussian cloud
  - Object-specific metadata
```

## Common Use Cases

### Multi-View Reconstruction
Combine multiple Gaussian clouds from different camera angles:
```cpp
for (int i = 0; i < viewCount; i++) {
    Sharp::GaussianCloud cloud = generateFromView(images[i]);
    ObjectID id = scene.addCloud(std::move(cloud), "view_" + ofToString(i));
    scene.setPosition(id, viewPositions[i]);
}
```

### Object Composition
Build complex scenes from individual objects:
```cpp
// Load individual objects
Sharp::GaussianCloud table = loadPLY("table.ply");
Sharp::GaussianCloud chair = loadPLY("chair.ply");
Sharp::GaussianCloud lamp = loadPLY("lamp.ply");

// Compose scene
ObjectID tableId = scene.addCloud(std::move(table), "table");
ObjectID chairId = scene.addCloud(std::move(chair), "chair");
ObjectID lampId = scene.addCloud(std::move(lamp), "lamp");

// Arrange objects
scene.setPosition(chairId, {1, 0, 0});
scene.setPosition(lampId, {0, 1, 0});
```

### Animation and Interaction
Create animated scenes with interactive objects:
```cpp
// Update loop
for (auto id : scene.getObjectIDs()) {
    if (scene.getName(id) == "animated_object") {
        float angle = ofGetElapsedTimef();
        scene.setRotation(id, angle, {0, 1, 0});
    }
}

// Mouse interaction
void mousePressed(int x, int y) {
    // Ray cast to find clicked object
    ObjectID clicked = findObjectAtScreenPos(x, y);
    scene.setVisible(clicked, !scene.isVisible(clicked));
}
```

## Technical Details

### Scene Graph Structure
- Flat scene graph (no hierarchy)
- Each object has independent transformation
- All objects share same renderer instance
- Efficient batch rendering with minimal state changes

### Coordinate System
- Right-handed coordinate system
- Y-up convention
- Transformations applied in order: Scale → Rotate → Translate

### Thread Safety
- Not thread-safe by default
- Render on main thread only
- Can generate clouds on background threads before adding to scene

## Next Steps

- **Example 24**: Sharp Video Export - Export scene animations to video
- **Example 25**: Sharp Real-time Camera - Generate Gaussian clouds from live camera
- See `src/addons/apple_native/ofxSharp/SharpScene.h` for complete API reference
- See `src/addons/apple_native/ofxSharp/README.md` for ofxSharp overview

## Related Examples
- **Example 22**: Sharp Basic - Single Gaussian cloud generation and rendering
- **Example 10**: FBO - Offscreen rendering techniques
- **Example 12**: Easy Cam - Camera control patterns
