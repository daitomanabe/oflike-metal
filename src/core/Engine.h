#pragma once

#include <memory>

// Forward declarations
class ofBaseApp;

/// Core application engine (LEGACY PATH ONLY)
///
/// ARCHITECTURE NOTE (Phase 2.2):
/// - SwiftUI Entry (DEFAULT): Engine is NOT used. SwiftBridge drives lifecycle directly.
/// - ofMain Entry (LEGACY): Engine drives setup/update/draw for backward compatibility.
///
/// This class provides lifecycle management for the ofMain legacy entry point.
/// For SwiftUI applications (the default and recommended path), the lifecycle is
/// managed directly by SwiftBridge without involving Engine.
///
/// Usage:
/// - SwiftUI apps: Ignore this class, use SwiftBridge (src/platform/bridge/)
/// - ofMain apps: Engine drives the app lifecycle via ofRunApp()
///
/// Phase 2.2: Engine class design (Legacy path only)
/// - Initialization / Shutdown
/// - Main loop management (for ofMain)
/// - App lifecycle coordination (for ofMain)
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
