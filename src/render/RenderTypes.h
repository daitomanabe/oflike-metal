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

    // Overlay blending: combines multiply and screen
    // Note: Hardware approximation using screen blend
    Overlay         = 7,

    // Soft light blending: subtle lightening/darkening
    // Note: Hardware approximation
    SoftLight       = 8,

    // Hard light blending: similar to overlay with swapped src/dst roles
    // Note: Hardware approximation using multiply blend
    HardLight       = 9,

    // Difference blending: abs(src - dst)
    // Note: Hardware approximation using max blend operation
    Difference      = 10,
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

// ============================================================================
// Texture Types (API-agnostic)
// ============================================================================

/// Texture wrap mode
enum class TextureWrap : uint32_t {
    Clamp   = 0,    // Clamp to edge
    Repeat  = 1,    // Repeat
    Mirror  = 2,    // Mirrored repeat
};

/// Texture filter mode
enum class TextureFilter : uint32_t {
    Nearest = 0,    // Nearest neighbor
    Linear  = 1,    // Linear interpolation
};

/// Texture format (API-agnostic)
enum class TextureFormat : uint32_t {
    R8       = 0,    // 8-bit single channel (grayscale)
    RGBA8    = 1,    // 8-bit RGBA
    R16F     = 2,    // 16-bit float single channel
    RGBA16F  = 3,    // 16-bit float RGBA
    R32F     = 4,    // 32-bit float single channel
    RGBA32F  = 5,    // 32-bit float RGBA
    R16      = 6,    // 16-bit unsigned integer single channel
    RGBA16   = 7,    // 16-bit unsigned integer RGBA
};

} // namespace render
