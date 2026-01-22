#pragma once

#include <vector>
#include <cmath>
#include "ofColor.h"
#include "ofPolyline.h"
#include "../types/ofRectangle.h"
#include "../../core/Context.h"

class ofPath {
public:
  ofPath() = default;

  // Command types
  struct Command {
    enum Type {
      moveTo,
      lineTo,
      curveTo,
      bezierTo,
      quadBezierTo,
      arc,
      arcNegative,
      close
    };

    Type type;
    float x{0}, y{0}, z{0};     // Destination point
    float cx1{0}, cy1{0};        // Control point 1
    float cx2{0}, cy2{0};        // Control point 2
    float radiusX{0}, radiusY{0};
    float angleBegin{0}, angleEnd{0};
  };

  // Path construction - moveTo
  void moveTo(float x, float y, float z = 0) {
    Command cmd;
    cmd.type = Command::moveTo;
    cmd.x = x; cmd.y = y; cmd.z = z;
    commands_.push_back(cmd);
    bNeedsPolylinesGeneration_ = true;
  }

  void moveTo(const ofVec2f& p) { moveTo(p.x, p.y, 0); }
  void moveTo(const ofVec3f& p) { moveTo(p.x, p.y, p.z); }

  // Path construction - lineTo
  void lineTo(float x, float y, float z = 0) {
    Command cmd;
    cmd.type = Command::lineTo;
    cmd.x = x; cmd.y = y; cmd.z = z;
    commands_.push_back(cmd);
    bNeedsPolylinesGeneration_ = true;
  }

  void lineTo(const ofVec2f& p) { lineTo(p.x, p.y, 0); }
  void lineTo(const ofVec3f& p) { lineTo(p.x, p.y, p.z); }

  // Path construction - curveTo (Catmull-Rom)
  void curveTo(float x, float y, float z = 0) {
    Command cmd;
    cmd.type = Command::curveTo;
    cmd.x = x; cmd.y = y; cmd.z = z;
    commands_.push_back(cmd);
    bNeedsPolylinesGeneration_ = true;
  }

  void curveTo(const ofVec2f& p) { curveTo(p.x, p.y, 0); }
  void curveTo(const ofVec3f& p) { curveTo(p.x, p.y, p.z); }

  // Path construction - bezierTo (cubic Bezier)
  void bezierTo(float cx1, float cy1, float cx2, float cy2, float x, float y, float z = 0) {
    Command cmd;
    cmd.type = Command::bezierTo;
    cmd.cx1 = cx1; cmd.cy1 = cy1;
    cmd.cx2 = cx2; cmd.cy2 = cy2;
    cmd.x = x; cmd.y = y; cmd.z = z;
    commands_.push_back(cmd);
    bNeedsPolylinesGeneration_ = true;
  }

  void bezierTo(const ofVec2f& cp1, const ofVec2f& cp2, const ofVec2f& to) {
    bezierTo(cp1.x, cp1.y, cp2.x, cp2.y, to.x, to.y, 0);
  }

  // Path construction - quadBezierTo (quadratic Bezier)
  void quadBezierTo(float cx1, float cy1, float cx2, float cy2, float x, float y, float z = 0) {
    Command cmd;
    cmd.type = Command::quadBezierTo;
    cmd.cx1 = cx1; cmd.cy1 = cy1;
    cmd.cx2 = cx2; cmd.cy2 = cy2;
    cmd.x = x; cmd.y = y; cmd.z = z;
    commands_.push_back(cmd);
    bNeedsPolylinesGeneration_ = true;
  }

  void quadBezierTo(const ofVec2f& cp1, const ofVec2f& cp2, const ofVec2f& to) {
    quadBezierTo(cp1.x, cp1.y, cp2.x, cp2.y, to.x, to.y, 0);
  }

  // Path construction - arc
  void arc(float x, float y, float radiusX, float radiusY, float angleBegin, float angleEnd, bool clockwise = true) {
    Command cmd;
    cmd.type = clockwise ? Command::arc : Command::arcNegative;
    cmd.x = x; cmd.y = y;
    cmd.radiusX = radiusX; cmd.radiusY = radiusY;
    cmd.angleBegin = angleBegin; cmd.angleEnd = angleEnd;
    commands_.push_back(cmd);
    bNeedsPolylinesGeneration_ = true;
  }

  void arc(const ofVec2f& center, float radiusX, float radiusY, float angleBegin, float angleEnd, bool clockwise = true) {
    arc(center.x, center.y, radiusX, radiusY, angleBegin, angleEnd, clockwise);
  }

  void arcNegative(float x, float y, float radiusX, float radiusY, float angleBegin, float angleEnd) {
    arc(x, y, radiusX, radiusY, angleBegin, angleEnd, false);
  }

  // Path construction - close
  void close() {
    Command cmd;
    cmd.type = Command::close;
    commands_.push_back(cmd);
    bNeedsPolylinesGeneration_ = true;
  }

  // Clear path
  void clear() {
    commands_.clear();
    polylines_.clear();
    bNeedsPolylinesGeneration_ = true;
  }

  // Convenience shapes
  void rectangle(float x, float y, float w, float h) {
    moveTo(x, y);
    lineTo(x + w, y);
    lineTo(x + w, y + h);
    lineTo(x, y + h);
    close();
  }

  void rectangle(const ofRectangle& r) {
    rectangle(r.x, r.y, r.width, r.height);
  }

  void rectRounded(float x, float y, float w, float h, float r) {
    rectRounded(x, y, w, h, r, r, r, r);
  }

  void rectRounded(const ofRectangle& rect, float r) {
    rectRounded(rect.x, rect.y, rect.width, rect.height, r);
  }

  void rectRounded(float x, float y, float w, float h,
                   float topLeftRadius, float topRightRadius,
                   float bottomRightRadius, float bottomLeftRadius) {
    // Clamp radii to half the minimum dimension
    float maxR = std::min(w, h) * 0.5f;
    topLeftRadius = std::min(topLeftRadius, maxR);
    topRightRadius = std::min(topRightRadius, maxR);
    bottomRightRadius = std::min(bottomRightRadius, maxR);
    bottomLeftRadius = std::min(bottomLeftRadius, maxR);

    // Start from top-left corner after the rounded part
    moveTo(x + topLeftRadius, y);

    // Top edge and top-right corner
    lineTo(x + w - topRightRadius, y);
    if (topRightRadius > 0) {
      arc(x + w - topRightRadius, y + topRightRadius, topRightRadius, topRightRadius, -90, 0);
    }

    // Right edge and bottom-right corner
    lineTo(x + w, y + h - bottomRightRadius);
    if (bottomRightRadius > 0) {
      arc(x + w - bottomRightRadius, y + h - bottomRightRadius, bottomRightRadius, bottomRightRadius, 0, 90);
    }

    // Bottom edge and bottom-left corner
    lineTo(x + bottomLeftRadius, y + h);
    if (bottomLeftRadius > 0) {
      arc(x + bottomLeftRadius, y + h - bottomLeftRadius, bottomLeftRadius, bottomLeftRadius, 90, 180);
    }

    // Left edge and top-left corner
    lineTo(x, y + topLeftRadius);
    if (topLeftRadius > 0) {
      arc(x + topLeftRadius, y + topLeftRadius, topLeftRadius, topLeftRadius, 180, 270);
    }

    close();
  }

  void circle(float x, float y, float radius) {
    arc(x, y, radius, radius, 0, 360);
    close();
  }

  void ellipse(float x, float y, float width, float height) {
    arc(x, y, width * 0.5f, height * 0.5f, 0, 360);
    close();
  }

  void triangle(float x1, float y1, float x2, float y2, float x3, float y3) {
    moveTo(x1, y1);
    lineTo(x2, y2);
    lineTo(x3, y3);
    close();
  }

  // Style settings
  void setFilled(bool hasFill) { bFill_ = hasFill; }
  bool isFilled() const { return bFill_; }

  void setStrokeWidth(float width) { strokeWidth_ = width; }
  float getStrokeWidth() const { return strokeWidth_; }
  bool hasOutline() const { return strokeWidth_ > 0; }

  // Color settings
  void setColor(const ofFloatColor& color) {
    fillColor_ = color;
    strokeColor_ = color;
  }

  void setColor(const ofColor& color) {
    setColor(ofFloatColor(color));
  }

  void setHexColor(int hex) {
    setColor(ofColor::fromHex(hex));
  }

  void setFillColor(const ofFloatColor& color) { fillColor_ = color; }
  void setFillColor(const ofColor& color) { fillColor_ = ofFloatColor(color); }
  void setFillHexColor(int hex) { setFillColor(ofColor::fromHex(hex)); }
  ofFloatColor getFillColor() const { return fillColor_; }

  void setStrokeColor(const ofFloatColor& color) { strokeColor_ = color; }
  void setStrokeColor(const ofColor& color) { strokeColor_ = ofFloatColor(color); }
  void setStrokeHexColor(int hex) { setStrokeColor(ofColor::fromHex(hex)); }
  ofFloatColor getStrokeColor() const { return strokeColor_; }

  void setUseShapeColor(bool use) { bUseShapeColor_ = use; }
  bool getUseShapeColor() const { return bUseShapeColor_; }

  // Resolution settings
  void setCurveResolution(int res) { curveResolution_ = res > 1 ? res : 1; }
  int getCurveResolution() const { return curveResolution_; }

  void setCircleResolution(int res) { circleResolution_ = res > 3 ? res : 3; }
  int getCircleResolution() const { return circleResolution_; }

  // Get commands
  std::vector<Command>& getCommands() { return commands_; }
  const std::vector<Command>& getCommands() const { return commands_; }

  // Get polylines (after generation)
  const std::vector<ofPolyline>& getOutline() {
    if (bNeedsPolylinesGeneration_) {
      generatePolylinesFromCommands();
    }
    return polylines_;
  }

  // Draw the path
  void draw() const {
    draw(0, 0);
  }

  void draw(float x, float y) const {
    // Generate polylines if needed
    if (bNeedsPolylinesGeneration_) {
      const_cast<ofPath*>(this)->generatePolylinesFromCommands();
    }

    auto& dl = oflike::engine().drawList();

    // Save current state
    auto savedColor = oflike::engine().getStyle().color;
    bool savedFilled = dl.isFilled();
    float savedLineWidth = dl.getLineWidth();

    if (x != 0 || y != 0) {
      dl.pushMatrix();
      dl.translate(x, y);
    }

    // Draw fill
    if (bFill_) {
      if (bUseShapeColor_) {
        dl.setColor(oflike::Color4f{fillColor_.r, fillColor_.g, fillColor_.b, fillColor_.a});
      }
      dl.setFilled(true);

      for (const auto& polyline : polylines_) {
        const auto& vertices = polyline.getVertices();
        if (vertices.size() < 3) continue;

        // Draw as triangle fan from center
        ofVec2f center(0, 0);
        for (const auto& v : vertices) {
          center.x += v.x;
          center.y += v.y;
        }
        center.x /= vertices.size();
        center.y /= vertices.size();

        for (size_t i = 0; i < vertices.size(); i++) {
          size_t next = (i + 1) % vertices.size();
          dl.addTriangle(center.x, center.y,
                         vertices[i].x, vertices[i].y,
                         vertices[next].x, vertices[next].y);
        }
      }
    }

    // Draw stroke
    if (strokeWidth_ > 0) {
      if (bUseShapeColor_) {
        dl.setColor(oflike::Color4f{strokeColor_.r, strokeColor_.g, strokeColor_.b, strokeColor_.a});
      }
      dl.setFilled(false);
      dl.setLineWidth(strokeWidth_);

      for (const auto& polyline : polylines_) {
        const auto& vertices = polyline.getVertices();
        if (vertices.size() < 2) continue;

        for (size_t i = 0; i < vertices.size(); i++) {
          size_t next = (i + 1) % vertices.size();
          if (next == 0 && !polyline.isClosed()) continue;
          dl.addLine(vertices[i].x, vertices[i].y,
                     vertices[next].x, vertices[next].y);
        }
      }
    }

    if (x != 0 || y != 0) {
      dl.popMatrix();
    }

    // Restore state
    dl.setColor(savedColor);
    dl.setFilled(savedFilled);
    dl.setLineWidth(savedLineWidth);
  }

private:
  void generatePolylinesFromCommands() {
    polylines_.clear();
    bNeedsPolylinesGeneration_ = false;

    if (commands_.empty()) return;

    ofPolyline currentPolyline;
    float curX = 0, curY = 0;
    std::vector<ofVec2f> curvePoints;

    auto flushCurvePoints = [&]() {
      if (curvePoints.size() >= 4) {
        // Process Catmull-Rom curve
        for (size_t i = 0; i < curvePoints.size() - 3; i++) {
          for (int j = 0; j <= curveResolution_; j++) {
            float t = static_cast<float>(j) / curveResolution_;
            ofVec2f p = catmullRom(curvePoints[i], curvePoints[i + 1],
                                   curvePoints[i + 2], curvePoints[i + 3], t);
            currentPolyline.addVertex(p);
          }
        }
        curX = curvePoints.back().x;
        curY = curvePoints.back().y;
      }
      curvePoints.clear();
    };

    for (const auto& cmd : commands_) {
      switch (cmd.type) {
        case Command::moveTo:
          flushCurvePoints();
          if (currentPolyline.size() > 0) {
            polylines_.push_back(currentPolyline);
            currentPolyline.clear();
          }
          curX = cmd.x;
          curY = cmd.y;
          currentPolyline.addVertex(curX, curY);
          break;

        case Command::lineTo:
          flushCurvePoints();
          curX = cmd.x;
          curY = cmd.y;
          currentPolyline.addVertex(curX, curY);
          break;

        case Command::curveTo:
          curvePoints.push_back(ofVec2f(cmd.x, cmd.y));
          break;

        case Command::bezierTo:
          flushCurvePoints();
          // Cubic bezier
          for (int i = 1; i <= curveResolution_; i++) {
            float t = static_cast<float>(i) / curveResolution_;
            ofVec2f p = cubicBezier(ofVec2f(curX, curY),
                                    ofVec2f(cmd.cx1, cmd.cy1),
                                    ofVec2f(cmd.cx2, cmd.cy2),
                                    ofVec2f(cmd.x, cmd.y), t);
            currentPolyline.addVertex(p);
          }
          curX = cmd.x;
          curY = cmd.y;
          break;

        case Command::quadBezierTo:
          flushCurvePoints();
          // Quadratic bezier (use control point as both)
          for (int i = 1; i <= curveResolution_; i++) {
            float t = static_cast<float>(i) / curveResolution_;
            ofVec2f p = quadraticBezier(ofVec2f(curX, curY),
                                        ofVec2f(cmd.cx1, cmd.cy1),
                                        ofVec2f(cmd.x, cmd.y), t);
            currentPolyline.addVertex(p);
          }
          curX = cmd.x;
          curY = cmd.y;
          break;

        case Command::arc:
        case Command::arcNegative: {
          flushCurvePoints();
          bool clockwise = (cmd.type == Command::arc);
          float startAngle = cmd.angleBegin * M_PI / 180.0f;
          float endAngle = cmd.angleEnd * M_PI / 180.0f;

          if (!clockwise) {
            std::swap(startAngle, endAngle);
          }

          float angleRange = endAngle - startAngle;
          int segments = std::max(1, static_cast<int>(std::abs(angleRange) / (2 * M_PI) * circleResolution_));

          for (int i = 0; i <= segments; i++) {
            float t = static_cast<float>(i) / segments;
            float angle = startAngle + t * angleRange;
            float px = cmd.x + cmd.radiusX * std::cos(angle);
            float py = cmd.y + cmd.radiusY * std::sin(angle);
            currentPolyline.addVertex(px, py);
          }

          curX = cmd.x + cmd.radiusX * std::cos(endAngle);
          curY = cmd.y + cmd.radiusY * std::sin(endAngle);
          break;
        }

        case Command::close:
          flushCurvePoints();
          currentPolyline.close();
          break;
      }
    }

    flushCurvePoints();
    if (currentPolyline.size() > 0) {
      polylines_.push_back(currentPolyline);
    }
  }

  // Catmull-Rom spline interpolation
  ofVec2f catmullRom(const ofVec2f& p0, const ofVec2f& p1,
                     const ofVec2f& p2, const ofVec2f& p3, float t) const {
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

    return ofVec2f(x, y);
  }

  // Cubic bezier interpolation
  ofVec2f cubicBezier(const ofVec2f& p0, const ofVec2f& p1,
                      const ofVec2f& p2, const ofVec2f& p3, float t) const {
    float u = 1 - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    float x = uuu * p0.x + 3 * uu * t * p1.x + 3 * u * tt * p2.x + ttt * p3.x;
    float y = uuu * p0.y + 3 * uu * t * p1.y + 3 * u * tt * p2.y + ttt * p3.y;

    return ofVec2f(x, y);
  }

  // Quadratic bezier interpolation
  ofVec2f quadraticBezier(const ofVec2f& p0, const ofVec2f& p1,
                          const ofVec2f& p2, float t) const {
    float u = 1 - t;
    float x = u * u * p0.x + 2 * u * t * p1.x + t * t * p2.x;
    float y = u * u * p0.y + 2 * u * t * p1.y + t * t * p2.y;
    return ofVec2f(x, y);
  }

private:
  std::vector<Command> commands_;
  std::vector<ofPolyline> polylines_;

  ofFloatColor fillColor_{1, 1, 1, 1};
  ofFloatColor strokeColor_{0, 0, 0, 1};
  float strokeWidth_{0};
  bool bFill_{true};
  bool bUseShapeColor_{true};

  int curveResolution_{20};
  int circleResolution_{20};

  bool bNeedsPolylinesGeneration_{true};
};
