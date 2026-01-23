# Foundation API - Core Classes

## Overview

The foundation layer provides the core classes for building openFrameworks-compatible applications using Metal rendering on macOS. These classes establish the application lifecycle, global context, and rendering engine.

## Architecture

- **ofBaseApp**: User application base class (inherit and override lifecycle methods)
- **Engine**: Application lifecycle manager (singleton)
- **Context**: Global state and services accessor (singleton)

---

## ofBaseApp

Base class for all openFrameworks-style applications. Users inherit from this class and override lifecycle methods to create interactive applications.

### Header

```cpp
#include <core/AppBase.h>
```

### Lifecycle Methods

#### setup()

```cpp
virtual void setup()
```

Called once at application startup. Use for initialization, loading resources, setting up state.

**Example:**
```cpp
class MyApp : public ofBaseApp {
    void setup() override {
        ofSetFrameRate(60);
        ofBackground(0);
        // Load resources, initialize state
    }
};
```

#### update()

```cpp
virtual void update()
```

Called every frame before `draw()`. Use for updating application logic, animations, physics, etc.

**Example:**
```cpp
void update() override {
    x += velocity;
    rotation += 0.5;
}
```

#### draw()

```cpp
virtual void draw()
```

Called every frame after `update()`. Use for all rendering operations.

**Example:**
```cpp
void draw() override {
    ofSetColor(255, 0, 0);
    ofDrawCircle(x, y, 50);
}
```

#### exit()

```cpp
virtual void exit()
```

Called when application is about to quit. Use for cleanup, saving state, releasing resources.

**Example:**
```cpp
void exit() override {
    // Save user preferences
    // Release resources
}
```

### Mouse Events

#### mouseMoved()

```cpp
virtual void mouseMoved(int x, int y)
```

Called when mouse moves without any buttons pressed.

**Parameters:**
- `x`: Mouse x position in pixels
- `y`: Mouse y position in pixels

#### mouseDragged()

```cpp
virtual void mouseDragged(int x, int y, int button)
```

Called when mouse moves with a button pressed.

**Parameters:**
- `x`: Mouse x position in pixels
- `y`: Mouse y position in pixels
- `button`: Button index (0=left, 1=right, 2=middle)

#### mousePressed()

```cpp
virtual void mousePressed(int x, int y, int button)
```

Called when mouse button is pressed.

**Parameters:**
- `x`: Mouse x position in pixels
- `y`: Mouse y position in pixels
- `button`: Button index (0=left, 1=right, 2=middle)

#### mouseReleased()

```cpp
virtual void mouseReleased(int x, int y, int button)
```

Called when mouse button is released.

**Parameters:**
- `x`: Mouse x position in pixels
- `y`: Mouse y position in pixels
- `button`: Button index (0=left, 1=right, 2=middle)

#### mouseScrolled()

```cpp
virtual void mouseScrolled(int x, int y, float scrollX, float scrollY)
```

Called when mouse wheel is scrolled.

**Parameters:**
- `x`: Mouse x position in pixels
- `y`: Mouse y position in pixels
- `scrollX`: Horizontal scroll amount
- `scrollY`: Vertical scroll amount

#### mouseEntered() / mouseExited()

```cpp
virtual void mouseEntered(int x, int y)
virtual void mouseExited(int x, int y)
```

Called when mouse enters or exits the window.

### Keyboard Events

#### keyPressed()

```cpp
virtual void keyPressed(int key)
```

Called when a key is pressed.

**Parameters:**
- `key`: Key character or key code

**Example:**
```cpp
void keyPressed(int key) override {
    if (key == 'f') {
        ofToggleFullscreen();
    }
}
```

#### keyReleased()

```cpp
virtual void keyReleased(int key)
```

Called when a key is released.

**Parameters:**
- `key`: Key character or key code

### Window Events

#### windowResized()

```cpp
virtual void windowResized(int w, int h)
```

Called when window is resized.

**Parameters:**
- `w`: New window width in pixels
- `h`: New window height in pixels

#### dragEvent()

```cpp
virtual void dragEvent(int x, int y)
```

Called when files are dragged onto the window.

**Parameters:**
- `x`: Drag position x in pixels
- `y`: Drag position y in pixels

---

## Engine

Core application engine managing the lifecycle and frame loop. Thread-safe singleton.

### Header

```cpp
#include <core/Engine.h>
```

### Singleton Access

```cpp
Engine& Engine::instance()
Engine& engine()  // Convenience accessor
```

Get the global engine instance.

**Example:**
```cpp
Engine::instance().initialize();
// or
engine().initialize();
```

### Initialization

#### initialize()

```cpp
void initialize()
```

Initialize the engine. Must be called once before use.

#### shutdown()

```cpp
void shutdown()
```

Shutdown the engine and cleanup resources.

#### isInitialized()

```cpp
bool isInitialized() const
```

Check if engine is initialized.

**Returns:** true if initialized, false otherwise

### App Management

#### setApp()

```cpp
void setApp(std::unique_ptr<ofBaseApp> app)
```

Set the user application instance. Engine takes ownership.

**Parameters:**
- `app`: Unique pointer to ofBaseApp subclass

**Example:**
```cpp
engine().setApp(std::make_unique<MyApp>());
```

#### getApp()

```cpp
ofBaseApp* getApp() const
```

Get the current app instance.

**Returns:** Pointer to app (nullptr if not set)

### Frame Loop

#### setup()

```cpp
void setup()
```

Call app->setup() once after initialization. Called internally.

#### update()

```cpp
void update()
```

Call app->update() every frame. Called internally.

#### draw()

```cpp
void draw()
```

Call app->draw() every frame. Called internally.

#### tick()

```cpp
void tick()
```

Execute one frame (update + draw). Called by platform layer.

### State

#### isSetupComplete()

```cpp
bool isSetupComplete() const
```

Check if setup() has been called.

**Returns:** true if setup complete, false otherwise

#### getFrameNum()

```cpp
unsigned long long getFrameNum() const
```

Get number of frames rendered since initialization.

**Returns:** Frame number

---

## Context

Global application context providing access to renderer, timing, and window information. Thread-safe singleton.

### Header

```cpp
#include <core/Context.h>
```

### Singleton Access

```cpp
Context& Context::instance()
Context& ctx()  // Convenience accessor
```

Get the global context instance.

**Example:**
```cpp
Context::instance().getWindowWidth();
// or
ctx().getWindowWidth();
```

### Initialization

#### initialize()

```cpp
void initialize(void* metalDevice)
```

Initialize context with Metal device. Must be called once.

**Parameters:**
- `metalDevice`: Native MTLDevice handle (__bridge void*)

#### isInitialized()

```cpp
bool isInitialized() const
```

Check if context is initialized.

**Returns:** true if initialized, false otherwise

#### shutdown()

```cpp
void shutdown()
```

Shutdown and cleanup the context.

### Renderer Access

#### renderer()

```cpp
render::metal::MetalRenderer* renderer() const
```

Get the Metal renderer.

**Returns:** Renderer instance (nullptr if not initialized)

#### getDrawList()

```cpp
render::DrawList& getDrawList()
```

Get the current frame's draw list.

**Returns:** Reference to draw list

### Timing

#### getElapsedTime()

```cpp
double getElapsedTime() const
```

Get elapsed time since initialization in seconds.

**Returns:** Elapsed time in seconds

**Example:**
```cpp
float t = ctx().getElapsedTime();
float x = sin(t) * 100;
```

#### getElapsedTimeMillis()

```cpp
unsigned long long getElapsedTimeMillis() const
```

Get elapsed time in milliseconds.

**Returns:** Elapsed time in milliseconds

#### getFrameNum()

```cpp
unsigned long long getFrameNum() const
```

Get current frame number (incremented each frame).

**Returns:** Frame number

#### getFrameRate()

```cpp
float getFrameRate() const
```

Get current frame rate (FPS).

**Returns:** Frames per second

#### setFrameRate()

```cpp
void setFrameRate(float fps)
```

Set target frame rate.

**Parameters:**
- `fps`: Target frames per second (default: 60)

**Example:**
```cpp
ctx().setFrameRate(30);  // Run at 30 FPS
```

### Window

#### getWindowWidth() / getWindowHeight()

```cpp
int getWindowWidth() const
int getWindowHeight() const
```

Get window dimensions in pixels.

**Returns:** Width or height in pixels

**Example:**
```cpp
int w = ctx().getWindowWidth();
int h = ctx().getWindowHeight();
ofDrawCircle(w/2, h/2, 100);  // Center of screen
```

#### getWidth() / getHeight()

```cpp
int getWidth() const
int getHeight() const
```

Aliases for getWindowWidth() / getWindowHeight().

#### setWindowSize()

```cpp
void setWindowSize(int width, int height)
```

Set window size. Called internally when window resizes.

**Parameters:**
- `width`: Window width in pixels
- `height`: Window height in pixels

#### requestWindowResize()

```cpp
void requestWindowResize(int width, int height)
```

Request window resize (calls back to Swift layer).

**Parameters:**
- `width`: Requested width in pixels
- `height`: Requested height in pixels

#### requestWindowPosition()

```cpp
void requestWindowPosition(int x, int y)
```

Request window position change.

**Parameters:**
- `x`: Requested x position
- `y`: Requested y position

#### requestWindowTitle()

```cpp
void requestWindowTitle(const std::string& title)
```

Request window title change.

**Parameters:**
- `title`: New window title

#### requestFullscreen()

```cpp
void requestFullscreen(bool fullscreen)
```

Request fullscreen mode change.

**Parameters:**
- `fullscreen`: true to enter fullscreen, false to exit

#### isFullscreen()

```cpp
bool isFullscreen() const
```

Get fullscreen state.

**Returns:** true if in fullscreen mode

#### getScreenWidth() / getScreenHeight()

```cpp
int getScreenWidth() const
int getScreenHeight() const
```

Get main screen dimensions in pixels.

**Returns:** Screen width or height in pixels

#### hideCursor() / showCursor()

```cpp
void hideCursor()
void showCursor()
```

Hide or show the mouse cursor.

**Example:**
```cpp
ctx().hideCursor();  // Hide cursor for fullscreen experience
```

### Matrix Stack

#### getCurrentMatrix()

```cpp
simd_float4x4 getCurrentMatrix() const
```

Get current transformation matrix.

**Returns:** Current matrix from the matrix stack

### Keyboard State

#### setKeyState()

```cpp
void setKeyState(int key, bool pressed)
```

Set key state. Called internally by event system.

**Parameters:**
- `key`: Key code
- `pressed`: true if pressed, false if released

#### getKeyPressed()

```cpp
bool getKeyPressed(int key) const
```

Get key state.

**Parameters:**
- `key`: Key code

**Returns:** true if key is currently pressed

**Example:**
```cpp
if (ctx().getKeyPressed(' ')) {
    // Space is pressed
}
```

---

## Complete Example

```cpp
#include <core/AppBase.h>
#include <core/Engine.h>
#include <core/Context.h>
#include <oflike/graphics/ofGraphics.h>

class MyApp : public ofBaseApp {
    float x = 0;
    float velocity = 2;

    void setup() override {
        ctx().setFrameRate(60);
        ofBackground(0);
    }

    void update() override {
        x += velocity;
        if (x > ctx().getWindowWidth() || x < 0) {
            velocity *= -1;
        }
    }

    void draw() override {
        ofSetColor(255, 0, 0);
        ofDrawCircle(x, ctx().getWindowHeight()/2, 50);
    }

    void keyPressed(int key) override {
        if (key == 'f') {
            ctx().requestFullscreen(!ctx().isFullscreen());
        }
    }
};

// In main or app initialization:
// engine().setApp(std::make_unique<MyApp>());
```

---

## Notes

- All foundation classes use the pImpl pattern for clean separation
- Thread-safe singleton access for Engine and Context
- 100% API compatible with openFrameworks
- Coordinate system: 2D uses top-left origin, 3D uses right-hand coordinate system

---

## See Also

- [02_graphics_2d.md](02_graphics_2d.md) - 2D drawing functions
- [10_events.md](10_events.md) - Event system details
- [ARCHITECTURE.md](../ARCHITECTURE.md) - Overall architecture
