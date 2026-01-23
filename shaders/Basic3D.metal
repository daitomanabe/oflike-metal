#include <metal_stdlib>
#include "Common.h"

using namespace metal;

// MARK: - Vertex Shader

/// Basic 3D vertex shader
/// Transforms vertices using projection and model-view matrices
/// Passes through normals and color
vertex RasterizerData3D vertex3D(
    uint vertexID [[vertex_id]],
    constant Vertex3D* vertices [[buffer(0)]],
    constant Uniforms3D& uniforms [[buffer(1)]]
) {
    RasterizerData3D out;

    Vertex3D in = vertices[vertexID];

    // Transform position
    float4 position = float4(in.position, 1.0);
    out.position = uniforms.projectionMatrix * uniforms.modelViewMatrix * position;

    // Transform normal (for future lighting)
    out.normal = (uniforms.normalMatrix * float4(in.normal, 0.0)).xyz;

    // World position (for future lighting)
    out.worldPosition = (uniforms.modelViewMatrix * position).xyz;

    // Pass through texture coordinates and color
    out.texCoord = in.texCoord;
    out.color = in.color;

    return out;
}

// MARK: - Fragment Shaders

/// Basic 3D fragment shader (solid color)
/// Uses vertex color only, no lighting
fragment float4 fragment3D(
    RasterizerData3D in [[stage_in]]
) {
    return in.color;
}

/// Textured 3D fragment shader
/// Samples texture and modulates with vertex color, no lighting
fragment float4 fragment3DTextured(
    RasterizerData3D in [[stage_in]],
    texture2d<float> colorTexture [[texture(0)]],
    sampler textureSampler [[sampler(0)]]
) {
    float4 texColor = colorTexture.sample(textureSampler, in.texCoord);
    return texColor * in.color;
}

/// Basic 3D fragment shader with simple diffuse lighting
/// Uses normalized normal as lighting (simple ambient + directional)
fragment float4 fragment3DLit(
    RasterizerData3D in [[stage_in]]
) {
    // Simple directional light from above
    float3 lightDir = normalize(float3(0.0, 1.0, 0.5));
    float3 normal = normalize(in.normal);

    // Diffuse lighting
    float diffuse = max(dot(normal, lightDir), 0.0);

    // Ambient + diffuse
    float ambient = 0.3;
    float lighting = ambient + (1.0 - ambient) * diffuse;

    return float4(in.color.rgb * lighting, in.color.a);
}

/// Textured 3D fragment shader with simple diffuse lighting
/// Samples texture, modulates with vertex color and lighting
fragment float4 fragment3DTexturedLit(
    RasterizerData3D in [[stage_in]],
    texture2d<float> colorTexture [[texture(0)]],
    sampler textureSampler [[sampler(0)]]
) {
    // Simple directional light from above
    float3 lightDir = normalize(float3(0.0, 1.0, 0.5));
    float3 normal = normalize(in.normal);

    // Diffuse lighting
    float diffuse = max(dot(normal, lightDir), 0.0);

    // Ambient + diffuse
    float ambient = 0.3;
    float lighting = ambient + (1.0 - ambient) * diffuse;

    // Sample texture and apply lighting
    float4 texColor = colorTexture.sample(textureSampler, in.texCoord);
    float4 color = texColor * in.color;

    return float4(color.rgb * lighting, color.a);
}
