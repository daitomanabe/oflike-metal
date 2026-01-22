#pragma once

#include <vector>
#include <cmath>
#include "../math/ofMath.h"
#include "ofGraphics.h"

class ofPolyline {
public:
  ofPolyline() = default;

  void addVertex(float x, float y) {
    points_.push_back(ofVec2f(x, y));
  }

  void addVertex(const ofVec2f& p) {
    points_.push_back(p);
  }

  void addVertices(const std::vector<ofVec2f>& verts) {
    for (const auto& v : verts) {
      points_.push_back(v);
    }
  }

  void insertVertex(const ofVec2f& p, int index) {
    if (index >= 0 && index <= (int)points_.size()) {
      points_.insert(points_.begin() + index, p);
    }
  }

  void curveTo(float x, float y, int curveResolution = 20) {
    curveTo(ofVec2f(x, y), curveResolution);
  }

  void curveTo(const ofVec2f& to, int curveResolution = 20) {
    if (points_.size() < 1) {
      addVertex(to);
      return;
    }

    ofVec2f from = points_.back();
    for (int i = 1; i <= curveResolution; i++) {
      float t = (float)i / curveResolution;
      float x = ofLerp(from.x, to.x, t);
      float y = ofLerp(from.y, to.y, t);
      addVertex(x, y);
    }
  }

  void bezierTo(float cx1, float cy1, float cx2, float cy2, float x, float y, int curveResolution = 20) {
    bezierTo(ofVec2f(cx1, cy1), ofVec2f(cx2, cy2), ofVec2f(x, y), curveResolution);
  }

  void bezierTo(const ofVec2f& cp1, const ofVec2f& cp2, const ofVec2f& to, int curveResolution = 20) {
    if (points_.empty()) return;

    ofVec2f from = points_.back();

    for (int i = 1; i <= curveResolution; i++) {
      float t = (float)i / curveResolution;
      float t2 = t * t;
      float t3 = t2 * t;
      float mt = 1 - t;
      float mt2 = mt * mt;
      float mt3 = mt2 * mt;

      float x = mt3 * from.x + 3 * mt2 * t * cp1.x + 3 * mt * t2 * cp2.x + t3 * to.x;
      float y = mt3 * from.y + 3 * mt2 * t * cp1.y + 3 * mt * t2 * cp2.y + t3 * to.y;
      addVertex(x, y);
    }
  }

  void quadBezierTo(float cx, float cy, float x, float y, int curveResolution = 20) {
    quadBezierTo(ofVec2f(cx, cy), ofVec2f(x, y), curveResolution);
  }

  void quadBezierTo(const ofVec2f& cp, const ofVec2f& to, int curveResolution = 20) {
    if (points_.empty()) return;

    ofVec2f from = points_.back();

    for (int i = 1; i <= curveResolution; i++) {
      float t = (float)i / curveResolution;
      float mt = 1 - t;

      float x = mt * mt * from.x + 2 * mt * t * cp.x + t * t * to.x;
      float y = mt * mt * from.y + 2 * mt * t * cp.y + t * t * to.y;
      addVertex(x, y);
    }
  }

  void arc(float x, float y, float radiusX, float radiusY, float angleBegin, float angleEnd, int circleResolution = 20) {
    float angleStep = (angleEnd - angleBegin) / circleResolution;
    for (int i = 0; i <= circleResolution; i++) {
      float angle = ofDegToRad(angleBegin + i * angleStep);
      addVertex(x + radiusX * std::cos(angle), y + radiusY * std::sin(angle));
    }
  }

  void close() {
    closed_ = true;
    if (points_.size() > 1 && points_.front().distance(points_.back()) > 0.0001f) {
      points_.push_back(points_.front());
    }
  }

  void setClosed(bool closed) {
    closed_ = closed;
    if (closed && points_.size() > 1 && points_.front().distance(points_.back()) > 0.0001f) {
      points_.push_back(points_.front());
    }
  }

  bool isClosed() const { return closed_; }

  void clear() {
    points_.clear();
    closed_ = false;
  }

  const std::vector<ofVec2f>& getVertices() const { return points_; }
  std::vector<ofVec2f>& getVertices() { return points_; }

  ofVec2f& operator[](int index) { return points_[index]; }
  const ofVec2f& operator[](int index) const { return points_[index]; }

  size_t size() const { return points_.size(); }
  bool empty() const { return points_.empty(); }

  ofVec2f getPointAtPercent(float percent) const {
    if (points_.empty()) return ofVec2f(0, 0);
    if (points_.size() == 1) return points_[0];

    float totalLen = getPerimeter();
    float targetLen = percent * totalLen;
    float currentLen = 0;

    for (size_t i = 0; i < points_.size() - 1; i++) {
      float segLen = points_[i].distance(points_[i + 1]);
      if (currentLen + segLen >= targetLen) {
        float t = (targetLen - currentLen) / segLen;
        return ofVec2f(
          ofLerp(points_[i].x, points_[i + 1].x, t),
          ofLerp(points_[i].y, points_[i + 1].y, t)
        );
      }
      currentLen += segLen;
    }

    return points_.back();
  }

  ofVec2f getTangentAtPercent(float percent) const {
    if (points_.size() < 2) return ofVec2f(1, 0);

    float totalLen = getPerimeter();
    float targetLen = percent * totalLen;
    float currentLen = 0;

    for (size_t i = 0; i < points_.size() - 1; i++) {
      float segLen = points_[i].distance(points_[i + 1]);
      if (currentLen + segLen >= targetLen) {
        ofVec2f tangent = points_[i + 1] - points_[i];
        tangent.normalize();
        return tangent;
      }
      currentLen += segLen;
    }

    ofVec2f tangent = points_.back() - points_[points_.size() - 2];
    tangent.normalize();
    return tangent;
  }

  ofVec2f getNormalAtPercent(float percent) const {
    ofVec2f tangent = getTangentAtPercent(percent);
    return ofVec2f(-tangent.y, tangent.x);
  }

  float getPerimeter() const {
    float len = 0;
    for (size_t i = 0; i < points_.size() - 1; i++) {
      len += points_[i].distance(points_[i + 1]);
    }
    return len;
  }

  void getBoundingBox(float& minX, float& minY, float& maxX, float& maxY) const {
    if (points_.empty()) {
      minX = minY = maxX = maxY = 0;
      return;
    }

    minX = maxX = points_[0].x;
    minY = maxY = points_[0].y;

    for (const auto& p : points_) {
      if (p.x < minX) minX = p.x;
      if (p.x > maxX) maxX = p.x;
      if (p.y < minY) minY = p.y;
      if (p.y > maxY) maxY = p.y;
    }
  }

  ofVec2f getCentroid() const {
    if (points_.empty()) return ofVec2f(0, 0);

    ofVec2f centroid(0, 0);
    for (const auto& p : points_) {
      centroid += p;
    }
    return centroid / (float)points_.size();
  }

  ofPolyline getResampledBySpacing(float spacing) const {
    ofPolyline result;
    if (points_.size() < 2) return result;

    result.addVertex(points_[0]);
    float currentDist = 0;
    float targetDist = spacing;

    for (size_t i = 0; i < points_.size() - 1; i++) {
      ofVec2f dir = points_[i + 1] - points_[i];
      float segLen = dir.length();
      dir.normalize();

      while (currentDist + segLen >= targetDist) {
        float t = (targetDist - currentDist) / segLen;
        ofVec2f newPoint = points_[i] + dir * (targetDist - currentDist);
        result.addVertex(newPoint);
        currentDist = 0;
        targetDist = spacing;
        segLen -= (targetDist - currentDist);
      }
      currentDist += segLen;
    }

    return result;
  }

  ofPolyline getResampledByCount(int count) const {
    if (count < 2 || points_.size() < 2) return *this;
    return getResampledBySpacing(getPerimeter() / (count - 1));
  }

  ofPolyline getSmoothed(int smoothingSize, float smoothingShape = 0) const {
    ofPolyline result;
    if (points_.size() < 3) return *this;

    int n = (int)points_.size();
    for (int i = 0; i < n; i++) {
      ofVec2f sum(0, 0);
      float weightSum = 0;

      for (int j = -smoothingSize; j <= smoothingSize; j++) {
        int idx = i + j;
        if (closed_) {
          idx = ((idx % n) + n) % n;
        } else {
          idx = (int)ofClamp((float)idx, 0.f, (float)(n - 1));
        }

        float weight = 1.0f - std::abs((float)j / (smoothingSize + 1));
        weight = std::pow(weight, 1.0f + smoothingShape);
        sum += points_[idx] * weight;
        weightSum += weight;
      }

      result.addVertex(sum / weightSum);
    }

    if (closed_) result.close();
    return result;
  }

  void draw() const {
    if (points_.size() < 2) return;

    ofBeginShape();
    for (const auto& p : points_) {
      ofVertex(p.x, p.y);
    }
    ofEndShape(closed_);
  }

  static ofPolyline fromRectangle(float x, float y, float w, float h) {
    ofPolyline p;
    p.addVertex(x, y);
    p.addVertex(x + w, y);
    p.addVertex(x + w, y + h);
    p.addVertex(x, y + h);
    p.close();
    return p;
  }

  static ofPolyline fromCircle(float x, float y, float radius, int resolution = 32) {
    ofPolyline p;
    for (int i = 0; i < resolution; i++) {
      float angle = i * TWO_PI / resolution;
      p.addVertex(x + radius * std::cos(angle), y + radius * std::sin(angle));
    }
    p.close();
    return p;
  }

private:
  std::vector<ofVec2f> points_;
  bool closed_{false};
};
