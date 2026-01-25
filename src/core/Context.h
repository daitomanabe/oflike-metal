#pragma once

#include <memory>
#include <simd/simd.h>

// Forward declarations
namespace render {
class IRenderer;
class DrawList;
namespace metal {
class MetalRenderer;
}
}  // namespace render

/// Global application context singleton
/// Provides access to renderer, timing, and window information
/// Thread-safe singleton access
///
/// Phase 2.1: Context singleton design
class Context {
public:
    /// Get the global context instance (thread-safe singleton)
    static Context& instance();

    /// Initialize the context with Metal device
    /// Must be called once before using the context
    /// @param metalDevice Native MTLDevice handle (__bridge void*)
    void initialize(void* metalDevice);

    /// Check if context is initialized
    bool isInitialized() const;

    /// Shutdown and cleanup the context
    void shutdown();

    // MARK: - Renderer Access

    /// Initialize the Metal renderer with device and view
    /// Must be called after initialize() and before using renderer()
    /// @param metalDevice Native MTLDevice handle (__bridge void*)
    /// @param metalView Native MTKView handle (__bridge void*)
    void initializeRenderer(void* metalDevice, void* metalView);

    /// Get the renderer
    /// @return Renderer instance (nullptr if not initialized)
    render::IRenderer* renderer() const;

    /// Get the current frame's draw list
    /// @return Reference to the draw list
    render::DrawList& getDrawList();

    /// Get the Metal device (Phase 3.3)
    /// @return Metal device (id<MTLDevice>) cast to void*, or nullptr if renderer not initialized
    void* getMetalDevice() const;

    /// Read pixels from a texture (GPU->CPU readback) (Phase 7.5)
    /// @param texture Texture handle (id<MTLTexture>) cast to void*
    /// @param data Destination buffer (must be pre-allocated)
    /// @param width Texture width
    /// @param height Texture height
    /// @param bytesPerRow Bytes per row
    /// @return true on success, false on failure
    bool readTexturePixels(void* texture, void* data, uint32_t width, uint32_t height, size_t bytesPerRow) const;

    // MARK: - Timing

    /// Get elapsed time since initialization (seconds)
    double getElapsedTime() const;

    /// Get elapsed time in milliseconds
    unsigned long long getElapsedTimeMillis() const;

    /// Get current frame number (incremented on each draw call)
    unsigned long long getFrameNum() const;

    /// Get current frame rate (FPS)
    float getFrameRate() const;

    /// Set target frame rate
    /// @param fps Target frames per second (default: 60)
    void setFrameRate(float fps);

    /// Increment frame counter (called internally by Engine)
    void incrementFrame();

    // MARK: - Window

    /// Get window width in pixels
    int getWindowWidth() const;

    /// Get window height in pixels
    int getWindowHeight() const;

    /// Set window size (called internally when window resizes)
    void setWindowSize(int width, int height);

    /// Request window resize (calls back to Swift layer)
    /// @param width Requested window width
    /// @param height Requested window height
    void requestWindowResize(int width, int height);

    /// Set callback for window resize requests (called by Swift layer)
    /// @param callback Function pointer to call when resize is requested
    void setWindowResizeCallback(void (*callback)(int width, int height));

    /// Request window position change (calls back to Swift layer)
    /// @param x Requested window x position
    /// @param y Requested window y position
    void requestWindowPosition(int x, int y);

    /// Set callback for window position requests (called by Swift layer)
    /// @param callback Function pointer to call when position change is requested
    void setWindowPositionCallback(void (*callback)(int x, int y));

    /// Request window title change (calls back to Swift layer)
    /// @param title Requested window title
    void requestWindowTitle(const std::string& title);

    /// Set callback for window title requests (called by Swift layer)
    /// @param callback Function pointer to call when title change is requested
    void setWindowTitleCallback(void (*callback)(const char* title));

    /// Request fullscreen mode change (calls back to Swift layer)
    /// @param fullscreen true to enter fullscreen, false to exit
    void requestFullscreen(bool fullscreen);

    /// Set callback for fullscreen requests (called by Swift layer)
    /// @param callback Function pointer to call when fullscreen change is requested
    void setFullscreenCallback(void (*callback)(bool fullscreen));

    /// Set fullscreen state (called internally when fullscreen mode changes)
    /// @param fullscreen true if fullscreen mode is active, false otherwise
    void setFullscreenState(bool fullscreen);

    /// Get fullscreen state
    /// @return true if fullscreen mode is active, false otherwise
    bool isFullscreen() const;

    /// Get window width (alias for getWindowWidth)
    int getWidth() const { return getWindowWidth(); }

    /// Get window height (alias for getWindowHeight)
    int getHeight() const { return getWindowHeight(); }

    /// Get main screen width in pixels
    /// @return Main screen width in pixels
    int getScreenWidth() const;

    /// Get main screen height in pixels
    /// @return Main screen height in pixels
    int getScreenHeight() const;

    /// Hide the mouse cursor
    void hideCursor();

    /// Show the mouse cursor
    void showCursor();

    // MARK: - Matrix Stack

    /// Get current transformation matrix
    /// @return Current matrix from the matrix stack
    simd_float4x4 getCurrentMatrix() const;

    // MARK: - Keyboard State

    /// Set key state (pressed or released)
    /// Called internally by event system
    /// @param key Key code
    /// @param pressed true if pressed, false if released
    void setKeyState(int key, bool pressed);

    /// Get key state
    /// @param key Key code
    /// @return true if key is currently pressed, false otherwise
    bool getKeyPressed(int key) const;

private:
    // Singleton: private constructor/destructor
    Context();
    ~Context();

    // Delete copy/move
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(Context&&) = delete;

    // pImpl pattern
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// MARK: - Global Accessor (convenience)

/// Get the global context instance
inline Context& ctx() { return Context::instance(); }
