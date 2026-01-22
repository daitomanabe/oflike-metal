#pragma once

#include <string>
#include <memory>
#include "ofTexture.h"

class ofImage {
public:
  ofImage();
  ~ofImage();

  ofImage(const ofImage&) = delete;
  ofImage& operator=(const ofImage&) = delete;

  ofImage(ofImage&& other) noexcept;
  ofImage& operator=(ofImage&& other) noexcept;

  bool load(const std::string& filepath);
  bool save(const std::string& filepath) const;
  void clear();

  void allocate(int width, int height, int channels = 4);
  void update();

  void draw(float x, float y) const;
  void draw(float x, float y, float w, float h) const;

  int getWidth() const { return width_; }
  int getHeight() const { return height_; }
  int getChannels() const { return channels_; }

  bool isAllocated() const { return pixels_ != nullptr; }

  unsigned char* getPixelData() { return pixels_; }
  const unsigned char* getPixelData() const { return pixels_; }

  oflike::ofPixels& getPixels() { return pixelContainer_; }
  const oflike::ofPixels& getPixels() const { return pixelContainer_; }

  void resize(int newWidth, int newHeight);
  void crop(int x, int y, int width, int height);
  void mirror(bool vertical, bool horizontal);

  const ofTexture& getTexture() const { return texture_; }

private:
  unsigned char* pixels_{nullptr};
  oflike::ofPixels pixelContainer_;
  ofTexture texture_;
  int width_{0};
  int height_{0};
  int channels_{4};
};
