#include "DrawList.h"

namespace oflike {

void DrawList::addRect(float x, float y, float w, float h) {
  if (filled_) {
    float x0 = x, y0 = y;
    float x1 = x + w, y1 = y;
    float x2 = x + w, y2 = y + h;
    float x3 = x, y3 = y + h;

    applyTransform(x0, y0);
    applyTransform(x1, y1);
    applyTransform(x2, y2);
    applyTransform(x3, y3);

    push(x0, y0, 0.f, 0.f);
    push(x1, y1, 1.f, 0.f);
    push(x2, y2, 1.f, 1.f);

    push(x0, y0, 0.f, 0.f);
    push(x2, y2, 1.f, 1.f);
    push(x3, y3, 0.f, 1.f);
  } else {
    addLine(x, y, x + w, y);
    addLine(x + w, y, x + w, y + h);
    addLine(x + w, y + h, x, y + h);
    addLine(x, y + h, x, y);
  }
}

void DrawList::addCircle(float cx, float cy, float radius, int segments) {
  addEllipse(cx, cy, radius, radius, segments);
}

void DrawList::addRectRounded(float x, float y, float w, float h, float r, int segments) {
  addRectRounded(x, y, w, h, r, r, r, r, segments);
}

void DrawList::addRectRounded(float x, float y, float w, float h,
                              float topLeftRadius, float topRightRadius,
                              float bottomRightRadius, float bottomLeftRadius,
                              int segments) {
  // Handle negative width/height
  if (w < 0.0f) {
    x += w;
    w *= -1.0f;
  }
  if (h < 0.0f) {
    y += h;
    h *= -1.0f;
  }

  // Clamp radii to maximum possible value
  float maxRadius = std::min(w / 2.0f, h / 2.0f);
  topLeftRadius = std::min(topLeftRadius, maxRadius);
  topRightRadius = std::min(topRightRadius, maxRadius);
  bottomRightRadius = std::min(bottomRightRadius, maxRadius);
  bottomLeftRadius = std::min(bottomLeftRadius, maxRadius);

  // If all radii are ~0, just draw a regular rectangle
  const float epsilon = 0.0001f;
  if (topLeftRadius < epsilon && topRightRadius < epsilon &&
      bottomRightRadius < epsilon && bottomLeftRadius < epsilon) {
    addRect(x, y, w, h);
    return;
  }

  float left = x;
  float right = x + w;
  float top = y;
  float bottom = y + h;

  // Build the path vertices
  std::vector<Vertex2D> pathVerts;

  // Start at top-left corner (after the radius)
  {
    float px = left + topLeftRadius, py = top;
    applyTransform(px, py);
    pathVerts.push_back({px, py, 0.f, 0.f, current_.r, current_.g, current_.b, current_.a});
  }

  // Top-right corner
  if (topRightRadius >= epsilon) {
    addArcVertices(pathVerts, right - topRightRadius, top + topRightRadius,
                   topRightRadius, topRightRadius, -90.f, 0.f, segments);
  } else {
    float px = right, py = top;
    applyTransform(px, py);
    pathVerts.push_back({px, py, 0.f, 0.f, current_.r, current_.g, current_.b, current_.a});
  }

  // Right edge to bottom-right corner
  {
    float px = right, py = bottom - bottomRightRadius;
    applyTransform(px, py);
    pathVerts.push_back({px, py, 0.f, 0.f, current_.r, current_.g, current_.b, current_.a});
  }

  // Bottom-right corner
  if (bottomRightRadius >= epsilon) {
    addArcVertices(pathVerts, right - bottomRightRadius, bottom - bottomRightRadius,
                   bottomRightRadius, bottomRightRadius, 0.f, 90.f, segments);
  }

  // Bottom edge to bottom-left corner
  {
    float px = left + bottomLeftRadius, py = bottom;
    applyTransform(px, py);
    pathVerts.push_back({px, py, 0.f, 0.f, current_.r, current_.g, current_.b, current_.a});
  }

  // Bottom-left corner
  if (bottomLeftRadius >= epsilon) {
    addArcVertices(pathVerts, left + bottomLeftRadius, bottom - bottomLeftRadius,
                   bottomLeftRadius, bottomLeftRadius, 90.f, 180.f, segments);
  }

  // Left edge to top-left corner
  {
    float px = left, py = top + topLeftRadius;
    applyTransform(px, py);
    pathVerts.push_back({px, py, 0.f, 0.f, current_.r, current_.g, current_.b, current_.a});
  }

  // Top-left corner
  if (topLeftRadius >= epsilon) {
    addArcVertices(pathVerts, left + topLeftRadius, top + topLeftRadius,
                   topLeftRadius, topLeftRadius, 180.f, 270.f, segments);
  }

  // Close the path
  pathVerts.push_back(pathVerts[0]);

  // Tessellate
  if (filled_) {
    // Fan triangulation from first vertex
    if (pathVerts.size() >= 3) {
      for (size_t i = 1; i < pathVerts.size() - 1; i++) {
        verts_.push_back(pathVerts[0]);
        verts_.push_back(pathVerts[i]);
        verts_.push_back(pathVerts[i + 1]);
      }
    }
  } else {
    // Draw outline
    for (size_t i = 0; i < pathVerts.size() - 1; i++) {
      addLineRaw(pathVerts[i].x, pathVerts[i].y,
                 pathVerts[i + 1].x, pathVerts[i + 1].y);
    }
  }
}

void DrawList::addArcVertices(std::vector<Vertex2D>& verts, float cx, float cy,
                               float rx, float ry, float startAngle, float endAngle,
                               int segments) {
  const float PI = 3.14159265358979f;
  float startRad = startAngle * PI / 180.f;
  float endRad = endAngle * PI / 180.f;
  float angleStep = (endRad - startRad) / segments;

  for (int i = 0; i <= segments; i++) {
    float angle = startRad + i * angleStep;
    float px = cx + rx * std::cos(angle);
    float py = cy + ry * std::sin(angle);
    applyTransform(px, py);
    verts.push_back({px, py, 0.f, 0.f, current_.r, current_.g, current_.b, current_.a});
  }
}

void DrawList::addEllipse(float cx, float cy, float rx, float ry, int segments) {
  if (segments < 3) segments = 3;

  const float angleStep = 2.0f * 3.14159265358979f / segments;

  if (filled_) {
    for (int i = 0; i < segments; ++i) {
      float a0 = i * angleStep;
      float a1 = (i + 1) * angleStep;

      float x0 = cx, y0 = cy;
      float x1 = cx + rx * std::cos(a0), y1 = cy + ry * std::sin(a0);
      float x2 = cx + rx * std::cos(a1), y2 = cy + ry * std::sin(a1);

      applyTransform(x0, y0);
      applyTransform(x1, y1);
      applyTransform(x2, y2);

      push(x0, y0, 0.5f, 0.5f);
      push(x1, y1, 0.5f + 0.5f * std::cos(a0), 0.5f + 0.5f * std::sin(a0));
      push(x2, y2, 0.5f + 0.5f * std::cos(a1), 0.5f + 0.5f * std::sin(a1));
    }
  } else {
    for (int i = 0; i < segments; ++i) {
      float a0 = i * angleStep;
      float a1 = (i + 1) * angleStep;

      float x0 = cx + rx * std::cos(a0);
      float y0 = cy + ry * std::sin(a0);
      float x1 = cx + rx * std::cos(a1);
      float y1 = cy + ry * std::sin(a1);

      addLine(x0, y0, x1, y1);
    }
  }
}

void DrawList::addTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
  if (filled_) {
    float px1 = x1, py1 = y1;
    float px2 = x2, py2 = y2;
    float px3 = x3, py3 = y3;

    applyTransform(px1, py1);
    applyTransform(px2, py2);
    applyTransform(px3, py3);

    push(px1, py1, 0.f, 0.f);
    push(px2, py2, 1.f, 0.f);
    push(px3, py3, 0.5f, 1.f);
  } else {
    addLine(x1, y1, x2, y2);
    addLine(x2, y2, x3, y3);
    addLine(x3, y3, x1, y1);
  }
}

void DrawList::addLine(float x1, float y1, float x2, float y2) {
  float px1 = x1, py1 = y1;
  float px2 = x2, py2 = y2;
  applyTransform(px1, py1);
  applyTransform(px2, py2);

  float dx = px2 - px1;
  float dy = py2 - py1;
  float len = std::sqrt(dx * dx + dy * dy);
  if (len < 0.0001f) return;

  float nx = -dy / len * lineWidth_ * 0.5f;
  float ny = dx / len * lineWidth_ * 0.5f;

  float ax = px1 + nx, ay = py1 + ny;
  float bx = px1 - nx, by = py1 - ny;
  float cx = px2 - nx, cy = py2 - ny;
  float dx2 = px2 + nx, dy2 = py2 + ny;

  pushLine(ax, ay, 0.f, 0.f);
  pushLine(bx, by, 0.f, 1.f);
  pushLine(cx, cy, 1.f, 1.f);

  pushLine(ax, ay, 0.f, 0.f);
  pushLine(cx, cy, 1.f, 1.f);
  pushLine(dx2, dy2, 1.f, 0.f);
}

void DrawList::addText(const std::string& text, float x, float y, float charW, float charH) {
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

    float x0 = cx, y0 = cy;
    float x1 = cx + charW, y1 = cy;
    float x2 = cx + charW, y2 = cy + charH;
    float x3 = cx, y3 = cy + charH;

    applyTransform(x0, y0);
    applyTransform(x1, y1);
    applyTransform(x2, y2);
    applyTransform(x3, y3);

    pushText(x0, y0, u0, v0);
    pushText(x1, y1, u1, v0);
    pushText(x2, y2, u1, v1);

    pushText(x0, y0, u0, v0);
    pushText(x2, y2, u1, v1);
    pushText(x3, y3, u0, v1);

    cx += charW;
  }
}

void DrawList::beginShape() {
  shapeVerts_.clear();
  inShape_ = true;
}

void DrawList::vertex(float x, float y) {
  if (!inShape_) return;
  float px = x, py = y;
  applyTransform(px, py);
  shapeVerts_.push_back({px, py, 0.f, 0.f, current_.r, current_.g, current_.b, current_.a});
}

void DrawList::vertex(float x, float y, float u, float v) {
  if (!inShape_) return;
  float px = x, py = y;
  applyTransform(px, py);
  shapeVerts_.push_back({px, py, u, v, current_.r, current_.g, current_.b, current_.a});
}

void DrawList::curveVertex(float x, float y) {
  if (!inShape_) return;
  float px = x, py = y;
  applyTransform(px, py);
  curveVerts_.push_back({px, py});
}

void DrawList::bezierVertex(float cx1, float cy1, float cx2, float cy2, float x, float y) {
  if (!inShape_ || shapeVerts_.empty()) return;

  float x0 = shapeVerts_.back().x;
  float y0 = shapeVerts_.back().y;

  float pcx1 = cx1, pcy1 = cy1;
  float pcx2 = cx2, pcy2 = cy2;
  float px = x, py = y;
  applyTransform(pcx1, pcy1);
  applyTransform(pcx2, pcy2);
  applyTransform(px, py);

  const int segments = 20;
  for (int i = 1; i <= segments; i++) {
    float t = (float)i / segments;
    float t2 = t * t;
    float t3 = t2 * t;
    float mt = 1 - t;
    float mt2 = mt * mt;
    float mt3 = mt2 * mt;

    float bx = mt3 * x0 + 3 * mt2 * t * pcx1 + 3 * mt * t2 * pcx2 + t3 * px;
    float by = mt3 * y0 + 3 * mt2 * t * pcy1 + 3 * mt * t2 * pcy2 + t3 * py;

    shapeVerts_.push_back({bx, by, 0.f, 0.f, current_.r, current_.g, current_.b, current_.a});
  }
}

void DrawList::nextContour(bool close) {
  if (!inShape_) return;

  // Process curve vertices first if any
  if (!curveVerts_.empty()) {
    processCurveVertices();
    curveVerts_.clear();
  }

  // Close the current contour if requested and we have enough vertices
  if (close && shapeVerts_.size() >= 3) {
    shapeVerts_.push_back(shapeVerts_[0]);
  }

  // Tessellate current contour
  if (filled_) {
    if (shapeVerts_.size() >= 3) {
      for (size_t i = 1; i < shapeVerts_.size() - 1; i++) {
        verts_.push_back(shapeVerts_[0]);
        verts_.push_back(shapeVerts_[i]);
        verts_.push_back(shapeVerts_[i + 1]);
      }
    }
  } else {
    for (size_t i = 0; i < shapeVerts_.size() - 1; i++) {
      addLineRaw(shapeVerts_[i].x, shapeVerts_[i].y,
                 shapeVerts_[i + 1].x, shapeVerts_[i + 1].y);
    }
  }

  // Clear for next contour
  shapeVerts_.clear();
}

void DrawList::endShape(bool close) {
  if (!inShape_) return;
  inShape_ = false;

  if (!curveVerts_.empty()) {
    processCurveVertices();
    curveVerts_.clear();
  }

  if (shapeVerts_.size() < 2) {
    shapeVerts_.clear();
    return;
  }

  if (close && shapeVerts_.size() >= 3) {
    shapeVerts_.push_back(shapeVerts_[0]);
  }

  if (filled_) {
    if (shapeVerts_.size() >= 3) {
      for (size_t i = 1; i < shapeVerts_.size() - 1; i++) {
        verts_.push_back(shapeVerts_[0]);
        verts_.push_back(shapeVerts_[i]);
        verts_.push_back(shapeVerts_[i + 1]);
      }
    }
  } else {
    for (size_t i = 0; i < shapeVerts_.size() - 1; i++) {
      addLineRaw(shapeVerts_[i].x, shapeVerts_[i].y,
                 shapeVerts_[i + 1].x, shapeVerts_[i + 1].y);
    }
  }

  shapeVerts_.clear();
}

void DrawList::addLineRaw(float x1, float y1, float x2, float y2) {
  float dx = x2 - x1;
  float dy = y2 - y1;
  float len = std::sqrt(dx * dx + dy * dy);
  if (len < 0.0001f) return;

  float nx = -dy / len * lineWidth_ * 0.5f;
  float ny = dx / len * lineWidth_ * 0.5f;

  float ax = x1 + nx, ay = y1 + ny;
  float bx = x1 - nx, by = y1 - ny;
  float cx = x2 - nx, cy = y2 - ny;
  float dx2 = x2 + nx, dy2 = y2 + ny;

  pushLine(ax, ay, 0.f, 0.f);
  pushLine(bx, by, 0.f, 1.f);
  pushLine(cx, cy, 1.f, 1.f);
  pushLine(ax, ay, 0.f, 0.f);
  pushLine(cx, cy, 1.f, 1.f);
  pushLine(dx2, dy2, 1.f, 0.f);
}

void DrawList::processCurveVertices() {
  if (curveVerts_.size() < 4) {
    for (auto& cv : curveVerts_) {
      shapeVerts_.push_back({cv.x, cv.y, 0.f, 0.f, current_.r, current_.g, current_.b, current_.a});
    }
    return;
  }

  const int segments = 10;
  for (size_t i = 0; i < curveVerts_.size() - 3; i++) {
    auto& p0 = curveVerts_[i];
    auto& p1 = curveVerts_[i + 1];
    auto& p2 = curveVerts_[i + 2];
    auto& p3 = curveVerts_[i + 3];

    for (int j = 0; j <= segments; j++) {
      float t = (float)j / segments;
      float t2 = t * t;
      float t3 = t2 * t;

      float x = 0.5f * ((2 * p1.x) +
                       (-p0.x + p2.x) * t +
                       (2 * p0.x - 5 * p1.x + 4 * p2.x - p3.x) * t2 +
                       (-p0.x + 3 * p1.x - 3 * p2.x + p3.x) * t3);
      float y = 0.5f * ((2 * p1.y) +
                       (-p0.y + p2.y) * t +
                       (2 * p0.y - 5 * p1.y + 4 * p2.y - p3.y) * t2 +
                       (-p0.y + 3 * p1.y - 3 * p2.y + p3.y) * t3);

      shapeVerts_.push_back({x, y, 0.f, 0.f, current_.r, current_.g, current_.b, current_.a});
    }
  }
}

} // namespace oflike
