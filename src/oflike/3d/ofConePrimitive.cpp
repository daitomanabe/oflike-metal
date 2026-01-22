#include "ofConePrimitive.h"
#include <cmath>

namespace oflike {

ofConePrimitive::ofConePrimitive() {
    set(50.0f, 100.0f, 12, 6, 2);
}

void ofConePrimitive::set(float radius, float height, int radiusRes, int heightRes, int capRes) {
    radius_ = radius;
    height_ = height;
    radiusRes_ = radiusRes;
    heightRes_ = heightRes;
    capRes_ = capRes;
    regenerateMesh();
}

void ofConePrimitive::setRadius(float radius) {
    radius_ = radius;
    regenerateMesh();
}

void ofConePrimitive::setHeight(float height) {
    height_ = height;
    regenerateMesh();
}

void ofConePrimitive::setResolutionRadius(int radiusRes) {
    radiusRes_ = radiusRes;
    regenerateMesh();
}

void ofConePrimitive::setResolutionHeight(int heightRes) {
    heightRes_ = heightRes;
    regenerateMesh();
}

void ofConePrimitive::setResolutionCap(int capRes) {
    capRes_ = capRes;
    regenerateMesh();
}

void ofConePrimitive::regenerateMesh() {
    mesh_.clear();

    const int segments = radiusRes_;
    const int rings = heightRes_;
    const float pi = 3.14159265359f;

    // Compute normal tilt angle for cone surface
    // Normal is perpendicular to cone surface (tilted outward)
    const float slant = std::sqrt(radius_ * radius_ + height_ * height_);
    const glm::vec3 normalTilt(height_ / slant, radius_ / slant, 0.0f);

    // --- Cone body vertices ---
    for (int ring = 0; ring <= rings; ++ring) {
        float t = static_cast<float>(ring) / rings;  // 0 (base) to 1 (apex)
        float y = -height_ * 0.5f + height_ * t;      // Y from bottom to top
        float r = radius_ * (1.0f - t);               // Radius tapers to 0

        for (int seg = 0; seg < segments; ++seg) {
            float theta = 2.0f * pi * seg / segments;
            float x = r * std::cos(theta);
            float z = r * std::sin(theta);

            mesh_.addVertex(glm::vec3(x, y, z));

            // Normal: rotate normalTilt around Y axis by theta
            float nx = normalTilt.x * std::cos(theta);
            float ny = normalTilt.y;
            float nz = normalTilt.x * std::sin(theta);
            mesh_.addNormal(glm::vec3(nx, ny, nz));

            // UV: cylindrical
            float u = static_cast<float>(seg) / segments;
            float v = t;
            mesh_.addTexCoord(glm::vec2(u, v));
        }
    }

    // --- Cone body indices ---
    for (int ring = 0; ring < rings; ++ring) {
        for (int seg = 0; seg < segments; ++seg) {
            int current = ring * segments + seg;
            int next = ring * segments + (seg + 1) % segments;
            int currentNext = (ring + 1) * segments + seg;
            int nextNext = (ring + 1) * segments + (seg + 1) % segments;

            // Two triangles per quad (unless apex ring degenerates)
            if (ring < rings - 1) {
                mesh_.addIndex(current);
                mesh_.addIndex(next);
                mesh_.addIndex(currentNext);

                mesh_.addIndex(next);
                mesh_.addIndex(nextNext);
                mesh_.addIndex(currentNext);
            } else {
                // Last ring forms single apex point - single triangle
                mesh_.addIndex(current);
                mesh_.addIndex(next);
                mesh_.addIndex(currentNext);
            }
        }
    }

    // --- Bottom cap (optional, controlled by capRes) ---
    if (capRes_ > 0) {
        const int capBaseVertex = mesh_.getNumVertices();
        const float capY = -height_ * 0.5f;

        // Center vertex
        mesh_.addVertex(glm::vec3(0.0f, capY, 0.0f));
        mesh_.addNormal(glm::vec3(0.0f, -1.0f, 0.0f));  // Down
        mesh_.addTexCoord(glm::vec2(0.5f, 0.5f));

        // Radial rings
        for (int capRing = 1; capRing <= capRes_; ++capRing) {
            float t = static_cast<float>(capRing) / capRes_;
            float r = radius_ * t;

            for (int seg = 0; seg < segments; ++seg) {
                float theta = 2.0f * pi * seg / segments;
                float x = r * std::cos(theta);
                float z = r * std::sin(theta);

                mesh_.addVertex(glm::vec3(x, capY, z));
                mesh_.addNormal(glm::vec3(0.0f, -1.0f, 0.0f));

                // UV: radial
                float u = 0.5f + 0.5f * (x / radius_);
                float v = 0.5f + 0.5f * (z / radius_);
                mesh_.addTexCoord(glm::vec2(u, v));
            }
        }

        // Cap indices
        // Connect center to first ring
        for (int seg = 0; seg < segments; ++seg) {
            int center = capBaseVertex;
            int current = capBaseVertex + 1 + seg;
            int next = capBaseVertex + 1 + (seg + 1) % segments;

            mesh_.addIndex(center);
            mesh_.addIndex(next);  // Reversed winding for -Y normal
            mesh_.addIndex(current);
        }

        // Connect rings
        for (int capRing = 1; capRing < capRes_; ++capRing) {
            int ringStart = capBaseVertex + 1 + (capRing - 1) * segments;
            int nextRingStart = capBaseVertex + 1 + capRing * segments;

            for (int seg = 0; seg < segments; ++seg) {
                int current = ringStart + seg;
                int next = ringStart + (seg + 1) % segments;
                int currentNext = nextRingStart + seg;
                int nextNext = nextRingStart + (seg + 1) % segments;

                mesh_.addIndex(current);
                mesh_.addIndex(nextNext);  // Reversed winding
                mesh_.addIndex(next);

                mesh_.addIndex(current);
                mesh_.addIndex(currentNext);
                mesh_.addIndex(nextNext);
            }
        }
    }
}

ofMesh ofConePrimitive::getCapMesh() const {
    // Extract bottom cap vertices (if capRes > 0)
    ofMesh capMesh;
    if (capRes_ <= 0) {
        return capMesh;
    }

    const int coneBodyVertices = (heightRes_ + 1) * radiusRes_;
    const int capVertices = 1 + capRes_ * radiusRes_;
    const int capBaseVertex = coneBodyVertices;

    for (int i = 0; i < capVertices; ++i) {
        int vertexIndex = capBaseVertex + i;
        if (vertexIndex < mesh_.getNumVertices()) {
            capMesh.addVertex(mesh_.getVertex(vertexIndex));
            capMesh.addNormal(mesh_.getNormal(vertexIndex));
            capMesh.addTexCoord(mesh_.getTexCoord(vertexIndex));
        }
    }

    // Extract cap indices (remap to local indexing)
    const int coneBodyIndices = (heightRes_ - 1) * radiusRes_ * 6 + radiusRes_ * 3;  // Approximate
    const auto& indices = mesh_.getIndices();
    for (size_t i = coneBodyIndices; i < indices.size(); ++i) {
        int idx = indices[i];
        if (idx >= capBaseVertex) {
            capMesh.addIndex(idx - capBaseVertex);
        }
    }

    return capMesh;
}

ofMesh ofConePrimitive::getConeMesh() const {
    // Extract cone body vertices (exclude cap)
    ofMesh coneMesh;

    const int coneBodyVertices = (heightRes_ + 1) * radiusRes_;
    for (int i = 0; i < coneBodyVertices && i < mesh_.getNumVertices(); ++i) {
        coneMesh.addVertex(mesh_.getVertex(i));
        coneMesh.addNormal(mesh_.getNormal(i));
        coneMesh.addTexCoord(mesh_.getTexCoord(i));
    }

    // Extract cone body indices (first part of index buffer)
    const int coneBodyIndices = (heightRes_ - 1) * radiusRes_ * 6 + radiusRes_ * 3;
    const auto& indices = mesh_.getIndices();
    for (int i = 0; i < coneBodyIndices && i < static_cast<int>(indices.size()); ++i) {
        coneMesh.addIndex(indices[i]);
    }

    return coneMesh;
}

} // namespace oflike
