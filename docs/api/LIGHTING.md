# Lighting API Design

ライティング関連のAPI設計です。

---

## ofLight

### 概要
ライト（光源）を管理するクラス。Point, Directional, Spotライトをサポート。

### 使用例（openFrameworks）

```cpp
ofLight pointLight;

// 色設定
pointLight.setDiffuseColor(ofFloatColor(.85, .85, .55));
pointLight.setSpecularColor(ofFloatColor(1.f, 1.f, 1.f));

// 位置
pointLight.setPosition(x, y, z);

// レンダリング
ofEnableLighting();
pointLight.enable();
  // 3D描画
pointLight.disable();
ofDisableLighting();

// ライト自体を描画（デバッグ用）
pointLight.draw();
```

### ライトタイプ

| タイプ | 説明 |
|--------|------|
| POINT | 点光源（全方向に光を放射） |
| DIRECTIONAL | 平行光源（太陽光のような無限遠からの光） |
| SPOT | スポットライト（円錐状に光を放射） |
| AREA | エリアライト（面光源） |

### クラス定義

```cpp
class ofLight : public ofNode {
public:
    enum Type { POINT, DIRECTIONAL, SPOT, AREA };

    ofLight();
    ~ofLight();

    // 有効/無効
    void enable();
    void disable();
    bool isEnabled() const;

    // タイプ
    void setType(Type type);
    Type getType() const;

    // ポイントライト設定
    void setPointLight();

    // ディレクショナルライト設定
    void setDirectional();

    // スポットライト設定
    void setSpotlight(float spotCutOff = 45.0f, float exponent = 0.0f);

    // エリアライト設定
    void setAreaLight(float width, float height);

    // 色
    void setAmbientColor(const ofFloatColor& c);
    void setDiffuseColor(const ofFloatColor& c);
    void setSpecularColor(const ofFloatColor& c);
    ofFloatColor getAmbientColor() const;
    ofFloatColor getDiffuseColor() const;
    ofFloatColor getSpecularColor() const;

    // 減衰（距離による光の弱まり）
    void setAttenuation(float constant = 1.0f, float linear = 0.0f, float quadratic = 0.0f);
    float getAttenuationConstant() const;
    float getAttenuationLinear() const;
    float getAttenuationQuadratic() const;

    // スポットライトパラメータ
    void setSpotConcentration(float concentration);
    void setSpotlightCutOff(float cutoff);
    float getSpotConcentration() const;
    float getSpotlightCutOff() const;

    // 描画（デバッグ用）
    void draw() const;

protected:
    Type type_ = POINT;
    ofFloatColor ambient_{0, 0, 0, 1};
    ofFloatColor diffuse_{1, 1, 1, 1};
    ofFloatColor specular_{1, 1, 1, 1};

    float attenuation_constant_ = 1.0f;
    float attenuation_linear_ = 0.0f;
    float attenuation_quadratic_ = 0.0f;

    float spotCutoff_ = 45.0f;
    float spotConcentration_ = 0.0f;

    bool enabled_ = false;
};
```

### ファイル配置
- `src/oflike/lighting/ofLight.h`
- `src/oflike/lighting/ofLight.cpp`

---

## ofMaterial

### 概要
マテリアル（材質）を管理するクラス。Phongシェーディング用のパラメータ。

### 使用例（openFrameworks）

```cpp
ofMaterial material;
material.setDiffuseColor(ofFloatColor(0.8, 0.2, 0.2));
material.setSpecularColor(ofFloatColor(1.0, 1.0, 1.0));
material.setShininess(64.0);

material.begin();
  // オブジェクト描画
  sphere.draw();
material.end();
```

### クラス定義

```cpp
class ofMaterial {
public:
    ofMaterial();

    // 適用
    void begin() const;
    void end() const;

    // 色設定
    void setDiffuseColor(const ofFloatColor& c);
    void setSpecularColor(const ofFloatColor& c);
    void setAmbientColor(const ofFloatColor& c);
    void setEmissiveColor(const ofFloatColor& c);
    void setShininess(float shininess);

    // 色取得
    ofFloatColor getDiffuseColor() const;
    ofFloatColor getSpecularColor() const;
    ofFloatColor getAmbientColor() const;
    ofFloatColor getEmissiveColor() const;
    float getShininess() const;

    // 一括設定
    void setColors(const ofFloatColor& diffuse,
                   const ofFloatColor& ambient,
                   const ofFloatColor& specular,
                   const ofFloatColor& emissive);

protected:
    ofFloatColor diffuse_{0.8f, 0.8f, 0.8f, 1.0f};
    ofFloatColor specular_{0.0f, 0.0f, 0.0f, 1.0f};
    ofFloatColor ambient_{0.2f, 0.2f, 0.2f, 1.0f};
    ofFloatColor emissive_{0.0f, 0.0f, 0.0f, 1.0f};
    float shininess_ = 0.2f;
};
```

### ファイル配置
- `src/oflike/lighting/ofMaterial.h`
- `src/oflike/lighting/ofMaterial.cpp`

---

## グローバル関数

### ライティング制御

```cpp
void ofEnableLighting();
void ofDisableLighting();
bool ofGetLightingEnabled();

void ofEnableSeparateSpecularLight();
void ofDisableSeparateSpecularLight();

void ofSetSmoothLighting(bool smooth);

void ofSetGlobalAmbientColor(const ofFloatColor& c);
ofFloatColor ofGetGlobalAmbientColor();
```

### ファイル配置
- `src/oflike/graphics/ofGraphics.h` に追加

---

## Metal Uniform構造体

ライトとマテリアルのデータをGPUに渡すための構造体。

### Light構造体

```cpp
// C++側
struct LightData {
    glm::vec3 position;
    float pad0;
    glm::vec3 direction;
    float pad1;
    glm::vec3 diffuseColor;
    float pad2;
    glm::vec3 specularColor;
    float pad3;
    glm::vec3 ambientColor;
    float pad4;
    glm::vec3 attenuation;  // constant, linear, quadratic
    int type;  // 0=point, 1=directional, 2=spot
    float spotCutoff;
    float spotConcentration;
    float pad5[2];
};
```

### Material構造体

```cpp
// C++側
struct MaterialData {
    glm::vec3 diffuse;
    float pad0;
    glm::vec3 specular;
    float pad1;
    glm::vec3 ambient;
    float pad2;
    glm::vec3 emissive;
    float shininess;
};
```

詳細は `metal/SHADERS.md` を参照。

---

## 依存関係

```
ofLight
  └── ofNode (→ CORE.md)

ofMaterial
  └── Metal Uniforms (→ metal/SHADERS.md)

グローバル関数
  └── Engine (style管理)
```
