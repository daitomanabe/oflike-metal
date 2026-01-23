#ifndef Common_h
#define Common_h

#include <metal_stdlib>
using namespace metal;

// MARK: - Vertex Definitions

/// 2D vertex with position, texture coordinates, and color
struct Vertex2D {
    float2 position [[attribute(0)]];
    float2 texCoord [[attribute(1)]];
    float4 color    [[attribute(2)]];
};

/// 3D vertex with position, normal, texture coordinates, and color
struct Vertex3D {
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
    float2 texCoord [[attribute(2)]];
    float4 color    [[attribute(3)]];
};

// MARK: - Uniform Buffers

/// 2D rendering uniforms
struct Uniforms2D {
    float4x4 projectionMatrix;
    float4x4 modelViewMatrix;
};

/// 3D rendering uniforms
struct Uniforms3D {
    float4x4 projectionMatrix;
    float4x4 modelViewMatrix;
    float4x4 normalMatrix;
};

// MARK: - Rasterizer Data

/// Data passed from vertex shader to fragment shader (2D)
struct RasterizerData2D {
    float4 position [[position]];
    float2 texCoord;
    float4 color;
};

/// Data passed from vertex shader to fragment shader (3D)
struct RasterizerData3D {
    float4 position [[position]];
    float3 normal;
    float2 texCoord;
    float4 color;
    float3 worldPosition;
};

#endif /* Common_h */
