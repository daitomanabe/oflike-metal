#pragma once

#include <vector>
#include <string>
#include <cmath>
#include <stack>
#include "Types.h"
#include "../oflike/math/ofVectorMath.h"  // For glm::mat4, vec3, etc.

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
  void translate(float x, float y, float z) {
    // 2D rendering: z is ignored for now (oF API compatibility)
    currentMatrix_ = currentMatrix_ * Matrix2D::translation(x, y);
    (void)z; // suppress unused warning
  }
  void rotate(float radians) {
    currentMatrix_ = currentMatrix_ * Matrix2D::rotation(radians);
  }
  void scale(float sx, float sy) {
    currentMatrix_ = currentMatrix_ * Matrix2D::scaling(sx, sy);
  }
  void resetMatrix() { currentMatrix_ = Matrix2D::identity(); }

  // 3D Matrix Stack
  void pushMatrix3D() { matrixStack3D_.push(modelMatrix3D_); }
  void popMatrix3D() {
    if (!matrixStack3D_.empty()) {
      modelMatrix3D_ = matrixStack3D_.top();
      matrixStack3D_.pop();
    }
  }
  void setModelMatrix(const glm::mat4& m) { modelMatrix3D_ = m; }
  void setViewMatrix(const glm::mat4& m) { viewMatrix3D_ = m; }
  void setProjectionMatrix(const glm::mat4& m) { projectionMatrix3D_ = m; }

  glm::mat4 getModelMatrix() const { return modelMatrix3D_; }
  glm::mat4 getViewMatrix() const { return viewMatrix3D_; }
  glm::mat4 getProjectionMatrix() const { return projectionMatrix3D_; }
  glm::mat4 getModelViewMatrix() const { return viewMatrix3D_ * modelMatrix3D_; }
  glm::mat4 getModelViewProjectionMatrix() const {
    return projectionMatrix3D_ * viewMatrix3D_ * modelMatrix3D_;
  }

  void translate3D(float x, float y, float z) {
    modelMatrix3D_ = glm::translate(modelMatrix3D_, glm::vec3(x, y, z));
  }
  void translate3D(const glm::vec3& v) {
    modelMatrix3D_ = glm::translate(modelMatrix3D_, v);
  }
  void rotate3D(float radians, const glm::vec3& axis) {
    modelMatrix3D_ = glm::rotate(modelMatrix3D_, radians, glm::normalize(axis));
  }
  void rotateX3D(float radians) { rotate3D(radians, glm::vec3(1, 0, 0)); }
  void rotateY3D(float radians) { rotate3D(radians, glm::vec3(0, 1, 0)); }
  void rotateZ3D(float radians) { rotate3D(radians, glm::vec3(0, 0, 1)); }
  void scale3D(float sx, float sy, float sz) {
    modelMatrix3D_ = glm::scale(modelMatrix3D_, glm::vec3(sx, sy, sz));
  }
  void scale3D(const glm::vec3& s) {
    modelMatrix3D_ = glm::scale(modelMatrix3D_, s);
  }
  void resetMatrix3D() {
    modelMatrix3D_ = glm::mat4(1.0f);
    viewMatrix3D_ = glm::mat4(1.0f);
    projectionMatrix3D_ = glm::mat4(1.0f);
  }

  // Primitive drawing
  void addRect(float x, float y, float w, float h);
  void addRectRounded(float x, float y, float w, float h, float r, int segments = 8);
  void addRectRounded(float x, float y, float w, float h,
                      float topLeftRadius, float topRightRadius,
                      float bottomRightRadius, float bottomLeftRadius,
                      int segments = 8);
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
  void nextContour(bool close = true);
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
  void addArcVertices(std::vector<Vertex2D>& verts, float cx, float cy, float rx, float ry,
                      float startAngle, float endAngle, int segments);
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

  // 2D Matrix stack
  Matrix2D currentMatrix_ = Matrix2D::identity();
  std::stack<Matrix2D> matrixStack_;

  // 3D Matrix stack (for View/Projection management)
  glm::mat4 modelMatrix3D_{1.0f};
  glm::mat4 viewMatrix3D_{1.0f};
  glm::mat4 projectionMatrix3D_{1.0f};
  std::stack<glm::mat4> matrixStack3D_;

  std::vector<Vertex2D> verts_;
  std::vector<Vertex2D> textVerts_;
  std::vector<Vertex2D> lineVerts_;

  // Shape drawing state
  bool inShape_{false};
  std::vector<Vertex2D> shapeVerts_;
  std::vector<CurvePoint> curveVerts_;
};

} // namespace oflike
