#include <metal_stdlib>
#include "Common.h"

using namespace metal;

// MARK: - Light Structures

/// Light type enumeration (matches ofLightType)
constant int LIGHT_TYPE_POINT = 0;
constant int LIGHT_TYPE_DIRECTIONAL = 1;
constant int LIGHT_TYPE_SPOT = 2;

/// Light data structure for shader uniform buffer
/// Layout matches ofLight::getUniformData() format (22 floats per light, tightly packed)
struct LightData {
    float type;              // Light type (0=Point, 1=Directional, 2=Spot)
    packed_float3 position;  // Light position (world space) - packed to avoid alignment padding
    packed_float3 direction; // Light direction (normalized)
    packed_float3 ambientColor;  // Ambient color (RGB)
    packed_float3 diffuseColor;  // Diffuse color (RGB)
    packed_float3 specularColor; // Specular color (RGB)
    packed_float3 attenuation;   // Attenuation (constant, linear, quadratic)
    float spotCutoff;        // Spotlight cutoff angle (degrees)
    float spotConcentration; // Spotlight concentration exponent
    float enabled;           // 1.0 if enabled, 0.0 if disabled
};

/// Material data structure for shader uniform buffer
/// Layout matches ofMaterial::getUniformData() format (13 floats, tightly packed)
struct MaterialData {
    packed_float3 ambientColor;  // Ambient color (RGB)
    packed_float3 diffuseColor;  // Diffuse color (RGB)
    packed_float3 specularColor; // Specular color (RGB)
    packed_float3 emissiveColor; // Emissive color (RGB)
    float shininess;             // Shininess exponent (0-128)
};

/// Lighting uniforms
struct LightingUniforms {
    float4x4 projectionMatrix;
    float4x4 modelViewMatrix;
    float4x4 normalMatrix;
    float3 cameraPosition;      // Camera position for specular calculations
    int numLights;              // Number of active lights
    int lightingEnabled;        // 1 if lighting enabled, 0 otherwise
    int smoothShading;          // 1 for Phong (smooth), 0 for flat
};

// MARK: - Phong Lighting Functions

/// Calculate attenuation factor based on distance
/// \param attenuation Attenuation parameters (constant, linear, quadratic)
/// \param distance Distance from light to fragment
/// \return Attenuation factor (0.0 to 1.0)
float calculateAttenuation(float3 attenuation, float distance) {
    // Optimized: fused multiply-add for better performance
    float denom = fma(distance, fma(distance, attenuation.z, attenuation.y), attenuation.x);
    return 1.0 / max(denom, 1.0); // Prevent division by zero
}

/// Calculate spotlight intensity
/// \param light Light data
/// \param lightDir Direction from fragment to light (normalized)
/// \return Spotlight intensity factor (0.0 to 1.0)
float calculateSpotlightEffect(constant LightData& light, float3 lightDir) {
    float3 spotDir = normalize(light.direction);
    float spotCos = dot(-lightDir, spotDir);
    // Convert degrees to radians using constant from Common.h
    float cutoffCos = fast::cos(light.spotCutoff * DEG_TO_RAD);

    if (spotCos < cutoffCos) {
        return 0.0; // Outside cone
    }

    // Smooth falloff using spotlight concentration
    return fast::powr(spotCos, light.spotConcentration); // powr is faster for positive base
}

/// Calculate Phong lighting contribution from a single light
/// \param light Light data
/// \param material Material data
/// \param fragPosition Fragment position (view space)
/// \param normal Surface normal (view space, normalized)
/// \param viewDir View direction (view space, normalized)
/// \return RGB color contribution from this light
float3 calculatePhongLight(
    constant LightData& light,
    constant MaterialData& material,
    float3 fragPosition,
    float3 normal,
    float3 viewDir
) {
    // Skip disabled lights
    if (light.enabled < 0.5) {
        return float3(0.0);
    }

    // Initialize light direction and attenuation
    float3 lightDir;
    float attenuation = 1.0;

    int lightType = int(light.type);

    if (lightType == LIGHT_TYPE_DIRECTIONAL) {
        // Directional light - parallel rays
        lightDir = normalize(-light.direction);
    } else {
        // Point or spot light - calculate direction from position
        float3 lightVec = light.position - fragPosition;
        float distance = length(lightVec);
        lightDir = lightVec / distance; // Normalize

        // Calculate attenuation
        attenuation = calculateAttenuation(light.attenuation, distance);

        // Apply spotlight effect if spot light
        if (lightType == LIGHT_TYPE_SPOT) {
            float spotEffect = calculateSpotlightEffect(light, lightDir);
            attenuation *= spotEffect;
        }
    }

    // Ambient component
    float3 ambient = light.ambientColor * material.ambientColor;

    // Diffuse component (Lambertian reflection)
    float diffuseIntensity = max(dot(normal, lightDir), 0.0);
    float3 diffuse = light.diffuseColor * material.diffuseColor * diffuseIntensity;

    // Specular component (Blinn-Phong reflection)
    float3 specular = float3(0.0);
    if (diffuseIntensity > 0.0) {
        // Use Blinn-Phong halfway vector for better performance
        float3 halfwayDir = fast::normalize(lightDir + viewDir); // fast math for normalization
        float specAngle = max(dot(normal, halfwayDir), 0.0);
        float specularIntensity = fast::powr(specAngle, material.shininess); // powr faster than pow
        specular = light.specularColor * material.specularColor * specularIntensity;
    }

    // Combine components with attenuation
    return (ambient + diffuse + specular) * attenuation;
}

// MARK: - Vertex Shader

/// Lighting vertex shader
/// Transforms vertices and passes data to fragment shader
vertex RasterizerData3D vertexLighting(
    uint vertexID [[vertex_id]],
    constant Vertex3D* vertices [[buffer(0)]],
    constant LightingUniforms& uniforms [[buffer(1)]]
) {
    RasterizerData3D out;

    Vertex3D in = vertices[vertexID];

    // Transform position to clip space
    float4 position = float4(in.position, 1.0);
    float4 viewPosition = uniforms.modelViewMatrix * position;
    out.position = uniforms.projectionMatrix * viewPosition;

    // Transform normal to view space
    out.normal = (uniforms.normalMatrix * float4(in.normal, 0.0)).xyz;

    // World/view position for lighting calculations
    out.worldPosition = viewPosition.xyz;

    // Pass through texture coordinates and color
    out.texCoord = in.texCoord;
    out.color = in.color;

    return out;
}

// MARK: - Fragment Shaders

/// Phong lighting fragment shader (multi-light)
/// Calculates lighting from all active lights using Phong model
fragment float4 fragmentPhongLighting(
    RasterizerData3D in [[stage_in]],
    constant LightingUniforms& uniforms [[buffer(1)]],
    constant MaterialData& material [[buffer(2)]],
    constant LightData* lights [[buffer(3)]]
) {
    // Normalize interpolated normal
    float3 normal = normalize(in.normal);

    // Calculate view direction (from fragment to camera)
    float3 viewDir = normalize(-in.worldPosition); // Camera at origin in view space

    // Initialize final color with emissive component
    float3 finalColor = material.emissiveColor;

    // Accumulate lighting from all active lights
    // Note: Metal compiler will unroll small loops automatically
    [[unroll_count(8)]] // Hint for typical light count
    for (int i = 0; i < uniforms.numLights; ++i) {
        finalColor += calculatePhongLight(lights[i], material, in.worldPosition, normal, viewDir);
    }

    // Modulate with vertex color
    finalColor *= in.color.rgb;

    // Clamp to valid range
    finalColor = clamp(finalColor, 0.0, 1.0);

    return float4(finalColor, in.color.a);
}

/// Phong lighting fragment shader with texture (multi-light)
/// Calculates lighting and samples texture
fragment float4 fragmentPhongLightingTextured(
    RasterizerData3D in [[stage_in]],
    constant LightingUniforms& uniforms [[buffer(1)]],
    constant MaterialData& material [[buffer(2)]],
    constant LightData* lights [[buffer(3)]],
    texture2d<float> colorTexture [[texture(0)]],
    sampler textureSampler [[sampler(0)]]
) {
    // Normalize interpolated normal
    float3 normal = normalize(in.normal);

    // Calculate view direction (from fragment to camera)
    float3 viewDir = normalize(-in.worldPosition); // Camera at origin in view space

    // Initialize final color with emissive component
    float3 finalColor = material.emissiveColor;

    // Accumulate lighting from all active lights
    // Note: Metal compiler will unroll small loops automatically
    [[unroll_count(8)]] // Hint for typical light count
    for (int i = 0; i < uniforms.numLights; ++i) {
        finalColor += calculatePhongLight(lights[i], material, in.worldPosition, normal, viewDir);
    }

    // Sample texture
    float4 texColor = colorTexture.sample(textureSampler, in.texCoord);

    // Modulate lighting with texture and vertex color
    finalColor *= texColor.rgb * in.color.rgb;

    // Clamp to valid range
    finalColor = clamp(finalColor, 0.0, 1.0);

    return float4(finalColor, in.color.a * texColor.a);
}

/// Flat shading variant (no normal interpolation)
/// Useful for stylized rendering or debugging
fragment float4 fragmentFlatLighting(
    RasterizerData3D in [[stage_in]],
    constant LightingUniforms& uniforms [[buffer(1)]],
    constant MaterialData& material [[buffer(2)]],
    constant LightData* lights [[buffer(3)]]
) {
    // Use face normal (via derivative) for flat shading
    float3 dFdxPos = dfdx(in.worldPosition);
    float3 dFdyPos = dfdy(in.worldPosition);
    float3 normal = normalize(cross(dFdxPos, dFdyPos));

    // Calculate view direction
    float3 viewDir = normalize(-in.worldPosition);

    // Initialize final color with emissive component
    float3 finalColor = material.emissiveColor;

    // Accumulate lighting from all active lights
    // Note: Metal compiler will unroll small loops automatically
    [[unroll_count(8)]] // Hint for typical light count
    for (int i = 0; i < uniforms.numLights; ++i) {
        finalColor += calculatePhongLight(lights[i], material, in.worldPosition, normal, viewDir);
    }

    // Modulate with vertex color
    finalColor *= in.color.rgb;

    // Clamp to valid range
    finalColor = clamp(finalColor, 0.0, 1.0);

    return float4(finalColor, in.color.a);
}
