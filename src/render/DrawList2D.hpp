#pragma once

#include <vector>
#include <string>

namespace oflike {

struct Color4f {
  float r{1.f}, g{1.f}, b{1.f}, a{1.f};
};

struct Vertex2D {
  float x{0.f}, y{0.f};
  float u{0.f}, v{0.f};
  float r{1.f}, g{1.f}, b{1.f}, a{1.f};
};

// DrawList2D records draw data in an immediate-mode style, but does not touch Metal.
// Renderer2D consumes the recorded vertices and issues Metal draw calls in a single flush.
class DrawList2D {
public:
  void reset() {
    verts_.clear();
    textVerts_.clear();
  }

  void reserve(std::size_t n) { verts_.reserve(n); }
  void reserveText(std::size_t n) { textVerts_.reserve(n); }

  void setColor(Color4f c) { current_ = c; }

  // Adds a rectangle as 2 triangles (6 vertices). Coordinates are in points.
  void addRect(float x, float y, float w, float h) {
    const float x0 = x;
    const float y0 = y;
    const float x1 = x + w;
    const float y1 = y + h;

    // Triangle 1
    push(x0, y0, 0.f, 0.f);
    push(x1, y0, 1.f, 0.f);
    push(x1, y1, 1.f, 1.f);

    // Triangle 2
    push(x0, y0, 0.f, 0.f);
    push(x1, y1, 1.f, 1.f);
    push(x0, y1, 0.f, 1.f);
  }

  // Add a text string using bitmap font
  // charW/charH: size of each character in points
  void addText(const std::string& text, float x, float y, float charW = 8.f, float charH = 12.f) {
    // Font atlas is 16x16 characters (256 ASCII chars), each 8x8 pixels
    constexpr int kAtlasCols = 16;
    constexpr int kAtlasRows = 16;
    constexpr float kCellU = 1.f / kAtlasCols;
    constexpr float kCellV = 1.f / kAtlasRows;

    float cx = x;
    float cy = y;

    for (char c : text) {
      if (c == '\n') {
        cx = x;
        cy += charH;
        continue;
      }

      unsigned char uc = static_cast<unsigned char>(c);
      int col = uc % kAtlasCols;
      int row = uc / kAtlasCols;

      float u0 = col * kCellU;
      float v0 = row * kCellV;
      float u1 = u0 + kCellU;
      float v1 = v0 + kCellV;

      float x0 = cx;
      float y0 = cy;
      float x1 = cx + charW;
      float y1 = cy + charH;

      // Triangle 1
      pushText(x0, y0, u0, v0);
      pushText(x1, y0, u1, v0);
      pushText(x1, y1, u1, v1);

      // Triangle 2
      pushText(x0, y0, u0, v0);
      pushText(x1, y1, u1, v1);
      pushText(x0, y1, u0, v1);

      cx += charW;
    }
  }

  const std::vector<Vertex2D>& vertices() const { return verts_; }
  const std::vector<Vertex2D>& textVertices() const { return textVerts_; }
  bool empty() const { return verts_.empty(); }
  bool textEmpty() const { return textVerts_.empty(); }

private:
  inline void push(float x, float y, float u, float v) {
    verts_.push_back(Vertex2D{x, y, u, v, current_.r, current_.g, current_.b, current_.a});
  }

  inline void pushText(float x, float y, float u, float v) {
    textVerts_.push_back(Vertex2D{x, y, u, v, current_.r, current_.g, current_.b, current_.a});
  }

  Color4f current_{};
  std::vector<Vertex2D> verts_;
  std::vector<Vertex2D> textVerts_;
};

} // namespace oflike
