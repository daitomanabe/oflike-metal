# 3D Graphics API Design

3Dプリミティブと描画関数の設計です。

---

## of3dPrimitive（基底クラス）

### 概要
3Dプリミティブの基底クラス。メッシュ生成と描画を管理。

### クラス定義

```cpp
class of3dPrimitive : public ofNode {
public:
    of3dPrimitive();
    virtual ~of3dPrimitive();

    // メッシュ取得
    ofMesh& getMesh();
    const ofMesh& getMesh() const;

    // 描画
    void draw() const;
    void drawWireframe() const;
    void drawAxes(float size) const;
    void drawNormals(float length, bool split = false) const;

    // テクスチャ座標
    void mapTexCoords(float u1, float v1, float u2, float v2);
    void mapTexCoordsFromTexture(const ofTexture& tex);
    void resizeToTexture(const ofTexture& tex, float scale = 1.0f);

    // モード
    void setMode(ofPrimitiveMode mode);
    ofPrimitiveMode getMode() const;

protected:
    ofMesh mesh_;
};
```

### ファイル配置
- `src/oflike/3d/of3dPrimitive.h`
- `src/oflike/3d/of3dPrimitive.cpp`

---

## ofBoxPrimitive

### 概要
直方体プリミティブ。

### 使用例（openFrameworks）

```cpp
ofBoxPrimitive box;
box.set(100, 150, 200);  // width, height, depth
box.setPosition(0, 0, 0);
box.draw();
box.drawWireframe();
```

### クラス定義

```cpp
class ofBoxPrimitive : public of3dPrimitive {
public:
    enum Side { SIDE_FRONT, SIDE_RIGHT, SIDE_LEFT, SIDE_BACK, SIDE_TOP, SIDE_BOTTOM, SIDES_TOTAL };

    void set(float size);
    void set(float width, float height, float depth);
    void setWidth(float w);
    void setHeight(float h);
    void setDepth(float d);

    float getWidth() const;
    float getHeight() const;
    float getDepth() const;

    void setResolution(int res);
    void setResolutionWidth(int res);
    void setResolutionHeight(int res);
    void setResolutionDepth(int res);
    int getResolutionWidth() const;
    int getResolutionHeight() const;
    int getResolutionDepth() const;

    ofMesh getSideMesh(int side) const;
};
```

### ファイル配置
- `src/oflike/3d/ofBoxPrimitive.h`

---

## ofSpherePrimitive

### 概要
球体プリミティブ。

### クラス定義

```cpp
class ofSpherePrimitive : public of3dPrimitive {
public:
    void set(float radius, int resolution);
    void setRadius(float radius);
    float getRadius() const;
    void setResolution(int resolution);
    int getResolution() const;
};
```

### ファイル配置
- `src/oflike/3d/ofSpherePrimitive.h`

---

## ofCylinderPrimitive

### 概要
円柱プリミティブ。

### クラス定義

```cpp
class ofCylinderPrimitive : public of3dPrimitive {
public:
    void set(float radius, float height, int radiusRes = 12, int heightRes = 6, int capRes = 2, bool capped = true);
    void setRadius(float radius);
    void setHeight(float height);
    float getRadius() const;
    float getHeight() const;

    void setResolutionRadius(int res);
    void setResolutionHeight(int res);
    void setResolutionCap(int res);
    int getResolutionRadius() const;
    int getResolutionHeight() const;
    int getResolutionCap() const;

    ofMesh getTopCapMesh() const;
    ofMesh getBottomCapMesh() const;
    ofMesh getCylinderMesh() const;
};
```

---

## ofConePrimitive

### 概要
円錐プリミティブ。

### クラス定義

```cpp
class ofConePrimitive : public of3dPrimitive {
public:
    void set(float radius, float height, int radiusRes = 12, int heightRes = 6, int capRes = 2);
    void setRadius(float radius);
    void setHeight(float height);
    float getRadius() const;
    float getHeight() const;

    ofMesh getCapMesh() const;
    ofMesh getConeMesh() const;
};
```

---

## ofPlanePrimitive

### 概要
平面プリミティブ。

### クラス定義

```cpp
class ofPlanePrimitive : public of3dPrimitive {
public:
    void set(float width, float height, int columns = 2, int rows = 2);
    void setWidth(float width);
    void setHeight(float height);
    float getWidth() const;
    float getHeight() const;

    void setResolution(int columns, int rows);
    int getNumColumns() const;
    int getNumRows() const;
    glm::vec2 getResolution() const;
};
```

---

## ofIcoSpherePrimitive

### 概要
正二十面体ベースの球体プリミティブ。均一な面分割が特徴。

### クラス定義

```cpp
class ofIcoSpherePrimitive : public of3dPrimitive {
public:
    void set(float radius, int iterations);
    void setRadius(float radius);
    float getRadius() const;
    void setResolution(int iterations);  // subdivision回数
    int getResolution() const;
};
```

---

## 3D描画関数

### 球体

```cpp
void ofDrawSphere(float x, float y, float z, float radius);
void ofDrawSphere(const glm::vec3& position, float radius);
void ofDrawSphere(float radius);  // 原点に描画
```

### 直方体

```cpp
void ofDrawBox(float x, float y, float z, float size);
void ofDrawBox(float x, float y, float z, float width, float height, float depth);
void ofDrawBox(const glm::vec3& position, float size);
void ofDrawBox(float size);  // 原点に描画
```

### 円錐

```cpp
void ofDrawCone(float x, float y, float z, float radius, float height);
void ofDrawCone(const glm::vec3& position, float radius, float height);
```

### 円柱

```cpp
void ofDrawCylinder(float x, float y, float z, float radius, float height);
void ofDrawCylinder(const glm::vec3& position, float radius, float height);
```

### 平面

```cpp
void ofDrawPlane(float x, float y, float z, float width, float height);
```

### グリッド・軸

```cpp
void ofDrawGrid(float stepSize = 1.0f, size_t numberOfSteps = 8,
                bool labels = false, bool x = true, bool y = true, bool z = true);
void ofDrawGridPlane(float stepSize, size_t numberOfSteps = 8, bool labels = false);

void ofDrawAxis(float size);
void ofDrawRotationAxes(float radius, float stripWidth = 10, int circleRes = 60);
```

### ファイル配置
- `src/oflike/graphics/ofGraphics.h` に追加

---

## 依存関係

```
of3dPrimitive
  ├── ofBoxPrimitive
  ├── ofSpherePrimitive
  ├── ofCylinderPrimitive
  ├── ofConePrimitive
  ├── ofPlanePrimitive
  └── ofIcoSpherePrimitive

依存:
  ofNode (→ CORE.md)
  ofMesh (→ MESH.md)
```
