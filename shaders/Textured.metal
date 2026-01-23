#include <metal_stdlib>
#include "Common.h"

using namespace metal;

// MARK: - Advanced Textured Shaders

/// Textured 2D shader with alpha blending support
/// Preserves alpha channel for transparency
fragment float4 fragment2DTexturedAlpha(
    RasterizerData2D in [[stage_in]],
    texture2d<float> colorTexture [[texture(0)]],
    sampler textureSampler [[sampler(0)]]
) {
    float4 texColor = colorTexture.sample(textureSampler, in.texCoord);
    float4 color = texColor * in.color;

    // Premultiplied alpha
    color.rgb *= color.a;

    return color;
}

/// Textured 2D shader with additive blending
/// For particle effects and light effects
fragment float4 fragment2DTexturedAdditive(
    RasterizerData2D in [[stage_in]],
    texture2d<float> colorTexture [[texture(0)]],
    sampler textureSampler [[sampler(0)]]
) {
    float4 texColor = colorTexture.sample(textureSampler, in.texCoord);
    float4 color = texColor * in.color;

    // Apply alpha to RGB for additive blending
    color.rgb *= color.a;
    color.a = 1.0;

    return color;
}

/// Textured 2D shader with multiply blending
/// For tinting and masking effects
fragment float4 fragment2DTexturedMultiply(
    RasterizerData2D in [[stage_in]],
    texture2d<float> colorTexture [[texture(0)]],
    sampler textureSampler [[sampler(0)]]
) {
    float4 texColor = colorTexture.sample(textureSampler, in.texCoord);
    return texColor * in.color;
}

// MARK: - Multi-Texture Shaders

/// Dual texture 2D shader
/// Blends two textures together
fragment float4 fragment2DDualTexture(
    RasterizerData2D in [[stage_in]],
    texture2d<float> texture0 [[texture(0)]],
    texture2d<float> texture1 [[texture(1)]],
    sampler textureSampler [[sampler(0)]]
) {
    float4 color0 = texture0.sample(textureSampler, in.texCoord);
    float4 color1 = texture1.sample(textureSampler, in.texCoord);

    // Mix textures using vertex color alpha as blend factor
    float4 blended = mix(color0, color1, in.color.a);

    // Modulate with vertex color RGB
    return float4(blended.rgb * in.color.rgb, blended.a);
}

// MARK: - Special Effects

/// Textured 2D shader with color replacement
/// Replaces a specific color range with vertex color
fragment float4 fragment2DColorReplace(
    RasterizerData2D in [[stage_in]],
    texture2d<float> colorTexture [[texture(0)]],
    sampler textureSampler [[sampler(0)]],
    constant float4& targetColor [[buffer(0)]],
    constant float& threshold [[buffer(1)]]
) {
    float4 texColor = colorTexture.sample(textureSampler, in.texCoord);

    // Calculate color distance
    float dist = distance(texColor.rgb, targetColor.rgb);

    // Replace color if within threshold
    if (dist < threshold) {
        return float4(in.color.rgb, texColor.a);
    }

    return texColor * in.color;
}

/// Textured 2D shader with grayscale conversion
/// Converts texture to grayscale and tints with vertex color
fragment float4 fragment2DGrayscale(
    RasterizerData2D in [[stage_in]],
    texture2d<float> colorTexture [[texture(0)]],
    sampler textureSampler [[sampler(0)]]
) {
    float4 texColor = colorTexture.sample(textureSampler, in.texCoord);

    // Convert to grayscale using luminance weights from Common.h
    float gray = dot(texColor.rgb, LUMA_WEIGHTS);

    // Tint with vertex color
    return float4(gray * in.color.rgb, texColor.a * in.color.a);
}

/// Textured 2D shader with sepia tone
/// Applies sepia effect to texture
fragment float4 fragment2DSepia(
    RasterizerData2D in [[stage_in]],
    texture2d<float> colorTexture [[texture(0)]],
    sampler textureSampler [[sampler(0)]]
) {
    float4 texColor = colorTexture.sample(textureSampler, in.texCoord);

    // Sepia transformation matrix
    float3 sepia;
    sepia.r = dot(texColor.rgb, float3(0.393, 0.769, 0.189));
    sepia.g = dot(texColor.rgb, float3(0.349, 0.686, 0.168));
    sepia.b = dot(texColor.rgb, float3(0.272, 0.534, 0.131));

    // Modulate with vertex color
    return float4(sepia * in.color.rgb, texColor.a * in.color.a);
}

/// Textured 2D shader with brightness adjustment
/// Adjusts brightness based on vertex color
fragment float4 fragment2DBrightness(
    RasterizerData2D in [[stage_in]],
    texture2d<float> colorTexture [[texture(0)]],
    sampler textureSampler [[sampler(0)]]
) {
    float4 texColor = colorTexture.sample(textureSampler, in.texCoord);

    // Use vertex color.r as brightness factor (0.0 = black, 1.0 = original, >1.0 = brighter)
    float brightness = in.color.r;

    return float4(texColor.rgb * brightness, texColor.a);
}
