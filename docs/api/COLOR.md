# Color API Design

色関連のAPI設計です。

---

## ofColor

### 概要
8ビット整数（0-255）で色を表現するクラス。テンプレートクラスとして実装。

### 使用例（openFrameworks）

```cpp
// 色の作成
ofColor red(255, 0, 0);
ofColor green = ofColor::green;
ofColor semiTransparent(128, 128, 128, 128);

// HSB変換
ofColor color;
color.setHsb(180, 200, 255);  // 色相, 彩度, 明度

// 16進数から
ofColor hexColor = ofColor::fromHex(0xFF00FF);

// 補間
ofColor blended = red.getLerped(green, 0.5f);

// 色の取得
float hue = red.getHue();
float saturation = red.getSaturation();
float brightness = red.getBrightness();

// ofSetColorで使用
ofSetColor(red);
ofSetColor(255, 0, 0, 255);
```

### クラス定義

```cpp
template<typename PixelType>
class ofColor_ {
public:
    // メンバ変数
    PixelType r, g, b, a;

    // コンストラクタ
    ofColor_();
    ofColor_(PixelType gray, PixelType alpha = limit());
    ofColor_(PixelType r, PixelType g, PixelType b, PixelType alpha = limit());

    // 静的ファクトリ
    static ofColor_ fromHsb(float hue, float saturation, float brightness, float alpha = limit());
    static ofColor_ fromHex(int hexColor, float alpha = limit());

    // 型変換
    template<typename SrcType>
    ofColor_(const ofColor_<SrcType>& other);

    // 設定
    void set(PixelType gray, PixelType alpha = limit());
    void set(PixelType r, PixelType g, PixelType b, PixelType alpha = limit());
    void setHex(int hex, float alpha = limit());
    void setHsb(float hue, float saturation, float brightness, float alpha = limit());
    void setHueAngle(float angle);
    void setSaturation(float saturation);
    void setBrightness(float brightness);

    // 取得
    int getHex() const;
    float getHue() const;
    float getHueAngle() const;
    float getSaturation() const;
    float getBrightness() const;
    float getLightness() const;
    void getHsb(float& hue, float& saturation, float& brightness) const;

    // 色演算
    ofColor_ getClamped() const;
    ofColor_ getInverted() const;
    ofColor_ getNormalized() const;
    ofColor_ getLerped(const ofColor_& target, float amount) const;

    // 演算子
    ofColor_ operator+(const ofColor_& other) const;
    ofColor_ operator-(const ofColor_& other) const;
    ofColor_ operator*(const ofColor_& other) const;
    ofColor_ operator*(float scalar) const;
    ofColor_ operator/(const ofColor_& other) const;
    ofColor_ operator/(float scalar) const;
    ofColor_& operator+=(const ofColor_& other);
    ofColor_& operator-=(const ofColor_& other);
    ofColor_& operator*=(const ofColor_& other);
    ofColor_& operator*=(float scalar);
    ofColor_& operator/=(const ofColor_& other);
    ofColor_& operator/=(float scalar);

    // 比較
    bool operator==(const ofColor_& other) const;
    bool operator!=(const ofColor_& other) const;

    // 配列アクセス
    PixelType& operator[](size_t n);
    const PixelType& operator[](size_t n) const;

    // 限界値
    static PixelType limit();

    // 定義済みカラー（静的メンバ）
    static const ofColor_ white;
    static const ofColor_ gray;
    static const ofColor_ black;
    static const ofColor_ red;
    static const ofColor_ green;
    static const ofColor_ blue;
    static const ofColor_ cyan;
    static const ofColor_ magenta;
    static const ofColor_ yellow;
    static const ofColor_ aliceBlue;
    static const ofColor_ antiqueWhite;
    static const ofColor_ aqua;
    static const ofColor_ aquamarine;
    // ... 他の定義済みカラー
};

// 型エイリアス
using ofColor = ofColor_<unsigned char>;
using ofFloatColor = ofColor_<float>;
using ofShortColor = ofColor_<unsigned short>;
```

### ファイル配置
- `src/oflike/graphics/ofColor.h`

---

## ofFloatColor

### 概要
浮動小数点（0.0-1.0）で色を表現。シェーダーやライティング計算で使用。

### 使用例

```cpp
ofFloatColor color(0.5f, 0.2f, 0.8f, 1.0f);

// ofColorとの相互変換
ofColor byteColor(128, 64, 200);
ofFloatColor fromByte(byteColor);
ofColor backToByte(fromByte);

// マテリアルカラー
ofMaterial mat;
mat.setDiffuseColor(ofFloatColor(0.8f, 0.2f, 0.2f));

// ライトカラー
ofLight light;
light.setDiffuseColor(ofFloatColor(1.0f, 1.0f, 0.9f));
```

### limit()の違い

| 型 | limit() |
|----|---------|
| ofColor | 255 |
| ofFloatColor | 1.0f |
| ofShortColor | 65535 |

---

## HSB色空間

### 概要
色相(Hue)、彩度(Saturation)、明度(Brightness)による色表現。

### 値の範囲

| 成分 | ofColor | ofFloatColor |
|------|---------|--------------|
| Hue | 0-255 | 0.0-1.0 |
| Saturation | 0-255 | 0.0-1.0 |
| Brightness | 0-255 | 0.0-1.0 |

### 使用例

```cpp
ofColor color;

// HSBで設定
color.setHsb(180, 255, 200);  // 色相180, 彩度最大, 明度200

// HSBで取得
float h, s, b;
color.getHsb(h, s, b);

// 色相のみ変更
for (int i = 0; i < 360; i++) {
    color.setHueAngle(i);
    // 虹色アニメーション
}

// 彩度のみ変更
color.setSaturation(128);

// 明度のみ変更
color.setBrightness(200);
```

---

## 定義済みカラー

### 基本色

```cpp
ofColor::white      // (255, 255, 255)
ofColor::black      // (0, 0, 0)
ofColor::gray       // (128, 128, 128)
ofColor::red        // (255, 0, 0)
ofColor::green      // (0, 128, 0)
ofColor::blue       // (0, 0, 255)
ofColor::cyan       // (0, 255, 255)
ofColor::magenta    // (255, 0, 255)
ofColor::yellow     // (255, 255, 0)
```

### 拡張色（一部）

```cpp
ofColor::aliceBlue
ofColor::antiqueWhite
ofColor::aqua
ofColor::aquamarine
ofColor::azure
ofColor::beige
ofColor::bisque
ofColor::blanchedAlmond
ofColor::blueViolet
ofColor::brown
ofColor::burlyWood
ofColor::cadetBlue
ofColor::chartreuse
ofColor::chocolate
ofColor::coral
ofColor::cornflowerBlue
ofColor::cornsilk
ofColor::crimson
ofColor::darkBlue
ofColor::darkCyan
ofColor::darkGoldenRod
ofColor::darkGray
ofColor::darkGreen
ofColor::darkKhaki
ofColor::darkMagenta
ofColor::darkOliveGreen
ofColor::darkOrange
ofColor::darkOrchid
ofColor::darkRed
ofColor::darkSalmon
ofColor::darkSeaGreen
ofColor::darkSlateBlue
ofColor::darkSlateGray
ofColor::darkTurquoise
ofColor::darkViolet
ofColor::deepPink
ofColor::deepSkyBlue
ofColor::dimGray
ofColor::dodgerBlue
ofColor::fireBrick
ofColor::floralWhite
ofColor::forestGreen
ofColor::fuchsia
ofColor::gainsboro
ofColor::ghostWhite
ofColor::gold
ofColor::goldenRod
// ... 他多数
```

---

## 色補間

### lerp（線形補間）

```cpp
ofColor c1(255, 0, 0);    // 赤
ofColor c2(0, 0, 255);    // 青

// 50%で補間
ofColor middle = c1.getLerped(c2, 0.5f);  // 紫

// グラデーション
for (float t = 0; t <= 1.0f; t += 0.1f) {
    ofColor step = c1.getLerped(c2, t);
    // 赤から青へのグラデーション
}
```

### HSBでの補間

```cpp
// より自然な色の遷移にはHSBで補間
ofColor c1, c2;
c1.setHsb(0, 255, 255);    // 赤 (Hue=0)
c2.setHsb(170, 255, 255);  // 青っぽい緑 (Hue=170)

for (float t = 0; t <= 1.0f; t += 0.1f) {
    float h1, s1, b1, h2, s2, b2;
    c1.getHsb(h1, s1, b1);
    c2.getHsb(h2, s2, b2);

    ofColor result;
    result.setHsb(
        ofLerp(h1, h2, t),
        ofLerp(s1, s2, t),
        ofLerp(b1, b2, t)
    );
}
```

---

## 16進数カラー

### 使用例

```cpp
// 16進数から作成
ofColor color = ofColor::fromHex(0xFF00FF);  // マゼンタ

// 16進数で設定
ofColor c;
c.setHex(0x00FF00);  // 緑

// 16進数を取得
int hex = color.getHex();  // 0xFF00FF

// アルファ付き16進数（ARGB）
ofColor withAlpha = ofColor::fromHex(0xFF00FF, 128);
```

---

## グローバル関数

### 色設定

```cpp
void ofSetColor(int gray);
void ofSetColor(int r, int g, int b);
void ofSetColor(int r, int g, int b, int a);
void ofSetColor(const ofColor& color);
void ofSetColor(const ofColor& color, int alpha);

void ofSetHexColor(int hex);

ofColor ofGetCurrentColor();
```

### ファイル配置
- `src/oflike/graphics/ofGraphics.h`

---

## Metal統合

### Uniform構造体での使用

```cpp
// シェーダーに渡す色データ
struct ColorData {
    simd::float4 color;  // RGBA (0.0-1.0)
};

// C++からMetalへの変換
ofFloatColor color(0.8f, 0.2f, 0.2f, 1.0f);
ColorData data;
data.color = simd::make_float4(color.r, color.g, color.b, color.a);
```

---

## 依存関係

```
ofColor
  └── テンプレート特殊化 (unsigned char, float, unsigned short)

ofFloatColor
  └── ofLight (diffuse, specular, ambient colors)
  └── ofMaterial (material colors)
  └── ofPath (fill, stroke colors)

ofColor
  └── ofSetColor
  └── ofBackground
  └── ofBackgroundGradient
```
