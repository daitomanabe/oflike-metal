# Core 3D API Design

3D空間の基盤となるクラス設計です。

---

## ofNode

### 概要
3D変換を持つノード。位置、回転、スケールを管理し、階層構造をサポート。

### クラス定義

```cpp
class ofNode {
public:
    ofNode();
    virtual ~ofNode();

    // 位置
    void setPosition(float x, float y, float z);
    void setPosition(const glm::vec3& p);
    void setGlobalPosition(const glm::vec3& p);
    glm::vec3 getPosition() const;
    glm::vec3 getGlobalPosition() const;
    float getX() const;
    float getY() const;
    float getZ() const;

    // 回転
    void setOrientation(const glm::quat& q);
    void setOrientation(const glm::vec3& eulerAngles);
    glm::quat getOrientationQuat() const;
    glm::vec3 getOrientationEuler() const;

    // 回転操作
    void rotateDeg(float degrees, const glm::vec3& axis);
    void rotateDeg(float degrees, float axisX, float axisY, float axisZ);
    void rotateRad(float radians, const glm::vec3& axis);
    void tiltDeg(float degrees);   // X軸回転
    void panDeg(float degrees);    // Y軸回転
    void rollDeg(float degrees);   // Z軸回転

    // スケール
    void setScale(float s);
    void setScale(float x, float y, float z);
    void setScale(const glm::vec3& s);
    glm::vec3 getScale() const;

    // 方向ベクトル
    glm::vec3 getXAxis() const;
    glm::vec3 getYAxis() const;
    glm::vec3 getZAxis() const;
    glm::vec3 getLookAtDir() const;

    // 注視
    void lookAt(const glm::vec3& target);
    void lookAt(const glm::vec3& target, const glm::vec3& up);
    void lookAt(const ofNode& target);

    // 行列
    glm::mat4 getLocalTransformMatrix() const;
    glm::mat4 getGlobalTransformMatrix() const;

    // 階層
    void setParent(ofNode& parent, bool keepGlobalTransform = false);
    void clearParent(bool keepGlobalTransform = false);
    ofNode* getParent() const;

    // 変換適用
    void transformGL() const;
    void restoreTransformGL() const;

    // 描画
    void draw();
    virtual void customDraw();

protected:
    glm::vec3 position_{0, 0, 0};
    glm::quat orientation_{1, 0, 0, 0};
    glm::vec3 scale_{1, 1, 1};
    ofNode* parent_ = nullptr;
};
```

### ファイル配置
- `src/oflike/3d/ofNode.h`
- `src/oflike/3d/ofNode.cpp`

---

## ofCamera

### 概要
3Dカメラを管理するクラス。View行列とProjection行列を計算し、3Dシーンをレンダリングする。

### 使用例（openFrameworks）

```cpp
ofCamera cam;

// カメラ設定
cam.setGlobalPosition({0, 0, cam.getImagePlaneDistance(ofGetCurrentViewport())});

// レンダリング
cam.begin();
  ofEnableDepthTest();
  // 3D描画
  ofDisableDepthTest();
cam.end();

// 座標変換
glm::vec3 screenPos = cam.worldToScreen(worldPos);
```

### クラス定義

```cpp
class ofCamera : public ofNode {
public:
    ofCamera();
    virtual ~ofCamera();

    // 投影設定
    void setFov(float fov);
    float getFov() const;
    void setNearClip(float near);
    void setFarClip(float far);
    float getNearClip() const;
    float getFarClip() const;
    void setAspectRatio(float aspectRatio);

    // 正投影
    void enableOrtho();
    void disableOrtho();
    bool isOrtho() const;

    // ターゲット
    void setTarget(const glm::vec3& target);
    void setTarget(ofNode& target);
    glm::vec3 getTarget() const;
    float getDistance() const;
    void setDistance(float distance);

    // 行列取得
    glm::mat4 getProjectionMatrix() const;
    glm::mat4 getProjectionMatrix(const ofRectangle& viewport) const;
    glm::mat4 getModelViewMatrix() const;
    glm::mat4 getModelViewProjectionMatrix() const;
    glm::mat4 getModelViewProjectionMatrix(const ofRectangle& viewport) const;

    // 座標変換
    glm::vec3 worldToScreen(const glm::vec3& worldPoint) const;
    glm::vec3 worldToScreen(const glm::vec3& worldPoint, const ofRectangle& viewport) const;
    glm::vec3 screenToWorld(const glm::vec3& screenPoint) const;
    glm::vec3 screenToWorld(const glm::vec3& screenPoint, const ofRectangle& viewport) const;

    // レンダリング
    void begin();
    void begin(const ofRectangle& viewport);
    void end();

    // ユーティリティ
    float getImagePlaneDistance(const ofRectangle& viewport) const;
    void drawFrustum(const ofRectangle& viewport) const;

protected:
    float fov_ = 60.0f;
    float nearClip_ = 0.1f;
    float farClip_ = 10000.0f;
    float aspectRatio_ = 4.0f / 3.0f;
    bool isOrtho_ = false;
    glm::vec3 target_;
    bool hasTarget_ = false;
};
```

### ファイル配置
- `src/oflike/3d/ofCamera.h`
- `src/oflike/3d/ofCamera.cpp`

---

## ofEasyCam

### 概要
マウス操作可能なカメラ。ドラッグで回転、スクロールでズーム。

### 使用例（openFrameworks）

```cpp
ofEasyCam camEasyCam;

// 設定
camEasyCam.setTarget(targetNode);
camEasyCam.setDistance(100);
camEasyCam.setNearClip(10);
camEasyCam.setFarClip(10000);

// ビューポート指定でレンダリング
cam.begin(viewport);
  drawScene();
cam.end();

// ノード階層
cam.setParent(parentNode);
cam.clearParent();
```

### クラス定義

```cpp
class ofEasyCam : public ofCamera {
public:
    ofEasyCam();

    // マウス操作有効/無効
    void enableMouseInput();
    void disableMouseInput();
    bool isMouseInputEnabled() const;

    // 感度設定
    void setDrag(float drag);
    void setRotationSensitivity(float x, float y, float z);
    void setTranslationSensitivity(float x, float y, float z);

    // マウスイベント処理（内部）
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseScrolled(float x, float y);

protected:
    bool bMouseInputEnabled_ = true;
    float drag_ = 0.9f;
    glm::vec3 lastMouse_;
};
```

### ファイル配置
- `src/oflike/3d/ofEasyCam.h`
- `src/oflike/3d/ofEasyCam.cpp`

---

## 依存関係

```
ofNode
  ├── ofCamera
  │     └── ofEasyCam
  ├── of3dPrimitive (→ GRAPHICS_3D.md)
  └── ofLight (→ LIGHTING.md)
```
