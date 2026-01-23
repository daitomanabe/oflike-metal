#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#include "Context.h"
#include "../render/DrawList.h"
// #include "../render/metal/MetalRenderer.h"  // TODO: Phase 4.4
#include <iostream>
#include <mutex>
#include <unordered_map>

// MARK: - Context Implementation

struct Context::Impl {
    // Metal device
    id<MTLDevice> device = nil;

    // Renderer (TODO: Phase 4.4)
    // std::unique_ptr<render::metal::MetalRenderer> renderer;

    // Draw list for current frame
    render::DrawList drawList;

    // Timing
    CFTimeInterval startTime = 0.0;
    CFTimeInterval lastFrameTime = 0.0;
    unsigned long long frameNum = 0;
    float targetFrameRate = 60.0f;
    float currentFrameRate = 0.0f;

    // FPS calculation
    CFTimeInterval fpsUpdateTime = 0.0;
    int framesSinceLastFPSUpdate = 0;

    // Window
    int windowWidth = 0;
    int windowHeight = 0;
    void (*windowResizeCallback)(int width, int height) = nullptr;
    void (*windowPositionCallback)(int x, int y) = nullptr;
    void (*windowTitleCallback)(const char* title) = nullptr;

    // Keyboard state (Phase 13.2)
    std::unordered_map<int, bool> keyStates;

    // State
    bool initialized = false;

    Impl() {
        std::cout << "[Context] Impl constructed" << std::endl;
    }

    ~Impl() {
        @autoreleasepool {
            if (device) {
                device = nil;  // ARC will release
            }
            std::cout << "[Context] Impl destructed" << std::endl;
        }
    }
};

// MARK: - Singleton Implementation

Context& Context::instance() {
    // Thread-safe singleton (C++11 magic static)
    static Context instance;
    return instance;
}

Context::Context() : impl_(std::make_unique<Impl>()) {
    std::cout << "[Context] Constructor called" << std::endl;
}

Context::~Context() {
    std::cout << "[Context] Destructor called" << std::endl;
}

// MARK: - Initialization

void Context::initialize(void* metalDevice) {
    @autoreleasepool {
        if (impl_->initialized) {
            std::cout << "[Context] Warning: Already initialized" << std::endl;
            return;
        }

        // Bridge Metal device from Swift
        impl_->device = (__bridge id<MTLDevice>)metalDevice;

        if (!impl_->device) {
            std::cerr << "[Context] Error: Invalid Metal device" << std::endl;
            return;
        }

        std::cout << "[Context] Initialized with Metal device: "
                  << impl_->device.name.UTF8String << std::endl;

        // Initialize timing
        impl_->startTime = CACurrentMediaTime();
        impl_->lastFrameTime = impl_->startTime;
        impl_->fpsUpdateTime = impl_->startTime;
        impl_->frameNum = 0;

        // TODO Phase 4.4: Initialize renderer
        // impl_->renderer = std::make_unique<render::metal::MetalRenderer>(impl_->device);

        impl_->initialized = true;
        std::cout << "[Context] Initialization complete" << std::endl;
    }
}

bool Context::isInitialized() const { return impl_->initialized; }

void Context::shutdown() {
    @autoreleasepool {
        if (!impl_->initialized) {
            return;
        }

        std::cout << "[Context] Shutting down..." << std::endl;

        // TODO Phase 4.4: Shutdown renderer
        // impl_->renderer.reset();

        impl_->device = nil;
        impl_->initialized = false;

        std::cout << "[Context] Shutdown complete" << std::endl;
    }
}

// MARK: - Renderer Access

render::metal::MetalRenderer* Context::renderer() const {
    // TODO Phase 4.4: Return renderer
    // if (!impl_->initialized || !impl_->renderer) {
    //     return nullptr;
    // }
    // return impl_->renderer.get();
    return nullptr;
}

render::DrawList& Context::getDrawList() {
    return impl_->drawList;
}

// MARK: - Timing

double Context::getElapsedTime() const {
    if (!impl_->initialized) {
        return 0.0;
    }

    CFTimeInterval currentTime = CACurrentMediaTime();
    return currentTime - impl_->startTime;
}

unsigned long long Context::getElapsedTimeMillis() const {
    return static_cast<unsigned long long>(getElapsedTime() * 1000.0);
}

unsigned long long Context::getFrameNum() const { return impl_->frameNum; }

float Context::getFrameRate() const { return impl_->currentFrameRate; }

void Context::setFrameRate(float fps) {
    if (fps > 0.0f) {
        impl_->targetFrameRate = fps;
        std::cout << "[Context] Target frame rate set to " << fps << " FPS"
                  << std::endl;
    }
}

void Context::incrementFrame() {
    if (!impl_->initialized) {
        return;
    }

    impl_->frameNum++;
    impl_->framesSinceLastFPSUpdate++;

    // Update FPS calculation
    CFTimeInterval currentTime = CACurrentMediaTime();
    CFTimeInterval elapsed = currentTime - impl_->fpsUpdateTime;

    // Update FPS every 0.5 seconds
    if (elapsed >= 0.5) {
        impl_->currentFrameRate =
            static_cast<float>(impl_->framesSinceLastFPSUpdate) /
            static_cast<float>(elapsed);
        impl_->framesSinceLastFPSUpdate = 0;
        impl_->fpsUpdateTime = currentTime;
    }

    impl_->lastFrameTime = currentTime;
}

// MARK: - Window

int Context::getWindowWidth() const { return impl_->windowWidth; }

int Context::getWindowHeight() const { return impl_->windowHeight; }

void Context::setWindowSize(int width, int height) {
    impl_->windowWidth = width;
    impl_->windowHeight = height;

    std::cout << "[Context] Window size set to " << width << "x" << height
              << std::endl;
}

void Context::requestWindowResize(int width, int height) {
    std::cout << "[Context] Window resize requested: " << width << "x" << height
              << std::endl;

    if (impl_->windowResizeCallback) {
        impl_->windowResizeCallback(width, height);
    } else {
        std::cerr << "[Context] Warning: No window resize callback set (SwiftUI doesn't support programmatic window resizing yet)" << std::endl;
        // TODO Phase 14.1: Implement full window resize through NSWindow access
        // For now, just store the requested size
        impl_->windowWidth = width;
        impl_->windowHeight = height;
    }
}

void Context::setWindowResizeCallback(void (*callback)(int width, int height)) {
    impl_->windowResizeCallback = callback;
    std::cout << "[Context] Window resize callback registered" << std::endl;
}

void Context::requestWindowPosition(int x, int y) {
    std::cout << "[Context] Window position change requested: " << x << "," << y
              << std::endl;

    if (impl_->windowPositionCallback) {
        impl_->windowPositionCallback(x, y);
    } else {
        std::cout << "[Context] Warning: No window position callback registered"
                  << std::endl;
    }
}

void Context::setWindowPositionCallback(void (*callback)(int x, int y)) {
    impl_->windowPositionCallback = callback;
    std::cout << "[Context] Window position callback registered" << std::endl;
}

void Context::requestWindowTitle(const std::string& title) {
    std::cout << "[Context] Window title change requested: " << title << std::endl;

    if (impl_->windowTitleCallback) {
        impl_->windowTitleCallback(title.c_str());
    } else {
        std::cout << "[Context] Warning: No window title callback registered"
                  << std::endl;
    }
}

void Context::setWindowTitleCallback(void (*callback)(const char* title)) {
    impl_->windowTitleCallback = callback;
    std::cout << "[Context] Window title callback registered" << std::endl;
}

// MARK: - Matrix Stack

simd_float4x4 Context::getCurrentMatrix() const {
    // TODO: Implement proper matrix stack
    // For now, return identity matrix
    return matrix_identity_float4x4;
}

// MARK: - Keyboard State

void Context::setKeyState(int key, bool pressed) {
    impl_->keyStates[key] = pressed;
}

bool Context::getKeyPressed(int key) const {
    auto it = impl_->keyStates.find(key);
    if (it != impl_->keyStates.end()) {
        return it->second;
    }
    return false;
}
