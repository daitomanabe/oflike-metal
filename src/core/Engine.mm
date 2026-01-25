#include "Engine.h"
#include "AppBase.h"
#include "Context.h"
#include <chrono>

using namespace std::chrono;

/// Engine implementation (pImpl pattern)
///
/// ARCHITECTURE NOTE (Phase 2.2):
/// Engine is ONLY used for ofMain legacy entry point.
/// SwiftUI entry (default) uses SwiftBridge directly without Engine.
struct Engine::Impl {
    // User application
    std::unique_ptr<ofBaseApp> app = nullptr;

    // State
    bool initialized = false;
    bool setupComplete = false;

    // Timing
    steady_clock::time_point startTime;
    steady_clock::time_point lastFrameTime;

    Impl() {
        startTime = steady_clock::now();
        lastFrameTime = startTime;
    }
};

// MARK: - Singleton

Engine& Engine::instance() {
    static Engine instance;
    return instance;
}

Engine::Engine() : impl_(std::make_unique<Impl>()) {}

Engine::~Engine() {
    shutdown();
}

// MARK: - Initialization

void Engine::initialize() {
    if (impl_->initialized) {
        return;  // Already initialized
    }

    // Reset timing
    impl_->startTime = steady_clock::now();
    impl_->lastFrameTime = impl_->startTime;
    impl_->setupComplete = false;

    impl_->initialized = true;
}

void Engine::shutdown() {
    if (!impl_->initialized) {
        return;  // Not initialized
    }

    // Call app exit callback
    if (impl_->app) {
        impl_->app->exit();
    }

    // Clean up app
    impl_->app.reset();

    impl_->initialized = false;
    impl_->setupComplete = false;
}

bool Engine::isInitialized() const {
    return impl_->initialized;
}

// MARK: - App Management

void Engine::setApp(std::unique_ptr<ofBaseApp> app) {
    impl_->app = std::move(app);
}

ofBaseApp* Engine::getApp() const {
    return impl_->app.get();
}

// MARK: - Frame Loop

void Engine::setup() {
    if (impl_->setupComplete) {
        return;  // Already setup
    }

    if (impl_->app) {
        impl_->app->setup();
    }

    impl_->setupComplete = true;
}

void Engine::update() {
    if (!impl_->setupComplete) {
        return;  // Setup not called yet
    }

    if (impl_->app) {
        impl_->app->update();
    }
}

void Engine::draw() {
    if (!impl_->setupComplete) {
        return;  // Setup not called yet
    }

    if (impl_->app) {
        impl_->app->draw();
    }

    // Update frame timing
    auto now = steady_clock::now();
    impl_->lastFrameTime = now;

    // Increment frame counter in context
    ctx().incrementFrame();
}

void Engine::tick() {
    // First frame: call setup
    if (!impl_->setupComplete) {
        setup();
    }

    // Update application logic
    update();

    // Render
    draw();
}

// MARK: - State

bool Engine::isSetupComplete() const {
    return impl_->setupComplete;
}

unsigned long long Engine::getFrameNum() const {
    return ctx().getFrameNum();
}
