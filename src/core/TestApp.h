#pragma once

#include <iostream>
#include <cmath>
#include "AppBase.h"
#include "../oflike/graphics/ofGraphics.h"

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
        // Phase 6.3: Test matrix transforms

        // Clear background
        ofBackground(50, 50, 50);

        // Test 1: Draw a rectangle without transform
        ofSetColor(255, 0, 0); // Red
        ofDrawRectangle(50, 50, 100, 100);

        // Test 2: Draw a rectangle with translation
        ofPushMatrix();
        ofTranslate(200, 0);
        ofSetColor(0, 255, 0); // Green
        ofDrawRectangle(50, 50, 100, 100);
        ofPopMatrix();

        // Test 3: Draw a circle with rotation
        ofPushMatrix();
        ofTranslate(450, 100);
        ofRotate(frameCount_ * 2.0f); // Rotate based on frame count
        ofSetColor(0, 0, 255); // Blue
        ofDrawRectangle(-50, -50, 100, 100); // Centered rectangle
        ofPopMatrix();

        // Test 4: Draw with scale
        ofPushMatrix();
        ofTranslate(100, 300);
        ofScale(1.0f + sin(frameCount_ * 0.05f) * 0.5f); // Pulsing scale
        ofSetColor(255, 255, 0); // Yellow
        ofDrawCircle(0, 0, 50);
        ofPopMatrix();

        // Test 5: Nested transforms
        ofPushMatrix();
        ofTranslate(400, 300);
        ofRotate(frameCount_ * 1.0f);
        ofSetColor(255, 0, 255); // Magenta
        ofDrawRectangle(-25, -25, 50, 50);

        ofPushMatrix();
        ofTranslate(75, 0);
        ofRotate(-frameCount_ * 2.0f);
        ofSetColor(0, 255, 255); // Cyan
        ofDrawCircle(0, 0, 20);
        ofPopMatrix();

        ofPopMatrix();
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
