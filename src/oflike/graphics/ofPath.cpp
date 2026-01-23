#include "ofPath.h"
#include "ofGraphics.h"
#include "../math/ofMatrix4x4.h"
#include "../math/ofMath.h"
#include "../../third_party/tess2/Include/tesselator.h"
#include <cmath>
#include <algorithm>

namespace oflike {

// ============================================================================
// Constructors & Destructors
// ============================================================================

ofPath::ofPath()
    : currentPosition_(0, 0, 0)
    , hasCurrentPosition_(false)
    , filled_(true)
    , strokeWidth_(1.0f)
    , tessellationDirty_(true)
{
    // Default fill color: white
    fillColor_[0] = 255;
    fillColor_[1] = 255;
    fillColor_[2] = 255;
    fillColor_[3] = 255;

    // Default stroke color: white
    strokeColor_[0] = 255;
    strokeColor_[1] = 255;
    strokeColor_[2] = 255;
    strokeColor_[3] = 255;
}

ofPath::~ofPath() = default;

ofPath::ofPath(const ofPath& other)
    : polylines_(other.polylines_)
    , currentPosition_(other.currentPosition_)
    , hasCurrentPosition_(other.hasCurrentPosition_)
    , filled_(other.filled_)
    , strokeWidth_(other.strokeWidth_)
    , tessellationDirty_(true) // Always dirty on copy
{
    std::copy(std::begin(other.fillColor_), std::end(other.fillColor_), fillColor_);
    std::copy(std::begin(other.strokeColor_), std::end(other.strokeColor_), strokeColor_);
}

ofPath& ofPath::operator=(const ofPath& other) {
    if (this != &other) {
        polylines_ = other.polylines_;
        currentPosition_ = other.currentPosition_;
        hasCurrentPosition_ = other.hasCurrentPosition_;
        filled_ = other.filled_;
        strokeWidth_ = other.strokeWidth_;
        std::copy(std::begin(other.fillColor_), std::end(other.fillColor_), fillColor_);
        std::copy(std::begin(other.strokeColor_), std::end(other.strokeColor_), strokeColor_);
        tessellationDirty_ = true;
        tessellationCache_.clear();
    }
    return *this;
}

ofPath::ofPath(ofPath&& other) noexcept
    : polylines_(std::move(other.polylines_))
    , currentPosition_(other.currentPosition_)
    , hasCurrentPosition_(other.hasCurrentPosition_)
    , filled_(other.filled_)
    , strokeWidth_(other.strokeWidth_)
    , tessellationCache_(std::move(other.tessellationCache_))
    , tessellationDirty_(other.tessellationDirty_)
{
    std::copy(std::begin(other.fillColor_), std::end(other.fillColor_), fillColor_);
    std::copy(std::begin(other.strokeColor_), std::end(other.strokeColor_), strokeColor_);
}

ofPath& ofPath::operator=(ofPath&& other) noexcept {
    if (this != &other) {
        polylines_ = std::move(other.polylines_);
        currentPosition_ = other.currentPosition_;
        hasCurrentPosition_ = other.hasCurrentPosition_;
        filled_ = other.filled_;
        strokeWidth_ = other.strokeWidth_;
        std::copy(std::begin(other.fillColor_), std::end(other.fillColor_), fillColor_);
        std::copy(std::begin(other.strokeColor_), std::end(other.strokeColor_), strokeColor_);
        tessellationCache_ = std::move(other.tessellationCache_);
        tessellationDirty_ = other.tessellationDirty_;
    }
    return *this;
}

// ============================================================================
// Path Construction
// ============================================================================

void ofPath::moveTo(float x, float y, float z) {
    moveTo(ofVec3f(x, y, z));
}

void ofPath::moveTo(const ofVec3f& p) {
    currentPosition_ = p;
    hasCurrentPosition_ = true;

    // Create a new polyline for this subpath
    polylines_.emplace_back();
    polylines_.back().addVertex(p);

    invalidateTessellation();
}

void ofPath::lineTo(float x, float y, float z) {
    lineTo(ofVec3f(x, y, z));
}

void ofPath::lineTo(const ofVec3f& p) {
    if (!hasCurrentPosition_) {
        // If no current position, treat as moveTo
        moveTo(p);
        return;
    }

    ensurePolyline();
    getCurrentPolyline().lineTo(p);
    currentPosition_ = p;

    invalidateTessellation();
}

void ofPath::curveTo(float x, float y, float z) {
    curveTo(ofVec3f(x, y, z));
}

void ofPath::curveTo(const ofVec3f& p) {
    if (!hasCurrentPosition_) {
        moveTo(p);
        return;
    }

    ensurePolyline();
    getCurrentPolyline().curveTo(p);
    currentPosition_ = p;

    invalidateTessellation();
}

void ofPath::bezierTo(float cx1, float cy1, float cz1,
                      float cx2, float cy2, float cz2,
                      float x, float y, float z) {
    bezierTo(ofVec3f(cx1, cy1, cz1), ofVec3f(cx2, cy2, cz2), ofVec3f(x, y, z));
}

void ofPath::bezierTo(const ofVec3f& cp1, const ofVec3f& cp2, const ofVec3f& p) {
    if (!hasCurrentPosition_) {
        moveTo(p);
        return;
    }

    ensurePolyline();
    getCurrentPolyline().bezierTo(cp1, cp2, p);
    currentPosition_ = p;

    invalidateTessellation();
}

void ofPath::arc(float x, float y, float radius, float angleBegin, float angleEnd) {
    if (!hasCurrentPosition_) {
        // Start from the beginning of the arc
        float rad = ofDegToRad(angleBegin);
        moveTo(x + radius * std::cos(rad), y + radius * std::sin(rad), 0);
    }

    ensurePolyline();
    getCurrentPolyline().arc(x, y, radius, angleBegin, angleEnd);

    // Update current position to end of arc
    float rad = ofDegToRad(angleEnd);
    currentPosition_ = ofVec3f(x + radius * std::cos(rad), y + radius * std::sin(rad), 0);

    invalidateTessellation();
}

void ofPath::arcNegative(float x, float y, float radius, float angleBegin, float angleEnd) {
    if (!hasCurrentPosition_) {
        float rad = ofDegToRad(angleBegin);
        moveTo(x + radius * std::cos(rad), y + radius * std::sin(rad), 0);
    }

    ensurePolyline();
    getCurrentPolyline().arcNegative(x, y, radius, angleBegin, angleEnd);

    // Update current position to end of arc
    float rad = ofDegToRad(angleEnd);
    currentPosition_ = ofVec3f(x + radius * std::cos(rad), y + radius * std::sin(rad), 0);

    invalidateTessellation();
}

void ofPath::close() {
    if (!polylines_.empty() && !polylines_.back().empty()) {
        polylines_.back().close();

        // Reset current position to start of closed polyline
        if (!polylines_.back().getVertices().empty()) {
            currentPosition_ = polylines_.back().getVertices().front();
        }

        invalidateTessellation();
    }
}

void ofPath::clear() {
    polylines_.clear();
    currentPosition_ = ofVec3f(0, 0, 0);
    hasCurrentPosition_ = false;
    invalidateTessellation();
}

// ============================================================================
// Rendering Properties
// ============================================================================

void ofPath::setFilled(bool filled) {
    filled_ = filled;
}

bool ofPath::isFilled() const {
    return filled_;
}

void ofPath::setStrokeWidth(float width) {
    strokeWidth_ = width;
}

float ofPath::getStrokeWidth() const {
    return strokeWidth_;
}

void ofPath::setFillColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    fillColor_[0] = r;
    fillColor_[1] = g;
    fillColor_[2] = b;
    fillColor_[3] = a;
}

void ofPath::setFillColor(uint8_t gray, uint8_t a) {
    setFillColor(gray, gray, gray, a);
}

void ofPath::setStrokeColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    strokeColor_[0] = r;
    strokeColor_[1] = g;
    strokeColor_[2] = b;
    strokeColor_[3] = a;
}

void ofPath::setStrokeColor(uint8_t gray, uint8_t a) {
    setStrokeColor(gray, gray, gray, a);
}

void ofPath::setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    setFillColor(r, g, b, a);
    setStrokeColor(r, g, b, a);
}

void ofPath::setColor(uint8_t gray, uint8_t a) {
    setColor(gray, gray, gray, a);
}

void ofPath::getFillColor(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) const {
    r = fillColor_[0];
    g = fillColor_[1];
    b = fillColor_[2];
    a = fillColor_[3];
}

void ofPath::getStrokeColor(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) const {
    r = strokeColor_[0];
    g = strokeColor_[1];
    b = strokeColor_[2];
    a = strokeColor_[3];
}

// ============================================================================
// Drawing
// ============================================================================

void ofPath::draw() const {
    draw(0, 0);
}

void ofPath::draw(float x, float y) const {
    if (polylines_.empty()) {
        return;
    }

    // Save current state
    uint8_t savedColor[4];
    ofGetColor(savedColor[0], savedColor[1], savedColor[2], savedColor[3]);
    float savedLineWidth = ofGetLineWidth();
    bool savedFill = ofGetFill();

    // Apply transformation if needed
    if (x != 0 || y != 0) {
        ofPushMatrix();
        ofTranslate(x, y, 0);
    }

    // Draw filled path
    if (filled_) {
        ofSetColor(fillColor_[0], fillColor_[1], fillColor_[2], fillColor_[3]);
        ofFill();

        // Get tessellated geometry and draw triangles
        auto tessellation = getTessellation();
        if (!tessellation.empty()) {
            // Draw triangles using basic triangle drawing
            // For now, we'll use ofDrawTriangle for each triangle
            for (size_t i = 0; i + 2 < tessellation.size(); i += 3) {
                const auto& v0 = tessellation[i];
                const auto& v1 = tessellation[i + 1];
                const auto& v2 = tessellation[i + 2];
                ofDrawTriangle(v0.x, v0.y, v0.z,
                             v1.x, v1.y, v1.z,
                             v2.x, v2.y, v2.z);
            }
        }
    }

    // Draw stroked path (outline)
    if (strokeWidth_ > 0) {
        ofSetColor(strokeColor_[0], strokeColor_[1], strokeColor_[2], strokeColor_[3]);
        ofNoFill();
        ofSetLineWidth(strokeWidth_);

        for (const auto& polyline : polylines_) {
            polyline.draw();
        }
    }

    // Restore transformation
    if (x != 0 || y != 0) {
        ofPopMatrix();
    }

    // Restore state
    ofSetColor(savedColor[0], savedColor[1], savedColor[2], savedColor[3]);
    ofSetLineWidth(savedLineWidth);
    if (savedFill) {
        ofFill();
    } else {
        ofNoFill();
    }
}

// ============================================================================
// Access to Geometry
// ============================================================================

const std::vector<ofPolyline>& ofPath::getOutline() const {
    return polylines_;
}

std::vector<ofVec3f> ofPath::getTessellation() const {
    if (!filled_) {
        return {};
    }

    if (!tessellationDirty_ && !tessellationCache_.empty()) {
        return tessellationCache_;
    }

    tessellate();
    return tessellationCache_;
}

// ============================================================================
// Simplification
// ============================================================================

void ofPath::simplify(float tolerance) {
    for (auto& polyline : polylines_) {
        polyline.simplify(tolerance);
    }
    invalidateTessellation();
}

// ============================================================================
// Transformations
// ============================================================================

void ofPath::translate(float x, float y, float z) {
    translate(ofVec3f(x, y, z));
}

void ofPath::translate(const ofVec3f& offset) {
    for (auto& polyline : polylines_) {
        auto& vertices = polyline.getVertices();
        for (auto& v : vertices) {
            v = v + offset;
        }
    }

    if (hasCurrentPosition_) {
        currentPosition_ = currentPosition_ + offset;
    }

    invalidateTessellation();
}

void ofPath::rotate(float angle) {
    // Rotate around Z axis (2D rotation)
    rotate(angle, ofVec3f(0, 0, 1));
}

void ofPath::rotate(float angle, const ofVec3f& axis) {
    ofMatrix4x4 rotation = ofMatrix4x4::identity();
    rotation.rotate(angle, axis);

    for (auto& polyline : polylines_) {
        auto& vertices = polyline.getVertices();
        for (auto& v : vertices) {
            v = rotation * v;
        }
    }

    if (hasCurrentPosition_) {
        currentPosition_ = rotation * currentPosition_;
    }

    invalidateTessellation();
}

void ofPath::scale(float sx, float sy, float sz) {
    for (auto& polyline : polylines_) {
        auto& vertices = polyline.getVertices();
        for (auto& v : vertices) {
            v.x *= sx;
            v.y *= sy;
            v.z *= sz;
        }
    }

    if (hasCurrentPosition_) {
        currentPosition_.x *= sx;
        currentPosition_.y *= sy;
        currentPosition_.z *= sz;
    }

    invalidateTessellation();
}

void ofPath::scale(float s) {
    scale(s, s, s);
}

// ============================================================================
// State Queries
// ============================================================================

bool ofPath::empty() const {
    return polylines_.empty();
}

size_t ofPath::getNumSubpaths() const {
    return polylines_.size();
}

ofVec3f ofPath::getCurrentPosition() const {
    return currentPosition_;
}

// ============================================================================
// Private Helper Methods
// ============================================================================

void ofPath::ensurePolyline() {
    if (polylines_.empty()) {
        polylines_.emplace_back();
        if (hasCurrentPosition_) {
            polylines_.back().addVertex(currentPosition_);
        }
    }
}

ofPolyline& ofPath::getCurrentPolyline() {
    ensurePolyline();
    return polylines_.back();
}

const ofPolyline& ofPath::getCurrentPolyline() const {
    if (polylines_.empty()) {
        static ofPolyline empty;
        return empty;
    }
    return polylines_.back();
}

void ofPath::tessellate() const {
    tessellationCache_.clear();
    tessellationDirty_ = false;

    if (polylines_.empty()) {
        return;
    }

    // Create tesselator
    TESStesselator* tess = tessNewTess(nullptr);
    if (!tess) {
        return;
    }

    // Add all polylines as contours
    for (const auto& polyline : polylines_) {
        const auto& vertices = polyline.getVertices();
        if (vertices.size() < 3) {
            continue; // Need at least 3 vertices for a valid contour
        }

        // Convert vertices to flat array for tess2
        std::vector<float> coords;
        coords.reserve(vertices.size() * 2);
        for (const auto& v : vertices) {
            coords.push_back(v.x);
            coords.push_back(v.y);
        }

        // Add contour to tesselator
        tessAddContour(tess, 2, coords.data(), sizeof(float) * 2,
                      static_cast<int>(vertices.size()));
    }

    // Perform tessellation
    // TESS_WINDING_ODD for typical path winding
    // TESS_POLYGONS to get triangles
    if (tessTesselate(tess, TESS_WINDING_ODD, TESS_POLYGONS, 3, 2, nullptr)) {
        const float* verts = tessGetVertices(tess);
        const int* elems = tessGetElements(tess);
        const int nelems = tessGetElementCount(tess);

        // Convert to ofVec3f triangles
        tessellationCache_.reserve(nelems * 3);

        for (int i = 0; i < nelems; ++i) {
            const int* tri = &elems[i * 3];
            for (int j = 0; j < 3; ++j) {
                if (tri[j] != TESS_UNDEF) {
                    const float* v = &verts[tri[j] * 2];
                    tessellationCache_.emplace_back(v[0], v[1], 0.0f);
                }
            }
        }
    }

    tessDeleteTess(tess);
}

void ofPath::invalidateTessellation() {
    tessellationDirty_ = true;
}

} // namespace oflike
