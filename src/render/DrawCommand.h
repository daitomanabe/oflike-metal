#pragma once

#include "RenderTypes.h"
#include <simd/simd.h>
#include <cstdint>
#include <cstring>
#include <vector>

namespace render {

// ============================================================================
// Forward Declarations
// ============================================================================

class DrawList;

// ============================================================================
// Command Types
// ============================================================================

/// Type of draw command
enum class CommandType : uint32_t {
    // Drawing commands
    Draw2D,                 // Draw 2D vertices
    Draw3D,                 // Draw 3D vertices

    // State commands
    SetViewport,            // Set viewport rectangle
    SetScissor,             // Set scissor test rectangle
    Clear,                  // Clear render target
    SetRenderTarget,        // Change render target
    SetCustomShader,        // Use custom shader pipeline

    // State changes
    SetBlendMode,           // Change blend mode
    SetDepthTest,           // Enable/disable depth test
    SetCullMode,            // Change culling mode
};

// ============================================================================
// Command Data Structures
// ============================================================================

/// Rectangle for viewport/scissor
struct Rect {
    float x, y;             // Origin (top-left)
    float width, height;    // Dimensions

    Rect() : x(0), y(0), width(0), height(0) {}
    Rect(float x_, float y_, float w, float h)
        : x(x_), y(y_), width(w), height(h) {}
};

/// Clear color and flags
struct ClearCommand {
    simd_float4 color;      // Clear color (RGBA)
    float depth;            // Clear depth value (0.0-1.0)
    uint32_t stencil;       // Clear stencil value
    bool clearColor;        // Clear color buffer
    bool clearDepth;        // Clear depth buffer
    bool clearStencil;      // Clear stencil buffer

    ClearCommand()
        : color(simd_make_float4(0, 0, 0, 1))
        , depth(1.0f)
        , stencil(0)
        , clearColor(true)
        , clearDepth(false)
        , clearStencil(false) {}
};

// ============================================================================
// Draw Commands
// ============================================================================

/// 2D Draw command
struct DrawCommand2D {
    CommandType type = CommandType::Draw2D;

    // Vertex data
    uint32_t vertexOffset;      // Offset into vertex buffer
    uint32_t vertexCount;       // Number of vertices

    // Index data (optional)
    uint32_t indexOffset;       // Offset into index buffer
    uint32_t indexCount;        // Number of indices (0 = no indices)

    // Rendering state
    PrimitiveType primitiveType;
    BlendMode blendMode;

    // Texture (optional, nullptr = no texture)
    void* texture;              // id<MTLTexture> handle

    // Transformation matrix (2D projection + model-view)
    simd_float4x4 transform;

    DrawCommand2D()
        : vertexOffset(0)
        , vertexCount(0)
        , indexOffset(0)
        , indexCount(0)
        , primitiveType(PrimitiveType::Triangle)
        , blendMode(BlendMode::Alpha)
        , texture(nullptr)
        , transform(matrix_identity_float4x4) {}
};

/// 3D Draw command
struct DrawCommand3D {
    CommandType type = CommandType::Draw3D;

    // Vertex data
    uint32_t vertexOffset;      // Offset into vertex buffer
    uint32_t vertexCount;       // Number of vertices

    // Index data (optional)
    uint32_t indexOffset;       // Offset into index buffer
    uint32_t indexCount;        // Number of indices (0 = no indices)

    // Rendering state
    PrimitiveType primitiveType;
    BlendMode blendMode;

    // Texture (optional, nullptr = no texture)
    void* texture;              // id<MTLTexture> handle

    // 3D Matrices
    simd_float4x4 modelViewMatrix;      // Model-view transformation
    simd_float4x4 projectionMatrix;     // Projection transformation
    simd_float3x3 normalMatrix;         // Normal transformation (for lighting)

    // Depth and culling
    bool depthTestEnabled;
    bool depthWriteEnabled;
    bool cullBackFace;

    // Lighting state (captured at command creation time)
    bool useLighting;                   // Whether to use lighting pipeline
    int lightCount;                     // Number of lights
    float materialData[16];             // Material uniform data (13 floats + padding)
    float lightData[256];               // Light uniform data (up to 8 lights, 23 floats each)

    DrawCommand3D()
        : vertexOffset(0)
        , vertexCount(0)
        , indexOffset(0)
        , indexCount(0)
        , primitiveType(PrimitiveType::Triangle)
        , blendMode(BlendMode::Alpha)
        , texture(nullptr)
        , modelViewMatrix(matrix_identity_float4x4)
        , projectionMatrix(matrix_identity_float4x4)
        , normalMatrix(matrix_identity_float3x3)
        , depthTestEnabled(false)
        , depthWriteEnabled(false)
        , cullBackFace(false)
        , useLighting(false)
        , lightCount(0) {
        std::memset(materialData, 0, sizeof(materialData));
        std::memset(lightData, 0, sizeof(lightData));
    }
};

// ============================================================================
// State Commands
// ============================================================================

/// Viewport command
struct SetViewportCommand {
    CommandType type = CommandType::SetViewport;
    Rect viewport;

    SetViewportCommand() = default;
    explicit SetViewportCommand(const Rect& vp) : viewport(vp) {}
};

/// Scissor test command
struct SetScissorCommand {
    CommandType type = CommandType::SetScissor;
    Rect scissor;
    bool enabled;

    SetScissorCommand()
        : enabled(false) {}

    SetScissorCommand(const Rect& rect, bool enable)
        : scissor(rect), enabled(enable) {}
};

/// Clear command
struct SetClearCommand {
    CommandType type = CommandType::Clear;
    ClearCommand clearData;

    SetClearCommand() = default;
    explicit SetClearCommand(const ClearCommand& data) : clearData(data) {}
};

/// Render target command
struct SetRenderTargetCommand {
    CommandType type = CommandType::SetRenderTarget;
    void* renderTarget;         // id<MTLTexture> handle or nullptr for screen

    SetRenderTargetCommand()
        : renderTarget(nullptr) {}

    explicit SetRenderTargetCommand(void* target)
        : renderTarget(target) {}
};

/// Custom shader command
struct SetCustomShaderCommand {
    CommandType type = CommandType::SetCustomShader;
    void* pipelineState;        // id<MTLRenderPipelineState> handle or nullptr for default
    void* uniformData;          // Uniform buffer data
    size_t uniformSize;         // Size of uniform buffer
    void* textures[16];         // Texture bindings (max 16 texture units)

    SetCustomShaderCommand()
        : pipelineState(nullptr)
        , uniformData(nullptr)
        , uniformSize(0) {
        for (int i = 0; i < 16; ++i) {
            textures[i] = nullptr;
        }
    }
};

// ============================================================================
// Generic Command Container
// ============================================================================

/// Union-like container for all command types
/// This allows storing different command types in a single list
struct DrawCommand {
    CommandType type;

    union {
        DrawCommand2D draw2D;
        DrawCommand3D draw3D;
        SetViewportCommand viewport;
        SetScissorCommand scissor;
        SetClearCommand clear;
        SetRenderTargetCommand renderTarget;
        SetCustomShaderCommand customShader;
    };

    // Constructors for different command types
    DrawCommand() : type(CommandType::Draw2D), draw2D() {}

    explicit DrawCommand(const DrawCommand2D& cmd)
        : type(CommandType::Draw2D), draw2D(cmd) {}

    explicit DrawCommand(const DrawCommand3D& cmd)
        : type(CommandType::Draw3D), draw3D(cmd) {}

    explicit DrawCommand(const SetViewportCommand& cmd)
        : type(CommandType::SetViewport), viewport(cmd) {}

    explicit DrawCommand(const SetScissorCommand& cmd)
        : type(CommandType::SetScissor), scissor(cmd) {}

    explicit DrawCommand(const SetClearCommand& cmd)
        : type(CommandType::Clear), clear(cmd) {}

    explicit DrawCommand(const SetRenderTargetCommand& cmd)
        : type(CommandType::SetRenderTarget), renderTarget(cmd) {}

    explicit DrawCommand(const SetCustomShaderCommand& cmd)
        : type(CommandType::SetCustomShader), customShader(cmd) {}

    // Copy constructor and assignment
    DrawCommand(const DrawCommand& other);
    DrawCommand& operator=(const DrawCommand& other);

    // Destructor
    ~DrawCommand();
};

} // namespace render
