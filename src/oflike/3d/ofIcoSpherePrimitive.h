#pragma once

#include "of3dPrimitive.h"
#include <map>

namespace oflike {

// 正二十面体ベースの球体プリミティブ
// 均一な三角形分布を持つ球体を生成
class ofIcoSpherePrimitive : public of3dPrimitive {
public:
    ofIcoSpherePrimitive();

    // 半径と分割回数を設定
    void set(float radius, int iterations);

    // 半径の設定
    void setRadius(float radius);
    float getRadius() const { return radius_; }

    // 解像度（分割回数）の設定
    void setResolution(int iterations);
    int getResolution() const { return iterations_; }

private:
    void generateMesh();
    void createIcosahedron();
    void subdivide();
    int getMiddlePoint(int p1, int p2);

    float radius_;
    int iterations_;

    // Temporary data for subdivision
    std::vector<glm::vec3> vertices_;
    std::vector<int> indices_;
    std::map<long long, int> middlePointCache_;
    int vertexIndex_;
};

} // namespace oflike
