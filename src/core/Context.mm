#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#import <AppKit/AppKit.h>
#include "Context.h"
#include "../render/IRenderer.h"
#include "../render/DrawList.h"
#include "../render/metal/MetalRenderer.h"
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <vector>

// MARK: - Context Implementation

struct Context::Impl {
    // Metal device
    id<MTLDevice> device = nil;

    // Renderer (Phase 3.1)
    std::unique_ptr<render::metal::MetalRenderer> renderer;

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
    void (*fullscreenCallback)(bool fullscreen) = nullptr;
    bool isFullscreen = false;

    // Keyboard state (Phase 13.2)
    std::unordered_map<int, bool> keyStates;

    // Camera matrix stack (Phase 8.1)
    struct ViewProjectionState {
        simd_float4x4 viewMatrix;
        simd_float4x4 projectionMatrix;
    };
    std::vector<ViewProjectionState> viewStack;
    simd_float4x4 currentViewMatrix = matrix_identity_float4x4;
    simd_float4x4 currentProjectionMatrix = matrix_identity_float4x4;

    // Material state stack (Phase 8.4)
    std::vector<std::vector<float>> materialStack;
    std::vector<float> currentMaterialData;

    // Light state (Phase 8.5)
    std::vector<std::vector<float>> registeredLights;  // Each light is 23 floats
    bool lightingEnabled = false;

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

        // Note: Renderer initialization is done separately via initializeRenderer()
        // which requires both device and view (Phase 3.1)

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

        // Shutdown renderer (Phase 3.1)
        if (impl_->renderer) {
            impl_->renderer->shutdown();
            impl_->renderer.reset();
        }

        impl_->device = nil;
        impl_->initialized = false;

        std::cout << "[Context] Shutdown complete" << std::endl;
    }
}

// MARK: - Renderer Access

void Context::initializeRenderer(void* metalDevice, void* metalView) {
    @autoreleasepool {
        if (!impl_->initialized) {
            std::cerr << "[Context] Error: Context must be initialized before renderer" << std::endl;
            return;
        }

        if (impl_->renderer) {
            std::cout << "[Context] Warning: Renderer already initialized" << std::endl;
            return;
        }

        std::cout << "[Context] Initializing Metal renderer..." << std::endl;

        // Create renderer with device and view (Phase 3.1)
        impl_->renderer = std::make_unique<render::metal::MetalRenderer>(metalDevice, metalView);

        // Initialize the renderer
        if (!impl_->renderer->initialize()) {
            std::cerr << "[Context] Error: Failed to initialize Metal renderer" << std::endl;
            impl_->renderer.reset();
            return;
        }

        std::cout << "[Context] Metal renderer initialized successfully" << std::endl;
    }
}

render::IRenderer* Context::renderer() const {
    // Phase 3.1: Return renderer pointer (Phase 7 fix: return as IRenderer* for layer abstraction)
    if (!impl_->initialized || !impl_->renderer) {
        return nullptr;
    }
    return impl_->renderer.get();
}

render::DrawList& Context::getDrawList() {
    return impl_->drawList;
}

void* Context::getMetalDevice() const {
    if (!impl_->renderer) {
        return nullptr;
    }
    return impl_->renderer->getDevice();
}

bool Context::readTexturePixels(void* texture, void* data, uint32_t width, uint32_t height, size_t bytesPerRow) const {
    if (!impl_->renderer) {
        return false;
    }
    return impl_->renderer->readTexturePixels(texture, data, width, height, bytesPerRow);
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

int Context::getScreenWidth() const {
    @autoreleasepool {
        NSScreen* mainScreen = [NSScreen mainScreen];
        if (mainScreen) {
            NSRect frame = [mainScreen frame];
            return static_cast<int>(frame.size.width);
        }
        return 1920; // Fallback default
    }
}

int Context::getScreenHeight() const {
    @autoreleasepool {
        NSScreen* mainScreen = [NSScreen mainScreen];
        if (mainScreen) {
            NSRect frame = [mainScreen frame];
            return static_cast<int>(frame.size.height);
        }
        return 1080; // Fallback default
    }
}

void Context::hideCursor() {
    @autoreleasepool {
        [NSCursor hide];
    }
}

void Context::showCursor() {
    @autoreleasepool {
        [NSCursor unhide];
    }
}

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

void Context::requestFullscreen(bool fullscreen) {
    std::cout << "[Context] Fullscreen mode requested: " << (fullscreen ? "ON" : "OFF") << std::endl;

    if (impl_->fullscreenCallback) {
        impl_->fullscreenCallback(fullscreen);
    } else {
        std::cout << "[Context] Warning: No fullscreen callback registered"
                  << std::endl;
    }
}

void Context::setFullscreenCallback(void (*callback)(bool fullscreen)) {
    impl_->fullscreenCallback = callback;
    std::cout << "[Context] Fullscreen callback registered" << std::endl;
}

void Context::setFullscreenState(bool fullscreen) {
    impl_->isFullscreen = fullscreen;
}

bool Context::isFullscreen() const {
    return impl_->isFullscreen;
}

// MARK: - Matrix Stack

simd_float4x4 Context::getCurrentMatrix() const {
    // TODO: Implement proper matrix stack
    // For now, return identity matrix
    return matrix_identity_float4x4;
}

// MARK: - Camera Matrix Stack (Phase 8.1)

void Context::pushView() {
    Impl::ViewProjectionState state;
    state.viewMatrix = impl_->currentViewMatrix;
    state.projectionMatrix = impl_->currentProjectionMatrix;
    impl_->viewStack.push_back(state);
}

void Context::popView() {
    if (impl_->viewStack.empty()) {
        std::cerr << "[Context] Warning: View stack underflow in popView()" << std::endl;
        return;
    }

    const auto& state = impl_->viewStack.back();
    impl_->currentViewMatrix = state.viewMatrix;
    impl_->currentProjectionMatrix = state.projectionMatrix;
    impl_->viewStack.pop_back();
}

void Context::setViewMatrix(const simd_float4x4& viewMatrix) {
    impl_->currentViewMatrix = viewMatrix;
}

simd_float4x4 Context::getViewMatrix() const {
    return impl_->currentViewMatrix;
}

void Context::setProjectionMatrix(const simd_float4x4& projectionMatrix) {
    impl_->currentProjectionMatrix = projectionMatrix;
}

simd_float4x4 Context::getProjectionMatrix() const {
    return impl_->currentProjectionMatrix;
}

simd_float4 Context::getViewport() const {
    // Return viewport as (x, y, width, height)
    return simd_make_float4(0.0f, 0.0f,
                            static_cast<float>(impl_->windowWidth),
                            static_cast<float>(impl_->windowHeight));
}

// MARK: - Material State (Phase 8.4)

void Context::pushMaterial() {
    // Push current material state onto stack
    impl_->materialStack.push_back(impl_->currentMaterialData);
}

void Context::popMaterial() {
    // Pop material state from stack
    if (!impl_->materialStack.empty()) {
        impl_->currentMaterialData = impl_->materialStack.back();
        impl_->materialStack.pop_back();
    }
}

void Context::setMaterialData(const std::vector<float>& materialData) {
    impl_->currentMaterialData = materialData;
}

std::vector<float> Context::getMaterialData() const {
    // If no material set, return default material
    // Default: ambient(0.2), diffuse(0.8), specular(1.0), emissive(0.0), shininess(64.0)
    if (impl_->currentMaterialData.empty()) {
        return {
            0.2f, 0.2f, 0.2f,   // ambient.rgb
            0.8f, 0.8f, 0.8f,   // diffuse.rgb
            1.0f, 1.0f, 1.0f,   // specular.rgb
            0.0f, 0.0f, 0.0f,   // emissive.rgb
            64.0f               // shininess
        };
    }
    return impl_->currentMaterialData;
}

bool Context::hasMaterial() const {
    // Material is active if there's data in the material stack (between begin/end)
    return !impl_->materialStack.empty();
}

// MARK: - Light State (Phase 8.5)

void Context::registerLight(const std::vector<float>& lightData) {
    impl_->registeredLights.push_back(lightData);
}

void Context::unregisterLight(const std::vector<float>& lightData) {
    auto& lights = impl_->registeredLights;
    for (auto it = lights.begin(); it != lights.end(); ++it) {
        // Compare light data (first few floats should be enough to identify)
        if (it->size() == lightData.size() && *it == lightData) {
            lights.erase(it);
            return;
        }
    }
}

void Context::clearLights() {
    impl_->registeredLights.clear();
}

int Context::getLightCount() const {
    return static_cast<int>(impl_->registeredLights.size());
}

std::vector<float> Context::getAllLightData() const {
    std::vector<float> allData;
    for (const auto& light : impl_->registeredLights) {
        allData.insert(allData.end(), light.begin(), light.end());
    }
    return allData;
}

void Context::setLightingEnabled(bool enabled) {
    impl_->lightingEnabled = enabled;
}

bool Context::isLightingEnabled() const {
    return impl_->lightingEnabled;
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
