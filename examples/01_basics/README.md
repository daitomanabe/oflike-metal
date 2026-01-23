# Example 01: Basic Drawing

This example demonstrates the fundamental drawing capabilities of oflike-metal.

## What This Example Shows

- **ofApp Structure**: The classic openFrameworks app pattern with `setup()`, `update()`, and `draw()` methods
- **Background Color**: Using `ofBackground()` to set the background
- **Basic 2D Shapes**: Drawing circles, rectangles, triangles, and lines
- **Color Control**: Using `ofSetColor()` to change drawing colors
- **Fill/Stroke**: Using `ofFill()` and `ofNoFill()` to toggle between filled and outlined shapes
- **Animation**: Simple circular animation using trigonometry
- **Event Handling**: Responding to keyboard input with `keyPressed()`
- **Window Resizing**: Handling window resize events with `windowResized()`

## Visual Elements

The example renders:
1. **Center Point**: A white dot marking the center of the window
2. **Orbit Path**: A gray circle showing the path of animation
3. **Animated Circle**: A red circle that orbits around the center
4. **Corner Shapes**: Static shapes demonstrating different drawing modes:
   - Top-left: Red filled rectangle
   - Top-right: Green filled triangle
   - Bottom-left: Blue filled circle
   - Bottom-right: Yellow outlined rectangle
5. **Connecting Lines**: Gray lines connecting the animated circle to the corner shapes
6. **FPS Indicator**: A simple bar at the top showing frame rate

## Key Concepts

### Setup
```cpp
void setup() override {
    // Initialize position
    // Set frame rate
}
```

### Update
```cpp
void update() override {
    // Update animation state
    // Calculate new positions
}
```

### Draw
```cpp
void draw() override {
    // Clear background
    // Draw shapes
    // Apply transformations
}
```

## How to Use This Example

Currently, oflike-metal uses a hardcoded TestApp in `SwiftBridge.mm`. To run this example:

1. Open `src/platform/bridge/SwiftBridge.mm`
2. Replace the TestApp include with this example
3. Replace `TestApp` with `BasicDrawingApp` in the setup method
4. Build and run the Xcode project

**Note**: A proper app runner system will be implemented in Phase 18 to allow easy switching between examples.

## Interactive Features

- **Space Key**: Toggle between filled and outlined shapes

## API Functions Used

- `ofSetFrameRate()` - Set target frame rate
- `ofGetFrameRate()` - Get current frame rate
- `ofGetWidth()` / `ofGetHeight()` - Get window dimensions
- `ofBackground()` - Clear screen with color
- `ofSetColor()` - Set drawing color
- `ofFill()` / `ofNoFill()` - Set fill mode
- `ofSetLineWidth()` - Set line thickness
- `ofDrawCircle()` - Draw circle
- `ofDrawRectangle()` - Draw rectangle
- `ofDrawTriangle()` - Draw triangle
- `ofDrawLine()` - Draw line

## Expected Output

When running, you should see:
- A smooth animation loop at 60 FPS
- A red circle orbiting the center of the window
- Static colored shapes in the four corners
- Gray lines connecting to the animated circle
- Responsive resizing when the window dimensions change

## Next Steps

After mastering this example, try:
- **Example 02: Shapes** - More complex shape drawing and polylines
- **Example 03: Color** - Color manipulation and gradients
- **Example 04: Images** - Loading and displaying textures
