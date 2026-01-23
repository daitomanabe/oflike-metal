#include "ofGraphics.h"
#include "../math/ofMatrix4x4.h"
#include <algorithm>
#include <simd/simd.h>
#include <vector>
#include <stack>

// ============================================================================
// Internal Graphics State
// ============================================================================

namespace {
    struct GraphicsState {
        // Current colors (0-255 range)
        uint8_t currentColor[4] = {255, 255, 255, 255};  // RGBA
        uint8_t backgroundColor[4] = {0, 0, 0, 255};     // RGBA

        // Fill mode
        bool fillEnabled = true;

        // Line and shape settings
        float lineWidth = 1.0f;
        uint32_t circleResolution = 32;
        uint32_t curveResolution = 20;
        uint32_t sphereResolution = 20;

        // Matrix stack
        std::stack<oflike::ofMatrix4x4> matrixStack;
        oflike::ofMatrix4x4 currentMatrix;

        GraphicsState() {
            // Initialize with identity matrix
            currentMatrix = oflike::ofMatrix4x4::identity();
        }
    };

    GraphicsState& getGraphicsState() {
        static GraphicsState state;
        return state;
    }

    // Convert 0-255 uint8_t color to 0.0-1.0 float4
    simd_float4 colorToFloat4(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        return simd_make_float4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }
}

// ============================================================================
// Color Management
// ============================================================================

void ofSetColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    auto& state = getGraphicsState();
    state.currentColor[0] = r;
    state.currentColor[1] = g;
    state.currentColor[2] = b;
    state.currentColor[3] = a;
}

void ofSetColor(uint8_t gray, uint8_t a) {
    ofSetColor(gray, gray, gray, a);
}

void ofSetHexColor(uint32_t hex, uint8_t a) {
    // Check if hex includes alpha (0xAARRGGBB)
    if (hex > 0xFFFFFF) {
        uint8_t alpha = (hex >> 24) & 0xFF;
        uint8_t r = (hex >> 16) & 0xFF;
        uint8_t g = (hex >> 8) & 0xFF;
        uint8_t b = hex & 0xFF;
        ofSetColor(r, g, b, alpha);
    } else {
        // 0xRRGGBB format
        uint8_t r = (hex >> 16) & 0xFF;
        uint8_t g = (hex >> 8) & 0xFF;
        uint8_t b = hex & 0xFF;
        ofSetColor(r, g, b, a);
    }
}

void ofSetBackgroundColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    auto& state = getGraphicsState();
    state.backgroundColor[0] = r;
    state.backgroundColor[1] = g;
    state.backgroundColor[2] = b;
    state.backgroundColor[3] = a;
}

void ofSetBackgroundColor(uint8_t gray, uint8_t a) {
    ofSetBackgroundColor(gray, gray, gray, a);
}

void ofSetBackgroundHexColor(uint32_t hex, uint8_t a) {
    if (hex > 0xFFFFFF) {
        uint8_t alpha = (hex >> 24) & 0xFF;
        uint8_t r = (hex >> 16) & 0xFF;
        uint8_t g = (hex >> 8) & 0xFF;
        uint8_t b = hex & 0xFF;
        ofSetBackgroundColor(r, g, b, alpha);
    } else {
        uint8_t r = (hex >> 16) & 0xFF;
        uint8_t g = (hex >> 8) & 0xFF;
        uint8_t b = hex & 0xFF;
        ofSetBackgroundColor(r, g, b, a);
    }
}

// ============================================================================
// Background Clearing
// ============================================================================

void ofBackground() {
    auto& state = getGraphicsState();
    ofBackground(state.backgroundColor[0],
                state.backgroundColor[1],
                state.backgroundColor[2],
                state.backgroundColor[3]);
}

void ofBackground(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    // Set background color and trigger clear
    // (Actual clear command will be issued by graphics system)
    ofSetBackgroundColor(r, g, b, a);
    // TODO: Issue clear command via graphics system when integrated
}

void ofBackground(uint8_t gray, uint8_t a) {
    ofBackground(gray, gray, gray, a);
}

void ofBackgroundHex(uint32_t hex, uint8_t a) {
    if (hex > 0xFFFFFF) {
        uint8_t alpha = (hex >> 24) & 0xFF;
        uint8_t r = (hex >> 16) & 0xFF;
        uint8_t g = (hex >> 8) & 0xFF;
        uint8_t b = hex & 0xFF;
        ofBackground(r, g, b, alpha);
    } else {
        uint8_t r = (hex >> 16) & 0xFF;
        uint8_t g = (hex >> 8) & 0xFF;
        uint8_t b = hex & 0xFF;
        ofBackground(r, g, b, a);
    }
}

void ofClear(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    // Set background color and trigger clear (color + depth)
    ofSetBackgroundColor(r, g, b, a);
    // TODO: Issue clear command via graphics system when integrated
}

void ofClear() {
    auto& state = getGraphicsState();
    ofClear(state.backgroundColor[0],
           state.backgroundColor[1],
           state.backgroundColor[2],
           state.backgroundColor[3]);
}

void ofClearDepth() {
    // Clear depth buffer only
    // TODO: Issue clear depth command via graphics system when integrated
}

void ofClearAlpha() {
    // Clear alpha by setting color to current background with alpha = 0
    auto& state = getGraphicsState();
    ofClear(state.backgroundColor[0],
           state.backgroundColor[1],
           state.backgroundColor[2],
           0);
}

// ============================================================================
// Fill & Stroke Mode
// ============================================================================

void ofFill() {
    getGraphicsState().fillEnabled = true;
}

void ofNoFill() {
    getGraphicsState().fillEnabled = false;
}

bool ofGetFill() {
    return getGraphicsState().fillEnabled;
}

// ============================================================================
// Line & Shape Settings
// ============================================================================

void ofSetLineWidth(float width) {
    getGraphicsState().lineWidth = std::max(0.0f, width);
}

float ofGetLineWidth() {
    return getGraphicsState().lineWidth;
}

void ofSetCircleResolution(uint32_t resolution) {
    getGraphicsState().circleResolution = std::max(3u, resolution);
}

uint32_t ofGetCircleResolution() {
    return getGraphicsState().circleResolution;
}

void ofSetCurveResolution(uint32_t resolution) {
    getGraphicsState().curveResolution = std::max(2u, resolution);
}

uint32_t ofGetCurveResolution() {
    return getGraphicsState().curveResolution;
}

void ofSetSphereResolution(uint32_t resolution) {
    getGraphicsState().sphereResolution = std::max(3u, resolution);
}

uint32_t ofGetSphereResolution() {
    return getGraphicsState().sphereResolution;
}

// ============================================================================
// Drawing State Query
// ============================================================================

void ofGetColor(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) {
    auto& state = getGraphicsState();
    r = state.currentColor[0];
    g = state.currentColor[1];
    b = state.currentColor[2];
    a = state.currentColor[3];
}

void ofGetBackgroundColor(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) {
    auto& state = getGraphicsState();
    r = state.backgroundColor[0];
    g = state.backgroundColor[1];
    b = state.backgroundColor[2];
    a = state.backgroundColor[3];
}

// ============================================================================
// Matrix Stack
// ============================================================================

void ofPushMatrix() {
    auto& state = getGraphicsState();
    state.matrixStack.push(state.currentMatrix);
}

void ofPopMatrix() {
    auto& state = getGraphicsState();
    if (!state.matrixStack.empty()) {
        state.currentMatrix = state.matrixStack.top();
        state.matrixStack.pop();
    }
    // If stack is empty, do nothing (silently fail, matching oF behavior)
}

void ofTranslate(float x, float y, float z) {
    auto& state = getGraphicsState();
    auto translationMatrix = oflike::ofMatrix4x4::newTranslationMatrix(x, y, z);
    state.currentMatrix = state.currentMatrix * translationMatrix;
}

void ofRotate(float angle, float x, float y, float z) {
    auto& state = getGraphicsState();
    auto rotationMatrix = oflike::ofMatrix4x4::newRotationMatrix(angle, x, y, z);
    state.currentMatrix = state.currentMatrix * rotationMatrix;
}

void ofRotate(float angle) {
    // 2D rotation around Z axis
    ofRotate(angle, 0.0f, 0.0f, 1.0f);
}

void ofRotateX(float angle) {
    ofRotate(angle, 1.0f, 0.0f, 0.0f);
}

void ofRotateY(float angle) {
    ofRotate(angle, 0.0f, 1.0f, 0.0f);
}

void ofRotateZ(float angle) {
    ofRotate(angle, 0.0f, 0.0f, 1.0f);
}

void ofScale(float x, float y, float z) {
    auto& state = getGraphicsState();
    auto scaleMatrix = oflike::ofMatrix4x4::newScaleMatrix(x, y, z);
    state.currentMatrix = state.currentMatrix * scaleMatrix;
}

void ofScale(float scale) {
    ofScale(scale, scale, scale);
}

void ofLoadIdentityMatrix() {
    auto& state = getGraphicsState();
    state.currentMatrix = oflike::ofMatrix4x4::identity();
}

void ofLoadMatrix(const oflike::ofMatrix4x4& m) {
    auto& state = getGraphicsState();
    state.currentMatrix = m;
}

void ofMultMatrix(const oflike::ofMatrix4x4& m) {
    auto& state = getGraphicsState();
    state.currentMatrix = state.currentMatrix * m;
}

oflike::ofMatrix4x4 ofGetCurrentMatrix() {
    return getGraphicsState().currentMatrix;
}

int ofGetMatrixStackDepth() {
    return static_cast<int>(getGraphicsState().matrixStack.size());
}
