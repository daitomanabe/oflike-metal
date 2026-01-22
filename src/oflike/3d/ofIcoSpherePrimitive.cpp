#include "ofIcoSpherePrimitive.h"
#include <cmath>

namespace oflike {

ofIcoSpherePrimitive::ofIcoSpherePrimitive()
    : radius_(100.0f), iterations_(2), vertexIndex_(0) {
    generateMesh();
}

void ofIcoSpherePrimitive::set(float radius, int iterations) {
    radius_ = radius;
    iterations_ = iterations;
    generateMesh();
}

void ofIcoSpherePrimitive::setRadius(float radius) {
    radius_ = radius;
    generateMesh();
}

void ofIcoSpherePrimitive::setResolution(int iterations) {
    iterations_ = iterations;
    generateMesh();
}

void ofIcoSpherePrimitive::generateMesh() {
    mesh_.clear();
    vertices_.clear();
    indices_.clear();
    middlePointCache_.clear();
    vertexIndex_ = 0;

    // Create base icosahedron
    createIcosahedron();

    // Subdivide
    for (int i = 0; i < iterations_; ++i) {
        subdivide();
    }

    // Transfer to mesh
    for (const auto& vertex : vertices_) {
        mesh_.addVertex(vertex);
        // Normal is just normalized position for sphere
        glm::vec3 normal = glm::normalize(vertex);
        mesh_.addNormal(normal);

        // UV mapping: spherical coordinates
        float u = 0.5f + std::atan2(normal.z, normal.x) / (2.0f * M_PI);
        float v = 0.5f - std::asin(normal.y) / M_PI;
        mesh_.addTexCoord(glm::vec2(u, v));
    }

    for (int idx : indices_) {
        mesh_.addIndex(idx);
    }

    mesh_.setMode(ofPrimitiveMode::OF_PRIMITIVE_TRIANGLES);
}

void ofIcoSpherePrimitive::createIcosahedron() {
    // Golden ratio
    const float t = (1.0f + std::sqrt(5.0f)) / 2.0f;

    // Create 12 vertices of icosahedron
    // Normalized to unit sphere, then scaled by radius
    std::vector<glm::vec3> baseVertices = {
        glm::vec3(-1,  t,  0),
        glm::vec3( 1,  t,  0),
        glm::vec3(-1, -t,  0),
        glm::vec3( 1, -t,  0),

        glm::vec3( 0, -1,  t),
        glm::vec3( 0,  1,  t),
        glm::vec3( 0, -1, -t),
        glm::vec3( 0,  1, -t),

        glm::vec3( t,  0, -1),
        glm::vec3( t,  0,  1),
        glm::vec3(-t,  0, -1),
        glm::vec3(-t,  0,  1),
    };

    // Normalize and scale
    for (auto& v : baseVertices) {
        v = glm::normalize(v) * radius_;
        vertices_.push_back(v);
    }
    vertexIndex_ = vertices_.size();

    // Create 20 triangular faces
    const std::vector<std::vector<int>> faces = {
        // 5 faces around point 0
        {0, 11, 5},
        {0, 5, 1},
        {0, 1, 7},
        {0, 7, 10},
        {0, 10, 11},

        // 5 adjacent faces
        {1, 5, 9},
        {5, 11, 4},
        {11, 10, 2},
        {10, 7, 6},
        {7, 1, 8},

        // 5 faces around point 3
        {3, 9, 4},
        {3, 4, 2},
        {3, 2, 6},
        {3, 6, 8},
        {3, 8, 9},

        // 5 adjacent faces
        {4, 9, 5},
        {2, 4, 11},
        {6, 2, 10},
        {8, 6, 7},
        {9, 8, 1},
    };

    for (const auto& face : faces) {
        indices_.push_back(face[0]);
        indices_.push_back(face[1]);
        indices_.push_back(face[2]);
    }
}

void ofIcoSpherePrimitive::subdivide() {
    std::vector<int> newIndices;

    // For each triangle
    for (size_t i = 0; i < indices_.size(); i += 3) {
        int v1 = indices_[i];
        int v2 = indices_[i + 1];
        int v3 = indices_[i + 2];

        // Get or create midpoints
        int a = getMiddlePoint(v1, v2);
        int b = getMiddlePoint(v2, v3);
        int c = getMiddlePoint(v3, v1);

        // Create 4 new triangles
        newIndices.push_back(v1);
        newIndices.push_back(a);
        newIndices.push_back(c);

        newIndices.push_back(v2);
        newIndices.push_back(b);
        newIndices.push_back(a);

        newIndices.push_back(v3);
        newIndices.push_back(c);
        newIndices.push_back(b);

        newIndices.push_back(a);
        newIndices.push_back(b);
        newIndices.push_back(c);
    }

    indices_ = newIndices;
}

int ofIcoSpherePrimitive::getMiddlePoint(int p1, int p2) {
    // Use smaller index first for consistent key
    bool smallerFirst = p1 < p2;
    long long smallerIndex = smallerFirst ? p1 : p2;
    long long greaterIndex = smallerFirst ? p2 : p1;
    long long key = (smallerIndex << 32) + greaterIndex;

    // Check if we already have this midpoint
    auto it = middlePointCache_.find(key);
    if (it != middlePointCache_.end()) {
        return it->second;
    }

    // Calculate midpoint
    glm::vec3 point1 = vertices_[p1];
    glm::vec3 point2 = vertices_[p2];
    glm::vec3 middle = (point1 + point2) * 0.5f;

    // Project to sphere surface
    middle = glm::normalize(middle) * radius_;

    // Add vertex
    vertices_.push_back(middle);
    int index = vertexIndex_++;

    // Cache for later use
    middlePointCache_[key] = index;

    return index;
}

} // namespace oflike
