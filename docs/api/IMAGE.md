# Image API Design

画像・テクスチャ関連のAPI設計です。

**重要**: ARCHITECTURE.md の方針に従い、ImageIO/MTKTextureLoader を使用します。stb_image は禁止です。

---

## ofTexture

### 概要
GPUテクスチャを管理するクラス。画像の読み込み、バインド、描画を行う。

### 使用例（openFrameworks）

```cpp
ofTexture texture;
ofLoadImage(texture, "tex.png");
texture.setTextureWrap(GL_REPEAT, GL_CLAMP);
texture.bind();
// メッシュ描画
texture.unbind();
float width = texture.getWidth();
```

### クラス定義

```cpp
class ofTexture {
public:
    ofTexture();
    ~ofTexture();

    // コピー禁止、ムーブ可能
    ofTexture(const ofTexture&) = delete;
    ofTexture& operator=(const ofTexture&) = delete;
    ofTexture(ofTexture&& other) noexcept;
    ofTexture& operator=(ofTexture&& other) noexcept;

    // 割り当て
    void allocate(int width, int height, int channels = 4);
    void loadData(const unsigned char* data, int width, int height, int channels = 4);
    void loadFromNativeTexture(void* metalTexture, int width, int height);
    void clear();

    // 状態
    bool isAllocated() const;
    int getWidth() const;
    int getHeight() const;

    // 描画
    void draw(float x, float y) const;
    void draw(float x, float y, float w, float h) const;

    // バインド（カスタム描画用）
    void bind(int textureLocation = 0) const;
    void unbind(int textureLocation = 0) const;

    // テクスチャ設定
    // wrapMode: 0=REPEAT, 1=CLAMP, 2=CLAMP_TO_EDGE, 3=MIRRORED_REPEAT
    void setTextureWrap(int wrapModeS, int wrapModeT);

    // filter: GL_NEAREST=0x2600, GL_LINEAR=0x2601
    void setTextureMinMagFilter(int minFilter, int magFilter);

    // Metal直接アクセス
    void* getTexturePtr() const;   // id<MTLTexture>
    void* getSamplerPtr() const;   // id<MTLSamplerState>

private:
    void* texture_{nullptr};   // id<MTLTexture>
    void* sampler_{nullptr};   // id<MTLSamplerState>
    int width_{0};
    int height_{0};
    int channels_{4};
};
```

### ファイル配置
- `src/oflike/image/ofTexture.h` - ヘッダー
- `src/render/metal/MetalTexture.mm` - Metal実装

---

## ofPixels

### 概要
CPUサイドのピクセルデータを管理するクラス。

### クラス定義

```cpp
namespace oflike {

class ofPixels {
public:
    ofPixels() = default;
    ~ofPixels();

    // コピー・ムーブ（必要に応じて実装）
    ofPixels(const ofPixels&) = delete;
    ofPixels& operator=(const ofPixels&) = delete;

    // メモリ管理
    void allocate(int width, int height, int channels = 4);
    void clear();

    // データアクセス
    unsigned char* getData();
    const unsigned char* getData() const;

    // サイズ
    int getWidth() const;
    int getHeight() const;
    int getChannels() const;

    // ピクセル操作
    void setColor(int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
    void getColor(int x, int y, unsigned char& r, unsigned char& g, unsigned char& b, unsigned char& a) const;

private:
    unsigned char* data_{nullptr};
    int width_{0};
    int height_{0};
    int channels_{4};
};

} // namespace oflike
```

### ファイル配置
- `src/oflike/image/ofTexture.h` - ヘッダー
- `src/render/metal/MetalTexture.mm` - 実装

---

## ofLoadImage

### 概要
画像ファイルを読み込む関数。

**実装方針（ARCHITECTURE.md準拠）**:
- `ofLoadImage(ofTexture&)`: MTKTextureLoader を優先使用（最速、直接GPU読み込み）
- `ofLoadImage(ofPixels&)`: ImageIO (CGImageSource) を使用

### 関数定義

```cpp
// ofTextureへ直接読み込み（MTKTextureLoader使用 - 最速）
bool ofLoadImage(ofTexture& tex, const std::string& path);

// ofPixelsへ読み込み（ImageIO使用）
bool ofLoadImage(oflike::ofPixels& pix, const std::string& path);
```

### 実装詳細

#### ofTexture への読み込み（MTKTextureLoader）

```objc
bool ofLoadImage(ofTexture& tex, const std::string& path) {
    @autoreleasepool {
        id<MTLDevice> device = ...;
        MTKTextureLoader* loader = [[MTKTextureLoader alloc] initWithDevice:device];

        NSDictionary* options = @{
            MTKTextureLoaderOptionSRGB: @NO,
            MTKTextureLoaderOptionGenerateMipmaps: @NO,
            MTKTextureLoaderOptionTextureUsage: @(MTLTextureUsageShaderRead),
            MTKTextureLoaderOptionTextureStorageMode: @(MTLStorageModeShared)
        };

        NSError* error = nil;
        id<MTLTexture> mtlTexture = [loader newTextureWithContentsOfURL:url
                                                                options:options
                                                                  error:&error];
        if (!mtlTexture || error) {
            // フォールバック: ImageIO経由
            oflike::ofPixels pix;
            if (!ofLoadImage(pix, path)) return false;
            tex.loadData(pix.getData(), pix.getWidth(), pix.getHeight(), pix.getChannels());
            return true;
        }

        tex.loadFromNativeTexture((__bridge void*)mtlTexture, w, h);
        return true;
    }
}
```

#### ofPixels への読み込み（ImageIO）

```objc
bool ofLoadImage(oflike::ofPixels& pix, const std::string& path) {
    @autoreleasepool {
        NSURL* url = [NSURL fileURLWithPath:@(path.c_str())];
        CGImageSourceRef imageSource = CGImageSourceCreateWithURL((__bridge CFURLRef)url, nil);
        CGImageRef cgImage = CGImageSourceCreateImageAtIndex(imageSource, 0, nil);

        // CGBitmapContext で RGBA バッファにレンダリング
        CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
        CGContextRef ctx = CGBitmapContextCreate(data, w, h, 8, w * 4, colorSpace, ...);
        CGContextDrawImage(ctx, CGRectMake(0, 0, w, h), cgImage);

        // Cleanup
        CFRelease(imageSource);
        CGImageRelease(cgImage);
        CGContextRelease(ctx);
        CGColorSpaceRelease(colorSpace);

        return true;
    }
}
```

### ファイル配置
- `src/render/metal/MetalTexture.mm` - 実装

---

## パフォーマンス比較

| 方式 | 速度 | CPU使用 | GPU直接 | 備考 |
|------|------|---------|---------|------|
| **MTKTextureLoader** | 最速 | 最小 | Yes | 推奨 |
| ImageIO | 高速 | 中 | No | フォールバック |
| NSImage | 遅い | 高 | No | 使用禁止 |
| stb_image | - | - | - | **禁止** |

---

## 依存関係

```
ofTexture
  └── MetalRenderer (bind/unbind)
  └── MTKTextureLoader (loadFromNativeTexture)

ofPixels
  └── ImageIO (CGImageSource)

ofLoadImage
  └── MTKTextureLoader (優先)
  └── ImageIO (フォールバック)
```

---

## 対応フォーマット

MTKTextureLoader / ImageIO が対応するフォーマット:
- PNG
- JPEG
- TIFF
- BMP
- GIF
- HEIF/HEIC (macOS 10.13+)
- WebP (macOS 11+)
