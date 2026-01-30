#include "of3dPrimitive.h"
#include "../graphics/ofGraphics.h"
#include <cmath>
#include <map>

namespace oflike {

// ============================================================================
// of3dPrimitive Base Class
// ============================================================================

of3dPrimitive::of3dPrimitive() {
}

of3dPrimitive::~of3dPrimitive() {
}

ofMesh& of3dPrimitive::getMesh() {
    return mesh_;
}

const ofMesh& of3dPrimitive::getMesh() const {
    return mesh_;
}

ofMesh* of3dPrimitive::getMeshPtr() {
    return &mesh_;
}

void of3dPrimitive::draw() const {
    ofPushMatrix();
    ofMultMatrix(getGlobalTransformMatrix());
    const_cast<ofMesh&>(mesh_).draw();
    ofPopMatrix();
}

void of3dPrimitive::drawWireframe() const {
    ofPushMatrix();
    ofMultMatrix(getGlobalTransformMatrix());
    const_cast<ofMesh&>(mesh_).drawWireframe();
    ofPopMatrix();
}

void of3dPrimitive::drawNormals(float length) const {
    ofPushMatrix();
    ofMultMatrix(getGlobalTransformMatrix());

    const auto& vertices = mesh_.getVertices();
    const auto& normals = mesh_.getNormals();

    for (size_t i = 0; i < std::min(vertices.size(), normals.size()); ++i) {
        ofVec3f start = vertices[i];
        ofVec3f end = start + normals[i] * length;
        ofDrawLine(start.x, start.y, start.z, end.x, end.y, end.z);
    }

    ofPopMatrix();
}

void of3dPrimitive::drawAxes(float length) const {
    ofPushMatrix();
    ofMultMatrix(getGlobalTransformMatrix());

    // X axis - red
    ofSetColor(255, 0, 0);
    ofDrawLine(0, 0, 0, length, 0, 0);

    // Y axis - green
    ofSetColor(0, 255, 0);
    ofDrawLine(0, 0, 0, 0, length, 0);

    // Z axis - blue
    ofSetColor(0, 0, 255);
    ofDrawLine(0, 0, 0, 0, 0, length);

    ofPopMatrix();
}

void of3dPrimitive::mapTexCoords(float u1, float v1, float u2, float v2) {
    auto& texCoords = mesh_.getTexCoords();
    for (auto& tc : texCoords) {
        tc.x = u1 + tc.x * (u2 - u1);
        tc.y = v1 + tc.y * (v2 - v1);
    }
}

void of3dPrimitive::setResolution(int res) {
    resolution_ = res;
}

int of3dPrimitive::getResolution() const {
    return resolution_;
}

// ============================================================================
// ofBoxPrimitive
// ============================================================================

ofBoxPrimitive::ofBoxPrimitive() {
    buildMesh();
}

ofBoxPrimitive::ofBoxPrimitive(float size)
    : width_(size), height_(size), depth_(size) {
    buildMesh();
}

ofBoxPrimitive::ofBoxPrimitive(float width, float height, float depth)
    : width_(width), height_(height), depth_(depth) {
    buildMesh();
}

void ofBoxPrimitive::set(float width, float height, float depth) {
    width_ = width;
    height_ = height;
    depth_ = depth;
    buildMesh();
}

void ofBoxPrimitive::set(float size) {
    set(size, size, size);
}

void ofBoxPrimitive::setWidth(float w) {
    width_ = w;
    buildMesh();
}

void ofBoxPrimitive::setHeight(float h) {
    height_ = h;
    buildMesh();
}

void ofBoxPrimitive::setDepth(float d) {
    depth_ = d;
    buildMesh();
}

void ofBoxPrimitive::setResolution(int res) {
    resolution_ = res;
    buildMesh();
}

void ofBoxPrimitive::buildMesh() {
    mesh_.clear();
    mesh_.setMode(OF_PRIMITIVE_TRIANGLES);

    float w = width_ / 2;
    float h = height_ / 2;
    float d = depth_ / 2;

    // 6 faces, each with 4 vertices and 2 triangles
    // Front face (+Z)
    mesh_.addVertex(ofVec3f(-w, -h, d)); mesh_.addNormal(ofVec3f(0, 0, 1)); mesh_.addTexCoord(ofVec2f(0, 1));
    mesh_.addVertex(ofVec3f(w, -h, d));  mesh_.addNormal(ofVec3f(0, 0, 1)); mesh_.addTexCoord(ofVec2f(1, 1));
    mesh_.addVertex(ofVec3f(w, h, d));   mesh_.addNormal(ofVec3f(0, 0, 1)); mesh_.addTexCoord(ofVec2f(1, 0));
    mesh_.addVertex(ofVec3f(-w, h, d));  mesh_.addNormal(ofVec3f(0, 0, 1)); mesh_.addTexCoord(ofVec2f(0, 0));

    // Back face (-Z)
    mesh_.addVertex(ofVec3f(w, -h, -d));  mesh_.addNormal(ofVec3f(0, 0, -1)); mesh_.addTexCoord(ofVec2f(0, 1));
    mesh_.addVertex(ofVec3f(-w, -h, -d)); mesh_.addNormal(ofVec3f(0, 0, -1)); mesh_.addTexCoord(ofVec2f(1, 1));
    mesh_.addVertex(ofVec3f(-w, h, -d));  mesh_.addNormal(ofVec3f(0, 0, -1)); mesh_.addTexCoord(ofVec2f(1, 0));
    mesh_.addVertex(ofVec3f(w, h, -d));   mesh_.addNormal(ofVec3f(0, 0, -1)); mesh_.addTexCoord(ofVec2f(0, 0));

    // Top face (+Y)
    mesh_.addVertex(ofVec3f(-w, h, d));  mesh_.addNormal(ofVec3f(0, 1, 0)); mesh_.addTexCoord(ofVec2f(0, 1));
    mesh_.addVertex(ofVec3f(w, h, d));   mesh_.addNormal(ofVec3f(0, 1, 0)); mesh_.addTexCoord(ofVec2f(1, 1));
    mesh_.addVertex(ofVec3f(w, h, -d));  mesh_.addNormal(ofVec3f(0, 1, 0)); mesh_.addTexCoord(ofVec2f(1, 0));
    mesh_.addVertex(ofVec3f(-w, h, -d)); mesh_.addNormal(ofVec3f(0, 1, 0)); mesh_.addTexCoord(ofVec2f(0, 0));

    // Bottom face (-Y)
    mesh_.addVertex(ofVec3f(-w, -h, -d)); mesh_.addNormal(ofVec3f(0, -1, 0)); mesh_.addTexCoord(ofVec2f(0, 1));
    mesh_.addVertex(ofVec3f(w, -h, -d));  mesh_.addNormal(ofVec3f(0, -1, 0)); mesh_.addTexCoord(ofVec2f(1, 1));
    mesh_.addVertex(ofVec3f(w, -h, d));   mesh_.addNormal(ofVec3f(0, -1, 0)); mesh_.addTexCoord(ofVec2f(1, 0));
    mesh_.addVertex(ofVec3f(-w, -h, d));  mesh_.addNormal(ofVec3f(0, -1, 0)); mesh_.addTexCoord(ofVec2f(0, 0));

    // Right face (+X)
    mesh_.addVertex(ofVec3f(w, -h, d));  mesh_.addNormal(ofVec3f(1, 0, 0)); mesh_.addTexCoord(ofVec2f(0, 1));
    mesh_.addVertex(ofVec3f(w, -h, -d)); mesh_.addNormal(ofVec3f(1, 0, 0)); mesh_.addTexCoord(ofVec2f(1, 1));
    mesh_.addVertex(ofVec3f(w, h, -d));  mesh_.addNormal(ofVec3f(1, 0, 0)); mesh_.addTexCoord(ofVec2f(1, 0));
    mesh_.addVertex(ofVec3f(w, h, d));   mesh_.addNormal(ofVec3f(1, 0, 0)); mesh_.addTexCoord(ofVec2f(0, 0));

    // Left face (-X)
    mesh_.addVertex(ofVec3f(-w, -h, -d)); mesh_.addNormal(ofVec3f(-1, 0, 0)); mesh_.addTexCoord(ofVec2f(0, 1));
    mesh_.addVertex(ofVec3f(-w, -h, d));  mesh_.addNormal(ofVec3f(-1, 0, 0)); mesh_.addTexCoord(ofVec2f(1, 1));
    mesh_.addVertex(ofVec3f(-w, h, d));   mesh_.addNormal(ofVec3f(-1, 0, 0)); mesh_.addTexCoord(ofVec2f(1, 0));
    mesh_.addVertex(ofVec3f(-w, h, -d));  mesh_.addNormal(ofVec3f(-1, 0, 0)); mesh_.addTexCoord(ofVec2f(0, 0));

    // Indices for 6 faces
    for (int face = 0; face < 6; ++face) {
        int base = face * 4;
        mesh_.addIndex(base + 0);
        mesh_.addIndex(base + 1);
        mesh_.addIndex(base + 2);
        mesh_.addIndex(base + 0);
        mesh_.addIndex(base + 2);
        mesh_.addIndex(base + 3);
    }
}

// ============================================================================
// ofSpherePrimitive
// ============================================================================

ofSpherePrimitive::ofSpherePrimitive() {
    resolution_ = 24;
    buildMesh();
}

ofSpherePrimitive::ofSpherePrimitive(float radius, int resolution)
    : radius_(radius) {
    resolution_ = resolution;
    buildMesh();
}

void ofSpherePrimitive::setRadius(float radius) {
    radius_ = radius;
    buildMesh();
}

void ofSpherePrimitive::setResolution(int res) {
    resolution_ = res;
    buildMesh();
}

void ofSpherePrimitive::buildMesh() {
    mesh_.clear();
    mesh_.setMode(OF_PRIMITIVE_TRIANGLES);

    const int stacks = resolution_;
    const int slices = resolution_;
    const float PI = 3.14159265358979f;

    // Generate vertices
    for (int i = 0; i <= stacks; ++i) {
        float phi = PI * static_cast<float>(i) / static_cast<float>(stacks);
        float y = radius_ * std::cos(phi);
        float r = radius_ * std::sin(phi);

        for (int j = 0; j <= slices; ++j) {
            float theta = 2.0f * PI * static_cast<float>(j) / static_cast<float>(slices);
            float x = r * std::cos(theta);
            float z = r * std::sin(theta);

            ofVec3f vertex(x, y, z);
            ofVec3f normal = vertex.getNormalized();

            mesh_.addVertex(vertex);
            mesh_.addNormal(normal);
            mesh_.addTexCoord(ofVec2f(
                static_cast<float>(j) / static_cast<float>(slices),
                static_cast<float>(i) / static_cast<float>(stacks)
            ));
        }
    }

    // Generate indices
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int first = i * (slices + 1) + j;
            int second = first + slices + 1;

            mesh_.addIndex(first);
            mesh_.addIndex(second);
            mesh_.addIndex(first + 1);

            mesh_.addIndex(second);
            mesh_.addIndex(second + 1);
            mesh_.addIndex(first + 1);
        }
    }
}

// ============================================================================
// ofCylinderPrimitive
// ============================================================================

ofCylinderPrimitive::ofCylinderPrimitive() {
    buildMesh();
}

ofCylinderPrimitive::ofCylinderPrimitive(float radius, float height,
                                         int radiusSegments, int heightSegments,
                                         bool bCapped)
    : radius_(radius), height_(height),
      radiusSegments_(radiusSegments), heightSegments_(heightSegments),
      capped_(bCapped) {
    buildMesh();
}

void ofCylinderPrimitive::set(float radius, float height,
                              int radiusSegments, int heightSegments,
                              bool bCapped) {
    radius_ = radius;
    height_ = height;
    radiusSegments_ = radiusSegments;
    heightSegments_ = heightSegments;
    capped_ = bCapped;
    buildMesh();
}

void ofCylinderPrimitive::setRadius(float radius) {
    radius_ = radius;
    buildMesh();
}

void ofCylinderPrimitive::setHeight(float height) {
    height_ = height;
    buildMesh();
}

void ofCylinderPrimitive::setCapped(bool capped) {
    capped_ = capped;
    buildMesh();
}

void ofCylinderPrimitive::setResolution(int res) {
    radiusSegments_ = res;
    buildMesh();
}

void ofCylinderPrimitive::buildMesh() {
    mesh_.clear();
    mesh_.setMode(OF_PRIMITIVE_TRIANGLES);

    const float PI = 3.14159265358979f;
    float halfH = height_ / 2;

    // Side surface
    for (int i = 0; i <= heightSegments_; ++i) {
        float y = -halfH + height_ * static_cast<float>(i) / static_cast<float>(heightSegments_);

        for (int j = 0; j <= radiusSegments_; ++j) {
            float theta = 2.0f * PI * static_cast<float>(j) / static_cast<float>(radiusSegments_);
            float x = radius_ * std::cos(theta);
            float z = radius_ * std::sin(theta);

            mesh_.addVertex(ofVec3f(x, y, z));
            mesh_.addNormal(ofVec3f(std::cos(theta), 0, std::sin(theta)));
            mesh_.addTexCoord(ofVec2f(
                static_cast<float>(j) / static_cast<float>(radiusSegments_),
                static_cast<float>(i) / static_cast<float>(heightSegments_)
            ));
        }
    }

    // Side indices
    for (int i = 0; i < heightSegments_; ++i) {
        for (int j = 0; j < radiusSegments_; ++j) {
            int first = i * (radiusSegments_ + 1) + j;
            int second = first + radiusSegments_ + 1;

            mesh_.addIndex(first);
            mesh_.addIndex(second);
            mesh_.addIndex(first + 1);

            mesh_.addIndex(second);
            mesh_.addIndex(second + 1);
            mesh_.addIndex(first + 1);
        }
    }

    // Caps
    if (capped_) {
        int baseIndex = mesh_.getNumVertices();

        // Top cap
        mesh_.addVertex(ofVec3f(0, halfH, 0));
        mesh_.addNormal(ofVec3f(0, 1, 0));
        mesh_.addTexCoord(ofVec2f(0.5f, 0.5f));
        int topCenter = baseIndex++;

        for (int j = 0; j <= radiusSegments_; ++j) {
            float theta = 2.0f * PI * static_cast<float>(j) / static_cast<float>(radiusSegments_);
            float x = radius_ * std::cos(theta);
            float z = radius_ * std::sin(theta);

            mesh_.addVertex(ofVec3f(x, halfH, z));
            mesh_.addNormal(ofVec3f(0, 1, 0));
            mesh_.addTexCoord(ofVec2f(0.5f + 0.5f * std::cos(theta), 0.5f + 0.5f * std::sin(theta)));
        }

        for (int j = 0; j < radiusSegments_; ++j) {
            mesh_.addIndex(topCenter);
            mesh_.addIndex(topCenter + 1 + j);
            mesh_.addIndex(topCenter + 2 + j);
        }

        baseIndex = mesh_.getNumVertices();

        // Bottom cap
        mesh_.addVertex(ofVec3f(0, -halfH, 0));
        mesh_.addNormal(ofVec3f(0, -1, 0));
        mesh_.addTexCoord(ofVec2f(0.5f, 0.5f));
        int bottomCenter = baseIndex++;

        for (int j = 0; j <= radiusSegments_; ++j) {
            float theta = 2.0f * PI * static_cast<float>(j) / static_cast<float>(radiusSegments_);
            float x = radius_ * std::cos(theta);
            float z = radius_ * std::sin(theta);

            mesh_.addVertex(ofVec3f(x, -halfH, z));
            mesh_.addNormal(ofVec3f(0, -1, 0));
            mesh_.addTexCoord(ofVec2f(0.5f + 0.5f * std::cos(theta), 0.5f + 0.5f * std::sin(theta)));
        }

        for (int j = 0; j < radiusSegments_; ++j) {
            mesh_.addIndex(bottomCenter);
            mesh_.addIndex(bottomCenter + 2 + j);
            mesh_.addIndex(bottomCenter + 1 + j);
        }
    }
}

// ============================================================================
// ofConePrimitive
// ============================================================================

ofConePrimitive::ofConePrimitive() {
    buildMesh();
}

ofConePrimitive::ofConePrimitive(float radius, float height,
                                 int radiusSegments, int heightSegments,
                                 bool bCapped)
    : radius_(radius), height_(height),
      radiusSegments_(radiusSegments), heightSegments_(heightSegments),
      capped_(bCapped) {
    buildMesh();
}

void ofConePrimitive::set(float radius, float height,
                          int radiusSegments, int heightSegments,
                          bool bCapped) {
    radius_ = radius;
    height_ = height;
    radiusSegments_ = radiusSegments;
    heightSegments_ = heightSegments;
    capped_ = bCapped;
    buildMesh();
}

void ofConePrimitive::setRadius(float radius) {
    radius_ = radius;
    buildMesh();
}

void ofConePrimitive::setHeight(float height) {
    height_ = height;
    buildMesh();
}

void ofConePrimitive::setCapped(bool capped) {
    capped_ = capped;
    buildMesh();
}

void ofConePrimitive::setResolution(int res) {
    radiusSegments_ = res;
    buildMesh();
}

void ofConePrimitive::buildMesh() {
    mesh_.clear();
    mesh_.setMode(OF_PRIMITIVE_TRIANGLES);

    const float PI = 3.14159265358979f;
    float halfH = height_ / 2;

    // Calculate the slope for normals
    float slopeAngle = std::atan2(radius_, height_);
    float ny = std::sin(slopeAngle);
    float nxz = std::cos(slopeAngle);

    // Side surface
    for (int i = 0; i <= heightSegments_; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(heightSegments_);
        float y = -halfH + height_ * t;
        float r = radius_ * (1.0f - t);  // Radius decreases to 0 at top

        for (int j = 0; j <= radiusSegments_; ++j) {
            float theta = 2.0f * PI * static_cast<float>(j) / static_cast<float>(radiusSegments_);
            float x = r * std::cos(theta);
            float z = r * std::sin(theta);

            mesh_.addVertex(ofVec3f(x, y, z));
            mesh_.addNormal(ofVec3f(nxz * std::cos(theta), ny, nxz * std::sin(theta)));
            mesh_.addTexCoord(ofVec2f(
                static_cast<float>(j) / static_cast<float>(radiusSegments_),
                t
            ));
        }
    }

    // Side indices
    for (int i = 0; i < heightSegments_; ++i) {
        for (int j = 0; j < radiusSegments_; ++j) {
            int first = i * (radiusSegments_ + 1) + j;
            int second = first + radiusSegments_ + 1;

            mesh_.addIndex(first);
            mesh_.addIndex(second);
            mesh_.addIndex(first + 1);

            mesh_.addIndex(second);
            mesh_.addIndex(second + 1);
            mesh_.addIndex(first + 1);
        }
    }

    // Bottom cap
    if (capped_) {
        int baseIndex = mesh_.getNumVertices();

        mesh_.addVertex(ofVec3f(0, -halfH, 0));
        mesh_.addNormal(ofVec3f(0, -1, 0));
        mesh_.addTexCoord(ofVec2f(0.5f, 0.5f));
        int bottomCenter = baseIndex++;

        for (int j = 0; j <= radiusSegments_; ++j) {
            float theta = 2.0f * PI * static_cast<float>(j) / static_cast<float>(radiusSegments_);
            float x = radius_ * std::cos(theta);
            float z = radius_ * std::sin(theta);

            mesh_.addVertex(ofVec3f(x, -halfH, z));
            mesh_.addNormal(ofVec3f(0, -1, 0));
            mesh_.addTexCoord(ofVec2f(0.5f + 0.5f * std::cos(theta), 0.5f + 0.5f * std::sin(theta)));
        }

        for (int j = 0; j < radiusSegments_; ++j) {
            mesh_.addIndex(bottomCenter);
            mesh_.addIndex(bottomCenter + 2 + j);
            mesh_.addIndex(bottomCenter + 1 + j);
        }
    }
}

// ============================================================================
// ofPlanePrimitive
// ============================================================================

ofPlanePrimitive::ofPlanePrimitive() {
    buildMesh();
}

ofPlanePrimitive::ofPlanePrimitive(float width, float height, int columns, int rows)
    : width_(width), height_(height), columns_(columns), rows_(rows) {
    buildMesh();
}

void ofPlanePrimitive::set(float width, float height, int columns, int rows) {
    width_ = width;
    height_ = height;
    columns_ = columns;
    rows_ = rows;
    buildMesh();
}

void ofPlanePrimitive::setWidth(float width) {
    width_ = width;
    buildMesh();
}

void ofPlanePrimitive::setHeight(float height) {
    height_ = height;
    buildMesh();
}

void ofPlanePrimitive::setResolution(int res) {
    columns_ = rows_ = res;
    buildMesh();
}

void ofPlanePrimitive::buildMesh() {
    mesh_.clear();
    mesh_.setMode(OF_PRIMITIVE_TRIANGLES);

    float halfW = width_ / 2;
    float halfH = height_ / 2;

    for (int i = 0; i <= rows_; ++i) {
        float y = -halfH + height_ * static_cast<float>(i) / static_cast<float>(rows_);

        for (int j = 0; j <= columns_; ++j) {
            float x = -halfW + width_ * static_cast<float>(j) / static_cast<float>(columns_);

            mesh_.addVertex(ofVec3f(x, y, 0));
            mesh_.addNormal(ofVec3f(0, 0, 1));
            mesh_.addTexCoord(ofVec2f(
                static_cast<float>(j) / static_cast<float>(columns_),
                1.0f - static_cast<float>(i) / static_cast<float>(rows_)
            ));
        }
    }

    for (int i = 0; i < rows_; ++i) {
        for (int j = 0; j < columns_; ++j) {
            int first = i * (columns_ + 1) + j;
            int second = first + columns_ + 1;

            mesh_.addIndex(first);
            mesh_.addIndex(second);
            mesh_.addIndex(first + 1);

            mesh_.addIndex(second);
            mesh_.addIndex(second + 1);
            mesh_.addIndex(first + 1);
        }
    }
}

// ============================================================================
// ofIcoSpherePrimitive
// ============================================================================

ofIcoSpherePrimitive::ofIcoSpherePrimitive() {
    buildMesh();
}

ofIcoSpherePrimitive::ofIcoSpherePrimitive(float radius, int iterations)
    : radius_(radius), iterations_(iterations) {
    buildMesh();
}

void ofIcoSpherePrimitive::setRadius(float radius) {
    radius_ = radius;
    buildMesh();
}

void ofIcoSpherePrimitive::setResolution(int iterations) {
    iterations_ = std::max(0, std::min(5, iterations));  // Limit to prevent too many vertices
    buildMesh();
}

void ofIcoSpherePrimitive::buildMesh() {
    mesh_.clear();
    mesh_.setMode(OF_PRIMITIVE_TRIANGLES);

    const float PI = 3.14159265358979f;
    const float t = (1.0f + std::sqrt(5.0f)) / 2.0f;  // Golden ratio

    // Initial icosahedron vertices
    std::vector<ofVec3f> vertices = {
        ofVec3f(-1, t, 0), ofVec3f(1, t, 0), ofVec3f(-1, -t, 0), ofVec3f(1, -t, 0),
        ofVec3f(0, -1, t), ofVec3f(0, 1, t), ofVec3f(0, -1, -t), ofVec3f(0, 1, -t),
        ofVec3f(t, 0, -1), ofVec3f(t, 0, 1), ofVec3f(-t, 0, -1), ofVec3f(-t, 0, 1)
    };

    // Normalize to unit sphere
    for (auto& v : vertices) {
        v.normalize();
    }

    // Initial icosahedron faces (20 triangles)
    std::vector<std::tuple<int, int, int>> faces = {
        {0, 11, 5}, {0, 5, 1}, {0, 1, 7}, {0, 7, 10}, {0, 10, 11},
        {1, 5, 9}, {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
        {3, 9, 4}, {3, 4, 2}, {3, 2, 6}, {3, 6, 8}, {3, 8, 9},
        {4, 9, 5}, {2, 4, 11}, {6, 2, 10}, {8, 6, 7}, {9, 8, 1}
    };

    // Helper to get midpoint index (with caching)
    std::map<std::pair<int, int>, int> midpointCache;
    auto getMidpoint = [&](int i1, int i2) -> int {
        auto key = std::make_pair(std::min(i1, i2), std::max(i1, i2));
        auto it = midpointCache.find(key);
        if (it != midpointCache.end()) {
            return it->second;
        }

        ofVec3f mid = (vertices[i1] + vertices[i2]) * 0.5f;
        mid.normalize();
        int index = static_cast<int>(vertices.size());
        vertices.push_back(mid);
        midpointCache[key] = index;
        return index;
    };

    // Subdivide
    for (int iter = 0; iter < iterations_; ++iter) {
        std::vector<std::tuple<int, int, int>> newFaces;
        midpointCache.clear();

        for (const auto& face : faces) {
            int v1 = std::get<0>(face);
            int v2 = std::get<1>(face);
            int v3 = std::get<2>(face);

            int a = getMidpoint(v1, v2);
            int b = getMidpoint(v2, v3);
            int c = getMidpoint(v3, v1);

            newFaces.push_back({v1, a, c});
            newFaces.push_back({v2, b, a});
            newFaces.push_back({v3, c, b});
            newFaces.push_back({a, b, c});
        }

        faces = std::move(newFaces);
    }

    // Build final mesh
    for (const auto& v : vertices) {
        ofVec3f pos = v * radius_;
        mesh_.addVertex(pos);
        mesh_.addNormal(v);

        // Spherical UV mapping
        float u = 0.5f + std::atan2(v.z, v.x) / (2.0f * PI);
        float vCoord = 0.5f - std::asin(v.y) / PI;
        mesh_.addTexCoord(ofVec2f(u, vCoord));
    }

    for (const auto& face : faces) {
        mesh_.addIndex(std::get<0>(face));
        mesh_.addIndex(std::get<1>(face));
        mesh_.addIndex(std::get<2>(face));
    }
}

} // namespace oflike
