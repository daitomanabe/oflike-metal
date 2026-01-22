#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <AppKit/AppKit.h>

#include "../../oflike/image/ofImage.h"
#include "../../core/Context.h"

ofImage::ofImage() = default;

ofImage::~ofImage() {
  clear();
}

ofImage::ofImage(ofImage&& other) noexcept
  : pixels_(other.pixels_), pixelContainer_(std::move(other.pixelContainer_)),
    texture_(std::move(other.texture_)), width_(other.width_), height_(other.height_),
    channels_(other.channels_) {
  other.pixels_ = nullptr;
  other.width_ = other.height_ = 0;
}

ofImage& ofImage::operator=(ofImage&& other) noexcept {
  if (this != &other) {
    clear();
    pixels_ = other.pixels_;
    pixelContainer_ = std::move(other.pixelContainer_);
    texture_ = std::move(other.texture_);
    width_ = other.width_;
    height_ = other.height_;
    channels_ = other.channels_;
    other.pixels_ = nullptr;
    other.width_ = other.height_ = 0;
  }
  return *this;
}

bool ofImage::load(const std::string& filepath) {
  clear();

  NSString* path = [NSString stringWithUTF8String:filepath.c_str()];
  NSImage* nsImage = [[NSImage alloc] initWithContentsOfFile:path];
  if (!nsImage) return false;

  CGImageRef cgImage = [nsImage CGImageForProposedRect:nil context:nil hints:nil];
  if (!cgImage) return false;

  width_ = (int)CGImageGetWidth(cgImage);
  height_ = (int)CGImageGetHeight(cgImage);
  channels_ = 4;

  allocate(width_, height_, channels_);

  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
  CGContextRef ctx = CGBitmapContextCreate(
    pixels_, width_, height_, 8, width_ * channels_, colorSpace,
    kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big
  );

  CGContextDrawImage(ctx, CGRectMake(0, 0, width_, height_), cgImage);

  CGContextRelease(ctx);
  CGColorSpaceRelease(colorSpace);

  update();
  return true;
}

bool ofImage::save(const std::string& filepath) const {
  if (!pixels_ || width_ <= 0 || height_ <= 0) return false;

  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
  CGContextRef ctx = CGBitmapContextCreate(
    (void*)pixels_, width_, height_, 8, width_ * channels_, colorSpace,
    kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast
  );

  CGImageRef cgImage = CGBitmapContextCreateImage(ctx);

  NSBitmapImageRep* rep = [[NSBitmapImageRep alloc] initWithCGImage:cgImage];
  NSData* pngData = [rep representationUsingType:NSBitmapImageFileTypePNG properties:@{}];

  NSString* path = [NSString stringWithUTF8String:filepath.c_str()];
  BOOL success = [pngData writeToFile:path atomically:YES];

  CGImageRelease(cgImage);
  CGContextRelease(ctx);
  CGColorSpaceRelease(colorSpace);

  return success == YES;
}

void ofImage::clear() {
  if (pixels_) {
    delete[] pixels_;
    pixels_ = nullptr;
  }
  texture_.clear();
  pixelContainer_.clear();
  width_ = height_ = 0;
}

void ofImage::allocate(int width, int height, int channels) {
  clear();

  width_ = width;
  height_ = height;
  channels_ = channels;

  pixels_ = new unsigned char[width * height * channels]();
  pixelContainer_.allocate(width, height, channels);

  // Sync pixel container data pointer
  memset(pixels_, 0, width * height * channels);
}

void ofImage::update() {
  if (!pixels_ || width_ <= 0 || height_ <= 0) return;

  // Sync from pixel container if it has data
  if (pixelContainer_.getData() && pixelContainer_.getWidth() == width_ && pixelContainer_.getHeight() == height_) {
    memcpy(pixels_, pixelContainer_.getData(), width_ * height_ * channels_);
  }

  texture_.loadData(pixels_, width_, height_, channels_);
}

void ofImage::draw(float x, float y) const {
  draw(x, y, (float)width_, (float)height_);
}

void ofImage::draw(float x, float y, float w, float h) const {
  texture_.draw(x, y, w, h);
}

void ofImage::resize(int newWidth, int newHeight) {
  if (!pixels_ || newWidth <= 0 || newHeight <= 0) return;

  unsigned char* newPixels = new unsigned char[newWidth * newHeight * channels_]();

  float xScale = (float)width_ / newWidth;
  float yScale = (float)height_ / newHeight;

  for (int y = 0; y < newHeight; y++) {
    for (int x = 0; x < newWidth; x++) {
      int srcX = (int)(x * xScale);
      int srcY = (int)(y * yScale);
      srcX = srcX < width_ ? srcX : width_ - 1;
      srcY = srcY < height_ ? srcY : height_ - 1;

      int srcIdx = (srcY * width_ + srcX) * channels_;
      int dstIdx = (y * newWidth + x) * channels_;

      for (int c = 0; c < channels_; c++) {
        newPixels[dstIdx + c] = pixels_[srcIdx + c];
      }
    }
  }

  delete[] pixels_;
  pixels_ = newPixels;
  width_ = newWidth;
  height_ = newHeight;

  pixelContainer_.allocate(width_, height_, channels_);
  memcpy(pixelContainer_.getData(), pixels_, width_ * height_ * channels_);

  update();
}

void ofImage::crop(int x, int y, int cropWidth, int cropHeight) {
  if (!pixels_ || x < 0 || y < 0 || x + cropWidth > width_ || y + cropHeight > height_) return;

  unsigned char* newPixels = new unsigned char[cropWidth * cropHeight * channels_]();

  for (int cy = 0; cy < cropHeight; cy++) {
    int srcIdx = ((y + cy) * width_ + x) * channels_;
    int dstIdx = cy * cropWidth * channels_;
    memcpy(newPixels + dstIdx, pixels_ + srcIdx, cropWidth * channels_);
  }

  delete[] pixels_;
  pixels_ = newPixels;
  width_ = cropWidth;
  height_ = cropHeight;

  pixelContainer_.allocate(width_, height_, channels_);
  memcpy(pixelContainer_.getData(), pixels_, width_ * height_ * channels_);

  update();
}

void ofImage::mirror(bool vertical, bool horizontal) {
  if (!pixels_) return;

  unsigned char* newPixels = new unsigned char[width_ * height_ * channels_]();

  for (int y = 0; y < height_; y++) {
    for (int x = 0; x < width_; x++) {
      int srcX = horizontal ? (width_ - 1 - x) : x;
      int srcY = vertical ? (height_ - 1 - y) : y;

      int srcIdx = (srcY * width_ + srcX) * channels_;
      int dstIdx = (y * width_ + x) * channels_;

      for (int c = 0; c < channels_; c++) {
        newPixels[dstIdx + c] = pixels_[srcIdx + c];
      }
    }
  }

  delete[] pixels_;
  pixels_ = newPixels;

  memcpy(pixelContainer_.getData(), pixels_, width_ * height_ * channels_);
  update();
}
