#include <metal_stdlib>
#include "Common.h"

using namespace metal;

// MARK: - Vertex Shader

/// Basic 2D vertex shader
/// Transforms vertices using projection and model-view matrices
vertex RasterizerData2D vertex2D(
    uint vertexID [[vertex_id]],
    constant Vertex2D* vertices [[buffer(0)]],
    constant Uniforms2D& uniforms [[buffer(1)]]
) {
    RasterizerData2D out;

    Vertex2D in = vertices[vertexID];

    // Transform position
    float4 position = float4(in.position, 0.0, 1.0);
    out.position = uniforms.projectionMatrix * uniforms.modelViewMatrix * position;

    // Pass through texture coordinates and color
    out.texCoord = in.texCoord;
    out.color = in.color;

    return out;
}

// MARK: - Fragment Shaders

/// Basic 2D fragment shader (solid color)
/// Uses vertex color only
fragment float4 fragment2D(
    RasterizerData2D in [[stage_in]]
) {
    return in.color;
}

/// Textured 2D fragment shader
/// Samples texture and modulates with vertex color
fragment float4 fragment2DTextured(
    RasterizerData2D in [[stage_in]],
    texture2d<float> colorTexture [[texture(0)]],
    sampler textureSampler [[sampler(0)]]
) {
    float4 texColor = colorTexture.sample(textureSampler, in.texCoord);
    return texColor * in.color;
}
