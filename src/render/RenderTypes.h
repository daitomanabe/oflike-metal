#pragma once

#include <simd/simd.h>
#include <cstdint>

namespace render {

// ============================================================================
// Vertex Structures
// ============================================================================

/// 2D Vertex with position, texture coordinates, and color
struct Vertex2D {
    simd_float2 position;   // 2D position (x, y)
    simd_float2 texCoord;   // Texture coordinates (u, v)
    simd_float4 color;      // RGBA color (0.0-1.0 range)

    Vertex2D()
        : position(simd_make_float2(0.0f, 0.0f))
        , texCoord(simd_make_float2(0.0f, 0.0f))
        , color(simd_make_float4(1.0f, 1.0f, 1.0f, 1.0f)) {}

    Vertex2D(simd_float2 pos, simd_float2 uv, simd_float4 col)
        : position(pos)
        , texCoord(uv)
        , color(col) {}

    Vertex2D(float x, float y, float u, float v, float r, float g, float b, float a)
        : position(simd_make_float2(x, y))
        , texCoord(simd_make_float2(u, v))
        , color(simd_make_float4(r, g, b, a)) {}
};

/// 3D Vertex with position, normal, texture coordinates, and color
struct Vertex3D {
    simd_float3 position;   // 3D position (x, y, z)
    simd_float3 normal;     // Normal vector
    simd_float2 texCoord;   // Texture coordinates (u, v)
    simd_float4 color;      // RGBA color (0.0-1.0 range)

    Vertex3D()
        : position(simd_make_float3(0.0f, 0.0f, 0.0f))
        , normal(simd_make_float3(0.0f, 0.0f, 1.0f))
        , texCoord(simd_make_float2(0.0f, 0.0f))
        , color(simd_make_float4(1.0f, 1.0f, 1.0f, 1.0f)) {}

    Vertex3D(simd_float3 pos, simd_float3 norm, simd_float2 uv, simd_float4 col)
        : position(pos)
        , normal(norm)
        , texCoord(uv)
        , color(col) {}

    Vertex3D(float x, float y, float z,
             float nx, float ny, float nz,
             float u, float v,
             float r, float g, float b, float a)
        : position(simd_make_float3(x, y, z))
        , normal(simd_make_float3(nx, ny, nz))
        , texCoord(simd_make_float2(u, v))
        , color(simd_make_float4(r, g, b, a)) {}
};

// ============================================================================
// Primitive Type
// ============================================================================

/// Primitive rendering type (compatible with Metal MTLPrimitiveType)
enum class PrimitiveType : uint32_t {
    Point           = 0,    // MTLPrimitiveTypePoint
    Line            = 1,    // MTLPrimitiveTypeLine
    LineStrip       = 2,    // MTLPrimitiveTypeLineStrip
    Triangle        = 3,    // MTLPrimitiveTypeTriangle
    TriangleStrip   = 4,    // MTLPrimitiveTypeTriangleStrip
};

// ============================================================================
// Blend Mode
// ============================================================================

/// Blend mode for alpha blending and compositing
enum class BlendMode : uint32_t {
    // No blending
    Disabled        = 0,

    // Standard alpha blending: src.rgb * src.a + dst.rgb * (1 - src.a)
    Alpha           = 1,

    // Additive blending: src.rgb + dst.rgb
    Add             = 2,

    // Subtractive blending: dst.rgb - src.rgb
    Subtract        = 3,

    // Multiply blending: src.rgb * dst.rgb
    Multiply        = 4,

    // Screen blending: 1 - (1 - src.rgb) * (1 - dst.rgb)
    Screen          = 5,

    // Premultiplied alpha: src.rgb + dst.rgb * (1 - src.a)
    // (assumes source RGB is already multiplied by alpha)
    PremultipliedAlpha = 6,
};

// ============================================================================
// Blend Mode Configuration
// ============================================================================

/// Blend configuration for Metal pipeline state
struct BlendConfig {
    bool blendingEnabled;

    // Metal blend factors (MTLBlendFactor)
    uint32_t sourceRGBBlendFactor;
    uint32_t destinationRGBBlendFactor;
    uint32_t rgbBlendOperation;

    uint32_t sourceAlphaBlendFactor;
    uint32_t destinationAlphaBlendFactor;
    uint32_t alphaBlendOperation;

    /// Get Metal blend configuration for a given blend mode
    static BlendConfig forMode(BlendMode mode);
};

} // namespace render
