# Example 11: Event Handling

Comprehensive demonstration of oflike-metal's event system, showcasing all types of user input handling with visual feedback.

## Features

### Mouse Events
- **Mouse Movement**: Track cursor position in real-time
- **Mouse Dragging**: Draw with click-and-drag
- **Mouse Clicks**: Left, right, and middle button detection
- **Mouse Scrolling**: Horizontal and vertical scroll detection
- **Mouse Entry/Exit**: Window boundary detection
- **Mouse Trail**: Visual history of mouse movement

### Keyboard Events
- **Key Pressed**: Detect any key press with display
- **Key Released**: Track key release events
- **Key State**: Query current key state with `ofGetKeyPressed()`
- **Text Input**: Display typed characters
- **Modifier Keys**: Detect Shift, Ctrl, Alt, Cmd

### Window Events
- **Window Resize**: Respond to window dimension changes
- **Drag & Drop**: Handle file drops onto window
- **Window Focus**: Detect window activation/deactivation

### Event Logging
- **Real-time Log**: Display recent events with timestamps
- **Color Coding**: Different colors for different event types
- **Event Count**: Track total events received
- **Log Clearing**: Clear log with 'C' key

## Controls

| Key/Action | Description |
|------------|-------------|
| **Mouse Move** | Track position, draw trail |
| **Mouse Drag** | Draw lines while dragging |
| **Mouse Click** | Register click events |
| **Mouse Scroll** | Track scroll wheel |
| **Type Keys** | Display keyboard input |
| **C** | Clear event log |
| **T** | Toggle mouse trail on/off |
| **L** | Toggle event logging on/off |
| **R** | Reset demo (clear all) |
| **Drag Files** | Test file drag-and-drop |
| **ESC** | Quit application |

## Code Structure

```cpp
class EventApp : public ofBaseApp {
    // Mouse Events
    void mouseMoved(int x, int y) override;
    void mouseDragged(int x, int y, int button) override;
    void mousePressed(int x, int y, int button) override;
    void mouseReleased(int x, int y, int button) override;
    void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
    void mouseEntered(int x, int y) override;
    void mouseExited(int x, int y) override;

    // Keyboard Events
    void keyPressed(int key) override;
    void keyReleased(int key) override;

    // Window Events
    void windowResized(int w, int h) override;
    void dragEvent(ofDragInfo dragInfo) override;
};
```

## What You'll Learn

1. **Event System Architecture**: Understanding the ofBaseApp event callback pattern
2. **Mouse Tracking**: Capturing and visualizing mouse input
3. **Keyboard Input**: Handling keyboard events and querying key states
4. **Event State**: Using `ofGetMouseX()`, `ofGetMouseY()`, `ofGetMousePressed()`, `ofGetKeyPressed()`
5. **Visual Feedback**: Creating interactive UI responses
6. **Event Logging**: Building debug tools for event monitoring
7. **Interactive Drawing**: Implementing drawing tools with mouse events

## Technical Details

### Mouse Trail Implementation
- Stores recent mouse positions with timestamps
- Fades older trail points based on time
- Circular buffer for efficient memory usage
- Color-coded by speed or time

### Event Log System
- Fixed-size circular buffer
- Timestamp for each event
- Color-coded by event type:
  - Blue: Mouse events
  - Green: Keyboard events
  - Orange: Window events
  - Red: Drag & drop events

### Performance Considerations
- Efficient event queuing
- Limited trail history to prevent memory growth
- Throttled rendering for high-frequency events (mouseMoved)

## Building

### Using Xcode
```bash
open examples/11_events/11_events.xcodeproj
# Build and Run (Cmd+R)
```

### Using CMake
```bash
mkdir build && cd build
cmake .. -G Xcode
cmake --build . --target 11_events --config Release
./11_events
```

## Related Examples

- **01_basics**: Basic drawing and animation
- **02_shapes**: Shape drawing (used for visualization)
- **05_typography**: Font rendering (used for text display)

## API Reference

- `ofBaseApp::mouseMoved()` - Mouse movement callback
- `ofBaseApp::mouseDragged()` - Mouse drag callback
- `ofBaseApp::mousePressed()` - Mouse button press callback
- `ofBaseApp::mouseReleased()` - Mouse button release callback
- `ofBaseApp::mouseScrolled()` - Mouse scroll callback
- `ofBaseApp::keyPressed()` - Key press callback
- `ofBaseApp::keyReleased()` - Key release callback
- `ofBaseApp::windowResized()` - Window resize callback
- `ofBaseApp::dragEvent()` - File drag & drop callback
- `ofGetMouseX()` - Current mouse X position
- `ofGetMouseY()` - Current mouse Y position
- `ofGetMousePressed()` - Current mouse button state
- `ofGetKeyPressed()` - Current key state

## Expected Output

When you run this example, you should see:
- A black background with colorful visualizations
- Mouse position indicator (crosshair)
- Mouse trail (fading line following cursor)
- Event log in top-left corner (recent events with timestamps)
- Keyboard input display (last pressed keys)
- Drawing canvas (drag to draw lines)
- Status information (event counts, toggles)
- Drag & drop area indicator

## Tips

1. **Move mouse around** to see position tracking and trail
2. **Click and drag** to draw on canvas
3. **Type on keyboard** to see character input
4. **Scroll mouse wheel** to see scroll events
5. **Drag files** onto window to test file drop handling
6. **Press 'C'** to clear the event log
7. **Press 'T'** to toggle mouse trail
8. **Press 'L'** to toggle event logging (reduces visual clutter)
9. **Press 'R'** to reset everything

## Further Exploration

- Implement gesture recognition (circles, swipes)
- Add multi-touch support (trackpad gestures)
- Create a simple drawing app with tool selection
- Build a UI library using these events
- Implement keyboard shortcuts for your app
- Create a custom event logging system

## Integration Notes

To use event handling in your own project:
1. Inherit from `ofBaseApp`
2. Override event methods you need
3. Implement your response logic
4. Use `ofGet*()` functions to query current state
5. Remember: events are called from the main thread
