// ofTexture - Metal-based texture implementation for macOS
// NOTE: This implementation uses macOS-specific frameworks:
//   - Metal for GPU textures
//   - ImageIO for image loading (NOT stb_image)
//   - MTKTextureLoader for optimized texture creation
// See ARCHITECTURE.md for the absolute policy on Mac-native APIs.

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

  // Load from native Metal texture (for MTKTextureLoader optimization)
  // Takes ownership of the texture (will be released on clear/destruction)
  void loadFromNativeTexture(void* metalTexture, int width, int height);

  void clear();

  bool isAllocated() const { return texture_ != nullptr; }

  int getWidth() const { return width_; }
  int getHeight() const { return height_; }

  void draw(float x, float y) const;
  void draw(float x, float y, float w, float h) const;

  // Binding (for use with custom rendering)
  void bind(int textureLocation = 0) const;
  void unbind(int textureLocation = 0) const;

  // Texture wrapping modes
  // For Metal: uses MTLSamplerAddressMode
  // GL_REPEAT = 0, GL_CLAMP = 1, GL_CLAMP_TO_EDGE = 2, GL_MIRRORED_REPEAT = 3
  void setTextureWrap(int wrapModeS, int wrapModeT);
  void setTextureMinMagFilter(int minFilter, int magFilter);

  void* getTexturePtr() const { return texture_; }
  void* getSamplerPtr() const { return sampler_; }

private:
  void* texture_{nullptr};
  void* sampler_{nullptr};
  int width_{0};
  int height_{0};
  int channels_{4};
};

// Global image loading functions
bool ofLoadImage(oflike::ofPixels& pix, const std::string& path);
bool ofLoadImage(ofTexture& tex, const std::string& path);
