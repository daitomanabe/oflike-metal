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

// ============================================================================
// Basic Shape Drawing
// ============================================================================

void ofDrawLine(float x1, float y1, float x2, float y2) {
    ofDrawLine(x1, y1, 0.0f, x2, y2, 0.0f);
}

void ofDrawLine(float x1, float y1, float z1, float x2, float y2, float z2) {
    auto& state = getGraphicsState();

    // Apply current transformation matrix
    simd_float4 p1 = simd_make_float4(x1, y1, z1, 1.0f);
    simd_float4 p2 = simd_make_float4(x2, y2, z2, 1.0f);

    // TODO: Transform points by currentMatrix
    // TODO: Issue line draw command via graphics system
    // For now, this is a placeholder for when DrawList is integrated
}

void ofDrawRectangle(float x, float y, float w, float h) {
    auto& state = getGraphicsState();

    if (state.fillEnabled) {
        // Draw filled rectangle (2 triangles)
        // Vertices: (x,y), (x+w,y), (x+w,y+h), (x,y+h)
        simd_float4 v1 = simd_make_float4(x, y, 0.0f, 1.0f);
        simd_float4 v2 = simd_make_float4(x + w, y, 0.0f, 1.0f);
        simd_float4 v3 = simd_make_float4(x + w, y + h, 0.0f, 1.0f);
        simd_float4 v4 = simd_make_float4(x, y + h, 0.0f, 1.0f);

        // TODO: Transform vertices by currentMatrix
        // TODO: Issue draw command via graphics system
        // Triangles: (v1, v2, v3) and (v1, v3, v4)
    } else {
        // Draw rectangle outline (4 lines)
        ofDrawLine(x, y, x + w, y);           // Top
        ofDrawLine(x + w, y, x + w, y + h);   // Right
        ofDrawLine(x + w, y + h, x, y + h);   // Bottom
        ofDrawLine(x, y + h, x, y);           // Left
    }
}

void ofDrawRectRounded(float x, float y, float w, float h, float r) {
    auto& state = getGraphicsState();

    // Clamp radius to not exceed half of smaller dimension
    r = std::min(r, std::min(w, h) / 2.0f);

    if (r <= 0.0f) {
        // No rounding, draw regular rectangle
        ofDrawRectangle(x, y, w, h);
        return;
    }

    // Calculate corner centers
    float x1 = x + r;
    float y1 = y + r;
    float x2 = x + w - r;
    float y2 = y + h - r;

    if (state.fillEnabled) {
        // Draw filled rounded rectangle
        // Center rectangle
        ofDrawRectangle(x1, y, x2 - x1, h);
        ofDrawRectangle(x, y1, r, y2 - y1);
        ofDrawRectangle(x2, y1, r, y2 - y1);

        // Draw 4 corner arcs (simplified - will need proper arc implementation)
        // TODO: Implement proper arc drawing for rounded corners
        // For now, draw quarter circles at each corner
        uint32_t resolution = state.circleResolution / 4;

        // Top-left corner
        for (uint32_t i = 0; i < resolution; i++) {
            float angle1 = M_PI + i * M_PI_2 / resolution;
            float angle2 = M_PI + (i + 1) * M_PI_2 / resolution;
            float cx1 = x1 + r * std::cos(angle1);
            float cy1 = y1 + r * std::sin(angle1);
            float cx2 = x1 + r * std::cos(angle2);
            float cy2 = y1 + r * std::sin(angle2);
            // TODO: Draw triangle fan segment
        }
        // Similar for other 3 corners (top-right, bottom-right, bottom-left)
    } else {
        // Draw outline with rounded corners
        // Top edge
        ofDrawLine(x1, y, x2, y);
        // Right edge
        ofDrawLine(x + w, y1, x + w, y2);
        // Bottom edge
        ofDrawLine(x2, y + h, x1, y + h);
        // Left edge
        ofDrawLine(x, y2, x, y1);

        // Draw corner arcs
        // TODO: Implement proper arc drawing
    }
}

void ofDrawCircle(float x, float y, float radius) {
    ofDrawCircle(x, y, 0.0f, radius);
}

void ofDrawCircle(float x, float y, float z, float radius) {
    auto& state = getGraphicsState();
    uint32_t resolution = state.circleResolution;

    if (state.fillEnabled) {
        // Draw filled circle using triangle fan
        std::vector<simd_float4> vertices;
        vertices.reserve(resolution + 2);

        // Center vertex
        vertices.push_back(simd_make_float4(x, y, z, 1.0f));

        // Perimeter vertices
        for (uint32_t i = 0; i <= resolution; i++) {
            float angle = (2.0f * M_PI * i) / resolution;
            float vx = x + radius * std::cos(angle);
            float vy = y + radius * std::sin(angle);
            vertices.push_back(simd_make_float4(vx, vy, z, 1.0f));
        }

        // TODO: Transform vertices by currentMatrix
        // TODO: Issue triangle fan draw command via graphics system
    } else {
        // Draw circle outline using line loop
        for (uint32_t i = 0; i < resolution; i++) {
            float angle1 = (2.0f * M_PI * i) / resolution;
            float angle2 = (2.0f * M_PI * (i + 1)) / resolution;

            float x1 = x + radius * std::cos(angle1);
            float y1 = y + radius * std::sin(angle1);
            float x2 = x + radius * std::cos(angle2);
            float y2 = y + radius * std::sin(angle2);

            ofDrawLine(x1, y1, z, x2, y2, z);
        }
    }
}

void ofDrawEllipse(float x, float y, float width, float height) {
    auto& state = getGraphicsState();
    uint32_t resolution = state.circleResolution;

    float radiusX = width / 2.0f;
    float radiusY = height / 2.0f;

    if (state.fillEnabled) {
        // Draw filled ellipse using triangle fan
        std::vector<simd_float4> vertices;
        vertices.reserve(resolution + 2);

        // Center vertex
        vertices.push_back(simd_make_float4(x, y, 0.0f, 1.0f));

        // Perimeter vertices
        for (uint32_t i = 0; i <= resolution; i++) {
            float angle = (2.0f * M_PI * i) / resolution;
            float vx = x + radiusX * std::cos(angle);
            float vy = y + radiusY * std::sin(angle);
            vertices.push_back(simd_make_float4(vx, vy, 0.0f, 1.0f));
        }

        // TODO: Transform vertices by currentMatrix
        // TODO: Issue triangle fan draw command via graphics system
    } else {
        // Draw ellipse outline
        for (uint32_t i = 0; i < resolution; i++) {
            float angle1 = (2.0f * M_PI * i) / resolution;
            float angle2 = (2.0f * M_PI * (i + 1)) / resolution;

            float x1 = x + radiusX * std::cos(angle1);
            float y1 = y + radiusY * std::sin(angle1);
            float x2 = x + radiusX * std::cos(angle2);
            float y2 = y + radiusY * std::sin(angle2);

            ofDrawLine(x1, y1, x2, y2);
        }
    }
}

void ofDrawTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
    ofDrawTriangle(x1, y1, 0.0f, x2, y2, 0.0f, x3, y3, 0.0f);
}

void ofDrawTriangle(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3) {
    auto& state = getGraphicsState();

    if (state.fillEnabled) {
        // Draw filled triangle
        simd_float4 v1 = simd_make_float4(x1, y1, z1, 1.0f);
        simd_float4 v2 = simd_make_float4(x2, y2, z2, 1.0f);
        simd_float4 v3 = simd_make_float4(x3, y3, z3, 1.0f);

        // TODO: Transform vertices by currentMatrix
        // TODO: Issue triangle draw command via graphics system
    } else {
        // Draw triangle outline (3 lines)
        ofDrawLine(x1, y1, z1, x2, y2, z2);
        ofDrawLine(x2, y2, z2, x3, y3, z3);
        ofDrawLine(x3, y3, z3, x1, y1, z1);
    }
}

// ============================================================================
// Curve Drawing
// ============================================================================

/**
 * Evaluate Catmull-Rom spline at parameter t.
 * Formula: P(t) = 0.5 * [2*P1 + (-P0 + P2)*t + (2*P0 - 5*P1 + 4*P2 - P3)*t^2 + (-P0 + 3*P1 - 3*P2 + P3)*t^3]
 */
namespace {
    float catmullRomInterpolate(float p0, float p1, float p2, float p3, float t) {
        float t2 = t * t;
        float t3 = t2 * t;

        return 0.5f * (
            2.0f * p1 +
            (-p0 + p2) * t +
            (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
            (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3
        );
    }
}

void ofDrawCurve(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3) {
    ofDrawCurve(x0, y0, 0.0f, x1, y1, 0.0f, x2, y2, 0.0f, x3, y3, 0.0f);
}

void ofDrawCurve(float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3) {
    auto& state = getGraphicsState();
    uint32_t resolution = state.curveResolution;

    // Draw curve as a series of line segments
    float prevX = x1, prevY = y1, prevZ = z1;

    for (uint32_t i = 1; i <= resolution; i++) {
        float t = static_cast<float>(i) / resolution;

        float x = catmullRomInterpolate(x0, x1, x2, x3, t);
        float y = catmullRomInterpolate(y0, y1, y2, y3, t);
        float z = catmullRomInterpolate(z0, z1, z2, z3, t);

        ofDrawLine(prevX, prevY, prevZ, x, y, z);

        prevX = x;
        prevY = y;
        prevZ = z;
    }
}

/**
 * Evaluate cubic Bezier curve at parameter t.
 * Formula: P(t) = (1-t)^3*P0 + 3*(1-t)^2*t*P1 + 3*(1-t)*t^2*P2 + t^3*P3
 */
namespace {
    float bezierInterpolate(float p0, float p1, float p2, float p3, float t) {
        float oneMinusT = 1.0f - t;
        float oneMinusT2 = oneMinusT * oneMinusT;
        float oneMinusT3 = oneMinusT2 * oneMinusT;
        float t2 = t * t;
        float t3 = t2 * t;

        return oneMinusT3 * p0 +
               3.0f * oneMinusT2 * t * p1 +
               3.0f * oneMinusT * t2 * p2 +
               t3 * p3;
    }
}

void ofDrawBezier(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3) {
    ofDrawBezier(x0, y0, 0.0f, x1, y1, 0.0f, x2, y2, 0.0f, x3, y3, 0.0f);
}

void ofDrawBezier(float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3) {
    auto& state = getGraphicsState();
    uint32_t resolution = state.curveResolution;

    // Draw Bezier curve as a series of line segments
    float prevX = x0, prevY = y0, prevZ = z0;

    for (uint32_t i = 1; i <= resolution; i++) {
        float t = static_cast<float>(i) / resolution;

        float x = bezierInterpolate(x0, x1, x2, x3, t);
        float y = bezierInterpolate(y0, y1, y2, y3, t);
        float z = bezierInterpolate(z0, z1, z2, z3, t);

        ofDrawLine(prevX, prevY, prevZ, x, y, z);

        prevX = x;
        prevY = y;
        prevZ = z;
    }
}
