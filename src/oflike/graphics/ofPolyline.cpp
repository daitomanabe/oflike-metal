#include "ofPolyline.h"
#include "ofGraphics.h"
#include "../math/ofMath.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace oflike {

// ============================================================================
// Constructors
// ============================================================================

ofPolyline::ofPolyline()
    : closed_(false) {
}

ofPolyline::ofPolyline(const std::vector<ofVec3f>& vertices)
    : vertices_(vertices), closed_(false) {
}

// ============================================================================
// Vertex Management
// ============================================================================

void ofPolyline::addVertex(float x, float y, float z) {
    vertices_.emplace_back(x, y, z);
}

void ofPolyline::addVertex(const ofVec3f& p) {
    vertices_.push_back(p);
}

void ofPolyline::addVertices(const std::vector<ofVec3f>& vertices) {
    vertices_.insert(vertices_.end(), vertices.begin(), vertices.end());
}

void ofPolyline::addVertices(const float* vertices, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        vertices_.emplace_back(vertices[i * 3], vertices[i * 3 + 1], vertices[i * 3 + 2]);
    }
}

// ============================================================================
// Line and Curve Operations
// ============================================================================

void ofPolyline::lineTo(float x, float y, float z) {
    addVertex(x, y, z);
}

void ofPolyline::lineTo(const ofVec3f& p) {
    addVertex(p);
}

void ofPolyline::curveTo(float x, float y, float z) {
    curveTo(ofVec3f(x, y, z));
}

void ofPolyline::curveTo(const ofVec3f& p) {
    size_t n = vertices_.size();
    if (n < 2) {
        // Not enough points for a curve, just add the point
        addVertex(p);
        return;
    }

    // Use the last two points and the new point to create a Catmull-Rom curve
    ofVec3f p0 = (n >= 3) ? vertices_[n - 3] : vertices_[n - 2];
    ofVec3f p1 = vertices_[n - 2];
    ofVec3f p2 = vertices_[n - 1];
    ofVec3f p3 = p;

    addCurveVertex(p0, p1, p2, p3);
}

void ofPolyline::bezierTo(float cx1, float cy1, float cz1,
                          float cx2, float cy2, float cz2,
                          float x, float y, float z) {
    bezierTo(ofVec3f(cx1, cy1, cz1), ofVec3f(cx2, cy2, cz2), ofVec3f(x, y, z));
}

void ofPolyline::bezierTo(const ofVec3f& cp1, const ofVec3f& cp2, const ofVec3f& p) {
    if (vertices_.empty()) {
        // No start point, just add the end point
        addVertex(p);
        return;
    }

    ofVec3f p0 = vertices_.back();
    addBezierVertex(p0, cp1, cp2, p);
}

// ============================================================================
// Arc Operations
// ============================================================================

void ofPolyline::arc(float x, float y, float radius, float angleBegin, float angleEnd) {
    addArcVertices(x, y, radius, angleBegin, angleEnd, false);
}

void ofPolyline::arcNegative(float x, float y, float radius, float angleBegin, float angleEnd) {
    addArcVertices(x, y, radius, angleBegin, angleEnd, true);
}

// ============================================================================
// Closing and State
// ============================================================================

void ofPolyline::close() {
    closed_ = true;
    // Optionally add closing vertex if not already there
    if (!vertices_.empty() && vertices_.front() != vertices_.back()) {
        vertices_.push_back(vertices_.front());
    }
}

bool ofPolyline::isClosed() const {
    return closed_;
}

void ofPolyline::clear() {
    vertices_.clear();
    closed_ = false;
}

// ============================================================================
// Drawing
// ============================================================================

void ofPolyline::draw() const {
    if (vertices_.size() < 2) {
        return;
    }

    // Draw line segments between consecutive vertices
    for (size_t i = 0; i < vertices_.size() - 1; ++i) {
        const ofVec3f& v1 = vertices_[i];
        const ofVec3f& v2 = vertices_[i + 1];
        ofDrawLine(v1.x, v1.y, v1.z, v2.x, v2.y, v2.z);
    }
}

// ============================================================================
// Access
// ============================================================================

const std::vector<ofVec3f>& ofPolyline::getVertices() const {
    return vertices_;
}

std::vector<ofVec3f>& ofPolyline::getVertices() {
    return vertices_;
}

size_t ofPolyline::size() const {
    return vertices_.size();
}

bool ofPolyline::empty() const {
    return vertices_.empty();
}

const ofVec3f& ofPolyline::operator[](size_t index) const {
    return vertices_[index];
}

ofVec3f& ofPolyline::operator[](size_t index) {
    return vertices_[index];
}

// ============================================================================
// Geometric Properties
// ============================================================================

float ofPolyline::getPerimeter() const {
    if (vertices_.size() < 2) {
        return 0.0f;
    }

    float perimeter = 0.0f;
    for (size_t i = 0; i < vertices_.size() - 1; ++i) {
        perimeter += vertices_[i].distance(vertices_[i + 1]);
    }

    if (closed_ && vertices_.size() > 2) {
        perimeter += vertices_.back().distance(vertices_.front());
    }

    return perimeter;
}

float ofPolyline::getArea() const {
    if (!closed_ || vertices_.size() < 3) {
        return 0.0f;
    }

    // Shoelace formula for 2D polygon area (uses XY coordinates only)
    float area = 0.0f;
    size_t n = vertices_.size();

    for (size_t i = 0; i < n; ++i) {
        size_t j = (i + 1) % n;
        area += vertices_[i].x * vertices_[j].y;
        area -= vertices_[j].x * vertices_[i].y;
    }

    return std::abs(area) * 0.5f;
}

ofVec3f ofPolyline::getCentroid2D() const {
    if (vertices_.empty()) {
        return ofVec3f(0, 0, 0);
    }

    ofVec3f centroid(0, 0, 0);
    for (const auto& v : vertices_) {
        centroid.x += v.x;
        centroid.y += v.y;
    }

    float n = static_cast<float>(vertices_.size());
    centroid.x /= n;
    centroid.y /= n;

    return centroid;
}

ofRectangle ofPolyline::getBoundingBox() const {
    if (vertices_.empty()) {
        return ofRectangle(0, 0, 0, 0);
    }

    float minX = vertices_[0].x;
    float maxX = vertices_[0].x;
    float minY = vertices_[0].y;
    float maxY = vertices_[0].y;

    for (const auto& v : vertices_) {
        minX = std::min(minX, v.x);
        maxX = std::max(maxX, v.x);
        minY = std::min(minY, v.y);
        maxY = std::max(maxY, v.y);
    }

    return ofRectangle(minX, minY, maxX - minX, maxY - minY);
}

// ============================================================================
// Geometric Queries
// ============================================================================

ofVec3f ofPolyline::getClosestPoint(const ofVec3f& target, size_t* nearestIndex) const {
    if (vertices_.empty()) {
        return ofVec3f(0, 0, 0);
    }

    if (vertices_.size() == 1) {
        if (nearestIndex) *nearestIndex = 0;
        return vertices_[0];
    }

    float minDist = std::numeric_limits<float>::max();
    ofVec3f closestPoint = vertices_[0];
    size_t closestSegment = 0;

    // Check each line segment
    for (size_t i = 0; i < vertices_.size() - 1; ++i) {
        const ofVec3f& p1 = vertices_[i];
        const ofVec3f& p2 = vertices_[i + 1];

        // Project target onto line segment
        ofVec3f line = p2 - p1;
        float lineLength = line.length();
        if (lineLength < 1e-6f) {
            // Degenerate segment
            float dist = target.distance(p1);
            if (dist < minDist) {
                minDist = dist;
                closestPoint = p1;
                closestSegment = i;
            }
            continue;
        }

        ofVec3f lineNorm = line / lineLength;
        float t = (target - p1).dot(lineNorm);
        t = ofClamp(t, 0.0f, lineLength);

        ofVec3f pointOnLine = p1 + lineNorm * t;
        float dist = target.distance(pointOnLine);

        if (dist < minDist) {
            minDist = dist;
            closestPoint = pointOnLine;
            closestSegment = i;
        }
    }

    if (nearestIndex) {
        *nearestIndex = closestSegment;
    }

    return closestPoint;
}

bool ofPolyline::inside(float x, float y) const {
    return inside(ofVec3f(x, y, 0));
}

bool ofPolyline::inside(const ofVec3f& p) const {
    if (!closed_ || vertices_.size() < 3) {
        return false;
    }

    // Ray casting algorithm for point-in-polygon test
    // Cast a ray from the point to the right and count intersections
    int intersections = 0;
    size_t n = vertices_.size();

    for (size_t i = 0; i < n; ++i) {
        size_t j = (i + 1) % n;
        const ofVec3f& v1 = vertices_[i];
        const ofVec3f& v2 = vertices_[j];

        // Check if ray crosses this edge
        if ((v1.y > p.y) != (v2.y > p.y)) {
            float xIntersect = v1.x + (p.y - v1.y) * (v2.x - v1.x) / (v2.y - v1.y);
            if (p.x < xIntersect) {
                intersections++;
            }
        }
    }

    return (intersections % 2) == 1;
}

// ============================================================================
// Simplification and Resampling
// ============================================================================

void ofPolyline::simplify(float tolerance) {
    if (vertices_.size() < 3) {
        return;
    }

    std::vector<bool> keepFlags(vertices_.size(), false);
    keepFlags[0] = true;
    keepFlags[vertices_.size() - 1] = true;

    simplifyDP(vertices_, tolerance, 0, vertices_.size() - 1, keepFlags);

    // Build new vertex list
    std::vector<ofVec3f> simplified;
    for (size_t i = 0; i < vertices_.size(); ++i) {
        if (keepFlags[i]) {
            simplified.push_back(vertices_[i]);
        }
    }

    vertices_ = simplified;
}

ofPolyline ofPolyline::getResampledBySpacing(float spacing) const {
    if (vertices_.size() < 2 || spacing <= 0.0f) {
        return *this;
    }

    ofPolyline resampled;
    resampled.vertices_.push_back(vertices_[0]);

    float traveled = 0.0f;

    for (size_t i = 0; i < vertices_.size() - 1; ++i) {
        ofVec3f p1 = vertices_[i];
        ofVec3f p2 = vertices_[i + 1];
        float segmentLength = p1.distance(p2);
        ofVec3f direction = (p2 - p1).normalize();

        while (traveled + segmentLength >= spacing) {
            float remaining = spacing - traveled;
            ofVec3f newPoint = p1 + direction * remaining;
            resampled.vertices_.push_back(newPoint);

            p1 = newPoint;
            segmentLength -= remaining;
            traveled = 0.0f;
        }

        traveled += segmentLength;
    }

    // Add last point if not already added
    if (resampled.vertices_.back() != vertices_.back()) {
        resampled.vertices_.push_back(vertices_.back());
    }

    resampled.closed_ = closed_;
    return resampled;
}

ofPolyline ofPolyline::getResampledByCount(size_t count) const {
    if (count < 2 || vertices_.size() < 2) {
        return *this;
    }

    float totalLength = getPerimeter();
    float spacing = totalLength / static_cast<float>(count - 1);

    return getResampledBySpacing(spacing);
}

ofPolyline ofPolyline::getSmoothed(int smoothingSize, float smoothingShape) const {
    if (vertices_.size() < 3 || smoothingSize < 1) {
        return *this;
    }

    ofPolyline smoothed;
    smoothed.vertices_.reserve(vertices_.size());

    for (size_t i = 0; i < vertices_.size(); ++i) {
        ofVec3f sum(0, 0, 0);
        float weightSum = 0.0f;

        for (int j = -smoothingSize; j <= smoothingSize; ++j) {
            int idx = static_cast<int>(i) + j;
            if (idx < 0 || idx >= static_cast<int>(vertices_.size())) {
                continue;
            }

            // Gaussian-like weight
            float weight = 1.0f - (std::abs(j) / static_cast<float>(smoothingSize)) * smoothingShape;
            weight = std::max(0.0f, weight);

            sum = sum + vertices_[idx] * weight;
            weightSum += weight;
        }

        if (weightSum > 0.0f) {
            smoothed.vertices_.push_back(sum / weightSum);
        } else {
            smoothed.vertices_.push_back(vertices_[i]);
        }
    }

    smoothed.closed_ = closed_;
    return smoothed;
}

// ============================================================================
// Private Helper Methods
// ============================================================================

void ofPolyline::addCurveVertex(const ofVec3f& p0, const ofVec3f& p1,
                                const ofVec3f& p2, const ofVec3f& p3) {
    // Catmull-Rom curve interpolation
    uint32_t resolution = ofGetCurveResolution();

    for (uint32_t i = 1; i <= resolution; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(resolution);
        float t2 = t * t;
        float t3 = t2 * t;

        // Catmull-Rom basis
        float b0 = -0.5f * t3 + t2 - 0.5f * t;
        float b1 = 1.5f * t3 - 2.5f * t2 + 1.0f;
        float b2 = -1.5f * t3 + 2.0f * t2 + 0.5f * t;
        float b3 = 0.5f * t3 - 0.5f * t2;

        ofVec3f point = p0 * b0 + p1 * b1 + p2 * b2 + p3 * b3;
        vertices_.push_back(point);
    }
}

void ofPolyline::addBezierVertex(const ofVec3f& p0, const ofVec3f& cp1,
                                 const ofVec3f& cp2, const ofVec3f& p1) {
    // Cubic Bezier curve interpolation
    uint32_t resolution = ofGetCurveResolution();

    for (uint32_t i = 1; i <= resolution; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(resolution);
        float t2 = t * t;
        float t3 = t2 * t;
        float mt = 1.0f - t;
        float mt2 = mt * mt;
        float mt3 = mt2 * mt;

        // Cubic Bezier basis
        ofVec3f point = p0 * mt3 + cp1 * (3.0f * mt2 * t) +
                       cp2 * (3.0f * mt * t2) + p1 * t3;
        vertices_.push_back(point);
    }
}

void ofPolyline::addArcVertices(float x, float y, float radius,
                               float angleBegin, float angleEnd, bool clockwise) {
    uint32_t resolution = ofGetCircleResolution();

    // Convert angles to radians
    float startRad = ofDegToRad(angleBegin);
    float endRad = ofDegToRad(angleEnd);

    // Calculate angle span
    float angleSpan = endRad - startRad;
    if (clockwise) {
        if (angleSpan > 0) angleSpan -= 2.0f * M_PI;
    } else {
        if (angleSpan < 0) angleSpan += 2.0f * M_PI;
    }

    // Determine number of segments based on arc length
    float arcLength = std::abs(angleSpan * radius);
    float circumference = 2.0f * M_PI * radius;
    uint32_t segments = static_cast<uint32_t>(resolution * arcLength / circumference);
    segments = std::max(3u, segments);

    // Generate arc vertices
    for (uint32_t i = 0; i <= segments; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(segments);
        float angle = startRad + angleSpan * t;
        float px = x + radius * std::cos(angle);
        float py = y + radius * std::sin(angle);
        vertices_.emplace_back(px, py, 0.0f);
    }
}

void ofPolyline::simplifyDP(const std::vector<ofVec3f>& points,
                           float tolerance,
                           size_t start, size_t end,
                           std::vector<bool>& keepFlags) const {
    if (end - start < 2) {
        return;
    }

    // Find point with maximum distance from line segment
    float maxDist = 0.0f;
    size_t maxIndex = start;

    ofVec3f lineStart = points[start];
    ofVec3f lineEnd = points[end];
    ofVec3f line = lineEnd - lineStart;
    float lineLength = line.length();

    if (lineLength < 1e-6f) {
        return;
    }

    ofVec3f lineNorm = line / lineLength;

    for (size_t i = start + 1; i < end; ++i) {
        ofVec3f point = points[i];
        ofVec3f toPoint = point - lineStart;
        float projection = toPoint.dot(lineNorm);
        projection = ofClamp(projection, 0.0f, lineLength);

        ofVec3f closest = lineStart + lineNorm * projection;
        float dist = point.distance(closest);

        if (dist > maxDist) {
            maxDist = dist;
            maxIndex = i;
        }
    }

    // If max distance is greater than tolerance, keep this point and recurse
    if (maxDist > tolerance) {
        keepFlags[maxIndex] = true;
        simplifyDP(points, tolerance, start, maxIndex, keepFlags);
        simplifyDP(points, tolerance, maxIndex, end, keepFlags);
    }
}

} // namespace oflike
