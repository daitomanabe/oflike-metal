#include <metal_stdlib>
#include "Common.h"

using namespace metal;

// Custom fragment uniforms at buffer(2) to avoid conflict with Uniforms2D
// Note: ofShader uses 16-byte alignment per uniform, so we use separate buffers
// For simplicity, we just use time from the uniform buffer (first float at offset 0)

// MARK: - Vertex Shader (reuse standard 2D vertex shader)
// Uses the same vertex shader as Basic2D for compatibility

vertex RasterizerData2D vertex_test(
    uint vertexID [[vertex_id]],
    constant Vertex2D* vertices [[buffer(0)]],
    constant Uniforms2D& uniforms [[buffer(1)]]
) {
    RasterizerData2D out;

    Vertex2D in = vertices[vertexID];

    // Apply transformation matrices
    float4 position = float4(in.position, 0.0, 1.0);
    out.position = uniforms.projectionMatrix * uniforms.modelViewMatrix * position;
    out.texCoord = in.texCoord;
    out.color = in.color;

    return out;
}

// MARK: - Fragment Shaders

/// Simple gradient shader - animates colors based on UV and time
fragment float4 fragment_gradient(
    RasterizerData2D in [[stage_in]],
    constant float* uniforms [[buffer(2)]]
) {
    float t = uniforms[0];  // time is first float in uniform buffer

    // Create animated gradient based on UV and time
    float r = 0.5 + 0.5 * sin(t + in.texCoord.x * 3.14159);
    float g = 0.5 + 0.5 * sin(t * 1.3 + in.texCoord.y * 3.14159);
    float b = 0.5 + 0.5 * sin(t * 0.7 + (in.texCoord.x + in.texCoord.y) * 3.14159);

    return float4(r, g, b, 1.0) * in.color;
}

/// Wave distortion shader - creates animated wave patterns
fragment float4 fragment_wave(
    RasterizerData2D in [[stage_in]],
    constant float* uniforms [[buffer(2)]]
) {
    float t = uniforms[0];
    float2 uv = in.texCoord;

    // Create wave pattern
    float wave = sin(uv.x * 10.0 + t * 2.0) * 0.1;
    wave += sin(uv.y * 8.0 - t * 1.5) * 0.1;

    // Create color based on wave
    float3 color = float3(
        0.5 + 0.5 * sin(wave * 10.0 + t),
        0.5 + 0.5 * cos(wave * 10.0 + t * 1.3),
        0.5 + 0.5 * sin(wave * 10.0 + t * 0.7)
    );

    return float4(color, 1.0) * in.color;
}

/// Circular pattern shader - creates concentric animated circles
fragment float4 fragment_circles(
    RasterizerData2D in [[stage_in]],
    constant float* uniforms [[buffer(2)]]
) {
    float t = uniforms[0];
    float2 uv = in.texCoord * 2.0 - 1.0; // Center UV (-1 to 1)

    float dist = length(uv);
    float angle = atan2(uv.y, uv.x);

    // Create concentric circles with rotation
    float pattern = sin(dist * 15.0 - t * 3.0) * 0.5 + 0.5;
    pattern *= sin(angle * 6.0 + t * 2.0) * 0.5 + 0.5;

    // Color based on pattern
    float3 color = mix(
        float3(0.2, 0.4, 0.8),
        float3(1.0, 0.6, 0.2),
        pattern
    );

    return float4(color, 1.0) * in.color;
}

/// Noise-like shader - creates pseudo-random animated pattern
fragment float4 fragment_noise(
    RasterizerData2D in [[stage_in]],
    constant float* uniforms [[buffer(2)]]
) {
    float t = uniforms[0];
    float2 uv = in.texCoord;

    // Simple hash function for noise-like effect
    float2 p = uv * 10.0 + float2(t * 0.5, t * 0.3);
    float n = fract(sin(dot(p, float2(12.9898, 78.233))) * 43758.5453);

    // Smooth noise by mixing with neighbors
    float2 p2 = p + float2(0.1, 0.0);
    float n2 = fract(sin(dot(p2, float2(12.9898, 78.233))) * 43758.5453);

    float noise = mix(n, n2, 0.5);

    // Create colorful noise
    float3 color = float3(
        noise,
        fract(noise * 1.5 + 0.3),
        fract(noise * 0.7 + 0.6)
    );

    return float4(color, 1.0) * in.color;
}
