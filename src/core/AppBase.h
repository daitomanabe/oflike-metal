#pragma once

/// Base class for openFrameworks-style applications
/// Users inherit from this class and override lifecycle methods
///
/// Phase 2.3: AppBase - ofBaseApp compatible class
/// - setup() / update() / draw() / exit()
/// - Event callbacks (empty implementation)
///
/// Example:
/// ```cpp
/// class MyApp : public ofBaseApp {
///     void setup() override { /* initialization */ }
///     void update() override { /* logic */ }
///     void draw() override { /* rendering */ }
/// };
/// ```
class ofBaseApp {
public:
    virtual ~ofBaseApp() = default;

    // MARK: - Lifecycle (Core)

    /// Called once at the start of the application
    /// Use this for initialization, loading resources, etc.
    virtual void setup() {}

    /// Called every frame before draw()
    /// Use this for updating application logic, animations, etc.
    virtual void update() {}

    /// Called every frame after update()
    /// Use this for rendering graphics
    virtual void draw() {}

    /// Called when the application is about to quit
    /// Use this for cleanup, saving state, etc.
    virtual void exit() {}

    // MARK: - Mouse Events

    /// Called when the mouse moves
    /// @param x Mouse x position in pixels
    /// @param y Mouse y position in pixels
    virtual void mouseMoved(int x, int y) {}

    /// Called when the mouse is dragged (moved while button pressed)
    /// @param x Mouse x position in pixels
    /// @param y Mouse y position in pixels
    /// @param button Mouse button index (0=left, 1=right, 2=middle)
    virtual void mouseDragged(int x, int y, int button) {}

    /// Called when a mouse button is pressed
    /// @param x Mouse x position in pixels
    /// @param y Mouse y position in pixels
    /// @param button Mouse button index (0=left, 1=right, 2=middle)
    virtual void mousePressed(int x, int y, int button) {}

    /// Called when a mouse button is released
    /// @param x Mouse x position in pixels
    /// @param y Mouse y position in pixels
    /// @param button Mouse button index (0=left, 1=right, 2=middle)
    virtual void mouseReleased(int x, int y, int button) {}

    /// Called when the mouse wheel is scrolled
    /// @param x Mouse x position in pixels
    /// @param y Mouse y position in pixels
    /// @param scrollX Horizontal scroll amount
    /// @param scrollY Vertical scroll amount
    virtual void mouseScrolled(int x, int y, float scrollX, float scrollY) {}

    /// Called when the mouse enters the window
    /// @param x Mouse x position in pixels
    /// @param y Mouse y position in pixels
    virtual void mouseEntered(int x, int y) {}

    /// Called when the mouse exits the window
    /// @param x Mouse x position in pixels
    /// @param y Mouse y position in pixels
    virtual void mouseExited(int x, int y) {}

    // MARK: - Keyboard Events

    /// Called when a key is pressed
    /// @param key Key character or code
    virtual void keyPressed(int key) {}

    /// Called when a key is released
    /// @param key Key character or code
    virtual void keyReleased(int key) {}

    // MARK: - Window Events

    /// Called when the window is resized
    /// @param w New window width in pixels
    /// @param h New window height in pixels
    virtual void windowResized(int w, int h) {}

    /// Called when files are dragged onto the window
    /// @param x Drag position x in pixels
    /// @param y Drag position y in pixels
    virtual void dragEvent(int x, int y) {}
};
