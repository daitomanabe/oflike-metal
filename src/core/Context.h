#pragma once

#include <memory>

// Forward declarations
namespace render {
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

    /// Get the Metal renderer
    /// @return Renderer instance (nullptr if not initialized)
    render::metal::MetalRenderer* renderer() const;

    /// Get the current frame's draw list
    /// @return Reference to the draw list
    render::DrawList& getDrawList();

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
