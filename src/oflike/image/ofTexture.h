#pragma once

#include <memory>

namespace oflike {

// Forward declaration of pixel data container
class ofPixels {
public:
  ofPixels() = default;
  ~ofPixels();

  void allocate(int width, int height, int channels = 4);
  void clear();

  unsigned char* getData() { return data_; }
  const unsigned char* getData() const { return data_; }

  int getWidth() const { return width_; }
  int getHeight() const { return height_; }
  int getChannels() const { return channels_; }

  void setColor(int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
  void getColor(int x, int y, unsigned char& r, unsigned char& g, unsigned char& b, unsigned char& a) const;

private:
  unsigned char* data_{nullptr};
  int width_{0};
  int height_{0};
  int channels_{4};
};

} // namespace oflike

class ofTexture {
public:
  ofTexture();
  ~ofTexture();

  ofTexture(const ofTexture&) = delete;
  ofTexture& operator=(const ofTexture&) = delete;

  ofTexture(ofTexture&& other) noexcept;
  ofTexture& operator=(ofTexture&& other) noexcept;

  void allocate(int width, int height, int channels = 4);
  void loadData(const unsigned char* data, int width, int height, int channels = 4);
  void clear();

  bool isAllocated() const { return texture_ != nullptr; }

  int getWidth() const { return width_; }
  int getHeight() const { return height_; }

  void draw(float x, float y) const;
  void draw(float x, float y, float w, float h) const;

  void* getTexturePtr() const { return texture_; }
  void* getSamplerPtr() const { return sampler_; }

private:
  void* texture_{nullptr};
  void* sampler_{nullptr};
  int width_{0};
  int height_{0};
  int channels_{4};
};
