#pragma once
#include <core/AppBase.h>
#include <oflike/ofMain.h>

/// Phase 11.2: Performance statistics validation test
/// This app renders multiple primitives to generate draw calls and verify
/// that PerformanceStats returns non-zero values for:
/// - drawCalls (number of draw commands issued per frame)
/// - vertices (number of vertices rendered per frame)
/// - gpuTime (GPU execution time in milliseconds)
class PerformanceTestApp : public ofBaseApp {
public:
    void setup() override;
    void update() override;
    void draw() override;

private:
    float angle = 0.0f;
    int numCircles = 20;  // Multiple circles to generate draw calls
};
