#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

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
