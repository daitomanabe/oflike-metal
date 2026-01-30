#include <metal_stdlib>
#include "Common.h"

using namespace metal;

// ============================================================================
// Programmable Blending Shaders
// Uses Metal's [[color(0)]] to read destination color for accurate blending
// Requires: Apple Silicon (M1+) or A11+ iOS devices
// ============================================================================

// MARK: - Vertex Shader (shared)

vertex RasterizerData2D vertex_blend(
    uint vertexID [[vertex_id]],
    constant Vertex2D* vertices [[buffer(0)]],
    constant Uniforms2D& uniforms [[buffer(1)]]
) {
    RasterizerData2D out;
    Vertex2D in = vertices[vertexID];

    float4 position = float4(in.position, 0.0, 1.0);
    out.position = uniforms.projectionMatrix * uniforms.modelViewMatrix * position;
    out.texCoord = in.texCoord;
    out.color = in.color;

    return out;
}

// MARK: - Helper Functions

/// Overlay blend: combines multiply and screen based on destination luminance
inline float3 blend_overlay(float3 src, float3 dst) {
    return mix(
        2.0 * src * dst,
        1.0 - 2.0 * (1.0 - src) * (1.0 - dst),
        step(0.5, dst)
    );
}

/// Soft light blend: subtle dodge/burn effect
inline float3 blend_soft_light(float3 src, float3 dst) {
    float3 result;
    // Pegtop formula (smoother than Photoshop)
    result = (1.0 - 2.0 * src) * dst * dst + 2.0 * src * dst;
    return result;
}

/// Hard light blend: overlay with src/dst swapped
inline float3 blend_hard_light(float3 src, float3 dst) {
    return mix(
        2.0 * src * dst,
        1.0 - 2.0 * (1.0 - src) * (1.0 - dst),
        step(0.5, src)  // Note: uses src, not dst (unlike overlay)
    );
}

/// Difference blend: absolute difference
inline float3 blend_difference(float3 src, float3 dst) {
    return abs(dst - src);
}

/// Exclusion blend: similar to difference but lower contrast
inline float3 blend_exclusion(float3 src, float3 dst) {
    return src + dst - 2.0 * src * dst;
}

// MARK: - Fragment Shaders with Programmable Blending

/// Overlay blend fragment shader
/// Uses [[color(0)]] to read current framebuffer color
fragment float4 fragment_blend_overlay(
    RasterizerData2D in [[stage_in]],
    float4 dst [[color(0)]]  // Current framebuffer color (programmable blending)
) {
    float4 src = in.color;

    // Apply overlay blend
    float3 blended = blend_overlay(src.rgb, dst.rgb);

    // Alpha compositing: blend result with alpha
    float3 result = mix(dst.rgb, blended, src.a);
    float alpha = src.a + dst.a * (1.0 - src.a);

    return float4(result, alpha);
}

/// Soft light blend fragment shader
fragment float4 fragment_blend_soft_light(
    RasterizerData2D in [[stage_in]],
    float4 dst [[color(0)]]
) {
    float4 src = in.color;

    float3 blended = blend_soft_light(src.rgb, dst.rgb);
    float3 result = mix(dst.rgb, blended, src.a);
    float alpha = src.a + dst.a * (1.0 - src.a);

    return float4(result, alpha);
}

/// Hard light blend fragment shader
fragment float4 fragment_blend_hard_light(
    RasterizerData2D in [[stage_in]],
    float4 dst [[color(0)]]
) {
    float4 src = in.color;

    float3 blended = blend_hard_light(src.rgb, dst.rgb);
    float3 result = mix(dst.rgb, blended, src.a);
    float alpha = src.a + dst.a * (1.0 - src.a);

    return float4(result, alpha);
}

/// Difference blend fragment shader
fragment float4 fragment_blend_difference(
    RasterizerData2D in [[stage_in]],
    float4 dst [[color(0)]]
) {
    float4 src = in.color;

    float3 blended = blend_difference(src.rgb, dst.rgb);
    float3 result = mix(dst.rgb, blended, src.a);
    float alpha = src.a + dst.a * (1.0 - src.a);

    return float4(result, alpha);
}

// MARK: - Textured Variants

/// Overlay blend with texture
fragment float4 fragment_blend_overlay_textured(
    RasterizerData2D in [[stage_in]],
    float4 dst [[color(0)]],
    texture2d<float> colorTexture [[texture(0)]],
    sampler textureSampler [[sampler(0)]]
) {
    float4 texColor = colorTexture.sample(textureSampler, in.texCoord);
    float4 src = texColor * in.color;

    float3 blended = blend_overlay(src.rgb, dst.rgb);
    float3 result = mix(dst.rgb, blended, src.a);
    float alpha = src.a + dst.a * (1.0 - src.a);

    return float4(result, alpha);
}

/// Soft light blend with texture
fragment float4 fragment_blend_soft_light_textured(
    RasterizerData2D in [[stage_in]],
    float4 dst [[color(0)]],
    texture2d<float> colorTexture [[texture(0)]],
    sampler textureSampler [[sampler(0)]]
) {
    float4 texColor = colorTexture.sample(textureSampler, in.texCoord);
    float4 src = texColor * in.color;

    float3 blended = blend_soft_light(src.rgb, dst.rgb);
    float3 result = mix(dst.rgb, blended, src.a);
    float alpha = src.a + dst.a * (1.0 - src.a);

    return float4(result, alpha);
}

/// Hard light blend with texture
fragment float4 fragment_blend_hard_light_textured(
    RasterizerData2D in [[stage_in]],
    float4 dst [[color(0)]],
    texture2d<float> colorTexture [[texture(0)]],
    sampler textureSampler [[sampler(0)]]
) {
    float4 texColor = colorTexture.sample(textureSampler, in.texCoord);
    float4 src = texColor * in.color;

    float3 blended = blend_hard_light(src.rgb, dst.rgb);
    float3 result = mix(dst.rgb, blended, src.a);
    float alpha = src.a + dst.a * (1.0 - src.a);

    return float4(result, alpha);
}

/// Difference blend with texture
fragment float4 fragment_blend_difference_textured(
    RasterizerData2D in [[stage_in]],
    float4 dst [[color(0)]],
    texture2d<float> colorTexture [[texture(0)]],
    sampler textureSampler [[sampler(0)]]
) {
    float4 texColor = colorTexture.sample(textureSampler, in.texCoord);
    float4 src = texColor * in.color;

    float3 blended = blend_difference(src.rgb, dst.rgb);
    float3 result = mix(dst.rgb, blended, src.a);
    float alpha = src.a + dst.a * (1.0 - src.a);

    return float4(result, alpha);
}
