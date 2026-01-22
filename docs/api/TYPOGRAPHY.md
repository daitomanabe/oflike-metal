# Typography API Design

フォント関連のAPI設計です。

**重要**: ARCHITECTURE.md の方針に従い、Core Text を使用します。FreeType は禁止です。

---

## ofTrueTypeFont

### 概要
TrueTypeフォントを読み込み、文字列の描画や測定を行うクラス。

### 使用例（openFrameworks）

```cpp
ofTrueTypeFont font;

// フォント読み込み
font.load("verdana.ttf", 64, true, true, true);  // makeContours=true

// 文字列をパスとして取得
std::vector<ofPath> textPaths = font.getStringAsPoints("openFrameworks", false, false);

// 文字列のバウンディングボックス取得
ofRectangle fontRect = font.getStringBoundingBox(textStringToRender, 0.0f, 0.f);

// 文字列描画（テクスチャベース）
font.drawString("text", x, y);

// 文字列をシェイプとして描画
font.drawStringAsShapes("text", x, y);
```

### クラス定義

```cpp
class ofTrueTypeFont {
public:
    ofTrueTypeFont();
    ~ofTrueTypeFont();

    // 読み込み
    bool load(const std::string& filename, int fontsize,
              bool antialiased = true, bool fullCharset = true,
              bool makeContours = false, float simplifyAmt = 0.0f, int dpi = 0);
    bool loadFont(const std::string& filename, int fontsize,
                  bool antialiased = true, bool fullCharset = true,
                  bool makeContours = false, float simplifyAmt = 0.0f, int dpi = 0);
    bool isLoaded() const;
    void unload();

    // 文字列描画（テクスチャベース）
    void drawString(const std::string& s, float x, float y) const;

    // 文字列をシェイプとして描画（makeContours=true必須）
    void drawStringAsShapes(const std::string& s, float x, float y) const;

    // 測定
    float stringWidth(const std::string& s) const;
    float stringHeight(const std::string& s) const;
    ofRectangle getStringBoundingBox(const std::string& s, float x, float y, bool vflip = true) const;

    // メトリクス
    float getLineHeight() const;
    void setLineHeight(float height);
    float getAscenderHeight() const;
    float getDescenderHeight() const;
    void setLetterSpacing(float spacing);
    float getLetterSpacing() const;
    void setSpaceSize(float size);
    float getSpaceSize() const;

    // パスとして取得（makeContours=true必須）
    std::vector<ofPath> getStringAsPoints(const std::string& str, bool vflip = true, bool filled = true) const;
    ofPath getCharacterAsPoints(uint32_t character, bool vflip = true, bool filled = true) const;

    // グリフ検証
    bool isValidGlyph(uint32_t codepoint) const;
    int getNumCharacters() const;

    // フォント情報
    int getSize() const;
    std::string getFontName() const;

private:
    // Core Text (Objective-C++で隠蔽)
    void* ctFont_;  // CTFontRef

    // グリフキャッシュ
    struct GlyphInfo {
        float width, height;
        float bearingX, bearingY;
        float advance;
        float texX, texY, texW, texH;
    };
    std::unordered_map<uint32_t, GlyphInfo> glyphCache_;

    // テクスチャアトラス
    ofTexture atlasTexture_;
    int atlasWidth_, atlasHeight_;

    // コンターキャッシュ
    mutable std::unordered_map<uint32_t, ofPath> contourCache_;

    // メトリクス
    std::string fontName_;
    int fontSize_;
    float lineHeight_;
    float ascenderHeight_;
    float descenderHeight_;
    float letterSpacing_ = 0;
    float spaceSize_ = 0;

    // 状態
    bool loaded_ = false;
    bool hasContours_ = false;
};
```

### ファイル配置
- `src/oflike/graphics/ofTrueTypeFont.h` - ヘッダー
- `src/oflike/graphics/ofTrueTypeFont.mm` - Objective-C++実装

---

## Core Text 統合

### フォント読み込み

```objc
// システムフォント名から
CTFontRef font = CTFontCreateWithName((__bridge CFStringRef)name, fontSize, NULL);

// ファイルから
CGDataProviderRef dataProvider = CGDataProviderCreateWithURL((__bridge CFURLRef)fontURL);
CGFontRef cgFont = CGFontCreateWithDataProvider(dataProvider);
CTFontRef font = CTFontCreateWithGraphicsFont(cgFont, fontSize, NULL, NULL);
CGFontRelease(cgFont);
CGDataProviderRelease(dataProvider);
```

### テキスト描画（テクスチャアトラス方式）

```objc
// 1. グリフのビットマップを生成
CGContextRef ctx = CGBitmapContextCreate(...);
CTFontDrawGlyphs(font, &glyph, &position, 1, ctx);

// 2. アトラステクスチャにパック
// 3. 描画時はアトラスから該当領域を描画
```

### コンター取得（getStringAsPoints用）

```objc
// CTFontCreatePathForGlyph でベクターパスを取得
CGPathRef cgPath = CTFontCreatePathForGlyph(font, glyph, NULL);

// CGPath を ofPath に変換
CGPathApply(cgPath, &converter, [](void* info, const CGPathElement* element) {
    PathConverter* conv = (PathConverter*)info;
    switch (element->type) {
        case kCGPathElementMoveToPoint:
            conv->path.moveTo(element->points[0].x, element->points[0].y);
            break;
        case kCGPathElementAddLineToPoint:
            conv->path.lineTo(element->points[0].x, element->points[0].y);
            break;
        case kCGPathElementAddCurveToPoint:
            conv->path.bezierTo(
                element->points[0].x, element->points[0].y,
                element->points[1].x, element->points[1].y,
                element->points[2].x, element->points[2].y
            );
            break;
        case kCGPathElementCloseSubpath:
            conv->path.close();
            break;
    }
});
CGPathRelease(cgPath);
```

---

## CMake設定

```cmake
# Core Text (FreeType は禁止)
find_library(CORETEXT_FRAMEWORK CoreText REQUIRED)
find_library(COREGRAPHICS_FRAMEWORK CoreGraphics REQUIRED)
target_link_libraries(${name} PRIVATE ${CORETEXT_FRAMEWORK} ${COREGRAPHICS_FRAMEWORK})
```

---

## 依存関係

```
ofTrueTypeFont
  ├── Core Text (CTFont)
  ├── Core Graphics (CGPath, CGContext)
  ├── ofTexture (アトラス)
  └── ofPath (コンター)
```

---

## フォント検索順序

1. 指定されたパスのファイル
2. `data/` ディレクトリ
3. システムフォント（名前で検索）
   - `Helvetica`, `Arial`, `Times New Roman` 等

---

## サポートするフォント形式

Core Text がサポートする形式:
- TrueType (.ttf)
- OpenType (.otf)
- TrueType Collection (.ttc)
- PostScript Type 1 (限定的)
