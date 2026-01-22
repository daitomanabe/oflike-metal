#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include "MetalRenderer.h"
#include "../DrawList.h"

namespace oflike {

// Uniform structure matching the shader
struct Uniforms {
  float viewportWidth;
  float viewportHeight;
};

// 8x8 bitmap font data (ASCII 32-127 printable characters)
static const uint8_t kFontData8x8[96][8] = {
  // Space (32)
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  // ! (33)
  {0x18, 0x3c, 0x3c, 0x18, 0x18, 0x00, 0x18, 0x00},
  // " (34)
  {0x6c, 0x6c, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00},
  // # (35)
  {0x6c, 0x6c, 0xfe, 0x6c, 0xfe, 0x6c, 0x6c, 0x00},
  // $ (36)
  {0x18, 0x7e, 0xc0, 0x7c, 0x06, 0xfc, 0x18, 0x00},
  // % (37)
  {0x00, 0xc6, 0xcc, 0x18, 0x30, 0x66, 0xc6, 0x00},
  // & (38)
  {0x38, 0x6c, 0x38, 0x76, 0xdc, 0xcc, 0x76, 0x00},
  // ' (39)
  {0x18, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00},
  // ( (40)
  {0x0c, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0c, 0x00},
  // ) (41)
  {0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x18, 0x30, 0x00},
  // * (42)
  {0x00, 0x66, 0x3c, 0xff, 0x3c, 0x66, 0x00, 0x00},
  // + (43)
  {0x00, 0x18, 0x18, 0x7e, 0x18, 0x18, 0x00, 0x00},
  // , (44)
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30},
  // - (45)
  {0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00},
  // . (46)
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00},
  // / (47)
  {0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x80, 0x00},
  // 0 (48)
  {0x7c, 0xc6, 0xce, 0xd6, 0xe6, 0xc6, 0x7c, 0x00},
  // 1 (49)
  {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x00},
  // 2 (50)
  {0x7c, 0xc6, 0x06, 0x1c, 0x30, 0x66, 0xfe, 0x00},
  // 3 (51)
  {0x7c, 0xc6, 0x06, 0x3c, 0x06, 0xc6, 0x7c, 0x00},
  // 4 (52)
  {0x1c, 0x3c, 0x6c, 0xcc, 0xfe, 0x0c, 0x1e, 0x00},
  // 5 (53)
  {0xfe, 0xc0, 0xc0, 0xfc, 0x06, 0xc6, 0x7c, 0x00},
  // 6 (54)
  {0x38, 0x60, 0xc0, 0xfc, 0xc6, 0xc6, 0x7c, 0x00},
  // 7 (55)
  {0xfe, 0xc6, 0x0c, 0x18, 0x30, 0x30, 0x30, 0x00},
  // 8 (56)
  {0x7c, 0xc6, 0xc6, 0x7c, 0xc6, 0xc6, 0x7c, 0x00},
  // 9 (57)
  {0x7c, 0xc6, 0xc6, 0x7e, 0x06, 0x0c, 0x78, 0x00},
  // : (58)
  {0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00},
  // ; (59)
  {0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x30},
  // < (60)
  {0x06, 0x0c, 0x18, 0x30, 0x18, 0x0c, 0x06, 0x00},
  // = (61)
  {0x00, 0x00, 0x7e, 0x00, 0x00, 0x7e, 0x00, 0x00},
  // > (62)
  {0x60, 0x30, 0x18, 0x0c, 0x18, 0x30, 0x60, 0x00},
  // ? (63)
  {0x7c, 0xc6, 0x0c, 0x18, 0x18, 0x00, 0x18, 0x00},
  // @ (64)
  {0x7c, 0xc6, 0xde, 0xde, 0xde, 0xc0, 0x78, 0x00},
  // A (65)
  {0x38, 0x6c, 0xc6, 0xfe, 0xc6, 0xc6, 0xc6, 0x00},
  // B (66)
  {0xfc, 0x66, 0x66, 0x7c, 0x66, 0x66, 0xfc, 0x00},
  // C (67)
  {0x3c, 0x66, 0xc0, 0xc0, 0xc0, 0x66, 0x3c, 0x00},
  // D (68)
  {0xf8, 0x6c, 0x66, 0x66, 0x66, 0x6c, 0xf8, 0x00},
  // E (69)
  {0xfe, 0x62, 0x68, 0x78, 0x68, 0x62, 0xfe, 0x00},
  // F (70)
  {0xfe, 0x62, 0x68, 0x78, 0x68, 0x60, 0xf0, 0x00},
  // G (71)
  {0x3c, 0x66, 0xc0, 0xc0, 0xce, 0x66, 0x3a, 0x00},
  // H (72)
  {0xc6, 0xc6, 0xc6, 0xfe, 0xc6, 0xc6, 0xc6, 0x00},
  // I (73)
  {0x3c, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00},
  // J (74)
  {0x1e, 0x0c, 0x0c, 0x0c, 0xcc, 0xcc, 0x78, 0x00},
  // K (75)
  {0xe6, 0x66, 0x6c, 0x78, 0x6c, 0x66, 0xe6, 0x00},
  // L (76)
  {0xf0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xfe, 0x00},
  // M (77)
  {0xc6, 0xee, 0xfe, 0xfe, 0xd6, 0xc6, 0xc6, 0x00},
  // N (78)
  {0xc6, 0xe6, 0xf6, 0xde, 0xce, 0xc6, 0xc6, 0x00},
  // O (79)
  {0x7c, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x00},
  // P (80)
  {0xfc, 0x66, 0x66, 0x7c, 0x60, 0x60, 0xf0, 0x00},
  // Q (81)
  {0x7c, 0xc6, 0xc6, 0xc6, 0xc6, 0xce, 0x7c, 0x0e},
  // R (82)
  {0xfc, 0x66, 0x66, 0x7c, 0x6c, 0x66, 0xe6, 0x00},
  // S (83)
  {0x7c, 0xc6, 0x60, 0x38, 0x0c, 0xc6, 0x7c, 0x00},
  // T (84)
  {0x7e, 0x7e, 0x5a, 0x18, 0x18, 0x18, 0x3c, 0x00},
  // U (85)
  {0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x00},
  // V (86)
  {0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x6c, 0x38, 0x00},
  // W (87)
  {0xc6, 0xc6, 0xc6, 0xd6, 0xd6, 0xfe, 0x6c, 0x00},
  // X (88)
  {0xc6, 0xc6, 0x6c, 0x38, 0x6c, 0xc6, 0xc6, 0x00},
  // Y (89)
  {0x66, 0x66, 0x66, 0x3c, 0x18, 0x18, 0x3c, 0x00},
  // Z (90)
  {0xfe, 0xc6, 0x8c, 0x18, 0x32, 0x66, 0xfe, 0x00},
  // [ (91)
  {0x3c, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3c, 0x00},
  // \ (92)
  {0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x02, 0x00},
  // ] (93)
  {0x3c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x3c, 0x00},
  // ^ (94)
  {0x10, 0x38, 0x6c, 0xc6, 0x00, 0x00, 0x00, 0x00},
  // _ (95)
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
  // ` (96)
  {0x30, 0x18, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00},
  // a (97)
  {0x00, 0x00, 0x78, 0x0c, 0x7c, 0xcc, 0x76, 0x00},
  // b (98)
  {0xe0, 0x60, 0x7c, 0x66, 0x66, 0x66, 0xdc, 0x00},
  // c (99)
  {0x00, 0x00, 0x7c, 0xc6, 0xc0, 0xc6, 0x7c, 0x00},
  // d (100)
  {0x1c, 0x0c, 0x7c, 0xcc, 0xcc, 0xcc, 0x76, 0x00},
  // e (101)
  {0x00, 0x00, 0x7c, 0xc6, 0xfe, 0xc0, 0x7c, 0x00},
  // f (102)
  {0x3c, 0x66, 0x60, 0xf8, 0x60, 0x60, 0xf0, 0x00},
  // g (103)
  {0x00, 0x00, 0x76, 0xcc, 0xcc, 0x7c, 0x0c, 0xf8},
  // h (104)
  {0xe0, 0x60, 0x6c, 0x76, 0x66, 0x66, 0xe6, 0x00},
  // i (105)
  {0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3c, 0x00},
  // j (106)
  {0x06, 0x00, 0x06, 0x06, 0x06, 0x66, 0x66, 0x3c},
  // k (107)
  {0xe0, 0x60, 0x66, 0x6c, 0x78, 0x6c, 0xe6, 0x00},
  // l (108)
  {0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00},
  // m (109)
  {0x00, 0x00, 0xec, 0xfe, 0xd6, 0xd6, 0xd6, 0x00},
  // n (110)
  {0x00, 0x00, 0xdc, 0x66, 0x66, 0x66, 0x66, 0x00},
  // o (111)
  {0x00, 0x00, 0x7c, 0xc6, 0xc6, 0xc6, 0x7c, 0x00},
  // p (112)
  {0x00, 0x00, 0xdc, 0x66, 0x66, 0x7c, 0x60, 0xf0},
  // q (113)
  {0x00, 0x00, 0x76, 0xcc, 0xcc, 0x7c, 0x0c, 0x1e},
  // r (114)
  {0x00, 0x00, 0xdc, 0x76, 0x60, 0x60, 0xf0, 0x00},
  // s (115)
  {0x00, 0x00, 0x7e, 0xc0, 0x7c, 0x06, 0xfc, 0x00},
  // t (116)
  {0x30, 0x30, 0xfc, 0x30, 0x30, 0x36, 0x1c, 0x00},
  // u (117)
  {0x00, 0x00, 0xcc, 0xcc, 0xcc, 0xcc, 0x76, 0x00},
  // v (118)
  {0x00, 0x00, 0xc6, 0xc6, 0xc6, 0x6c, 0x38, 0x00},
  // w (119)
  {0x00, 0x00, 0xc6, 0xd6, 0xd6, 0xfe, 0x6c, 0x00},
  // x (120)
  {0x00, 0x00, 0xc6, 0x6c, 0x38, 0x6c, 0xc6, 0x00},
  // y (121)
  {0x00, 0x00, 0xc6, 0xc6, 0xc6, 0x7e, 0x06, 0xfc},
  // z (122)
  {0x00, 0x00, 0x7e, 0x4c, 0x18, 0x32, 0x7e, 0x00},
  // { (123)
  {0x0e, 0x18, 0x18, 0x70, 0x18, 0x18, 0x0e, 0x00},
  // | (124)
  {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00},
  // } (125)
  {0x70, 0x18, 0x18, 0x0e, 0x18, 0x18, 0x70, 0x00},
  // ~ (126)
  {0x76, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  // DEL (127) - filled block
  {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
};

struct MetalRenderer::Impl {
  __strong id<MTLDevice> device = nil;
  __strong id<MTLCommandQueue> queue = nil;
  __strong id<MTLRenderPipelineState> pipeline = nil;
  __strong id<MTLRenderPipelineState> textPipeline = nil;
  __strong id<MTLRenderPipelineState> texturePipeline = nil;
  __strong id<MTLBuffer> vertexBuffer = nil;
  __strong id<MTLBuffer> textVertexBuffer = nil;
  __strong id<MTLBuffer> lineVertexBuffer = nil;
  __strong id<MTLBuffer> textureVertexBuffer = nil;
  __strong id<MTLBuffer> uniformBuffer = nil;
  __strong id<MTLTexture> fontTexture = nil;
  __strong id<MTLSamplerState> sampler = nil;
  __strong id<MTLCommandBuffer> cmd = nil;
  __strong id<MTLRenderCommandEncoder> enc = nil;
  __strong id<CAMetalDrawable> drawable = nil;

  Color4f clear{0.f, 0.f, 0.f, 1.f};
  int viewportW = 1;
  int viewportH = 1;
  float contentScale = 1.0f;

  static constexpr size_t kMaxVertices = 65536;

  bool createFontTexture();
  bool createPipeline();
};

bool MetalRenderer::Impl::createFontTexture() {
  MTLTextureDescriptor* desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatR8Unorm
                                                                                  width:128
                                                                                 height:128
                                                                              mipmapped:NO];
  desc.usage = MTLTextureUsageShaderRead;
  desc.storageMode = MTLStorageModeShared;

  fontTexture = [device newTextureWithDescriptor:desc];
  if (!fontTexture) return false;

  uint8_t texData[128 * 128] = {0};

  for (int charIdx = 0; charIdx < 96; charIdx++) {
    int ascii = charIdx + 32;
    int cellX = ascii % 16;
    int cellY = ascii / 16;

    for (int row = 0; row < 8; row++) {
      uint8_t bits = kFontData8x8[charIdx][row];
      for (int col = 0; col < 8; col++) {
        int px = cellX * 8 + col;
        int py = cellY * 8 + row;
        texData[py * 128 + px] = (bits & (0x80 >> col)) ? 255 : 0;
      }
    }
  }

  MTLRegion region = MTLRegionMake2D(0, 0, 128, 128);
  [fontTexture replaceRegion:region mipmapLevel:0 withBytes:texData bytesPerRow:128];

  MTLSamplerDescriptor* samplerDesc = [[MTLSamplerDescriptor alloc] init];
  samplerDesc.minFilter = MTLSamplerMinMagFilterNearest;
  samplerDesc.magFilter = MTLSamplerMinMagFilterNearest;
  samplerDesc.sAddressMode = MTLSamplerAddressModeClampToEdge;
  samplerDesc.tAddressMode = MTLSamplerAddressModeClampToEdge;
  sampler = [device newSamplerStateWithDescriptor:samplerDesc];

  return sampler != nil;
}

bool MetalRenderer::Impl::createPipeline() {
  if (!device) return false;

  NSString* solidShaderSource = @R"(
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
  float2 viewportSize;
};

vertex VSOut vs_basic2d(VertexIn in [[stage_in]],
                        constant Uniforms& u [[buffer(1)]]) {
  VSOut o;
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
)";

  NSString* textShaderSource = @R"(
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
  float2 viewportSize;
};

vertex VSOut vs_text(VertexIn in [[stage_in]],
                     constant Uniforms& u [[buffer(1)]]) {
  VSOut o;
  float2 ndc = float2((in.position.x / u.viewportSize.x) * 2.0 - 1.0,
                      (in.position.y / u.viewportSize.y) * 2.0 - 1.0);
  ndc.y = -ndc.y;
  o.position = float4(ndc, 0.0, 1.0);
  o.uv = in.uv;
  o.color = in.color;
  return o;
}

fragment float4 fs_text(VSOut in [[stage_in]],
                        texture2d<float> fontTex [[texture(0)]],
                        sampler samp [[sampler(0)]]) {
  float alpha = fontTex.sample(samp, in.uv).r;
  return float4(in.color.rgb, in.color.a * alpha);
}
)";

  NSString* textureShaderSource = @R"(
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
  float2 viewportSize;
};

vertex VSOut vs_texture(VertexIn in [[stage_in]],
                        constant Uniforms& u [[buffer(1)]]) {
  VSOut o;
  float2 ndc = float2((in.position.x / u.viewportSize.x) * 2.0 - 1.0,
                      (in.position.y / u.viewportSize.y) * 2.0 - 1.0);
  ndc.y = -ndc.y;
  o.position = float4(ndc, 0.0, 1.0);
  o.uv = in.uv;
  o.color = in.color;
  return o;
}

fragment float4 fs_texture(VSOut in [[stage_in]],
                           texture2d<float> tex [[texture(0)]],
                           sampler samp [[sampler(0)]]) {
  float4 texColor = tex.sample(samp, in.uv);
  return texColor * in.color;
}
)";

  NSError* error = nil;

  id<MTLLibrary> solidLib = [device newLibraryWithSource:solidShaderSource options:nil error:&error];
  if (!solidLib) {
    NSLog(@"Solid shader compile error: %@", error.localizedDescription);
    return false;
  }

  id<MTLLibrary> textLib = [device newLibraryWithSource:textShaderSource options:nil error:&error];
  if (!textLib) {
    NSLog(@"Text shader compile error: %@", error.localizedDescription);
    return false;
  }

  id<MTLLibrary> textureLib = [device newLibraryWithSource:textureShaderSource options:nil error:&error];
  if (!textureLib) {
    NSLog(@"Texture shader compile error: %@", error.localizedDescription);
    return false;
  }

  id<MTLFunction> vs = [solidLib newFunctionWithName:@"vs_basic2d"];
  id<MTLFunction> fs = [solidLib newFunctionWithName:@"fs_basic2d"];
  id<MTLFunction> vsText = [textLib newFunctionWithName:@"vs_text"];
  id<MTLFunction> fsText = [textLib newFunctionWithName:@"fs_text"];
  id<MTLFunction> vsTexture = [textureLib newFunctionWithName:@"vs_texture"];
  id<MTLFunction> fsTexture = [textureLib newFunctionWithName:@"fs_texture"];

  if (!vs || !fs || !vsText || !fsText || !vsTexture || !fsTexture) {
    NSLog(@"Failed to find shader functions");
    return false;
  }

  MTLVertexDescriptor* vd = [[MTLVertexDescriptor alloc] init];
  vd.attributes[0].format = MTLVertexFormatFloat2;
  vd.attributes[0].offset = 0;
  vd.attributes[0].bufferIndex = 0;
  vd.attributes[1].format = MTLVertexFormatFloat2;
  vd.attributes[1].offset = sizeof(float) * 2;
  vd.attributes[1].bufferIndex = 0;
  vd.attributes[2].format = MTLVertexFormatFloat4;
  vd.attributes[2].offset = sizeof(float) * 4;
  vd.attributes[2].bufferIndex = 0;
  vd.layouts[0].stride = sizeof(float) * 8;
  vd.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

  MTLRenderPipelineDescriptor* desc = [[MTLRenderPipelineDescriptor alloc] init];
  desc.vertexFunction = vs;
  desc.fragmentFunction = fs;
  desc.vertexDescriptor = vd;
  desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
  desc.colorAttachments[0].blendingEnabled = YES;
  desc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
  desc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
  desc.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
  desc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
  desc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
  desc.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;

  pipeline = [device newRenderPipelineStateWithDescriptor:desc error:&error];
  if (!pipeline) {
    NSLog(@"Solid pipeline creation error: %@", error.localizedDescription);
    return false;
  }

  desc.vertexFunction = vsText;
  desc.fragmentFunction = fsText;
  textPipeline = [device newRenderPipelineStateWithDescriptor:desc error:&error];
  if (!textPipeline) {
    NSLog(@"Text pipeline creation error: %@", error.localizedDescription);
    return false;
  }

  desc.vertexFunction = vsTexture;
  desc.fragmentFunction = fsTexture;
  texturePipeline = [device newRenderPipelineStateWithDescriptor:desc error:&error];
  if (!texturePipeline) {
    NSLog(@"Texture pipeline creation error: %@", error.localizedDescription);
    return false;
  }

  vertexBuffer = [device newBufferWithLength:kMaxVertices * sizeof(float) * 8
                                     options:MTLResourceStorageModeShared];
  textVertexBuffer = [device newBufferWithLength:kMaxVertices * sizeof(float) * 8
                                         options:MTLResourceStorageModeShared];
  lineVertexBuffer = [device newBufferWithLength:kMaxVertices * sizeof(float) * 8
                                         options:MTLResourceStorageModeShared];
  textureVertexBuffer = [device newBufferWithLength:kMaxVertices * sizeof(float) * 8
                                            options:MTLResourceStorageModeShared];
  uniformBuffer = [device newBufferWithLength:sizeof(Uniforms)
                                      options:MTLResourceStorageModeShared];

  if (!createFontTexture()) {
    NSLog(@"Failed to create font texture");
    return false;
  }

  return true;
}

MetalRenderer::MetalRenderer() : impl_(std::make_unique<Impl>()) {}
MetalRenderer::~MetalRenderer() = default;

bool MetalRenderer::attachToView(void* mtkView) {
  auto* view = (__bridge MTKView*)mtkView;
  if (!view) return false;

  impl_->device = view.device ? view.device : MTLCreateSystemDefaultDevice();
  if (!impl_->device) return false;

  view.device = impl_->device;
  view.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;

  impl_->queue = [impl_->device newCommandQueue];
  if (!impl_->queue) return false;

  return impl_->createPipeline();
}

void MetalRenderer::setClearColor(Color4f c) {
  impl_->clear = c;
}

void MetalRenderer::beginFrame(void* renderPassDescriptor, void* drawable, int pixelW, int pixelH, float contentScale) {
  auto* pass = (__bridge MTLRenderPassDescriptor*)renderPassDescriptor;
  impl_->drawable = (__bridge id<CAMetalDrawable>)drawable;
  impl_->viewportW = pixelW;
  impl_->viewportH = pixelH;
  impl_->contentScale = contentScale;

  pass.colorAttachments[0].clearColor = MTLClearColorMake(impl_->clear.r, impl_->clear.g, impl_->clear.b, impl_->clear.a);
  pass.colorAttachments[0].loadAction = MTLLoadActionClear;
  pass.colorAttachments[0].storeAction = MTLStoreActionStore;

  impl_->cmd = [impl_->queue commandBuffer];
  impl_->enc = [impl_->cmd renderCommandEncoderWithDescriptor:pass];

  MTLViewport vp = {0, 0, (double)pixelW, (double)pixelH, 0.0, 1.0};
  [impl_->enc setViewport:vp];

  if (impl_->uniformBuffer) {
    auto* uniforms = (Uniforms*)[impl_->uniformBuffer contents];
    float scale = contentScale > 0 ? contentScale : 1.0f;
    uniforms->viewportWidth = (float)pixelW / scale;
    uniforms->viewportHeight = (float)pixelH / scale;
    [impl_->enc setVertexBuffer:impl_->uniformBuffer offset:0 atIndex:1];
  }
}

void MetalRenderer::draw(const DrawList& list) {
  if (!impl_->enc || !impl_->vertexBuffer) {
    return;
  }

  // Draw solid shapes
  if (!list.empty() && impl_->pipeline) {
    const auto& verts = list.vertices();
    size_t vertexCount = verts.size();
    if (vertexCount > Impl::kMaxVertices) {
      vertexCount = Impl::kMaxVertices;
    }

    size_t byteSize = vertexCount * sizeof(Vertex2D);
    memcpy([impl_->vertexBuffer contents], verts.data(), byteSize);

    [impl_->enc setRenderPipelineState:impl_->pipeline];
    [impl_->enc setVertexBuffer:impl_->vertexBuffer offset:0 atIndex:0];
    [impl_->enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:vertexCount];
  }

  // Draw lines
  if (!list.lineEmpty() && impl_->pipeline && impl_->lineVertexBuffer) {
    const auto& lineVerts = list.lineVertices();
    size_t lineVertexCount = lineVerts.size();
    if (lineVertexCount > Impl::kMaxVertices) {
      lineVertexCount = Impl::kMaxVertices;
    }

    size_t lineByteSize = lineVertexCount * sizeof(Vertex2D);
    memcpy([impl_->lineVertexBuffer contents], lineVerts.data(), lineByteSize);

    [impl_->enc setRenderPipelineState:impl_->pipeline];
    [impl_->enc setVertexBuffer:impl_->lineVertexBuffer offset:0 atIndex:0];
    [impl_->enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:lineVertexCount];
  }

  // Draw text
  if (!list.textEmpty() && impl_->textPipeline && impl_->fontTexture && impl_->sampler && impl_->textVertexBuffer) {
    const auto& textVerts = list.textVertices();
    size_t textVertexCount = textVerts.size();
    if (textVertexCount > Impl::kMaxVertices) {
      textVertexCount = Impl::kMaxVertices;
    }

    size_t textByteSize = textVertexCount * sizeof(Vertex2D);
    memcpy([impl_->textVertexBuffer contents], textVerts.data(), textByteSize);

    [impl_->enc setRenderPipelineState:impl_->textPipeline];
    [impl_->enc setVertexBuffer:impl_->textVertexBuffer offset:0 atIndex:0];
    [impl_->enc setFragmentTexture:impl_->fontTexture atIndex:0];
    [impl_->enc setFragmentSamplerState:impl_->sampler atIndex:0];
    [impl_->enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:textVertexCount];
  }
}

void MetalRenderer::endFrame() {
  if (impl_->enc) {
    [impl_->enc endEncoding];
    impl_->enc = nil;
  }

  if (impl_->cmd && impl_->drawable) {
    [impl_->cmd presentDrawable:impl_->drawable];
    [impl_->cmd commit];
  }

  impl_->cmd = nil;
  impl_->drawable = nil;
}

void* MetalRenderer::getDevice() const {
  return (__bridge void*)impl_->device;
}

float MetalRenderer::getContentScale() const {
  return impl_->contentScale;
}

void MetalRenderer::drawTexture(void* texture, void* samplerState, float x, float y, float w, float h) {
  if (!impl_->enc || !impl_->texturePipeline || !impl_->textureVertexBuffer || !texture) {
    return;
  }

  id<MTLTexture> tex = (__bridge id<MTLTexture>)texture;
  id<MTLSamplerState> samp = (__bridge id<MTLSamplerState>)samplerState;
  if (!samp) samp = impl_->sampler;

  Vertex2D verts[6];
  float x0 = x, y0 = y;
  float x1 = x + w, y1 = y + h;
  float r = 1.f, g = 1.f, b = 1.f, a = 1.f;

  verts[0] = {x0, y0, 0.f, 0.f, r, g, b, a};
  verts[1] = {x1, y0, 1.f, 0.f, r, g, b, a};
  verts[2] = {x1, y1, 1.f, 1.f, r, g, b, a};
  verts[3] = {x0, y0, 0.f, 0.f, r, g, b, a};
  verts[4] = {x1, y1, 1.f, 1.f, r, g, b, a};
  verts[5] = {x0, y1, 0.f, 1.f, r, g, b, a};

  memcpy([impl_->textureVertexBuffer contents], verts, sizeof(verts));

  [impl_->enc setRenderPipelineState:impl_->texturePipeline];
  [impl_->enc setVertexBuffer:impl_->textureVertexBuffer offset:0 atIndex:0];
  [impl_->enc setFragmentTexture:tex atIndex:0];
  [impl_->enc setFragmentSamplerState:samp atIndex:0];
  [impl_->enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6];
}

} // namespace oflike
