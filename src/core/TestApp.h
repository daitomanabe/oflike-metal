#pragma once

#include <iostream>

/// Test application to verify Swift-C++ bridge
/// This is a temporary class for Phase 1.4 testing
/// Will be replaced by proper ofBaseApp in Phase 2.3
class TestApp {
public:
    TestApp() {
        std::cout << "[TestApp] Constructor called" << std::endl;
    }

    ~TestApp() {
        std::cout << "[TestApp] Destructor called" << std::endl;
    }

    void setup() {
        std::cout << "[TestApp] setup() called - Bridge working!" << std::endl;
    }

    void update() {
        // Don't print every frame to avoid spam
        static int updateCount = 0;
        updateCount++;
        if (updateCount % 60 == 0) {
            std::cout << "[TestApp] update() called (frame " << updateCount << ")" << std::endl;
        }
    }

    void draw() {
        // Don't print every frame to avoid spam
        static int drawCount = 0;
        drawCount++;
        if (drawCount % 60 == 0) {
            std::cout << "[TestApp] draw() called (frame " << drawCount << ")" << std::endl;
        }
    }

    void exit() {
        std::cout << "[TestApp] exit() called" << std::endl;
    }

    void windowResized(int w, int h) {
        std::cout << "[TestApp] windowResized(" << w << ", " << h << ")" << std::endl;
    }
};
