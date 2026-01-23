#pragma once

#include <memory>

// Forward declarations
class ofBaseApp;

/// Core application engine
/// Manages the main application lifecycle, frame loop, and user app callbacks
/// Coordinates setup(), update(), and draw() calls
///
/// Phase 2.2: Engine class design
/// - Initialization / Shutdown
/// - Main loop management
/// - App lifecycle coordination
class Engine {
public:
    /// Get the global engine instance (thread-safe singleton)
    static Engine& instance();

    /// Initialize the engine
    /// Sets up the application context and prepares for the frame loop
    void initialize();

    /// Shutdown the engine
    /// Cleans up resources and terminates the application
    void shutdown();

    /// Check if engine is initialized
    bool isInitialized() const;

    // MARK: - App Management

    /// Set the user application instance
    /// The engine will call setup(), update(), and draw() on this app
    /// @param app Pointer to user's ofBaseApp subclass (engine takes ownership)
    void setApp(std::unique_ptr<ofBaseApp> app);

    /// Get the current app instance
    /// @return Pointer to the app (nullptr if not set)
    ofBaseApp* getApp() const;

    // MARK: - Frame Loop

    /// Called once after initialization to setup the user app
    /// Calls app->setup()
    void setup();

    /// Called every frame to update application logic
    /// Calls app->update()
    void update();

    /// Called every frame to render
    /// Calls app->draw() and manages frame timing
    void draw();

    /// Execute one frame tick (update + draw)
    /// This is the main entry point called by the platform layer
    void tick();

    // MARK: - State

    /// Check if setup() has been called
    bool isSetupComplete() const;

    /// Get the number of frames rendered since initialization
    unsigned long long getFrameNum() const;

private:
    // Singleton: private constructor/destructor
    Engine();
    ~Engine();

    // Delete copy/move
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(Engine&&) = delete;

    // pImpl pattern
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// MARK: - Global Accessor (convenience)

/// Get the global engine instance
inline Engine& engine() { return Engine::instance(); }
