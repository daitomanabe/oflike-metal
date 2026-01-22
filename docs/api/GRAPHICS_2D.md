# 2D Graphics API Design

2Dグラフィックス関連のAPI設計です。

---

## ofPath

### 概要
ベクターグラフィックスのパスを構築するクラス。moveTo/lineTo/curveTo等のコマンドでパスを定義し、フィルとストロークで描画できる。

### 使用例（openFrameworks）

```cpp
// 角丸矩形
ofPath testRectPath;
testRectPath.setCurveResolution(16);
testRectPath.setCircleResolution(120);
testRectPath.rectRounded(outlineRect, rectRadius);
testRectPath.setFilled(false);
testRectPath.setFillColor(ofColor::lightBlue);
testRectPath.setStrokeWidth(2.0);
testRectPath.setStrokeColor(ofColor(60));
testRectPath.draw();

// フォントから取得したパス
std::vector<ofPath> textPaths = font.getStringAsPoints("openFrameworks", false, false);
for (auto& fp : textPaths) {
    fp.setFilled(true);
    fp.setFillColor(floatColor);
    fp.draw();
}
```

### クラス定義

```cpp
class ofPath {
public:
    // パス構築
    void moveTo(const glm::vec3& p);
    void moveTo(float x, float y, float z = 0);
    void lineTo(const glm::vec3& p);
    void lineTo(float x, float y, float z = 0);
    void curveTo(const glm::vec3& p);
    void curveTo(float x, float y, float z = 0);
    void bezierTo(const glm::vec3& cp1, const glm::vec3& cp2, const glm::vec3& to);
    void bezierTo(float cx1, float cy1, float cx2, float cy2, float x, float y);
    void quadBezierTo(const glm::vec3& cp1, const glm::vec3& cp2, const glm::vec3& to);
    void arc(const glm::vec3& center, float radiusX, float radiusY, float angleBegin, float angleEnd);
    void close();
    void clear();

    // 便利メソッド
    void rectangle(float x, float y, float w, float h);
    void rectangle(const ofRectangle& r);
    void rectRounded(float x, float y, float w, float h, float r);
    void rectRounded(const ofRectangle& r, float r);
    void rectRounded(float x, float y, float w, float h, float tl, float tr, float br, float bl);
    void circle(float x, float y, float radius);
    void ellipse(float x, float y, float width, float height);
    void triangle(float x1, float y1, float x2, float y2, float x3, float y3);

    // フィル・ストローク設定
    void setFilled(bool hasFill);
    bool isFilled() const;
    void setStrokeWidth(float width);
    float getStrokeWidth() const;
    bool hasOutline() const;

    // 色設定
    void setColor(const ofFloatColor& color);
    void setHexColor(int hex);
    void setFillColor(const ofFloatColor& color);
    void setFillHexColor(int hex);
    void setStrokeColor(const ofFloatColor& color);
    void setStrokeHexColor(int hex);
    ofFloatColor getFillColor() const;
    ofFloatColor getStrokeColor() const;
    void setUseShapeColor(bool useColor);
    bool getUseShapeColor() const;

    // 解像度設定
    void setCurveResolution(int res);
    int getCurveResolution() const;
    void setCircleResolution(int res);
    int getCircleResolution() const;

    // 描画
    void draw() const;
    void draw(float x, float y) const;

    // ポリライン取得
    const std::vector<ofPolyline>& getOutline() const;
    std::vector<ofPolyline>& getOutline();

private:
    struct Command {
        enum Type { moveTo, lineTo, curveTo, bezierTo, quadBezierTo, arc, arcNegative, close };
        Type type;
        glm::vec3 to;
        glm::vec3 cp1, cp2;
        float radiusX, radiusY;
        float angleBegin, angleEnd;
    };

    std::vector<Command> commands;
    std::vector<ofPolyline> polylines;

    ofFloatColor fillColor_{1, 1, 1, 1};
    ofFloatColor strokeColor_{0, 0, 0, 1};
    float strokeWidth_ = 0;
    bool bFill_ = true;

    int curveResolution_ = 20;
    int circleResolution_ = 20;
};
```

### ファイル配置
- `src/oflike/graphics/ofPath.h`

---

## ofPolyline

### 概要
頂点の配列で構成される線分。ofPathの内部表現として使用。

### クラス定義

```cpp
class ofPolyline {
public:
    void addVertex(const glm::vec3& p);
    void addVertex(float x, float y, float z = 0);
    void lineTo(const glm::vec3& p);
    void lineTo(float x, float y, float z = 0);
    void curveTo(const glm::vec3& p);
    void bezierTo(const glm::vec3& cp1, const glm::vec3& cp2, const glm::vec3& to);
    void arc(const glm::vec3& center, float radiusX, float radiusY, float angleBegin, float angleEnd);
    void close();
    void clear();

    void setClosed(bool closed);
    bool isClosed() const;

    const std::vector<glm::vec3>& getVertices() const;
    std::vector<glm::vec3>& getVertices();
    size_t size() const;

    ofRectangle getBoundingBox() const;
    glm::vec3 getCentroid2D() const;
    float getPerimeter() const;
    float getArea() const;

    bool inside(float x, float y) const;
    bool inside(const glm::vec3& p) const;

    void draw() const;

private:
    std::vector<glm::vec3> points_;
    bool bClosed_ = false;
    int curveResolution_ = 20;
};
```

### ファイル配置
- `src/oflike/types/ofPolyline.h`

---

## 描画関数（ofGraphics.h）

### 基本形状

```cpp
void ofDrawRectangle(float x, float y, float w, float h);
void ofDrawRectangle(const ofRectangle& r);
void ofDrawRectRounded(float x, float y, float w, float h, float r);

void ofDrawCircle(float x, float y, float radius);
void ofDrawCircle(float x, float y, float z, float radius);
void ofDrawEllipse(float x, float y, float width, float height);

void ofDrawTriangle(float x1, float y1, float x2, float y2, float x3, float y3);

void ofDrawLine(float x1, float y1, float x2, float y2);
void ofDrawLine(float x1, float y1, float z1, float x2, float y2, float z2);
```

### 背景

```cpp
void ofBackground(int gray);
void ofBackground(int r, int g, int b, int a = 255);
void ofBackground(const ofColor& c);

void ofBackgroundGradient(const ofColor& start, const ofColor& end,
                          ofGradientMode mode = OF_GRADIENT_CIRCULAR);

enum ofGradientMode {
    OF_GRADIENT_LINEAR,
    OF_GRADIENT_CIRCULAR,
    OF_GRADIENT_BAR
};
```

### 色設定

```cpp
void ofSetColor(int gray);
void ofSetColor(int r, int g, int b, int a = 255);
void ofSetColor(const ofColor& c);
void ofSetHexColor(int hex);
```

### 変換

```cpp
void ofPushMatrix();
void ofPopMatrix();

void ofTranslate(float x, float y, float z = 0);
void ofRotateDeg(float degrees);
void ofRotateRad(float radians);
void ofRotateXRad(float radians);
void ofRotateYRad(float radians);
void ofRotateZRad(float radians);
void ofScale(float s);
void ofScale(float x, float y, float z = 1);
```

### 深度テスト

```cpp
void ofEnableDepthTest();
void ofDisableDepthTest();
bool ofGetDepthTestEnabled();
```

### 塗りつぶしモード

```cpp
void ofFill();
void ofNoFill();
bool ofGetFill();
void ofSetLineWidth(float width);
```

### ファイル配置
- `src/oflike/graphics/ofGraphics.h`

---

## ofRectangle

### 概要
矩形を表すユーティリティクラス。

### クラス定義

```cpp
class ofRectangle {
public:
    float x = 0, y = 0;
    float width = 0, height = 0;

    ofRectangle() = default;
    ofRectangle(float x, float y, float w, float h);

    float getX() const { return x; }
    float getY() const { return y; }
    float getWidth() const { return width; }
    float getHeight() const { return height; }
    float getLeft() const { return x; }
    float getRight() const { return x + width; }
    float getTop() const { return y; }
    float getBottom() const { return y + height; }
    glm::vec2 getCenter() const;

    void set(float x, float y, float w, float h);
    void setFromCenter(float cx, float cy, float w, float h);

    bool inside(float px, float py) const;
    bool intersects(const ofRectangle& other) const;
};
```

### ファイル配置
- `src/oflike/types/ofRectangle.h`

---

## 依存関係

```
ofPolyline ← ofPath
ofRectangle ← ofPath, ofTrueTypeFont
ofGraphics.h ← DrawList, Engine
```
