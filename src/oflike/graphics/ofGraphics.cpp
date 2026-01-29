#include "ofGraphics.h"
#include "../math/ofMatrix4x4.h"
#include "../math/ofVec3f.h"
#include "ofPath.h"
#include "../../core/Context.h"
#include "../../render/IRenderer.h"
#include "../../render/RenderTypes.h"
#include "../../render/DrawList.h"
#include "../../render/DrawCommand.h"
#include <algorithm>
#include <simd/simd.h>
#include <vector>
#include <stack>
#include <map>
#include <array>

// ============================================================================
// Internal Graphics State
// ============================================================================

namespace {
    // Vertex type for shape building
    enum class ShapeVertexType {
        Vertex,       // Regular linear vertex
        CurveVertex,  // Catmull-Rom curve vertex
        BezierVertex  // Bezier curve control point
    };

    struct ShapeVertex {
        oflike::ofVec3f position;
        ShapeVertexType type;
    };

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

        // Blend mode (default: alpha blending)
        int blendMode = OF_BLENDMODE_ALPHA;

        // Depth testing and culling (default: disabled for 2D compatibility)
        bool depthTestEnabled = false;
        bool depthWriteEnabled = true;
        bool cullingEnabled = false;

        // Lighting (default: disabled for 2D compatibility)
        bool lightingEnabled = false;
        bool smoothLighting = true;

        // Matrix stack
        std::stack<oflike::ofMatrix4x4> matrixStack;
        oflike::ofMatrix4x4 currentMatrix;

        // Texture binding (Phase 7.2)
        void* activeTexture = nullptr;  // Currently bound texture (id<MTLTexture> handle)

        // Shape API state
        bool shapeBegun = false;
        std::vector<std::vector<ShapeVertex>> shapeContours;  // Multiple contours for holes
        std::vector<ShapeVertex> currentContour;

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
    ofSetBackgroundColor(r, g, b, a);

    // Emit clear command via DrawList
    render::ClearCommand clearData;
    clearData.color = colorToFloat4(r, g, b, a);
    clearData.clearColor = true;
    clearData.clearDepth = false;
    clearData.clearStencil = false;

    render::SetClearCommand clearCmd(clearData);
    Context::instance().getDrawList().addCommand(clearCmd);
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

    // Emit clear command via DrawList (clears both color and depth)
    render::ClearCommand clearData;
    clearData.color = colorToFloat4(r, g, b, a);
    clearData.clearColor = true;
    clearData.clearDepth = true;
    clearData.clearStencil = false;

    render::SetClearCommand clearCmd(clearData);
    Context::instance().getDrawList().addCommand(clearCmd);
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
    render::ClearCommand clearData;
    clearData.clearColor = false;
    clearData.clearDepth = true;
    clearData.clearStencil = false;

    render::SetClearCommand clearCmd(clearData);
    Context::instance().getDrawList().addCommand(clearCmd);
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
// Texture Binding (Phase 7.2)
// ============================================================================

void ofSetActiveTexture(void* texture) {
    getGraphicsState().activeTexture = texture;
}

void* ofGetActiveTexture() {
    return getGraphicsState().activeTexture;
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

    // Create line vertices (2D rendering)
    simd_float4 color = colorToFloat4(state.currentColor[0], state.currentColor[1],
                                       state.currentColor[2], state.currentColor[3]);

    render::Vertex2D vertices[2];
    vertices[0] = render::Vertex2D(x1, y1, 0.0f, 0.0f, color.x, color.y, color.z, color.w);
    vertices[1] = render::Vertex2D(x2, y2, 0.0f, 0.0f, color.x, color.y, color.z, color.w);

    // Add vertices to DrawList
    auto& drawList = Context::instance().getDrawList();
    uint32_t vtxOffset = drawList.addVertices2D(vertices, 2);

    // Create draw command
    render::DrawCommand2D cmd;
    cmd.vertexOffset = vtxOffset;
    cmd.vertexCount = 2;
    cmd.primitiveType = render::PrimitiveType::Line;
    cmd.blendMode = static_cast<render::BlendMode>(state.blendMode);
    cmd.texture = nullptr;

    // Convert ofMatrix4x4 to simd_float4x4
    auto& m = state.currentMatrix;
    cmd.transform = simd_matrix(
        simd_make_float4(m(0,0), m(1,0), m(2,0), m(3,0)),
        simd_make_float4(m(0,1), m(1,1), m(2,1), m(3,1)),
        simd_make_float4(m(0,2), m(1,2), m(2,2), m(3,2)),
        simd_make_float4(m(0,3), m(1,3), m(2,3), m(3,3))
    );

    drawList.addCommand(cmd);
}

void ofDrawRectangle(float x, float y, float w, float h) {
    auto& state = getGraphicsState();

    if (state.fillEnabled) {
        // Draw filled rectangle (2 triangles)
        simd_float4 color = colorToFloat4(state.currentColor[0], state.currentColor[1],
                                           state.currentColor[2], state.currentColor[3]);

        // Create 4 vertices for the rectangle
        render::Vertex2D vertices[4];
        vertices[0] = render::Vertex2D(x, y, 0.0f, 0.0f, color.x, color.y, color.z, color.w);
        vertices[1] = render::Vertex2D(x + w, y, 1.0f, 0.0f, color.x, color.y, color.z, color.w);
        vertices[2] = render::Vertex2D(x + w, y + h, 1.0f, 1.0f, color.x, color.y, color.z, color.w);
        vertices[3] = render::Vertex2D(x, y + h, 0.0f, 1.0f, color.x, color.y, color.z, color.w);

        // Create indices for 2 triangles (0,1,2) and (0,2,3)
        uint32_t indices[6] = {0, 1, 2, 0, 2, 3};

        // Add vertices and indices to DrawList
        auto& drawList = Context::instance().getDrawList();
        uint32_t vtxOffset = drawList.addVertices2D(vertices, 4);
        uint32_t idxOffset = drawList.addIndices(indices, 6);

        // Create draw command
        render::DrawCommand2D cmd;
        cmd.vertexOffset = vtxOffset;
        cmd.vertexCount = 4;
        cmd.indexOffset = idxOffset;
        cmd.indexCount = 6;
        cmd.primitiveType = render::PrimitiveType::Triangle;
        cmd.blendMode = static_cast<render::BlendMode>(state.blendMode);
        cmd.texture = nullptr;

        // Convert ofMatrix4x4 to simd_float4x4
        auto& m = state.currentMatrix;
        cmd.transform = simd_matrix(
            simd_make_float4(m(0,0), m(1,0), m(2,0), m(3,0)),
            simd_make_float4(m(0,1), m(1,1), m(2,1), m(3,1)),
            simd_make_float4(m(0,2), m(1,2), m(2,2), m(3,2)),
            simd_make_float4(m(0,3), m(1,3), m(2,3), m(3,3))
        );

        drawList.addCommand(cmd);
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

        // Draw 4 corner arcs using triangle fans
        uint32_t resolution = state.circleResolution / 4;
        simd_float4 color = colorToFloat4(state.currentColor[0], state.currentColor[1],
                                           state.currentColor[2], state.currentColor[3]);

        // Helper lambda to draw a quarter circle arc (filled)
        auto drawQuarterCircle = [&](float cx, float cy, float radius, float startAngle) {
            std::vector<render::Vertex2D> vertices;
            vertices.reserve(resolution + 2);

            // Center vertex
            vertices.push_back(render::Vertex2D(cx, cy, 0.5f, 0.5f, color.x, color.y, color.z, color.w));

            // Arc vertices
            for (uint32_t i = 0; i <= resolution; i++) {
                float angle = startAngle + (M_PI_2 * i) / resolution;
                float vx = cx + radius * std::cos(angle);
                float vy = cy + radius * std::sin(angle);
                float u = 0.5f + 0.5f * std::cos(angle);
                float v = 0.5f + 0.5f * std::sin(angle);
                vertices.push_back(render::Vertex2D(vx, vy, u, v, color.x, color.y, color.z, color.w));
            }

            // Create indices for triangle fan
            std::vector<uint32_t> indices;
            indices.reserve(resolution * 3);
            for (uint32_t i = 1; i <= resolution; i++) {
                indices.push_back(0);
                indices.push_back(i);
                indices.push_back(i + 1);
            }

            // Add to DrawList
            auto& drawList = Context::instance().getDrawList();
            uint32_t vtxOffset = drawList.addVertices2D(vertices);
            uint32_t idxOffset = drawList.addIndices(indices);

            render::DrawCommand2D cmd;
            cmd.vertexOffset = vtxOffset;
            cmd.vertexCount = static_cast<uint32_t>(vertices.size());
            cmd.indexOffset = idxOffset;
            cmd.indexCount = static_cast<uint32_t>(indices.size());
            cmd.primitiveType = render::PrimitiveType::Triangle;
            cmd.blendMode = static_cast<render::BlendMode>(state.blendMode);
            cmd.texture = nullptr;

            auto& m = state.currentMatrix;
            cmd.transform = simd_matrix(
                simd_make_float4(m(0,0), m(1,0), m(2,0), m(3,0)),
                simd_make_float4(m(0,1), m(1,1), m(2,1), m(3,1)),
                simd_make_float4(m(0,2), m(1,2), m(2,2), m(3,2)),
                simd_make_float4(m(0,3), m(1,3), m(2,3), m(3,3))
            );

            drawList.addCommand(cmd);
        };

        // Draw 4 corner arcs
        drawQuarterCircle(x1, y1, r, M_PI);          // Top-left
        drawQuarterCircle(x2, y1, r, M_PI + M_PI_2); // Top-right
        drawQuarterCircle(x2, y2, r, 0.0f);          // Bottom-right
        drawQuarterCircle(x1, y2, r, M_PI_2);        // Bottom-left
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

        // Draw corner arcs (quarter circles)
        uint32_t resolution = state.circleResolution / 4;

        // Helper lambda to draw a quarter circle arc (outline)
        auto drawQuarterCircleOutline = [&](float cx, float cy, float radius, float startAngle) {
            for (uint32_t i = 0; i < resolution; i++) {
                float angle1 = startAngle + (M_PI_2 * i) / resolution;
                float angle2 = startAngle + (M_PI_2 * (i + 1)) / resolution;

                float x1 = cx + radius * std::cos(angle1);
                float y1 = cy + radius * std::sin(angle1);
                float x2 = cx + radius * std::cos(angle2);
                float y2 = cy + radius * std::sin(angle2);

                ofDrawLine(x1, y1, x2, y2);
            }
        };

        // Draw 4 corner arcs
        drawQuarterCircleOutline(x1, y1, r, M_PI);          // Top-left
        drawQuarterCircleOutline(x2, y1, r, M_PI + M_PI_2); // Top-right
        drawQuarterCircleOutline(x2, y2, r, 0.0f);          // Bottom-right
        drawQuarterCircleOutline(x1, y2, r, M_PI_2);        // Bottom-left
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
        simd_float4 color = colorToFloat4(state.currentColor[0], state.currentColor[1],
                                           state.currentColor[2], state.currentColor[3]);

        // Create vertices: center + perimeter
        std::vector<render::Vertex2D> vertices;
        vertices.reserve(resolution + 2);

        // Center vertex
        vertices.push_back(render::Vertex2D(x, y, 0.5f, 0.5f, color.x, color.y, color.z, color.w));

        // Perimeter vertices
        for (uint32_t i = 0; i <= resolution; i++) {
            float angle = (2.0f * M_PI * i) / resolution;
            float vx = x + radius * std::cos(angle);
            float vy = y + radius * std::sin(angle);
            // Texture coordinates map to circle (for potential texture mapping)
            float u = 0.5f + 0.5f * std::cos(angle);
            float v = 0.5f + 0.5f * std::sin(angle);
            vertices.push_back(render::Vertex2D(vx, vy, u, v, color.x, color.y, color.z, color.w));
        }

        // Create indices for triangle fan
        std::vector<uint32_t> indices;
        indices.reserve(resolution * 3);
        for (uint32_t i = 1; i <= resolution; i++) {
            indices.push_back(0);       // Center
            indices.push_back(i);       // Current perimeter point
            indices.push_back(i + 1);   // Next perimeter point
        }

        // Add vertices and indices to DrawList
        auto& drawList = Context::instance().getDrawList();
        uint32_t vtxOffset = drawList.addVertices2D(vertices);
        uint32_t idxOffset = drawList.addIndices(indices);

        // Create draw command
        render::DrawCommand2D cmd;
        cmd.vertexOffset = vtxOffset;
        cmd.vertexCount = static_cast<uint32_t>(vertices.size());
        cmd.indexOffset = idxOffset;
        cmd.indexCount = static_cast<uint32_t>(indices.size());
        cmd.primitiveType = render::PrimitiveType::Triangle;
        cmd.blendMode = static_cast<render::BlendMode>(state.blendMode);
        cmd.texture = nullptr;

        // Convert ofMatrix4x4 to simd_float4x4
        auto& m = state.currentMatrix;
        cmd.transform = simd_matrix(
            simd_make_float4(m(0,0), m(1,0), m(2,0), m(3,0)),
            simd_make_float4(m(0,1), m(1,1), m(2,1), m(3,1)),
            simd_make_float4(m(0,2), m(1,2), m(2,2), m(3,2)),
            simd_make_float4(m(0,3), m(1,3), m(2,3), m(3,3))
        );

        drawList.addCommand(cmd);
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
        simd_float4 color = colorToFloat4(state.currentColor[0], state.currentColor[1],
                                           state.currentColor[2], state.currentColor[3]);

        // Create vertices: center + perimeter
        std::vector<render::Vertex2D> vertices;
        vertices.reserve(resolution + 2);

        // Center vertex
        vertices.push_back(render::Vertex2D(x, y, 0.5f, 0.5f, color.x, color.y, color.z, color.w));

        // Perimeter vertices
        for (uint32_t i = 0; i <= resolution; i++) {
            float angle = (2.0f * M_PI * i) / resolution;
            float vx = x + radiusX * std::cos(angle);
            float vy = y + radiusY * std::sin(angle);
            // Texture coordinates map to ellipse
            float u = 0.5f + 0.5f * std::cos(angle);
            float v = 0.5f + 0.5f * std::sin(angle);
            vertices.push_back(render::Vertex2D(vx, vy, u, v, color.x, color.y, color.z, color.w));
        }

        // Create indices for triangle fan
        std::vector<uint32_t> indices;
        indices.reserve(resolution * 3);
        for (uint32_t i = 1; i <= resolution; i++) {
            indices.push_back(0);       // Center
            indices.push_back(i);       // Current perimeter point
            indices.push_back(i + 1);   // Next perimeter point
        }

        // Add vertices and indices to DrawList
        auto& drawList = Context::instance().getDrawList();
        uint32_t vtxOffset = drawList.addVertices2D(vertices);
        uint32_t idxOffset = drawList.addIndices(indices);

        // Create draw command
        render::DrawCommand2D cmd;
        cmd.vertexOffset = vtxOffset;
        cmd.vertexCount = static_cast<uint32_t>(vertices.size());
        cmd.indexOffset = idxOffset;
        cmd.indexCount = static_cast<uint32_t>(indices.size());
        cmd.primitiveType = render::PrimitiveType::Triangle;
        cmd.blendMode = static_cast<render::BlendMode>(state.blendMode);
        cmd.texture = nullptr;

        // Convert ofMatrix4x4 to simd_float4x4
        auto& m = state.currentMatrix;
        cmd.transform = simd_matrix(
            simd_make_float4(m(0,0), m(1,0), m(2,0), m(3,0)),
            simd_make_float4(m(0,1), m(1,1), m(2,1), m(3,1)),
            simd_make_float4(m(0,2), m(1,2), m(2,2), m(3,2)),
            simd_make_float4(m(0,3), m(1,3), m(2,3), m(3,3))
        );

        drawList.addCommand(cmd);
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
        simd_float4 color = colorToFloat4(state.currentColor[0], state.currentColor[1],
                                           state.currentColor[2], state.currentColor[3]);

        // Create 3 vertices for the triangle
        render::Vertex2D vertices[3];
        vertices[0] = render::Vertex2D(x1, y1, 0.0f, 0.0f, color.x, color.y, color.z, color.w);
        vertices[1] = render::Vertex2D(x2, y2, 1.0f, 0.0f, color.x, color.y, color.z, color.w);
        vertices[2] = render::Vertex2D(x3, y3, 0.5f, 1.0f, color.x, color.y, color.z, color.w);

        // Add vertices to DrawList
        auto& drawList = Context::instance().getDrawList();
        uint32_t vtxOffset = drawList.addVertices2D(vertices, 3);

        // Create draw command (no indices needed for a single triangle)
        render::DrawCommand2D cmd;
        cmd.vertexOffset = vtxOffset;
        cmd.vertexCount = 3;
        cmd.indexOffset = 0;
        cmd.indexCount = 0;  // No indices
        cmd.primitiveType = render::PrimitiveType::Triangle;
        cmd.blendMode = static_cast<render::BlendMode>(state.blendMode);
        cmd.texture = nullptr;

        // Convert ofMatrix4x4 to simd_float4x4
        auto& m = state.currentMatrix;
        cmd.transform = simd_matrix(
            simd_make_float4(m(0,0), m(1,0), m(2,0), m(3,0)),
            simd_make_float4(m(0,1), m(1,1), m(2,1), m(3,1)),
            simd_make_float4(m(0,2), m(1,2), m(2,2), m(3,2)),
            simd_make_float4(m(0,3), m(1,3), m(2,3), m(3,3))
        );

        drawList.addCommand(cmd);
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

// ============================================================================
// Shape API (Immediate Mode)
// ============================================================================

void ofBeginShape() {
    auto& state = getGraphicsState();

    // Clear any previous shape data
    state.shapeContours.clear();
    state.currentContour.clear();
    state.shapeBegun = true;
}

void ofEndShape(bool close) {
    auto& state = getGraphicsState();

    if (!state.shapeBegun) {
        // No shape was begun, silently return
        return;
    }

    // Add the current contour if it has vertices
    if (!state.currentContour.empty()) {
        state.shapeContours.push_back(state.currentContour);
    }

    // If no contours were created, nothing to draw
    if (state.shapeContours.empty()) {
        state.shapeBegun = false;
        return;
    }

    // Use ofPath to render the shape
    oflike::ofPath path;

    // Set rendering properties
    path.setFilled(state.fillEnabled);
    path.setStrokeWidth(state.lineWidth);
    path.setColor(state.currentColor[0], state.currentColor[1],
                  state.currentColor[2], state.currentColor[3]);

    // Process each contour
    for (size_t contourIdx = 0; contourIdx < state.shapeContours.size(); ++contourIdx) {
        const auto& contour = state.shapeContours[contourIdx];

        if (contour.empty()) {
            continue;
        }

        // Start new contour (for holes support)
        if (contourIdx > 0) {
            // Note: ofPath doesn't have nextContour, but we can work around this
            // by drawing each contour separately for now
            // TODO: Implement proper multi-contour support with holes
        }

        // Move to first vertex
        path.moveTo(contour[0].position);

        // Process vertices based on their type
        size_t i = 1;
        while (i < contour.size()) {
            const auto& vertex = contour[i];

            switch (vertex.type) {
                case ShapeVertexType::Vertex:
                    // Regular linear vertex
                    path.lineTo(vertex.position);
                    i++;
                    break;

                case ShapeVertexType::CurveVertex:
                    // Catmull-Rom curve vertex
                    // Need at least 4 vertices for a Catmull-Rom spline
                    if (i >= 1 && i + 1 < contour.size()) {
                        path.curveTo(vertex.position);
                    } else {
                        // Not enough points, fall back to line
                        path.lineTo(vertex.position);
                    }
                    i++;
                    break;

                case ShapeVertexType::BezierVertex:
                    // Bezier vertex: need 3 vertices (cp1, cp2, endpoint)
                    if (i + 2 < contour.size()) {
                        const auto& cp1 = contour[i].position;
                        const auto& cp2 = contour[i + 1].position;
                        const auto& endpoint = contour[i + 2].position;

                        path.bezierTo(cp1, cp2, endpoint);
                        i += 3;  // Skip all 3 vertices
                    } else {
                        // Not enough points for Bezier, fall back to line
                        path.lineTo(vertex.position);
                        i++;
                    }
                    break;
            }
        }

        // Close the contour if requested
        if (close) {
            path.close();
        }
    }

    // Draw the completed path
    path.draw();

    // Clear shape state
    state.shapeBegun = false;
    state.shapeContours.clear();
    state.currentContour.clear();
}

void ofVertex(float x, float y) {
    ofVertex(x, y, 0.0f);
}

void ofVertex(float x, float y, float z) {
    auto& state = getGraphicsState();

    if (!state.shapeBegun) {
        // Shape not begun, silently return
        return;
    }

    ShapeVertex vertex;
    vertex.position = oflike::ofVec3f(x, y, z);
    vertex.type = ShapeVertexType::Vertex;

    state.currentContour.push_back(vertex);
}

void ofCurveVertex(float x, float y) {
    ofCurveVertex(x, y, 0.0f);
}

void ofCurveVertex(float x, float y, float z) {
    auto& state = getGraphicsState();

    if (!state.shapeBegun) {
        // Shape not begun, silently return
        return;
    }

    ShapeVertex vertex;
    vertex.position = oflike::ofVec3f(x, y, z);
    vertex.type = ShapeVertexType::CurveVertex;

    state.currentContour.push_back(vertex);
}

void ofBezierVertex(float cx1, float cy1, float cx2, float cy2, float x, float y) {
    ofBezierVertex(cx1, cy1, 0.0f, cx2, cy2, 0.0f, x, y, 0.0f);
}

void ofBezierVertex(float cx1, float cy1, float cz1, float cx2, float cy2, float cz2, float x, float y, float z) {
    auto& state = getGraphicsState();

    if (!state.shapeBegun) {
        // Shape not begun, silently return
        return;
    }

    // Add three vertices: two control points and the endpoint
    // All marked as BezierVertex type so ofEndShape knows to group them
    ShapeVertex v1, v2, v3;
    v1.position = oflike::ofVec3f(cx1, cy1, cz1);
    v1.type = ShapeVertexType::BezierVertex;

    v2.position = oflike::ofVec3f(cx2, cy2, cz2);
    v2.type = ShapeVertexType::BezierVertex;

    v3.position = oflike::ofVec3f(x, y, z);
    v3.type = ShapeVertexType::BezierVertex;

    state.currentContour.push_back(v1);
    state.currentContour.push_back(v2);
    state.currentContour.push_back(v3);
}

void ofNextContour() {
    auto& state = getGraphicsState();

    if (!state.shapeBegun) {
        // Shape not begun, silently return
        return;
    }

    // Finalize current contour and start a new one
    if (!state.currentContour.empty()) {
        state.shapeContours.push_back(state.currentContour);
        state.currentContour.clear();
    }
}

// ============================================================================
// Blend Mode
// ============================================================================

void ofEnableAlphaBlending() {
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
}

void ofDisableAlphaBlending() {
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
}

void ofEnableBlendMode(int blendMode) {
    auto& state = getGraphicsState();

    // Clamp blend mode to valid range
    if (blendMode < OF_BLENDMODE_DISABLED || blendMode > OF_BLENDMODE_SCREEN) {
        blendMode = OF_BLENDMODE_ALPHA;  // Default to alpha if invalid
    }

    state.blendMode = blendMode;

    // Apply to renderer if context is initialized
    auto* renderer = ctx().renderer();
    if (renderer) {
        // Convert oflike blend mode constant to render::BlendMode enum
        render::BlendMode renderBlendMode = static_cast<render::BlendMode>(blendMode);
        renderer->setBlendMode(renderBlendMode);
    }
}

// ============================================================================
// 3D Primitives Implementation
// ============================================================================

void ofDrawBox(float x, float y, float z, float size) {
    ofDrawBox(x, y, z, size, size, size);
}

void ofDrawBox(float x, float y, float z, float width, float height, float depth) {
    auto& state = getGraphicsState();

    float hw = width / 2.0f;
    float hh = height / 2.0f;
    float hd = depth / 2.0f;

    simd_float4 color = colorToFloat4(state.currentColor[0], state.currentColor[1],
                                       state.currentColor[2], state.currentColor[3]);

    // Box has 6 faces, each with 4 vertices (24 vertices total for proper normals)
    // Each face has 2 triangles (6 indices per face, 36 indices total)

    std::vector<render::Vertex3D> vertices;
    std::vector<uint32_t> indices;
    vertices.reserve(24);
    indices.reserve(36);

    auto addFace = [&](simd_float3 p0, simd_float3 p1, simd_float3 p2, simd_float3 p3, simd_float3 normal) {
        uint32_t baseIdx = static_cast<uint32_t>(vertices.size());

        // Add 4 vertices for this face
        vertices.push_back(render::Vertex3D(p0.x, p0.y, p0.z, normal.x, normal.y, normal.z, 0, 0, color.x, color.y, color.z, color.w));
        vertices.push_back(render::Vertex3D(p1.x, p1.y, p1.z, normal.x, normal.y, normal.z, 1, 0, color.x, color.y, color.z, color.w));
        vertices.push_back(render::Vertex3D(p2.x, p2.y, p2.z, normal.x, normal.y, normal.z, 1, 1, color.x, color.y, color.z, color.w));
        vertices.push_back(render::Vertex3D(p3.x, p3.y, p3.z, normal.x, normal.y, normal.z, 0, 1, color.x, color.y, color.z, color.w));

        // Add 6 indices for 2 triangles (CCW winding)
        indices.push_back(baseIdx + 0);
        indices.push_back(baseIdx + 1);
        indices.push_back(baseIdx + 2);
        indices.push_back(baseIdx + 0);
        indices.push_back(baseIdx + 2);
        indices.push_back(baseIdx + 3);
    };

    // Define box corners (centered at x, y, z)
    // Front face (z+)
    addFace(simd_make_float3(x - hw, y - hh, z + hd),
            simd_make_float3(x + hw, y - hh, z + hd),
            simd_make_float3(x + hw, y + hh, z + hd),
            simd_make_float3(x - hw, y + hh, z + hd),
            simd_make_float3(0, 0, 1));

    // Back face (z-)
    addFace(simd_make_float3(x + hw, y - hh, z - hd),
            simd_make_float3(x - hw, y - hh, z - hd),
            simd_make_float3(x - hw, y + hh, z - hd),
            simd_make_float3(x + hw, y + hh, z - hd),
            simd_make_float3(0, 0, -1));

    // Top face (y+)
    addFace(simd_make_float3(x - hw, y + hh, z + hd),
            simd_make_float3(x + hw, y + hh, z + hd),
            simd_make_float3(x + hw, y + hh, z - hd),
            simd_make_float3(x - hw, y + hh, z - hd),
            simd_make_float3(0, 1, 0));

    // Bottom face (y-)
    addFace(simd_make_float3(x - hw, y - hh, z - hd),
            simd_make_float3(x + hw, y - hh, z - hd),
            simd_make_float3(x + hw, y - hh, z + hd),
            simd_make_float3(x - hw, y - hh, z + hd),
            simd_make_float3(0, -1, 0));

    // Right face (x+)
    addFace(simd_make_float3(x + hw, y - hh, z + hd),
            simd_make_float3(x + hw, y - hh, z - hd),
            simd_make_float3(x + hw, y + hh, z - hd),
            simd_make_float3(x + hw, y + hh, z + hd),
            simd_make_float3(1, 0, 0));

    // Left face (x-)
    addFace(simd_make_float3(x - hw, y - hh, z - hd),
            simd_make_float3(x - hw, y - hh, z + hd),
            simd_make_float3(x - hw, y + hh, z + hd),
            simd_make_float3(x - hw, y + hh, z - hd),
            simd_make_float3(-1, 0, 0));

    if (state.fillEnabled) {
        // Add vertices and indices to DrawList
        auto& drawList = Context::instance().getDrawList();
        uint32_t vtxOffset = drawList.addVertices3D(vertices.data(), vertices.size());
        uint32_t idxOffset = drawList.addIndices(indices.data(), indices.size());

        // Create 3D draw command
        render::DrawCommand3D cmd;
        cmd.vertexOffset = vtxOffset;
        cmd.vertexCount = static_cast<uint32_t>(vertices.size());
        cmd.indexOffset = idxOffset;
        cmd.indexCount = static_cast<uint32_t>(indices.size());
        cmd.primitiveType = render::PrimitiveType::Triangle;
        cmd.blendMode = static_cast<render::BlendMode>(state.blendMode);
        cmd.texture = nullptr;

        // Get view matrix from Context (set by camera)
        simd_float4x4 viewMatrix = Context::instance().getViewMatrix();

        // Model matrix from current transform
        auto& m = state.currentMatrix;
        simd_float4x4 modelMatrix = simd_matrix(
            simd_make_float4(m(0,0), m(1,0), m(2,0), m(3,0)),
            simd_make_float4(m(0,1), m(1,1), m(2,1), m(3,1)),
            simd_make_float4(m(0,2), m(1,2), m(2,2), m(3,2)),
            simd_make_float4(m(0,3), m(1,3), m(2,3), m(3,3))
        );

        // ModelView = View * Model
        cmd.modelViewMatrix = simd_mul(viewMatrix, modelMatrix);

        // Projection matrix from Context (set by camera)
        cmd.projectionMatrix = Context::instance().getProjectionMatrix();

        // Normal matrix (upper-left 3x3 of modelView)
        cmd.normalMatrix = simd_matrix(
            simd_make_float3(cmd.modelViewMatrix.columns[0].x, cmd.modelViewMatrix.columns[0].y, cmd.modelViewMatrix.columns[0].z),
            simd_make_float3(cmd.modelViewMatrix.columns[1].x, cmd.modelViewMatrix.columns[1].y, cmd.modelViewMatrix.columns[1].z),
            simd_make_float3(cmd.modelViewMatrix.columns[2].x, cmd.modelViewMatrix.columns[2].y, cmd.modelViewMatrix.columns[2].z)
        );

        cmd.depthTestEnabled = state.depthTestEnabled;
        cmd.depthWriteEnabled = state.depthWriteEnabled;
        cmd.cullBackFace = state.cullingEnabled;

        drawList.addCommand(cmd);
    } else {
        // Wireframe mode: draw 12 edges
        ofDrawLine(x - hw, y - hh, z + hd, x + hw, y - hh, z + hd);
        ofDrawLine(x + hw, y - hh, z + hd, x + hw, y + hh, z + hd);
        ofDrawLine(x + hw, y + hh, z + hd, x - hw, y + hh, z + hd);
        ofDrawLine(x - hw, y + hh, z + hd, x - hw, y - hh, z + hd);

        ofDrawLine(x - hw, y - hh, z - hd, x + hw, y - hh, z - hd);
        ofDrawLine(x + hw, y - hh, z - hd, x + hw, y + hh, z - hd);
        ofDrawLine(x + hw, y + hh, z - hd, x - hw, y + hh, z - hd);
        ofDrawLine(x - hw, y + hh, z - hd, x - hw, y - hh, z - hd);

        ofDrawLine(x - hw, y - hh, z + hd, x - hw, y - hh, z - hd);
        ofDrawLine(x + hw, y - hh, z + hd, x + hw, y - hh, z - hd);
        ofDrawLine(x + hw, y + hh, z + hd, x + hw, y + hh, z - hd);
        ofDrawLine(x - hw, y + hh, z + hd, x - hw, y + hh, z - hd);
    }
}

void ofDrawBox(float size) {
    ofDrawBox(0.0f, 0.0f, 0.0f, size, size, size);
}

// Helper function to submit 3D geometry
static void submit3DGeometry(const std::vector<render::Vertex3D>& vertices,
                             const std::vector<uint32_t>& indices) {
    auto& state = getGraphicsState();

    auto& drawList = Context::instance().getDrawList();
    uint32_t vtxOffset = drawList.addVertices3D(vertices.data(), vertices.size());
    uint32_t idxOffset = drawList.addIndices(indices.data(), indices.size());

    render::DrawCommand3D cmd;
    cmd.vertexOffset = vtxOffset;
    cmd.vertexCount = static_cast<uint32_t>(vertices.size());
    cmd.indexOffset = idxOffset;
    cmd.indexCount = static_cast<uint32_t>(indices.size());
    cmd.primitiveType = render::PrimitiveType::Triangle;
    cmd.blendMode = static_cast<render::BlendMode>(state.blendMode);
    cmd.texture = nullptr;

    // Get view matrix from Context
    simd_float4x4 viewMatrix = Context::instance().getViewMatrix();

    // Model matrix from current transform
    auto& m = state.currentMatrix;
    simd_float4x4 modelMatrix = simd_matrix(
        simd_make_float4(m(0,0), m(1,0), m(2,0), m(3,0)),
        simd_make_float4(m(0,1), m(1,1), m(2,1), m(3,1)),
        simd_make_float4(m(0,2), m(1,2), m(2,2), m(3,2)),
        simd_make_float4(m(0,3), m(1,3), m(2,3), m(3,3))
    );

    cmd.modelViewMatrix = simd_mul(viewMatrix, modelMatrix);
    cmd.projectionMatrix = Context::instance().getProjectionMatrix();

    cmd.normalMatrix = simd_matrix(
        simd_make_float3(cmd.modelViewMatrix.columns[0].x, cmd.modelViewMatrix.columns[0].y, cmd.modelViewMatrix.columns[0].z),
        simd_make_float3(cmd.modelViewMatrix.columns[1].x, cmd.modelViewMatrix.columns[1].y, cmd.modelViewMatrix.columns[1].z),
        simd_make_float3(cmd.modelViewMatrix.columns[2].x, cmd.modelViewMatrix.columns[2].y, cmd.modelViewMatrix.columns[2].z)
    );

    cmd.depthTestEnabled = state.depthTestEnabled;
    cmd.depthWriteEnabled = state.depthWriteEnabled;
    cmd.cullBackFace = state.cullingEnabled;

    drawList.addCommand(cmd);
}

void ofDrawSphere(float x, float y, float z, float radius) {
    auto& state = getGraphicsState();
    uint32_t resolution = state.sphereResolution;
    if (resolution < 4) resolution = 4;

    simd_float4 color = colorToFloat4(state.currentColor[0], state.currentColor[1],
                                       state.currentColor[2], state.currentColor[3]);

    std::vector<render::Vertex3D> vertices;
    std::vector<uint32_t> indices;

    // UV sphere: latitude rings × longitude segments
    uint32_t latSegments = resolution;
    uint32_t lonSegments = resolution * 2;

    // Generate vertices
    for (uint32_t lat = 0; lat <= latSegments; ++lat) {
        float theta = lat * M_PI / latSegments;  // 0 to PI
        float sinTheta = std::sin(theta);
        float cosTheta = std::cos(theta);

        for (uint32_t lon = 0; lon <= lonSegments; ++lon) {
            float phi = lon * 2.0f * M_PI / lonSegments;  // 0 to 2*PI
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            // Normal (unit sphere)
            float nx = sinTheta * cosPhi;
            float ny = cosTheta;
            float nz = sinTheta * sinPhi;

            // Position
            float px = x + radius * nx;
            float py = y + radius * ny;
            float pz = z + radius * nz;

            // UV coordinates
            float u = (float)lon / lonSegments;
            float v = (float)lat / latSegments;

            vertices.push_back(render::Vertex3D(px, py, pz, nx, ny, nz, u, v,
                                                 color.x, color.y, color.z, color.w));
        }
    }

    // Generate indices
    for (uint32_t lat = 0; lat < latSegments; ++lat) {
        for (uint32_t lon = 0; lon < lonSegments; ++lon) {
            uint32_t first = lat * (lonSegments + 1) + lon;
            uint32_t second = first + lonSegments + 1;

            // Two triangles per quad
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    if (state.fillEnabled) {
        submit3DGeometry(vertices, indices);
    } else {
        // Wireframe: draw latitude and longitude lines
        for (uint32_t lat = 0; lat <= latSegments; ++lat) {
            float theta = lat * M_PI / latSegments;
            float r = radius * std::sin(theta);
            float py = y + radius * std::cos(theta);

            for (uint32_t lon = 0; lon < lonSegments; ++lon) {
                float phi1 = lon * 2.0f * M_PI / lonSegments;
                float phi2 = (lon + 1) * 2.0f * M_PI / lonSegments;

                float x1 = x + r * std::cos(phi1);
                float z1 = z + r * std::sin(phi1);
                float x2 = x + r * std::cos(phi2);
                float z2 = z + r * std::sin(phi2);

                ofDrawLine(x1, py, z1, x2, py, z2);
            }
        }

        for (uint32_t lon = 0; lon < lonSegments; ++lon) {
            float phi = lon * 2.0f * M_PI / lonSegments;
            float cosPhi = std::cos(phi);
            float sinPhi = std::sin(phi);

            for (uint32_t lat = 0; lat < latSegments; ++lat) {
                float theta1 = lat * M_PI / latSegments;
                float theta2 = (lat + 1) * M_PI / latSegments;

                float x1 = x + radius * std::sin(theta1) * cosPhi;
                float y1 = y + radius * std::cos(theta1);
                float z1 = z + radius * std::sin(theta1) * sinPhi;

                float x2 = x + radius * std::sin(theta2) * cosPhi;
                float y2 = y + radius * std::cos(theta2);
                float z2 = z + radius * std::sin(theta2) * sinPhi;

                ofDrawLine(x1, y1, z1, x2, y2, z2);
            }
        }
    }
}

void ofDrawSphere(float radius) {
    ofDrawSphere(0.0f, 0.0f, 0.0f, radius);
}

void ofDrawCone(float x, float y, float z, float radius, float height) {
    auto& state = getGraphicsState();
    uint32_t resolution = state.circleResolution;
    if (resolution < 3) resolution = 3;

    simd_float4 color = colorToFloat4(state.currentColor[0], state.currentColor[1],
                                       state.currentColor[2], state.currentColor[3]);

    float halfHeight = height / 2.0f;
    float apexY = y + halfHeight;
    float baseY = y - halfHeight;

    std::vector<render::Vertex3D> vertices;
    std::vector<uint32_t> indices;

    if (state.fillEnabled) {
        // Apex vertex (index 0)
        vertices.push_back(render::Vertex3D(x, apexY, z, 0, 1, 0, 0.5f, 0,
                                             color.x, color.y, color.z, color.w));

        // Side vertices (base circle for cone side)
        float slopeAngle = std::atan2(radius, height);
        float normalY = std::cos(slopeAngle);
        float normalXZScale = std::sin(slopeAngle);

        for (uint32_t i = 0; i <= resolution; ++i) {
            float angle = i * 2.0f * M_PI / resolution;
            float cosA = std::cos(angle);
            float sinA = std::sin(angle);

            float px = x + radius * cosA;
            float pz = z + radius * sinA;

            // Normal points outward along the cone surface
            float nx = normalXZScale * cosA;
            float nz = normalXZScale * sinA;

            float u = (float)i / resolution;
            vertices.push_back(render::Vertex3D(px, baseY, pz, nx, normalY, nz, u, 1,
                                                 color.x, color.y, color.z, color.w));
        }

        // Side triangles (triangle fan from apex)
        for (uint32_t i = 0; i < resolution; ++i) {
            indices.push_back(0);           // Apex
            indices.push_back(i + 1);       // Current base vertex
            indices.push_back(i + 2);       // Next base vertex
        }

        // Base center vertex
        uint32_t baseCenterIdx = static_cast<uint32_t>(vertices.size());
        vertices.push_back(render::Vertex3D(x, baseY, z, 0, -1, 0, 0.5f, 0.5f,
                                             color.x, color.y, color.z, color.w));

        // Base circle vertices
        uint32_t baseStartIdx = static_cast<uint32_t>(vertices.size());
        for (uint32_t i = 0; i <= resolution; ++i) {
            float angle = i * 2.0f * M_PI / resolution;
            float cosA = std::cos(angle);
            float sinA = std::sin(angle);

            float px = x + radius * cosA;
            float pz = z + radius * sinA;

            vertices.push_back(render::Vertex3D(px, baseY, pz, 0, -1, 0,
                                                 0.5f + 0.5f * cosA, 0.5f + 0.5f * sinA,
                                                 color.x, color.y, color.z, color.w));
        }

        // Base triangles (reverse winding for bottom face)
        for (uint32_t i = 0; i < resolution; ++i) {
            indices.push_back(baseCenterIdx);
            indices.push_back(baseStartIdx + i + 1);
            indices.push_back(baseStartIdx + i);
        }

        submit3DGeometry(vertices, indices);
    } else {
        // Wireframe mode
        // Draw base circle
        for (uint32_t i = 0; i < resolution; ++i) {
            float angle1 = i * 2.0f * M_PI / resolution;
            float angle2 = (i + 1) * 2.0f * M_PI / resolution;

            float x1 = x + radius * std::cos(angle1);
            float z1 = z + radius * std::sin(angle1);
            float x2 = x + radius * std::cos(angle2);
            float z2 = z + radius * std::sin(angle2);

            ofDrawLine(x1, baseY, z1, x2, baseY, z2);
        }

        // Draw lines from apex to base
        for (uint32_t i = 0; i < resolution; ++i) {
            float angle = i * 2.0f * M_PI / resolution;
            float bx = x + radius * std::cos(angle);
            float bz = z + radius * std::sin(angle);

            ofDrawLine(x, apexY, z, bx, baseY, bz);
        }
    }
}

void ofDrawCone(float radius, float height) {
    ofDrawCone(0.0f, 0.0f, 0.0f, radius, height);
}

void ofDrawCylinder(float x, float y, float z, float radius, float height) {
    auto& state = getGraphicsState();
    uint32_t resolution = state.circleResolution;
    if (resolution < 3) resolution = 3;

    simd_float4 color = colorToFloat4(state.currentColor[0], state.currentColor[1],
                                       state.currentColor[2], state.currentColor[3]);

    float halfHeight = height / 2.0f;
    float topY = y + halfHeight;
    float bottomY = y - halfHeight;

    std::vector<render::Vertex3D> vertices;
    std::vector<uint32_t> indices;

    if (state.fillEnabled) {
        // Side vertices (two rings)
        for (uint32_t i = 0; i <= resolution; ++i) {
            float angle = i * 2.0f * M_PI / resolution;
            float cosA = std::cos(angle);
            float sinA = std::sin(angle);

            float px = x + radius * cosA;
            float pz = z + radius * sinA;
            float u = (float)i / resolution;

            // Top ring vertex (side)
            vertices.push_back(render::Vertex3D(px, topY, pz, cosA, 0, sinA, u, 0,
                                                 color.x, color.y, color.z, color.w));
            // Bottom ring vertex (side)
            vertices.push_back(render::Vertex3D(px, bottomY, pz, cosA, 0, sinA, u, 1,
                                                 color.x, color.y, color.z, color.w));
        }

        // Side quads
        for (uint32_t i = 0; i < resolution; ++i) {
            uint32_t topLeft = i * 2;
            uint32_t bottomLeft = i * 2 + 1;
            uint32_t topRight = (i + 1) * 2;
            uint32_t bottomRight = (i + 1) * 2 + 1;

            // First triangle
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            // Second triangle
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }

        // Top cap
        uint32_t topCenterIdx = static_cast<uint32_t>(vertices.size());
        vertices.push_back(render::Vertex3D(x, topY, z, 0, 1, 0, 0.5f, 0.5f,
                                             color.x, color.y, color.z, color.w));

        uint32_t topCapStart = static_cast<uint32_t>(vertices.size());
        for (uint32_t i = 0; i <= resolution; ++i) {
            float angle = i * 2.0f * M_PI / resolution;
            float cosA = std::cos(angle);
            float sinA = std::sin(angle);

            float px = x + radius * cosA;
            float pz = z + radius * sinA;

            vertices.push_back(render::Vertex3D(px, topY, pz, 0, 1, 0,
                                                 0.5f + 0.5f * cosA, 0.5f + 0.5f * sinA,
                                                 color.x, color.y, color.z, color.w));
        }

        for (uint32_t i = 0; i < resolution; ++i) {
            indices.push_back(topCenterIdx);
            indices.push_back(topCapStart + i);
            indices.push_back(topCapStart + i + 1);
        }

        // Bottom cap
        uint32_t bottomCenterIdx = static_cast<uint32_t>(vertices.size());
        vertices.push_back(render::Vertex3D(x, bottomY, z, 0, -1, 0, 0.5f, 0.5f,
                                             color.x, color.y, color.z, color.w));

        uint32_t bottomCapStart = static_cast<uint32_t>(vertices.size());
        for (uint32_t i = 0; i <= resolution; ++i) {
            float angle = i * 2.0f * M_PI / resolution;
            float cosA = std::cos(angle);
            float sinA = std::sin(angle);

            float px = x + radius * cosA;
            float pz = z + radius * sinA;

            vertices.push_back(render::Vertex3D(px, bottomY, pz, 0, -1, 0,
                                                 0.5f + 0.5f * cosA, 0.5f + 0.5f * sinA,
                                                 color.x, color.y, color.z, color.w));
        }

        for (uint32_t i = 0; i < resolution; ++i) {
            indices.push_back(bottomCenterIdx);
            indices.push_back(bottomCapStart + i + 1);
            indices.push_back(bottomCapStart + i);
        }

        submit3DGeometry(vertices, indices);
    } else {
        // Wireframe mode
        // Draw top and bottom circles
        for (uint32_t i = 0; i < resolution; ++i) {
            float angle1 = i * 2.0f * M_PI / resolution;
            float angle2 = (i + 1) * 2.0f * M_PI / resolution;

            float x1 = x + radius * std::cos(angle1);
            float z1 = z + radius * std::sin(angle1);
            float x2 = x + radius * std::cos(angle2);
            float z2 = z + radius * std::sin(angle2);

            ofDrawLine(x1, topY, z1, x2, topY, z2);
            ofDrawLine(x1, bottomY, z1, x2, bottomY, z2);
        }

        // Draw vertical lines
        for (uint32_t i = 0; i < resolution; ++i) {
            float angle = i * 2.0f * M_PI / resolution;
            float px = x + radius * std::cos(angle);
            float pz = z + radius * std::sin(angle);

            ofDrawLine(px, topY, pz, px, bottomY, pz);
        }
    }
}

void ofDrawCylinder(float radius, float height) {
    ofDrawCylinder(0.0f, 0.0f, 0.0f, radius, height);
}

void ofDrawPlane(float x, float y, float z, float width, float height) {
    auto& state = getGraphicsState();

    simd_float4 color = colorToFloat4(state.currentColor[0], state.currentColor[1],
                                       state.currentColor[2], state.currentColor[3]);

    float hw = width / 2.0f;
    float hh = height / 2.0f;

    if (state.fillEnabled) {
        std::vector<render::Vertex3D> vertices;
        std::vector<uint32_t> indices;

        // Four corners of the plane (XZ plane, Y is up)
        // Normal pointing up (+Y)
        vertices.push_back(render::Vertex3D(x - hw, y, z - hh, 0, 1, 0, 0, 0,
                                             color.x, color.y, color.z, color.w));
        vertices.push_back(render::Vertex3D(x + hw, y, z - hh, 0, 1, 0, 1, 0,
                                             color.x, color.y, color.z, color.w));
        vertices.push_back(render::Vertex3D(x + hw, y, z + hh, 0, 1, 0, 1, 1,
                                             color.x, color.y, color.z, color.w));
        vertices.push_back(render::Vertex3D(x - hw, y, z + hh, 0, 1, 0, 0, 1,
                                             color.x, color.y, color.z, color.w));

        // Two triangles
        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(2);
        indices.push_back(0);
        indices.push_back(2);
        indices.push_back(3);

        submit3DGeometry(vertices, indices);
    } else {
        // Wireframe: draw rectangle outline
        ofDrawLine(x - hw, y, z - hh, x + hw, y, z - hh);
        ofDrawLine(x + hw, y, z - hh, x + hw, y, z + hh);
        ofDrawLine(x + hw, y, z + hh, x - hw, y, z + hh);
        ofDrawLine(x - hw, y, z + hh, x - hw, y, z - hh);
    }
}

void ofDrawPlane(float width, float height) {
    ofDrawPlane(0.0f, 0.0f, 0.0f, width, height);
}

void ofDrawIcoSphere(float x, float y, float z, float radius, int subdivisions) {
    auto& state = getGraphicsState();

    simd_float4 color = colorToFloat4(state.currentColor[0], state.currentColor[1],
                                       state.currentColor[2], state.currentColor[3]);

    if (subdivisions < 0) subdivisions = 0;
    if (subdivisions > 5) subdivisions = 5;  // Limit to prevent excessive geometry

    // Golden ratio
    const float t = (1.0f + std::sqrt(5.0f)) / 2.0f;

    // Icosahedron vertices (normalized)
    std::vector<simd_float3> icoVertices = {
        simd_normalize(simd_make_float3(-1,  t,  0)),
        simd_normalize(simd_make_float3( 1,  t,  0)),
        simd_normalize(simd_make_float3(-1, -t,  0)),
        simd_normalize(simd_make_float3( 1, -t,  0)),
        simd_normalize(simd_make_float3( 0, -1,  t)),
        simd_normalize(simd_make_float3( 0,  1,  t)),
        simd_normalize(simd_make_float3( 0, -1, -t)),
        simd_normalize(simd_make_float3( 0,  1, -t)),
        simd_normalize(simd_make_float3( t,  0, -1)),
        simd_normalize(simd_make_float3( t,  0,  1)),
        simd_normalize(simd_make_float3(-t,  0, -1)),
        simd_normalize(simd_make_float3(-t,  0,  1))
    };

    // Icosahedron faces (20 triangles)
    std::vector<std::array<uint32_t, 3>> faces = {
        {0, 11, 5}, {0, 5, 1}, {0, 1, 7}, {0, 7, 10}, {0, 10, 11},
        {1, 5, 9}, {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
        {3, 9, 4}, {3, 4, 2}, {3, 2, 6}, {3, 6, 8}, {3, 8, 9},
        {4, 9, 5}, {2, 4, 11}, {6, 2, 10}, {8, 6, 7}, {9, 8, 1}
    };

    // Subdivide
    for (int s = 0; s < subdivisions; ++s) {
        std::vector<std::array<uint32_t, 3>> newFaces;
        std::map<std::pair<uint32_t, uint32_t>, uint32_t> midpointCache;

        auto getMidpoint = [&](uint32_t i1, uint32_t i2) -> uint32_t {
            auto key = std::make_pair(std::min(i1, i2), std::max(i1, i2));
            auto it = midpointCache.find(key);
            if (it != midpointCache.end()) {
                return it->second;
            }

            simd_float3 mid = simd_normalize((icoVertices[i1] + icoVertices[i2]) * 0.5f);
            uint32_t idx = static_cast<uint32_t>(icoVertices.size());
            icoVertices.push_back(mid);
            midpointCache[key] = idx;
            return idx;
        };

        for (const auto& face : faces) {
            uint32_t a = getMidpoint(face[0], face[1]);
            uint32_t b = getMidpoint(face[1], face[2]);
            uint32_t c = getMidpoint(face[2], face[0]);

            newFaces.push_back({face[0], a, c});
            newFaces.push_back({face[1], b, a});
            newFaces.push_back({face[2], c, b});
            newFaces.push_back({a, b, c});
        }

        faces = std::move(newFaces);
    }

    // Generate final vertices and indices
    std::vector<render::Vertex3D> vertices;
    std::vector<uint32_t> indices;

    for (const auto& v : icoVertices) {
        float px = x + radius * v.x;
        float py = y + radius * v.y;
        float pz = z + radius * v.z;

        // UV mapping (spherical)
        float u = 0.5f + std::atan2(v.z, v.x) / (2.0f * M_PI);
        float vCoord = 0.5f - std::asin(v.y) / M_PI;

        vertices.push_back(render::Vertex3D(px, py, pz, v.x, v.y, v.z, u, vCoord,
                                             color.x, color.y, color.z, color.w));
    }

    for (const auto& face : faces) {
        indices.push_back(face[0]);
        indices.push_back(face[1]);
        indices.push_back(face[2]);
    }

    if (state.fillEnabled) {
        submit3DGeometry(vertices, indices);
    } else {
        // Wireframe: draw edges of each triangle
        for (const auto& face : faces) {
            const auto& v0 = vertices[face[0]];
            const auto& v1 = vertices[face[1]];
            const auto& v2 = vertices[face[2]];

            ofDrawLine(v0.position.x, v0.position.y, v0.position.z,
                       v1.position.x, v1.position.y, v1.position.z);
            ofDrawLine(v1.position.x, v1.position.y, v1.position.z,
                       v2.position.x, v2.position.y, v2.position.z);
            ofDrawLine(v2.position.x, v2.position.y, v2.position.z,
                       v0.position.x, v0.position.y, v0.position.z);
        }
    }
}

void ofDrawIcoSphere(float radius, int subdivisions) {
    ofDrawIcoSphere(0.0f, 0.0f, 0.0f, radius, subdivisions);
}

// ============================================================================
// 3D Helper Functions Implementation
// ============================================================================

void ofDrawAxis(float size) {
    // TODO: Implement 3D axis drawing
    // Draw three lines from origin:
    // - X axis: Red line from (0,0,0) to (size,0,0)
    // - Y axis: Green line from (0,0,0) to (0,size,0)
    // - Z axis: Blue line from (0,0,0) to (0,0,size)

    // Save current color
    auto& state = getGraphicsState();
    uint8_t savedColor[4];
    std::copy(state.currentColor, state.currentColor + 4, savedColor);

    // X axis - red
    ofSetColor(255, 0, 0);
    ofDrawLine(0.0f, 0.0f, 0.0f, size, 0.0f, 0.0f);

    // Y axis - green
    ofSetColor(0, 255, 0);
    ofDrawLine(0.0f, 0.0f, 0.0f, 0.0f, size, 0.0f);

    // Z axis - blue
    ofSetColor(0, 0, 255);
    ofDrawLine(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, size);

    // Restore color
    std::copy(savedColor, savedColor + 4, state.currentColor);
}

void ofDrawGrid(float stepSize, size_t numberOfSteps, bool labels, bool x, bool y, bool z) {
    // TODO: Implement 3D grid drawing on XZ plane
    // Grid extends from (-width/2, 0, -height/2) to (width/2, 0, height/2)
    // where width = height = stepSize * numberOfSteps * 2

    (void)stepSize; (void)numberOfSteps;
    (void)labels; (void)x; (void)y; (void)z;
}

void ofDrawGrid(float stepSize, size_t numberOfSteps, bool labels) {
    ofDrawGrid(stepSize, numberOfSteps, labels, true, true, true);
}

void ofDrawGrid(float x, float y, float z, float stepSize, size_t numberOfSteps, bool labels, bool xLines, bool yLines, bool zLines) {
    // TODO: Implement 3D grid drawing at position
    // Save current matrix, translate to (x,y,z), draw grid, restore matrix

    (void)x; (void)y; (void)z;
    (void)stepSize; (void)numberOfSteps;
    (void)labels; (void)xLines; (void)yLines; (void)zLines;
}

void ofDrawGridPlane(float width, float height, int columns, int rows) {
    // TODO: Implement simple grid plane on XZ
    // Draw a grid with specified width/height and column/row divisions
    // Grid lines parallel to X and Z axes

    float columnStep = width / columns;
    float rowStep = height / rows;

    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;

    // Lines parallel to X axis (along Z direction)
    for (int i = 0; i <= rows; i++) {
        float zPos = -halfHeight + i * rowStep;
        ofDrawLine(-halfWidth, 0.0f, zPos, halfWidth, 0.0f, zPos);
    }

    // Lines parallel to Z axis (along X direction)
    for (int i = 0; i <= columns; i++) {
        float xPos = -halfWidth + i * columnStep;
        ofDrawLine(xPos, 0.0f, -halfHeight, xPos, 0.0f, halfHeight);
    }
}

void ofDrawGridPlane(int columns, int rows) {
    ofDrawGridPlane(1.0f, 1.0f, columns, rows);
}

void ofDrawArrow(const oflike::ofVec3f& start, const oflike::ofVec3f& end, float headSize) {
    // TODO: Implement 3D arrow drawing
    // 1. Draw line from start to end
    // 2. Calculate direction vector: end - start
    // 3. Draw cone at end position pointing in direction
    // 4. Cone height = headSize, radius = headSize * 0.5

    // Draw shaft
    ofDrawLine(start.x, start.y, start.z, end.x, end.y, end.z);

    // Calculate direction for arrow head
    oflike::ofVec3f direction = end;
    direction.x -= start.x;
    direction.y -= start.y;
    direction.z -= start.z;

    // TODO: Normalize direction and position cone at end
    // For now, just draw a small cone at end position
    (void)direction;
    (void)headSize;

    // ofDrawCone(end.x, end.y, end.z, headSize * 0.5f, headSize);
}

void ofDrawRotationAxes(float radius, float stripWidth) {
    // TODO: Implement rotation axes visualization
    // Draw three colored arrows showing orientation:
    // - Red arrow: X axis direction (1,0,0) transformed by current matrix
    // - Green arrow: Y axis direction (0,1,0) transformed by current matrix
    // - Blue arrow: Z axis direction (0,0,1) transformed by current matrix

    (void)radius; (void)stripWidth;

    // For now, just draw standard axes
    ofDrawAxis(radius);
}

// ============================================================================
// Depth Testing & Culling Implementation
// ============================================================================

void ofEnableDepthTest() {
    auto& state = getGraphicsState();
    state.depthTestEnabled = true;

    // Update renderer state
    auto* renderer = ctx().renderer();
    if (renderer) {
        renderer->setDepthTestEnabled(true);
    }
}

void ofDisableDepthTest() {
    auto& state = getGraphicsState();
    state.depthTestEnabled = false;

    // Update renderer state
    auto* renderer = ctx().renderer();
    if (renderer) {
        renderer->setDepthTestEnabled(false);
    }
}

bool ofGetDepthTestEnabled() {
    auto& state = getGraphicsState();
    return state.depthTestEnabled;
}

void ofSetDepthWrite(bool enabled) {
    auto& state = getGraphicsState();
    state.depthWriteEnabled = enabled;

    // Update renderer state
    auto* renderer = ctx().renderer();
    if (renderer) {
        renderer->setDepthWriteEnabled(enabled);
    }
}

bool ofGetDepthWrite() {
    auto& state = getGraphicsState();
    return state.depthWriteEnabled;
}

void ofEnableCulling() {
    auto& state = getGraphicsState();
    state.cullingEnabled = true;

    // Update renderer state (backface culling)
    auto* renderer = ctx().renderer();
    if (renderer) {
        renderer->setCullingMode(true, true);  // cullBack=true, enabled=true
    }
}

void ofDisableCulling() {
    auto& state = getGraphicsState();
    state.cullingEnabled = false;

    // Update renderer state
    auto* renderer = ctx().renderer();
    if (renderer) {
        renderer->setCullingMode(false, false);  // cullBack doesn't matter, enabled=false
    }
}

// ============================================================================
// Lighting Implementation
// ============================================================================

void ofEnableLighting() {
    auto& state = getGraphicsState();
    state.lightingEnabled = true;

    // TODO: Update renderer state when lighting system is integrated
    // For now, just track the state
}

void ofDisableLighting() {
    auto& state = getGraphicsState();
    state.lightingEnabled = false;

    // TODO: Update renderer state when lighting system is integrated
}

bool ofGetLightingEnabled() {
    auto& state = getGraphicsState();
    return state.lightingEnabled;
}

void ofSetSmoothLighting(bool smooth) {
    auto& state = getGraphicsState();
    state.smoothLighting = smooth;

    // TODO: Update renderer state when shader integration is complete
    // This will switch between smooth (Phong) and flat shading
}
