#pragma once

#include <vector>
#include <cstdint>
#include "../graphics/ofColor.h"
#include "../graphics/ofGraphics.h"
#include "../../core/Context.h"
#include "../math/ofVectorMath.h"

// ofMesh template class
// V = vertex type, N = normal type, C = color type, T = texcoord type
template<class V = glm::vec3, class N = glm::vec3, class C = ofFloatColor, class T = glm::vec2>
class ofMesh_ {
public:
  ofMesh_() = default;

  // Mode
  void setMode(ofPrimitiveMode mode) { mode_ = mode; }
  ofPrimitiveMode getMode() const { return mode_; }

  // Vertices
  void addVertex(const V& v) {
    vertices_.push_back(v);
  }

  void addVertices(const std::vector<V>& verts) {
    vertices_.insert(vertices_.end(), verts.begin(), verts.end());
  }

  void addVertices(const V* verts, std::size_t count) {
    vertices_.insert(vertices_.end(), verts, verts + count);
  }

  void setVertex(std::size_t index, const V& v) {
    if (index < vertices_.size()) vertices_[index] = v;
  }

  V& getVertex(std::size_t index) { return vertices_[index]; }
  const V& getVertex(std::size_t index) const { return vertices_[index]; }

  std::vector<V>& getVertices() { return vertices_; }
  const std::vector<V>& getVertices() const { return vertices_; }

  std::size_t getNumVertices() const { return vertices_.size(); }

  void clearVertices() { vertices_.clear(); }

  // Colors
  void addColor(const C& c) {
    colors_.push_back(c);
  }

  void addColors(const std::vector<C>& cols) {
    colors_.insert(colors_.end(), cols.begin(), cols.end());
  }

  void setColor(std::size_t index, const C& c) {
    if (index < colors_.size()) colors_[index] = c;
  }

  C& getColor(std::size_t index) { return colors_[index]; }
  const C& getColor(std::size_t index) const { return colors_[index]; }

  std::vector<C>& getColors() { return colors_; }
  const std::vector<C>& getColors() const { return colors_; }

  std::size_t getNumColors() const { return colors_.size(); }

  void clearColors() { colors_.clear(); }

  void enableColors() { useColors_ = true; }
  void disableColors() { useColors_ = false; }
  bool usingColors() const { return useColors_; }

  // Normals
  void addNormal(const N& n) {
    normals_.push_back(n);
  }

  void addNormals(const std::vector<N>& norms) {
    normals_.insert(normals_.end(), norms.begin(), norms.end());
  }

  void setNormal(std::size_t index, const N& n) {
    if (index < normals_.size()) normals_[index] = n;
  }

  N& getNormal(std::size_t index) { return normals_[index]; }
  const N& getNormal(std::size_t index) const { return normals_[index]; }

  std::vector<N>& getNormals() { return normals_; }
  const std::vector<N>& getNormals() const { return normals_; }

  std::size_t getNumNormals() const { return normals_.size(); }

  void clearNormals() { normals_.clear(); }

  void enableNormals() { useNormals_ = true; }
  void disableNormals() { useNormals_ = false; }
  bool usingNormals() const { return useNormals_; }

  // Texture Coordinates
  void addTexCoord(const T& t) {
    texCoords_.push_back(t);
  }

  void addTexCoords(const std::vector<T>& coords) {
    texCoords_.insert(texCoords_.end(), coords.begin(), coords.end());
  }

  void setTexCoord(std::size_t index, const T& t) {
    if (index < texCoords_.size()) texCoords_[index] = t;
  }

  T& getTexCoord(std::size_t index) { return texCoords_[index]; }
  const T& getTexCoord(std::size_t index) const { return texCoords_[index]; }

  std::vector<T>& getTexCoords() { return texCoords_; }
  const std::vector<T>& getTexCoords() const { return texCoords_; }

  std::size_t getNumTexCoords() const { return texCoords_.size(); }

  void clearTexCoords() { texCoords_.clear(); }

  void enableTextures() { useTextures_ = true; }
  void disableTextures() { useTextures_ = false; }
  bool usingTextures() const { return useTextures_; }

  // Indices
  void addIndex(uint32_t i) {
    indices_.push_back(i);
  }

  void addIndices(const std::vector<uint32_t>& inds) {
    indices_.insert(indices_.end(), inds.begin(), inds.end());
  }

  void addIndices(const uint32_t* inds, std::size_t count) {
    indices_.insert(indices_.end(), inds, inds + count);
  }

  void addTriangle(uint32_t i1, uint32_t i2, uint32_t i3) {
    indices_.push_back(i1);
    indices_.push_back(i2);
    indices_.push_back(i3);
  }

  void setIndex(std::size_t pos, uint32_t i) {
    if (pos < indices_.size()) indices_[pos] = i;
  }

  uint32_t getIndex(std::size_t pos) const { return indices_[pos]; }

  std::vector<uint32_t>& getIndices() { return indices_; }
  const std::vector<uint32_t>& getIndices() const { return indices_; }

  std::size_t getNumIndices() const { return indices_.size(); }

  void clearIndices() { indices_.clear(); }

  void enableIndices() { useIndices_ = true; }
  void disableIndices() { useIndices_ = false; }
  bool hasIndices() const { return !indices_.empty(); }
  bool usingIndices() const { return useIndices_ && !indices_.empty(); }

  // Clear all
  void clear() {
    clearVertices();
    clearColors();
    clearNormals();
    clearTexCoords();
    clearIndices();
  }

  // Draw the mesh
  void draw() const {
    if (vertices_.empty()) return;

    auto& dl = oflike::engine().drawList();

    // Get vertex order (use indices if available, otherwise sequential)
    std::vector<std::size_t> order;
    if (usingIndices()) {
      order.reserve(indices_.size());
      for (auto idx : indices_) {
        order.push_back(static_cast<std::size_t>(idx));
      }
    } else {
      order.reserve(vertices_.size());
      for (std::size_t i = 0; i < vertices_.size(); i++) {
        order.push_back(i);
      }
    }

    // Determine if we have per-vertex colors
    bool hasColors = useColors_ && colors_.size() >= vertices_.size();

    // Save current color
    auto savedColor = oflike::engine().getStyle().color;

    switch (mode_) {
      case OF_PRIMITIVE_LINE_STRIP:
        drawLineStrip(dl, order, hasColors);
        break;
      case OF_PRIMITIVE_LINE_LOOP:
        drawLineLoop(dl, order, hasColors);
        break;
      case OF_PRIMITIVE_LINES:
        drawLines(dl, order, hasColors);
        break;
      case OF_PRIMITIVE_TRIANGLES:
        drawTriangles(dl, order, hasColors);
        break;
      case OF_PRIMITIVE_TRIANGLE_STRIP:
        drawTriangleStrip(dl, order, hasColors);
        break;
      case OF_PRIMITIVE_TRIANGLE_FAN:
        drawTriangleFan(dl, order, hasColors);
        break;
      case OF_PRIMITIVE_POINTS:
        drawPoints(dl, order, hasColors);
        break;
    }

    // Restore color
    dl.setColor(savedColor);
  }

  void drawWireframe() const {
    // For wireframe, draw as lines regardless of mode
    if (vertices_.empty()) return;

    auto& dl = oflike::engine().drawList();
    bool hasColors = useColors_ && colors_.size() >= vertices_.size();
    auto savedColor = oflike::engine().getStyle().color;

    std::vector<std::size_t> order;
    if (usingIndices()) {
      for (auto idx : indices_) order.push_back(static_cast<std::size_t>(idx));
    } else {
      for (std::size_t i = 0; i < vertices_.size(); i++) order.push_back(i);
    }

    if (mode_ == OF_PRIMITIVE_TRIANGLES) {
      // Draw triangle edges
      for (std::size_t i = 0; i + 2 < order.size(); i += 3) {
        drawLineSegment(dl, order[i], order[i + 1], hasColors);
        drawLineSegment(dl, order[i + 1], order[i + 2], hasColors);
        drawLineSegment(dl, order[i + 2], order[i], hasColors);
      }
    } else {
      drawLineStrip(dl, order, hasColors);
    }

    dl.setColor(savedColor);
  }

  void drawVertices() const {
    // Draw as points
    if (vertices_.empty()) return;

    auto& dl = oflike::engine().drawList();
    bool hasColors = useColors_ && colors_.size() >= vertices_.size();
    auto savedColor = oflike::engine().getStyle().color;

    for (std::size_t i = 0; i < vertices_.size(); i++) {
      if (hasColors) {
        setDrawListColor(dl, colors_[i]);
      }
      const auto& v = vertices_[i];
      // Draw a small circle for each point
      float pointSize = oflike::engine().getPointSize();
      dl.addCircle(v.x, v.y, pointSize * 0.5f, 8);
    }

    dl.setColor(savedColor);
  }

private:
  void setDrawListColor(oflike::DrawList& dl, const C& c) const {
    dl.setColor(oflike::Color4f{c.r, c.g, c.b, c.a});
  }

  void drawLineSegment(oflike::DrawList& dl, std::size_t i1, std::size_t i2, bool hasColors) const {
    if (hasColors) {
      // Use average color for the line
      const auto& c1 = colors_[i1];
      const auto& c2 = colors_[i2];
      dl.setColor(oflike::Color4f{
        (c1.r + c2.r) * 0.5f,
        (c1.g + c2.g) * 0.5f,
        (c1.b + c2.b) * 0.5f,
        (c1.a + c2.a) * 0.5f
      });
    }
    const auto& v1 = vertices_[i1];
    const auto& v2 = vertices_[i2];
    dl.addLine(v1.x, v1.y, v2.x, v2.y);
  }

  void drawLineStrip(oflike::DrawList& dl, const std::vector<std::size_t>& order, bool hasColors) const {
    if (order.size() < 2) return;

    for (std::size_t i = 0; i + 1 < order.size(); i++) {
      drawLineSegment(dl, order[i], order[i + 1], hasColors);
    }
  }

  void drawLineLoop(oflike::DrawList& dl, const std::vector<std::size_t>& order, bool hasColors) const {
    if (order.size() < 2) return;

    drawLineStrip(dl, order, hasColors);
    // Close the loop
    drawLineSegment(dl, order.back(), order.front(), hasColors);
  }

  void drawLines(oflike::DrawList& dl, const std::vector<std::size_t>& order, bool hasColors) const {
    for (std::size_t i = 0; i + 1 < order.size(); i += 2) {
      drawLineSegment(dl, order[i], order[i + 1], hasColors);
    }
  }

  void drawTriangles(oflike::DrawList& dl, const std::vector<std::size_t>& order, bool hasColors) const {
    bool wasFilled = dl.isFilled();
    dl.setFilled(true);

    for (std::size_t i = 0; i + 2 < order.size(); i += 3) {
      std::size_t i1 = order[i], i2 = order[i + 1], i3 = order[i + 2];

      if (hasColors) {
        // Use average color for the triangle
        const auto& c1 = colors_[i1];
        const auto& c2 = colors_[i2];
        const auto& c3 = colors_[i3];
        dl.setColor(oflike::Color4f{
          (c1.r + c2.r + c3.r) / 3.f,
          (c1.g + c2.g + c3.g) / 3.f,
          (c1.b + c2.b + c3.b) / 3.f,
          (c1.a + c2.a + c3.a) / 3.f
        });
      }

      const auto& v1 = vertices_[i1];
      const auto& v2 = vertices_[i2];
      const auto& v3 = vertices_[i3];
      dl.addTriangle(v1.x, v1.y, v2.x, v2.y, v3.x, v3.y);
    }

    dl.setFilled(wasFilled);
  }

  void drawTriangleStrip(oflike::DrawList& dl, const std::vector<std::size_t>& order, bool hasColors) const {
    if (order.size() < 3) return;

    bool wasFilled = dl.isFilled();
    dl.setFilled(true);

    for (std::size_t i = 0; i + 2 < order.size(); i++) {
      std::size_t i1, i2, i3;
      if (i % 2 == 0) {
        i1 = order[i]; i2 = order[i + 1]; i3 = order[i + 2];
      } else {
        i1 = order[i + 1]; i2 = order[i]; i3 = order[i + 2];
      }

      if (hasColors) {
        const auto& c1 = colors_[i1];
        const auto& c2 = colors_[i2];
        const auto& c3 = colors_[i3];
        dl.setColor(oflike::Color4f{
          (c1.r + c2.r + c3.r) / 3.f,
          (c1.g + c2.g + c3.g) / 3.f,
          (c1.b + c2.b + c3.b) / 3.f,
          (c1.a + c2.a + c3.a) / 3.f
        });
      }

      const auto& v1 = vertices_[i1];
      const auto& v2 = vertices_[i2];
      const auto& v3 = vertices_[i3];
      dl.addTriangle(v1.x, v1.y, v2.x, v2.y, v3.x, v3.y);
    }

    dl.setFilled(wasFilled);
  }

  void drawTriangleFan(oflike::DrawList& dl, const std::vector<std::size_t>& order, bool hasColors) const {
    if (order.size() < 3) return;

    bool wasFilled = dl.isFilled();
    dl.setFilled(true);

    std::size_t centerIdx = order[0];

    for (std::size_t i = 1; i + 1 < order.size(); i++) {
      std::size_t i1 = centerIdx, i2 = order[i], i3 = order[i + 1];

      if (hasColors) {
        const auto& c1 = colors_[i1];
        const auto& c2 = colors_[i2];
        const auto& c3 = colors_[i3];
        dl.setColor(oflike::Color4f{
          (c1.r + c2.r + c3.r) / 3.f,
          (c1.g + c2.g + c3.g) / 3.f,
          (c1.b + c2.b + c3.b) / 3.f,
          (c1.a + c2.a + c3.a) / 3.f
        });
      }

      const auto& v1 = vertices_[i1];
      const auto& v2 = vertices_[i2];
      const auto& v3 = vertices_[i3];
      dl.addTriangle(v1.x, v1.y, v2.x, v2.y, v3.x, v3.y);
    }

    dl.setFilled(wasFilled);
  }

  void drawPoints(oflike::DrawList& dl, const std::vector<std::size_t>& order, bool hasColors) const {
    float pointSize = oflike::engine().getPointSize();

    for (std::size_t idx : order) {
      if (hasColors) {
        setDrawListColor(dl, colors_[idx]);
      }
      const auto& v = vertices_[idx];
      dl.addCircle(v.x, v.y, pointSize * 0.5f, 8);
    }
  }

private:
  std::vector<V> vertices_;
  std::vector<C> colors_;
  std::vector<N> normals_;
  std::vector<T> texCoords_;
  std::vector<uint32_t> indices_;

  ofPrimitiveMode mode_ = OF_PRIMITIVE_TRIANGLES;
  bool useColors_ = true;
  bool useNormals_ = true;
  bool useTextures_ = true;
  bool useIndices_ = true;
};

// Default type alias
using ofMesh = ofMesh_<glm::vec3, glm::vec3, ofFloatColor, glm::vec2>;
