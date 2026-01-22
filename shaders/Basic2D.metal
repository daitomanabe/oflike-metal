#include <metal_stdlib>
using namespace metal;

struct VertexIn {
  float2 position [[attribute(0)]];
  float2 uv       [[attribute(1)]];
  float4 color    [[attribute(2)]];
};

struct VSOut {
  float4 position [[position]];
  float2 uv;
  float4 color;
};

struct Uniforms {
  float2 viewportSize; // in pixels
};

vertex VSOut vs_basic2d(VertexIn in [[stage_in]],
                        constant Uniforms& u [[buffer(1)]]) {
  VSOut o;

  // Convert from pixel coordinates (origin top-left) to clip space.
  float2 ndc = float2((in.position.x / u.viewportSize.x) * 2.0 - 1.0,
                      (in.position.y / u.viewportSize.y) * 2.0 - 1.0);
  ndc.y = -ndc.y;

  o.position = float4(ndc, 0.0, 1.0);
  o.uv = in.uv;
  o.color = in.color;
  return o;
}

fragment float4 fs_basic2d(VSOut in [[stage_in]]) {
  return in.color;
}
