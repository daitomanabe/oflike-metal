#include "ofPlanePrimitive.h"

namespace oflike {

ofPlanePrimitive::ofPlanePrimitive()
    : width_(200.0f), height_(200.0f), columns_(2), rows_(2) {
    generateMesh();
}

void ofPlanePrimitive::set(float width, float height, int columns, int rows) {
    width_ = width;
    height_ = height;
    columns_ = columns;
    rows_ = rows;
    generateMesh();
}

void ofPlanePrimitive::setWidth(float width) {
    width_ = width;
    generateMesh();
}

void ofPlanePrimitive::setHeight(float height) {
    height_ = height;
    generateMesh();
}

void ofPlanePrimitive::setResolution(int columns, int rows) {
    columns_ = columns;
    rows_ = rows;
    generateMesh();
}

void ofPlanePrimitive::generateMesh() {
    mesh_.clear();

    // Plane lies in XZ plane, with Y pointing up
    // Generate grid of vertices from -width/2 to +width/2 in X
    // and -height/2 to +height/2 in Z

    const float halfW = width_ * 0.5f;
    const float halfH = height_ * 0.5f;

    // Generate vertices
    for (int row = 0; row <= rows_; ++row) {
        float v = static_cast<float>(row) / rows_;
        float z = -halfH + v * height_;

        for (int col = 0; col <= columns_; ++col) {
            float u = static_cast<float>(col) / columns_;
            float x = -halfW + u * width_;

            // Position (XZ plane, Y=0)
            mesh_.addVertex(glm::vec3(x, 0.0f, z));

            // Normal (pointing up +Y)
            mesh_.addNormal(glm::vec3(0.0f, 1.0f, 0.0f));

            // UV coordinates
            mesh_.addTexCoord(glm::vec2(u, v));
        }
    }

    // Generate triangle indices
    for (int row = 0; row < rows_; ++row) {
        for (int col = 0; col < columns_; ++col) {
            int i0 = row * (columns_ + 1) + col;
            int i1 = i0 + 1;
            int i2 = (row + 1) * (columns_ + 1) + col;
            int i3 = i2 + 1;

            // First triangle (CCW from above)
            mesh_.addIndex(i0);
            mesh_.addIndex(i2);
            mesh_.addIndex(i1);

            // Second triangle (CCW from above)
            mesh_.addIndex(i1);
            mesh_.addIndex(i2);
            mesh_.addIndex(i3);
        }
    }

    mesh_.setMode(ofPrimitiveMode::OF_PRIMITIVE_TRIANGLES);
}

} // namespace oflike
