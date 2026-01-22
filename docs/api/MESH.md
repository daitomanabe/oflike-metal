# Mesh API Design

メッシュ関連のAPI設計です。

---

## ofMesh

### 概要
3D頂点メッシュを管理するクラス。頂点、色、法線、テクスチャ座標、インデックスを保持し、様々なプリミティブモード（点、線、三角形）で描画できる。

### 使用例（openFrameworks）

```cpp
// LINE_STRIP - 連続する線分
ofMesh lineMesh;
lineMesh.setMode(OF_PRIMITIVE_LINE_STRIP);
for (int i = 0; i < numPts; i++) {
    lineMesh.addVertex(pt);
    lineMesh.addColor(lineColor);
    lineMesh.addTexCoord(texCoord);
}
lineMesh.draw();

// LINE_LOOP - 閉じた線分
ofMesh boxMesh;
boxMesh.setMode(OF_PRIMITIVE_LINE_LOOP);
boxMesh.addVertex(glm::vec3(-bsize, -bsize, 0.f));
boxMesh.addVertex(glm::vec3(bsize, -bsize, 0.f));
boxMesh.addVertex(glm::vec3(bsize, bsize, 0.f));
boxMesh.addVertex(glm::vec3(-bsize, bsize, 0.f));
boxMesh.draw();
```

### プリミティブモード

```cpp
enum ofPrimitiveMode {
    OF_PRIMITIVE_TRIANGLES,
    OF_PRIMITIVE_TRIANGLE_STRIP,
    OF_PRIMITIVE_TRIANGLE_FAN,
    OF_PRIMITIVE_LINES,
    OF_PRIMITIVE_LINE_STRIP,
    OF_PRIMITIVE_LINE_LOOP,
    OF_PRIMITIVE_POINTS
};
```

### クラス定義

```cpp
template<class V = glm::vec3, class N = glm::vec3, class C = ofFloatColor, class T = glm::vec2>
class ofMesh_ {
public:
    // モード設定
    void setMode(ofPrimitiveMode mode);
    ofPrimitiveMode getMode() const;

    // 頂点管理
    void addVertex(const V& v);
    void addVertices(const std::vector<V>& verts);
    V& getVertex(size_t i);
    const V& getVertex(size_t i) const;
    std::vector<V>& getVertices();
    const std::vector<V>& getVertices() const;
    size_t getNumVertices() const;
    void clearVertices();

    // 色管理
    void addColor(const C& c);
    void addColors(const std::vector<C>& cols);
    C& getColor(size_t i);
    std::vector<C>& getColors();
    size_t getNumColors() const;
    void clearColors();
    void enableColors();
    void disableColors();
    bool usingColors() const;

    // インデックス管理
    void addIndex(uint32_t i);
    void addIndices(const std::vector<uint32_t>& inds);
    uint32_t getIndex(size_t i) const;
    std::vector<uint32_t>& getIndices();
    const std::vector<uint32_t>& getIndices() const;
    size_t getNumIndices() const;
    void clearIndices();
    void enableIndices();
    void disableIndices();
    bool hasIndices() const;

    // テクスチャ座標管理
    void addTexCoord(const T& t);
    void addTexCoords(const std::vector<T>& coords);
    T& getTexCoord(size_t i);
    std::vector<T>& getTexCoords();
    size_t getNumTexCoords() const;
    void clearTexCoords();
    void enableTextures();
    void disableTextures();
    bool usingTextures() const;

    // 法線管理
    void addNormal(const N& n);
    void addNormals(const std::vector<N>& norms);
    N& getNormal(size_t i);
    std::vector<N>& getNormals();
    size_t getNumNormals() const;
    void clearNormals();
    void enableNormals();
    void disableNormals();
    bool usingNormals() const;

    // 描画
    void draw() const;
    void drawWireframe() const;
    void drawVertices() const;
    void drawFaces() const;

    // クリア
    void clear();

private:
    std::vector<V> vertices_;
    std::vector<C> colors_;
    std::vector<N> normals_;
    std::vector<T> texCoords_;
    std::vector<uint32_t> indices_;

    ofPrimitiveMode mode_ = OF_PRIMITIVE_TRIANGLES;
    bool useColors_ = true;
    bool useNormals_ = true;
    bool useTextures_ = true;
    bool useIndices_ = true;
};

using ofMesh = ofMesh_<glm::vec3, glm::vec3, ofFloatColor, glm::vec2>;
```

### ファイル配置
- `src/oflike/3d/ofMesh.h`

---

## Metal統合

### プリミティブモードのマッピング

| ofPrimitiveMode | MTLPrimitiveType | 備考 |
|----------------|------------------|------|
| OF_PRIMITIVE_TRIANGLES | MTLPrimitiveTypeTriangle | |
| OF_PRIMITIVE_TRIANGLE_STRIP | MTLPrimitiveTypeTriangleStrip | |
| OF_PRIMITIVE_TRIANGLE_FAN | (エミュレート) | インデックスで実現 |
| OF_PRIMITIVE_LINES | MTLPrimitiveTypeLine | |
| OF_PRIMITIVE_LINE_STRIP | MTLPrimitiveTypeLineStrip | |
| OF_PRIMITIVE_LINE_LOOP | MTLPrimitiveTypeLineStrip | 最後に最初の頂点を追加 |
| OF_PRIMITIVE_POINTS | MTLPrimitiveTypePoint | |

---

## ofVboMesh（予定）

### 概要
GPU最適化されたメッシュ。頂点データをGPUバッファに保持し、高速描画を実現。

### クラス定義（予定）

```cpp
class ofVboMesh : public ofMesh {
public:
    void setUsage(int usage);  // GL_STATIC_DRAW等
    void update();  // GPUバッファを更新

    // Metal固有
    void* getVertexBuffer() const;  // id<MTLBuffer>
    void* getIndexBuffer() const;   // id<MTLBuffer>

private:
    void* vertexBuffer_ = nullptr;
    void* indexBuffer_ = nullptr;
    bool needsUpdate_ = true;
};
```

---

## ofMeshFace（予定）

### 概要
三角形面を表すクラス。

### クラス定義（予定）

```cpp
class ofMeshFace {
public:
    glm::vec3 getVertex(int i) const;
    glm::vec3 getNormal(int i) const;
    glm::vec2 getTexCoord(int i) const;
    ofFloatColor getColor(int i) const;

    void setVertex(int i, const glm::vec3& v);
    void setNormal(int i, const glm::vec3& n);
    void setTexCoord(int i, const glm::vec2& t);
    void setColor(int i, const ofFloatColor& c);

    glm::vec3 getFaceNormal() const;

private:
    std::array<glm::vec3, 3> vertices_;
    std::array<glm::vec3, 3> normals_;
    std::array<glm::vec2, 3> texCoords_;
    std::array<ofFloatColor, 3> colors_;
};
```

---

## 依存関係

```
ofMesh
  └── of3dPrimitive (→ GRAPHICS_3D.md)

ofVboMesh
  └── Metal Buffer管理 (→ metal/PIPELINE.md)
```
