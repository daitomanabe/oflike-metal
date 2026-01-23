#pragma once

#include <memory>

// Forward declaration
class ofBaseApp;

/// Global event dispatcher singleton
/// Routes events from SwiftUI layer to C++ application
/// Manages mouse and keyboard state tracking
///
/// Phase 13.4: EventDispatcher class
/// - Centralizes event routing from SwiftUI → C++
/// - Tracks mouse and keyboard state
/// - Thread-safe singleton access
///
/// Usage:
/// ```cpp
/// EventDispatcher::instance().setApp(myApp);
/// EventDispatcher::instance().dispatchMouseMoved(x, y);
/// ```
class EventDispatcher {
public:
    /// Get the global event dispatcher instance (thread-safe singleton)
    static EventDispatcher& instance();

    /// Set the application instance to receive events
    /// @param app Pointer to the application instance (must outlive dispatcher)
    void setApp(ofBaseApp* app);

    /// Get the current application instance
    /// @return Pointer to the application instance (nullptr if not set)
    ofBaseApp* getApp() const;

    // MARK: - Mouse Events

    /// Dispatch mouse moved event
    /// @param x Mouse x position in pixels
    /// @param y Mouse y position in pixels
    void dispatchMouseMoved(int x, int y);

    /// Dispatch mouse dragged event
    /// @param x Mouse x position in pixels
    /// @param y Mouse y position in pixels
    /// @param button Mouse button index (0=left, 1=right, 2=middle)
    void dispatchMouseDragged(int x, int y, int button);

    /// Dispatch mouse pressed event
    /// @param x Mouse x position in pixels
    /// @param y Mouse y position in pixels
    /// @param button Mouse button index (0=left, 1=right, 2=middle)
    void dispatchMousePressed(int x, int y, int button);

    /// Dispatch mouse released event
    /// @param x Mouse x position in pixels
    /// @param y Mouse y position in pixels
    /// @param button Mouse button index (0=left, 1=right, 2=middle)
    void dispatchMouseReleased(int x, int y, int button);

    /// Dispatch mouse scrolled event
    /// @param x Mouse x position in pixels
    /// @param y Mouse y position in pixels
    /// @param scrollX Horizontal scroll amount
    /// @param scrollY Vertical scroll amount
    void dispatchMouseScrolled(int x, int y, float scrollX, float scrollY);

    /// Dispatch mouse entered event
    /// @param x Mouse x position in pixels
    /// @param y Mouse y position in pixels
    void dispatchMouseEntered(int x, int y);

    /// Dispatch mouse exited event
    /// @param x Mouse x position in pixels
    /// @param y Mouse y position in pixels
    void dispatchMouseExited(int x, int y);

    // MARK: - Keyboard Events

    /// Dispatch key pressed event
    /// @param key Key character or code
    void dispatchKeyPressed(int key);

    /// Dispatch key released event
    /// @param key Key character or code
    void dispatchKeyReleased(int key);

    // MARK: - Window Events

    /// Dispatch window resized event
    /// @param w New window width in pixels
    /// @param h New window height in pixels
    void dispatchWindowResized(int w, int h);

    /// Dispatch drag event (files dragged onto window)
    /// @param x Drag position x in pixels
    /// @param y Drag position y in pixels
    void dispatchDragEvent(int x, int y);

    // MARK: - Mouse State Queries (Phase 13.5)

    /// Get current mouse x position
    /// @return Mouse x position in pixels
    int getMouseX() const;

    /// Get current mouse y position
    /// @return Mouse y position in pixels
    int getMouseY() const;

    /// Get previous mouse x position (before last event)
    /// @return Previous mouse x position in pixels
    int getPreviousMouseX() const;

    /// Get previous mouse y position (before last event)
    /// @return Previous mouse y position in pixels
    int getPreviousMouseY() const;

    /// Get mouse button pressed state
    /// @param button Mouse button index (0=left, 1=right, 2=middle)
    /// @return true if button is currently pressed, false otherwise
    bool getMousePressed(int button) const;

private:
    // Singleton: private constructor/destructor
    EventDispatcher();
    ~EventDispatcher();

    // Delete copy/move
    EventDispatcher(const EventDispatcher&) = delete;
    EventDispatcher& operator=(const EventDispatcher&) = delete;
    EventDispatcher(EventDispatcher&&) = delete;
    EventDispatcher& operator=(EventDispatcher&&) = delete;

    // pImpl pattern to hide implementation details
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// MARK: - Global Accessor (convenience)

/// Get the global event dispatcher instance
inline EventDispatcher& eventDispatcher() { return EventDispatcher::instance(); }
