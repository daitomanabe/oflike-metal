#include "ofCylinderPrimitive.h"
#include <cmath>

namespace oflike {

ofCylinderPrimitive::ofCylinderPrimitive()
    : radius_(50.0f)
    , height_(100.0f)
    , radiusRes_(12)
    , heightRes_(6)
    , capRes_(2)
    , capped_(true)
{
    regenerateMesh();
}

void ofCylinderPrimitive::set(float radius, float height, int radiusRes, int heightRes, int capRes, bool capped) {
    radius_ = radius;
    height_ = height;
    radiusRes_ = radiusRes;
    heightRes_ = heightRes;
    capRes_ = capRes;
    capped_ = capped;
    regenerateMesh();
}

void ofCylinderPrimitive::setRadius(float radius) {
    radius_ = radius;
    regenerateMesh();
}

void ofCylinderPrimitive::setHeight(float height) {
    height_ = height;
    regenerateMesh();
}

void ofCylinderPrimitive::setResolutionRadius(int res) {
    radiusRes_ = res;
    regenerateMesh();
}

void ofCylinderPrimitive::setResolutionHeight(int res) {
    heightRes_ = res;
    regenerateMesh();
}

void ofCylinderPrimitive::setResolutionCap(int res) {
    capRes_ = res;
    regenerateMesh();
}

void ofCylinderPrimitive::regenerateMesh() {
    mesh_.clear();
    mesh_.setMode(OF_PRIMITIVE_TRIANGLES);

    const float halfHeight = height_ * 0.5f;
    const int segments = radiusRes_;
    const int rings = heightRes_;

    // Generate cylinder body vertices
    for (int ring = 0; ring <= rings; ++ring) {
        float y = -halfHeight + (height_ * ring / float(rings));
        float v = ring / float(rings);

        for (int seg = 0; seg <= segments; ++seg) {
            float angle = (seg / float(segments)) * 2.0f * M_PI;
            float x = radius_ * std::cos(angle);
            float z = radius_ * std::sin(angle);
            float u = seg / float(segments);

            // Position
            mesh_.addVertex(glm::vec3(x, y, z));

            // Normal (pointing outward, perpendicular to Y axis)
            glm::vec3 normal = glm::normalize(glm::vec3(x, 0.0f, z));
            mesh_.addNormal(normal);

            // UV
            mesh_.addTexCoord(glm::vec2(u, v));
        }
    }

    // Generate cylinder body indices
    for (int ring = 0; ring < rings; ++ring) {
        for (int seg = 0; seg < segments; ++seg) {
            int curr = ring * (segments + 1) + seg;
            int next = curr + segments + 1;

            // Two triangles per quad
            mesh_.addIndex(curr);
            mesh_.addIndex(next);
            mesh_.addIndex(curr + 1);

            mesh_.addIndex(curr + 1);
            mesh_.addIndex(next);
            mesh_.addIndex(next + 1);
        }
    }

    // Generate caps if enabled
    if (capped_) {
        int bodyVertexCount = mesh_.getVertices().size();

        // Top cap (y = halfHeight)
        // Center vertex
        int topCenterIdx = mesh_.getVertices().size();
        mesh_.addVertex(glm::vec3(0.0f, halfHeight, 0.0f));
        mesh_.addNormal(glm::vec3(0.0f, 1.0f, 0.0f));
        mesh_.addTexCoord(glm::vec2(0.5f, 0.5f));

        // Radial vertices for top cap
        for (int seg = 0; seg <= segments; ++seg) {
            float angle = (seg / float(segments)) * 2.0f * M_PI;
            float x = radius_ * std::cos(angle);
            float z = radius_ * std::sin(angle);
            float u = 0.5f + 0.5f * std::cos(angle);
            float v = 0.5f + 0.5f * std::sin(angle);

            mesh_.addVertex(glm::vec3(x, halfHeight, z));
            mesh_.addNormal(glm::vec3(0.0f, 1.0f, 0.0f));
            mesh_.addTexCoord(glm::vec2(u, v));
        }

        // Top cap indices
        for (int seg = 0; seg < segments; ++seg) {
            mesh_.addIndex(topCenterIdx);
            mesh_.addIndex(topCenterIdx + seg + 1);
            mesh_.addIndex(topCenterIdx + seg + 2);
        }

        // Bottom cap (y = -halfHeight)
        // Center vertex
        int bottomCenterIdx = mesh_.getVertices().size();
        mesh_.addVertex(glm::vec3(0.0f, -halfHeight, 0.0f));
        mesh_.addNormal(glm::vec3(0.0f, -1.0f, 0.0f));
        mesh_.addTexCoord(glm::vec2(0.5f, 0.5f));

        // Radial vertices for bottom cap
        for (int seg = 0; seg <= segments; ++seg) {
            float angle = (seg / float(segments)) * 2.0f * M_PI;
            float x = radius_ * std::cos(angle);
            float z = radius_ * std::sin(angle);
            float u = 0.5f + 0.5f * std::cos(angle);
            float v = 0.5f + 0.5f * std::sin(angle);

            mesh_.addVertex(glm::vec3(x, -halfHeight, z));
            mesh_.addNormal(glm::vec3(0.0f, -1.0f, 0.0f));
            mesh_.addTexCoord(glm::vec2(u, v));
        }

        // Bottom cap indices (reversed winding for correct facing)
        for (int seg = 0; seg < segments; ++seg) {
            mesh_.addIndex(bottomCenterIdx);
            mesh_.addIndex(bottomCenterIdx + seg + 2);
            mesh_.addIndex(bottomCenterIdx + seg + 1);
        }
    }
}

ofMesh ofCylinderPrimitive::getTopCapMesh() const {
    ofMesh capMesh;
    if (!capped_) return capMesh;

    capMesh.setMode(OF_PRIMITIVE_TRIANGLES);

    const float halfHeight = height_ * 0.5f;
    const int segments = radiusRes_;

    // Center vertex
    capMesh.addVertex(glm::vec3(0.0f, halfHeight, 0.0f));
    capMesh.addNormal(glm::vec3(0.0f, 1.0f, 0.0f));
    capMesh.addTexCoord(glm::vec2(0.5f, 0.5f));

    // Radial vertices
    for (int seg = 0; seg <= segments; ++seg) {
        float angle = (seg / float(segments)) * 2.0f * M_PI;
        float x = radius_ * std::cos(angle);
        float z = radius_ * std::sin(angle);
        float u = 0.5f + 0.5f * std::cos(angle);
        float v = 0.5f + 0.5f * std::sin(angle);

        capMesh.addVertex(glm::vec3(x, halfHeight, z));
        capMesh.addNormal(glm::vec3(0.0f, 1.0f, 0.0f));
        capMesh.addTexCoord(glm::vec2(u, v));
    }

    // Indices
    for (int seg = 0; seg < segments; ++seg) {
        capMesh.addIndex(0);
        capMesh.addIndex(seg + 1);
        capMesh.addIndex(seg + 2);
    }

    return capMesh;
}

ofMesh ofCylinderPrimitive::getBottomCapMesh() const {
    ofMesh capMesh;
    if (!capped_) return capMesh;

    capMesh.setMode(OF_PRIMITIVE_TRIANGLES);

    const float halfHeight = height_ * 0.5f;
    const int segments = radiusRes_;

    // Center vertex
    capMesh.addVertex(glm::vec3(0.0f, -halfHeight, 0.0f));
    capMesh.addNormal(glm::vec3(0.0f, -1.0f, 0.0f));
    capMesh.addTexCoord(glm::vec2(0.5f, 0.5f));

    // Radial vertices
    for (int seg = 0; seg <= segments; ++seg) {
        float angle = (seg / float(segments)) * 2.0f * M_PI;
        float x = radius_ * std::cos(angle);
        float z = radius_ * std::sin(angle);
        float u = 0.5f + 0.5f * std::cos(angle);
        float v = 0.5f + 0.5f * std::sin(angle);

        capMesh.addVertex(glm::vec3(x, -halfHeight, z));
        capMesh.addNormal(glm::vec3(0.0f, -1.0f, 0.0f));
        capMesh.addTexCoord(glm::vec2(u, v));
    }

    // Indices (reversed winding)
    for (int seg = 0; seg < segments; ++seg) {
        capMesh.addIndex(0);
        capMesh.addIndex(seg + 2);
        capMesh.addIndex(seg + 1);
    }

    return capMesh;
}

ofMesh ofCylinderPrimitive::getCylinderMesh() const {
    ofMesh cylinderMesh;
    cylinderMesh.setMode(OF_PRIMITIVE_TRIANGLES);

    const float halfHeight = height_ * 0.5f;
    const int segments = radiusRes_;
    const int rings = heightRes_;

    // Generate cylinder body vertices
    for (int ring = 0; ring <= rings; ++ring) {
        float y = -halfHeight + (height_ * ring / float(rings));
        float v = ring / float(rings);

        for (int seg = 0; seg <= segments; ++seg) {
            float angle = (seg / float(segments)) * 2.0f * M_PI;
            float x = radius_ * std::cos(angle);
            float z = radius_ * std::sin(angle);
            float u = seg / float(segments);

            // Position
            cylinderMesh.addVertex(glm::vec3(x, y, z));

            // Normal
            glm::vec3 normal = glm::normalize(glm::vec3(x, 0.0f, z));
            cylinderMesh.addNormal(normal);

            // UV
            cylinderMesh.addTexCoord(glm::vec2(u, v));
        }
    }

    // Generate cylinder body indices
    for (int ring = 0; ring < rings; ++ring) {
        for (int seg = 0; seg < segments; ++seg) {
            int curr = ring * (segments + 1) + seg;
            int next = curr + segments + 1;

            cylinderMesh.addIndex(curr);
            cylinderMesh.addIndex(next);
            cylinderMesh.addIndex(curr + 1);

            cylinderMesh.addIndex(curr + 1);
            cylinderMesh.addIndex(next);
            cylinderMesh.addIndex(next + 1);
        }
    }

    return cylinderMesh;
}

} // namespace oflike
