#pragma once

#include <iostream>
#include "AppBase.h"

/// Test application to verify Swift-C++ bridge
/// Phase 1.5: Basic drawing verification (triangle, FPS)
/// Phase 13.1: Now inherits from ofBaseApp to receive events
class TestApp : public ofBaseApp {
public:
    TestApp() : frameCount_(0), startTime_(0.0) {
        std::cout << "[TestApp] Constructor called" << std::endl;
    }

    ~TestApp() {
        std::cout << "[TestApp] Destructor called" << std::endl;
    }

    void setup() override {
        std::cout << "[TestApp] setup() called - Bridge working!" << std::endl;
        std::cout << "[TestApp] Phase 1.5: Basic drawing verification starting" << std::endl;

        // Initialize start time (will be set on first update)
        startTime_ = getCurrentTime();
    }

    void update() override {
        frameCount_++;

        // Calculate FPS every 60 frames
        if (frameCount_ % 60 == 0) {
            double currentTime = getCurrentTime();
            double elapsed = currentTime - startTime_;
            double fps = static_cast<double>(frameCount_) / elapsed;

            std::cout << "[TestApp] Frame: " << frameCount_
                      << " | FPS: " << static_cast<int>(fps)
                      << " | Elapsed: " << static_cast<int>(elapsed) << "s"
                      << std::endl;
        }
    }

    void draw() override {
        // Phase 1.5: Triangle drawing happens in Swift/Metal layer
        // This method confirms C++ draw() is being called
        // Actual Metal rendering is in MetalViewCoordinator
    }

    void exit() override {
        std::cout << "[TestApp] exit() called" << std::endl;
        std::cout << "[TestApp] Total frames rendered: " << frameCount_ << std::endl;
    }

    void windowResized(int w, int h) override {
        std::cout << "[TestApp] windowResized(" << w << ", " << h << ")" << std::endl;
    }

    // Phase 13.1: Mouse event handler
    void mouseMoved(int x, int y) override {
        std::cout << "[TestApp] mouseMoved(" << x << ", " << y << ")" << std::endl;
    }

    // Accessors for frame info
    unsigned long long getFrameCount() const { return frameCount_; }
    double getFPS() const {
        double elapsed = getCurrentTime() - startTime_;
        return (elapsed > 0.0) ? static_cast<double>(frameCount_) / elapsed : 0.0;
    }

private:
    unsigned long long frameCount_;
    double startTime_;

    // Get current time in seconds (simple implementation for testing)
    double getCurrentTime() const {
        return static_cast<double>(clock()) / CLOCKS_PER_SEC;
    }
};
