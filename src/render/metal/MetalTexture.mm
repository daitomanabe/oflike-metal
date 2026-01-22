// MetalTexture.mm - Mac-native optimized texture implementation
// NOTE: This implementation uses macOS-specific frameworks for best performance.
// See ARCHITECTURE.md for the absolute policy on Mac-native APIs.

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <ImageIO/ImageIO.h>
#import <Accelerate/Accelerate.h>

#include "../../oflike/image/ofTexture.h"
#include "../../core/Context.h"
#include "MetalRenderer.h"

namespace oflike {

ofPixels::~ofPixels() {
  clear();
}

void ofPixels::allocate(int width, int height, int channels) {
  clear();
  width_ = width;
  height_ = height;
  channels_ = channels;
  data_ = new unsigned char[width * height * channels]();
}

void ofPixels::clear() {
  if (data_) {
    delete[] data_;
    data_ = nullptr;
  }
  width_ = height_ = 0;
}

void ofPixels::setColor(int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
  if (!data_ || x < 0 || x >= width_ || y < 0 || y >= height_) return;
  int idx = (y * width_ + x) * channels_;
  data_[idx] = r;
  if (channels_ > 1) data_[idx + 1] = g;
  if (channels_ > 2) data_[idx + 2] = b;
  if (channels_ > 3) data_[idx + 3] = a;
}

void ofPixels::getColor(int x, int y, unsigned char& r, unsigned char& g, unsigned char& b, unsigned char& a) const {
  if (!data_ || x < 0 || x >= width_ || y < 0 || y >= height_) {
    r = g = b = a = 0;
    return;
  }
  int idx = (y * width_ + x) * channels_;
  r = data_[idx];
  g = channels_ > 1 ? data_[idx + 1] : r;
  b = channels_ > 2 ? data_[idx + 2] : r;
  a = channels_ > 3 ? data_[idx + 3] : 255;
}

} // namespace oflike

ofTexture::ofTexture() = default;

ofTexture::~ofTexture() {
  clear();
}

ofTexture::ofTexture(ofTexture&& other) noexcept
  : texture_(other.texture_), sampler_(other.sampler_),
    width_(other.width_), height_(other.height_), channels_(other.channels_) {
  other.texture_ = nullptr;
  other.sampler_ = nullptr;
  other.width_ = other.height_ = 0;
}

ofTexture& ofTexture::operator=(ofTexture&& other) noexcept {
  if (this != &other) {
    clear();
    texture_ = other.texture_;
    sampler_ = other.sampler_;
    width_ = other.width_;
    height_ = other.height_;
    channels_ = other.channels_;
    other.texture_ = nullptr;
    other.sampler_ = nullptr;
    other.width_ = other.height_ = 0;
  }
  return *this;
}

void ofTexture::allocate(int width, int height, int channels) {
  clear();

  id<MTLDevice> device = (__bridge id<MTLDevice>)oflike::engine().renderer().getDevice();
  if (!device) return;

  MTLPixelFormat format = MTLPixelFormatRGBA8Unorm;
  if (channels == 1) format = MTLPixelFormatR8Unorm;
  else if (channels == 2) format = MTLPixelFormatRG8Unorm;

  MTLTextureDescriptor* desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format
                                                                                  width:width
                                                                                 height:height
                                                                              mipmapped:NO];
  desc.usage = MTLTextureUsageShaderRead;
  desc.storageMode = MTLStorageModeShared;

  id<MTLTexture> tex = [device newTextureWithDescriptor:desc];
  if (!tex) return;

  MTLSamplerDescriptor* samplerDesc = [[MTLSamplerDescriptor alloc] init];
  samplerDesc.minFilter = MTLSamplerMinMagFilterLinear;
  samplerDesc.magFilter = MTLSamplerMinMagFilterLinear;
  samplerDesc.sAddressMode = MTLSamplerAddressModeClampToEdge;
  samplerDesc.tAddressMode = MTLSamplerAddressModeClampToEdge;
  id<MTLSamplerState> samp = [device newSamplerStateWithDescriptor:samplerDesc];

  texture_ = (__bridge_retained void*)tex;
  sampler_ = (__bridge_retained void*)samp;
  width_ = width;
  height_ = height;
  channels_ = channels;
}

void ofTexture::loadData(const unsigned char* data, int width, int height, int channels) {
  if (!texture_ || width != width_ || height != height_) {
    allocate(width, height, channels);
  }

  if (!texture_ || !data) return;

  id<MTLTexture> tex = (__bridge id<MTLTexture>)texture_;
  MTLRegion region = MTLRegionMake2D(0, 0, width, height);
  [tex replaceRegion:region mipmapLevel:0 withBytes:data bytesPerRow:width * channels];
}

// Load from native Metal texture (MTKTextureLoader optimization)
// This method takes ownership of the texture and creates a sampler
void ofTexture::loadFromNativeTexture(void* metalTexture, int width, int height) {
  clear();

  if (!metalTexture) return;

  id<MTLDevice> device = (__bridge id<MTLDevice>)oflike::engine().renderer().getDevice();
  if (!device) return;

  // Retain the texture (take ownership)
  id<MTLTexture> tex = (__bridge id<MTLTexture>)metalTexture;
  texture_ = (__bridge_retained void*)[tex retain];

  // Create default sampler
  MTLSamplerDescriptor* samplerDesc = [[MTLSamplerDescriptor alloc] init];
  samplerDesc.minFilter = MTLSamplerMinMagFilterLinear;
  samplerDesc.magFilter = MTLSamplerMinMagFilterLinear;
  samplerDesc.sAddressMode = MTLSamplerAddressModeClampToEdge;
  samplerDesc.tAddressMode = MTLSamplerAddressModeClampToEdge;
  id<MTLSamplerState> samp = [device newSamplerStateWithDescriptor:samplerDesc];
  sampler_ = (__bridge_retained void*)samp;

  width_ = width;
  height_ = height;
  channels_ = 4;  // MTKTextureLoader typically loads as RGBA
}

void ofTexture::clear() {
  if (texture_) {
    CFRelease(texture_);
    texture_ = nullptr;
  }
  if (sampler_) {
    CFRelease(sampler_);
    sampler_ = nullptr;
  }
  width_ = height_ = 0;
}

void ofTexture::draw(float x, float y) const {
  draw(x, y, (float)width_, (float)height_);
}

void ofTexture::draw(float x, float y, float w, float h) const {
  if (!texture_) return;
  oflike::engine().renderer().drawTexture(texture_, sampler_, x, y, w, h);
}

void ofTexture::bind(int textureLocation) const {
  if (!texture_) return;
  oflike::engine().renderer().bindTexture(texture_, sampler_, textureLocation);
}

void ofTexture::unbind(int textureLocation) const {
  oflike::engine().renderer().unbindTexture(textureLocation);
}

void ofTexture::setTextureWrap(int wrapModeS, int wrapModeT) {
  if (!texture_) return;

  id<MTLDevice> device = (__bridge id<MTLDevice>)oflike::engine().renderer().getDevice();
  if (!device) return;

  // Convert GL-style wrap modes to Metal
  auto convertWrapMode = [](int glMode) -> MTLSamplerAddressMode {
    switch (glMode) {
      case 0: return MTLSamplerAddressModeRepeat;       // GL_REPEAT
      case 1: return MTLSamplerAddressModeClampToEdge;  // GL_CLAMP
      case 2: return MTLSamplerAddressModeClampToEdge;  // GL_CLAMP_TO_EDGE
      case 3: return MTLSamplerAddressModeMirrorRepeat; // GL_MIRRORED_REPEAT
      default: return MTLSamplerAddressModeClampToEdge;
    }
  };

  MTLSamplerDescriptor* samplerDesc = [[MTLSamplerDescriptor alloc] init];
  samplerDesc.minFilter = MTLSamplerMinMagFilterLinear;
  samplerDesc.magFilter = MTLSamplerMinMagFilterLinear;
  samplerDesc.sAddressMode = convertWrapMode(wrapModeS);
  samplerDesc.tAddressMode = convertWrapMode(wrapModeT);

  if (sampler_) {
    CFRelease(sampler_);
  }
  id<MTLSamplerState> samp = [device newSamplerStateWithDescriptor:samplerDesc];
  sampler_ = (__bridge_retained void*)samp;
}

void ofTexture::setTextureMinMagFilter(int minFilter, int magFilter) {
  if (!texture_) return;

  id<MTLDevice> device = (__bridge id<MTLDevice>)oflike::engine().renderer().getDevice();
  if (!device) return;

  // Convert GL-style filter modes to Metal
  auto convertFilter = [](int glMode) -> MTLSamplerMinMagFilter {
    // GL_NEAREST = 0x2600, GL_LINEAR = 0x2601
    return (glMode == 0x2600) ? MTLSamplerMinMagFilterNearest : MTLSamplerMinMagFilterLinear;
  };

  MTLSamplerDescriptor* samplerDesc = [[MTLSamplerDescriptor alloc] init];
  samplerDesc.minFilter = convertFilter(minFilter);
  samplerDesc.magFilter = convertFilter(magFilter);
  samplerDesc.sAddressMode = MTLSamplerAddressModeClampToEdge;
  samplerDesc.tAddressMode = MTLSamplerAddressModeClampToEdge;

  if (sampler_) {
    CFRelease(sampler_);
  }
  id<MTLSamplerState> samp = [device newSamplerStateWithDescriptor:samplerDesc];
  sampler_ = (__bridge_retained void*)samp;
}

// Global ofLoadImage functions using ImageIO (Mac-native high-performance API)
// NOTE: This uses CGImageSource instead of NSImage for faster decoding.
// ImageIO provides hardware-accelerated decoding on supported hardware.
bool ofLoadImage(oflike::ofPixels& pix, const std::string& path) {
  @autoreleasepool {
    // Create URL from path
    NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
    NSURL* url = [NSURL fileURLWithPath:nsPath];
    if (!url) return false;

    // Create image source using ImageIO (faster than NSImage)
    CGImageSourceRef imageSource = CGImageSourceCreateWithURL((__bridge CFURLRef)url, nil);
    if (!imageSource) return false;

    // Create CGImage from source
    CGImageRef cgImage = CGImageSourceCreateImageAtIndex(imageSource, 0, nil);
    CFRelease(imageSource);
    if (!cgImage) return false;

    // Get image dimensions
    int w = (int)CGImageGetWidth(cgImage);
    int h = (int)CGImageGetHeight(cgImage);

    // Allocate pixel buffer
    pix.allocate(w, h, 4);
    unsigned char* data = pix.getData();

    // Create color space (sRGB for consistent color handling)
    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);

    // Create bitmap context for RGBA output
    CGBitmapInfo bitmapInfo = (CGBitmapInfo)(kCGImageAlphaPremultipliedLast) | kCGBitmapByteOrder32Big;
    CGContextRef ctx = CGBitmapContextCreate(
      data,
      w, h,
      8,                          // bits per component
      w * 4,                      // bytes per row
      colorSpace,
      bitmapInfo
    );

    CGColorSpaceRelease(colorSpace);

    if (!ctx) {
      CGImageRelease(cgImage);
      return false;
    }

    // Draw image to context (this performs the decode)
    CGContextDrawImage(ctx, CGRectMake(0, 0, w, h), cgImage);

    // Cleanup
    CGContextRelease(ctx);
    CGImageRelease(cgImage);

    return true;
  }
}

// Direct texture loading using MTKTextureLoader (Metal-native, FASTEST method)
// MTKTextureLoader performs hardware-accelerated decoding directly to GPU memory.
// This bypasses CPU-side pixel manipulation entirely.
bool ofLoadImage(ofTexture& tex, const std::string& path) {
  @autoreleasepool {
    // Get Metal device
    id<MTLDevice> device = (__bridge id<MTLDevice>)oflike::engine().renderer().getDevice();
    if (!device) {
      // Fallback to pixel-based loading (ImageIO)
      oflike::ofPixels pix;
      if (!ofLoadImage(pix, path)) return false;
      tex.loadData(pix.getData(), pix.getWidth(), pix.getHeight(), pix.getChannels());
      return true;
    }

    // Use MTKTextureLoader for direct GPU texture loading (FASTEST)
    NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
    NSURL* url = [NSURL fileURLWithPath:nsPath];
    if (!url) return false;

    MTKTextureLoader* loader = [[MTKTextureLoader alloc] initWithDevice:device];

    // Options for texture loading - optimized for performance
    NSDictionary* options = @{
      MTKTextureLoaderOptionSRGB: @NO,
      MTKTextureLoaderOptionGenerateMipmaps: @NO,
      MTKTextureLoaderOptionTextureUsage: @(MTLTextureUsageShaderRead),
      MTKTextureLoaderOptionTextureStorageMode: @(MTLStorageModeShared)
    };

    NSError* error = nil;
    id<MTLTexture> mtlTexture = [loader newTextureWithContentsOfURL:url options:options error:&error];

    if (!mtlTexture || error) {
      // Fallback to ImageIO-based loading if MTKTextureLoader fails
      oflike::ofPixels pix;
      if (!ofLoadImage(pix, path)) return false;
      tex.loadData(pix.getData(), pix.getWidth(), pix.getHeight(), pix.getChannels());
      return true;
    }

    // Use loadFromNativeTexture for direct Metal texture assignment (no CPU copy!)
    int w = (int)mtlTexture.width;
    int h = (int)mtlTexture.height;
    tex.loadFromNativeTexture((__bridge void*)mtlTexture, w, h);

    return true;
  }
}
