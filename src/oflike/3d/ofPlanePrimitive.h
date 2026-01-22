#pragma once

#include "of3dPrimitive.h"

namespace oflike {

// 平面プリミティブ
class ofPlanePrimitive : public of3dPrimitive {
public:
    ofPlanePrimitive();

    // 平面のサイズと解像度を設定
    void set(float width, float height, int columns = 2, int rows = 2);

    // 幅と高さの設定
    void setWidth(float width);
    void setHeight(float height);
    float getWidth() const { return width_; }
    float getHeight() const { return height_; }

    // 解像度（分割数）の設定
    void setResolution(int columns, int rows);
    int getNumColumns() const { return columns_; }
    int getNumRows() const { return rows_; }
    glm::vec2 getResolution() const { return glm::vec2(columns_, rows_); }

private:
    void generateMesh();

    float width_;
    float height_;
    int columns_;
    int rows_;
};

} // namespace oflike
