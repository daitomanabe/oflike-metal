# Math API Design

数学関連のAPI設計です。

**重要**: GLMを使用し、openFrameworksのof*型をエイリアスとして提供します。

---

## ofVectorMath（実装済み）

### 概要
GLMの型をopenFrameworks互換のエイリアスとして提供。

### 型エイリアス

```cpp
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

// ベクトル型
using ofVec2f = glm::vec2;
using ofVec3f = glm::vec3;
using ofVec4f = glm::vec4;

// 行列型
using ofMatrix3x3 = glm::mat3;
using ofMatrix4x4 = glm::mat4;

// クォータニオン
using ofQuaternion = glm::quat;

// ポイント（ofVec3fのエイリアス）
using ofPoint = glm::vec3;
```

### ファイル配置
- `src/oflike/math/ofVectorMath.h`

---

## ofVec2f / ofVec3f / ofVec4f

### 概要
2D/3D/4Dベクトル。GLMのglm::vec2/vec3/vec4のエイリアス。

### 使用例（openFrameworks）

```cpp
ofVec3f v1(1, 2, 3);
ofVec3f v2(4, 5, 6);

// 演算
ofVec3f sum = v1 + v2;
ofVec3f diff = v1 - v2;
ofVec3f scaled = v1 * 2.0f;
float dot = glm::dot(v1, v2);
ofVec3f cross = glm::cross(v1, v2);

// 正規化
ofVec3f normalized = glm::normalize(v1);
float len = glm::length(v1);

// 距離
float dist = glm::distance(v1, v2);

// 補間
ofVec3f lerped = glm::mix(v1, v2, 0.5f);
```

### 主要関数（GLM）

```cpp
// 長さ
float glm::length(const vec& v);
float glm::length2(const vec& v);  // 長さの2乗

// 正規化
vec glm::normalize(const vec& v);

// 内積・外積
float glm::dot(const vec& a, const vec& b);
vec3 glm::cross(const vec3& a, const vec3& b);

// 距離
float glm::distance(const vec& a, const vec& b);
float glm::distance2(const vec& a, const vec& b);

// 補間
vec glm::mix(const vec& a, const vec& b, float t);
vec glm::lerp(const vec& a, const vec& b, float t);  // mixと同等

// 反射・屈折
vec glm::reflect(const vec& I, const vec& N);
vec glm::refract(const vec& I, const vec& N, float eta);

// クランプ
vec glm::clamp(const vec& v, float minVal, float maxVal);
vec glm::clamp(const vec& v, const vec& minVal, const vec& maxVal);

// 最小・最大
vec glm::min(const vec& a, const vec& b);
vec glm::max(const vec& a, const vec& b);

// 絶対値
vec glm::abs(const vec& v);

// 床・天井・丸め
vec glm::floor(const vec& v);
vec glm::ceil(const vec& v);
vec glm::round(const vec& v);
```

---

## ofMatrix3x3

### 概要
3x3行列。2D変換や法線変換に使用。

### クラス定義（予定）

```cpp
class ofMatrix3x3 {
public:
    // GLMのglm::mat3をラップ

    // コンストラクタ
    ofMatrix3x3();  // 単位行列
    ofMatrix3x3(float a00, float a01, float a02,
                float a10, float a11, float a12,
                float a20, float a21, float a22);

    // 演算
    ofMatrix3x3 operator*(const ofMatrix3x3& other) const;
    ofVec3f operator*(const ofVec3f& v) const;

    // 変換
    void rotate(float degrees);
    void scale(float x, float y);
    void translate(float x, float y);

    // 逆行列・転置
    ofMatrix3x3 getInverse() const;
    ofMatrix3x3 getTransposed() const;

    // 行列式
    float determinant() const;

    // データアクセス
    float* getPtr();
    const float* getPtr() const;

private:
    glm::mat3 mat_;
};
```

### ファイル配置（予定）
- `src/oflike/math/ofMatrix3x3.h`
- `src/oflike/math/ofMatrix3x3.cpp`

---

## ofMatrix4x4

### 概要
4x4行列。3D変換（モデル、ビュー、プロジェクション）に使用。

### 使用例（openFrameworks）

```cpp
ofMatrix4x4 mat;

// 変換
mat.translate(100, 200, 0);
mat.rotate(45, 0, 1, 0);
mat.scale(2, 2, 2);

// 行列の合成
ofMatrix4x4 combined = mat1 * mat2;

// ベクトル変換
ofVec3f transformed = mat * ofVec4f(v, 1.0f);

// 逆行列
ofMatrix4x4 inv = mat.getInverse();
```

### クラス定義（予定）

```cpp
class ofMatrix4x4 {
public:
    // コンストラクタ
    ofMatrix4x4();  // 単位行列
    ofMatrix4x4(const glm::mat4& m);
    ofMatrix4x4(float a00, float a01, float a02, float a03,
                float a10, float a11, float a12, float a13,
                float a20, float a21, float a22, float a23,
                float a30, float a31, float a32, float a33);

    // 行列生成（静的）
    static ofMatrix4x4 newIdentityMatrix();
    static ofMatrix4x4 newTranslationMatrix(const ofVec3f& v);
    static ofMatrix4x4 newTranslationMatrix(float x, float y, float z);
    static ofMatrix4x4 newScaleMatrix(const ofVec3f& v);
    static ofMatrix4x4 newScaleMatrix(float x, float y, float z);
    static ofMatrix4x4 newRotationMatrix(float angle, float x, float y, float z);
    static ofMatrix4x4 newRotationMatrix(const ofQuaternion& q);
    static ofMatrix4x4 newLookAtMatrix(const ofVec3f& eye, const ofVec3f& target, const ofVec3f& up);
    static ofMatrix4x4 newPerspectiveMatrix(float fov, float aspect, float near, float far);
    static ofMatrix4x4 newOrthoMatrix(float left, float right, float bottom, float top, float near, float far);

    // 変換操作
    void translate(const ofVec3f& v);
    void translate(float x, float y, float z);
    void rotate(float degrees, float x, float y, float z);
    void rotateRad(float radians, float x, float y, float z);
    void scale(const ofVec3f& v);
    void scale(float x, float y, float z);
    void scale(float s);

    // 逆行列・転置
    ofMatrix4x4 getInverse() const;
    ofMatrix4x4 getTransposed() const;
    bool isValid() const;
    bool isIdentity() const;

    // 行列式
    float determinant() const;

    // 分解
    ofVec3f getTranslation() const;
    ofQuaternion getRotate() const;
    ofVec3f getScale() const;
    void decompose(ofVec3f& translation, ofQuaternion& rotation, ofVec3f& scale) const;

    // 演算
    ofMatrix4x4 operator*(const ofMatrix4x4& other) const;
    ofMatrix4x4& operator*=(const ofMatrix4x4& other);
    ofVec4f operator*(const ofVec4f& v) const;
    ofVec3f preMult(const ofVec3f& v) const;
    ofVec3f postMult(const ofVec3f& v) const;

    // 比較
    bool operator==(const ofMatrix4x4& other) const;
    bool operator!=(const ofMatrix4x4& other) const;

    // データアクセス
    float& operator()(int row, int col);
    float operator()(int row, int col) const;
    float* getPtr();
    const float* getPtr() const;
    glm::mat4& getGlmMatrix();
    const glm::mat4& getGlmMatrix() const;

private:
    glm::mat4 mat_;
};
```

### ファイル配置（予定）
- `src/oflike/math/ofMatrix4x4.h`
- `src/oflike/math/ofMatrix4x4.cpp`

---

## ofQuaternion

### 概要
クォータニオン（四元数）。回転を表現するのに使用。ジンバルロックを回避できる。

### 使用例（openFrameworks）

```cpp
ofQuaternion q1, q2;

// 軸回転
q1 = glm::angleAxis(glm::radians(45.0f), glm::vec3(0, 1, 0));

// 球面線形補間
ofQuaternion result = glm::slerp(q1, q2, 0.5f);

// 行列への変換
glm::mat4 rotMat = glm::toMat4(q1);

// オイラー角への変換
glm::vec3 euler = glm::eulerAngles(q1);
```

### 主要関数（GLM）

```cpp
// 作成
glm::quat glm::angleAxis(float angle, const vec3& axis);
glm::quat glm::quat(const vec3& eulerAngles);

// 補間
glm::quat glm::slerp(const quat& x, const quat& y, float a);
glm::quat glm::lerp(const quat& x, const quat& y, float a);

// 変換
glm::mat3 glm::toMat3(const quat& q);
glm::mat4 glm::toMat4(const quat& q);
glm::vec3 glm::eulerAngles(const quat& q);

// 演算
glm::quat glm::normalize(const quat& q);
glm::quat glm::conjugate(const quat& q);
glm::quat glm::inverse(const quat& q);
float glm::dot(const quat& a, const quat& b);

// ベクトル回転
glm::vec3 glm::rotate(const quat& q, const vec3& v);
```

### ファイル配置（予定）
- `src/oflike/math/ofQuaternion.h`
- `src/oflike/math/ofQuaternion.cpp`

---

## ofMath

### 概要
数学ユーティリティ関数。

### 関数定義（予定）

```cpp
// 乱数
float ofRandom(float max);
float ofRandom(float min, float max);
float ofRandomf();   // -1.0 〜 1.0
float ofRandomuf();  // 0.0 〜 1.0
void ofSeedRandom();
void ofSeedRandom(int seed);

// 補間
float ofLerp(float start, float end, float amt);
float ofClamp(float value, float min, float max);

// マッピング
float ofMap(float value, float inputMin, float inputMax, float outputMin, float outputMax, bool clamp = false);

// 距離
float ofDist(float x1, float y1, float x2, float y2);
float ofDist(float x1, float y1, float z1, float x2, float y2, float z2);
float ofDistSquared(float x1, float y1, float x2, float y2);

// 角度変換
float ofRadToDeg(float radians);
float ofDegToRad(float degrees);

// 符号
int ofSign(float n);
float ofSignedNormalize(float value, float min, float max);

// ラップ
float ofWrap(float value, float min, float max);
float ofWrapDegrees(float angle, float from = -180, float to = 180);
float ofWrapRadians(float angle, float from = -PI, float to = PI);

// 補間関数
float ofLerpDegrees(float start, float end, float amt);
float ofLerpRadians(float start, float end, float amt);

// 角度差
float ofAngleDifferenceDegrees(float a, float b);
float ofAngleDifferenceRadians(float a, float b);

// 三角関数（便利関数）
bool ofInRange(float value, float min, float max);
bool ofInsidePoly(float x, float y, const std::vector<glm::vec3>& poly);
bool ofInsidePoly(const glm::vec3& p, const std::vector<glm::vec3>& poly);

// 線分交差
bool ofLineSegmentIntersection(const glm::vec3& p1, const glm::vec3& p2,
                               const glm::vec3& p3, const glm::vec3& p4,
                               glm::vec3& intersection);

// ベジェ
glm::vec3 ofBezierPoint(const glm::vec3& a, const glm::vec3& b,
                        const glm::vec3& c, const glm::vec3& d, float t);
glm::vec3 ofCurvePoint(const glm::vec3& a, const glm::vec3& b,
                       const glm::vec3& c, const glm::vec3& d, float t);
glm::vec3 ofBezierTangent(const glm::vec3& a, const glm::vec3& b,
                          const glm::vec3& c, const glm::vec3& d, float t);
glm::vec3 ofCurveTangent(const glm::vec3& a, const glm::vec3& b,
                         const glm::vec3& c, const glm::vec3& d, float t);

// 次の2のべき乗
int ofNextPow2(int n);
```

### ファイル配置（予定）
- `src/oflike/math/ofMath.h`
- `src/oflike/math/ofMath.cpp`

---

## ofMathConstants

### 概要
数学定数。

### 定義（予定）

```cpp
#ifndef PI
#define PI 3.14159265358979323846f
#endif

#ifndef TWO_PI
#define TWO_PI 6.28318530717958647693f
#endif

#ifndef HALF_PI
#define HALF_PI 1.57079632679489661923f
#endif

#ifndef QUARTER_PI
#define QUARTER_PI 0.78539816339744830962f
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD (PI / 180.0f)
#endif

#ifndef RAD_TO_DEG
#define RAD_TO_DEG (180.0f / PI)
#endif

#ifndef EPSILON
#define EPSILON 0.0001f
#endif
```

### ファイル配置（予定）
- `src/oflike/math/ofMathConstants.h`

---

## ofNoise（予定）

### 概要
パーリンノイズ等のノイズ関数。

### 関数定義（予定）

```cpp
// パーリンノイズ
float ofNoise(float x);
float ofNoise(float x, float y);
float ofNoise(float x, float y, float z);
float ofNoise(float x, float y, float z, float w);
float ofNoise(const glm::vec2& p);
float ofNoise(const glm::vec3& p);
float ofNoise(const glm::vec4& p);

// 符号付きノイズ（-1〜1）
float ofSignedNoise(float x);
float ofSignedNoise(float x, float y);
float ofSignedNoise(float x, float y, float z);
float ofSignedNoise(float x, float y, float z, float w);
float ofSignedNoise(const glm::vec2& p);
float ofSignedNoise(const glm::vec3& p);
float ofSignedNoise(const glm::vec4& p);

// ノイズシード
void ofSetNoiseScale(float scale);
```

### ファイル配置（予定）
- `src/oflike/utils/ofNoise.h`

---

## 依存関係

```
ofVectorMath.h
  └── GLM (glm/glm.hpp, glm/gtc/quaternion.hpp)

ofMatrix3x3 / ofMatrix4x4
  └── GLM (glm/mat3x3.hpp, glm/mat4x4.hpp)

ofMath
  └── C++ <cmath>, <random>
  └── GLM

ofNoise
  └── 内部実装（Simplex noise等）
```
