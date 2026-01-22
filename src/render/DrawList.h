#pragma once

#include <vector>
#include <string>
#include <cmath>
#include <stack>
#include "Types.h"

namespace oflike {

// DrawList records draw data in an immediate-mode style.
// The renderer consumes the recorded vertices and issues draw calls.
class DrawList {
public:
  void reset() {
    verts_.clear();
    textVerts_.clear();
    lineVerts_.clear();
  }

  void reserve(std::size_t n) { verts_.reserve(n); }
  void reserveText(std::size_t n) { textVerts_.reserve(n); }

  void setColor(Color4f c) { current_ = c; }
  void setFilled(bool filled) { filled_ = filled; }
  bool isFilled() const { return filled_; }
  void setLineWidth(float w) { lineWidth_ = w; }
  float getLineWidth() const { return lineWidth_; }

  // Matrix stack operations
  void pushMatrix() { matrixStack_.push(currentMatrix_); }
  void popMatrix() {
    if (!matrixStack_.empty()) {
      currentMatrix_ = matrixStack_.top();
      matrixStack_.pop();
    }
  }
  void translate(float x, float y) {
    currentMatrix_ = currentMatrix_ * Matrix2D::translation(x, y);
  }
  void rotate(float radians) {
    currentMatrix_ = currentMatrix_ * Matrix2D::rotation(radians);
  }
  void scale(float sx, float sy) {
    currentMatrix_ = currentMatrix_ * Matrix2D::scaling(sx, sy);
  }
  void resetMatrix() { currentMatrix_ = Matrix2D::identity(); }

  // Primitive drawing
  void addRect(float x, float y, float w, float h);
  void addCircle(float cx, float cy, float radius, int segments = 32);
  void addEllipse(float cx, float cy, float rx, float ry, int segments = 32);
  void addTriangle(float x1, float y1, float x2, float y2, float x3, float y3);
  void addLine(float x1, float y1, float x2, float y2);

  // Text drawing
  void addText(const std::string& text, float x, float y, float charW = 8.f, float charH = 12.f);

  // Custom shape drawing
  void beginShape();
  void vertex(float x, float y);
  void vertex(float x, float y, float u, float v);
  void curveVertex(float x, float y);
  void bezierVertex(float cx1, float cy1, float cx2, float cy2, float x, float y);
  void endShape(bool close = false);

  // Access recorded vertices
  const std::vector<Vertex2D>& vertices() const { return verts_; }
  const std::vector<Vertex2D>& textVertices() const { return textVerts_; }
  const std::vector<Vertex2D>& lineVertices() const { return lineVerts_; }
  bool empty() const { return verts_.empty(); }
  bool textEmpty() const { return textVerts_.empty(); }
  bool lineEmpty() const { return lineVerts_.empty(); }

private:
  struct CurvePoint { float x, y; };

  void addLineRaw(float x1, float y1, float x2, float y2);
  void processCurveVertices();

  inline void applyTransform(float& x, float& y) {
    currentMatrix_.transform(x, y);
  }

  inline void push(float x, float y, float u, float v) {
    verts_.push_back(Vertex2D{x, y, u, v, current_.r, current_.g, current_.b, current_.a});
  }

  inline void pushText(float x, float y, float u, float v) {
    textVerts_.push_back(Vertex2D{x, y, u, v, current_.r, current_.g, current_.b, current_.a});
  }

  inline void pushLine(float x, float y, float u, float v) {
    lineVerts_.push_back(Vertex2D{x, y, u, v, current_.r, current_.g, current_.b, current_.a});
  }

  Color4f current_{};
  bool filled_{true};
  float lineWidth_{1.0f};
  Matrix2D currentMatrix_ = Matrix2D::identity();
  std::stack<Matrix2D> matrixStack_;
  std::vector<Vertex2D> verts_;
  std::vector<Vertex2D> textVerts_;
  std::vector<Vertex2D> lineVerts_;

  // Shape drawing state
  bool inShape_{false};
  std::vector<Vertex2D> shapeVerts_;
  std::vector<CurvePoint> curveVerts_;
};

} // namespace oflike
