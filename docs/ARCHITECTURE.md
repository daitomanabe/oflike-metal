# oflike-metal Architecture - Absolute Policies

**Version**: 2.0.0  
**Last Updated**: 2025-01-23  
**Status**: Active

> このドキュメントはプロジェクトの絶対的な方針を定義します。  
> 全ての実装はこの方針に従う必要があります。  
> 変更には全メンバーの合意が必要です。

---

## 1. プロジェクト概要

### 1.1 ミッション

**openFrameworks の macOS ネイティブ再実装**

- **外見**: openFrameworks API 互換
- **中身**: SwiftUI + Metal + Apple Native Frameworks
- **記述**: C++

### 1.2 4つの柱

| 柱 | 内容 |
|---|------|
| **SwiftUI** | ウィンドウ、イベント、UI |
| **Metal** | GPUレンダリング、シェーダー |
| **C++記述** | アプリケーションロジックはC++で記述 |
| **oF API互換** | openFrameworks の API シグネチャを維持 |

### 1.3 ユーザー視点

```cpp
// ユーザーは従来の openFrameworks と同じ感覚で書ける
class ofApp : public ofBaseApp {
    void setup() override {
        ofSetBackgroundColor(0);
    }
    
    void draw() override {
        ofSetColor(255, 0, 0);
        ofDrawCircle(ofGetWidth()/2, ofGetHeight()/2, 100);
    }
};

// 内部は SwiftUI + Metal で動作（ユーザーは意識しない）
```

### 1.4 技術スタック

```
ユーザーコード (C++)
       │
       ▼
oflike API (C++) ─── oF互換インターフェース
       │
       ▼
render (C++) ─────── 描画抽象化
       │
       ▼
Metal (Obj-C++) ──── GPU描画
       │
       ▼
SwiftUI (Swift) ──── ウィンドウ・イベント・UI
       │
       ▼
macOS Frameworks
```

### 1.5 プラットフォーム要件

- **macOS専用** (最小サポート: macOS 13.0 Ventura)
- **Apple Silicon最適化** (Intel互換は維持)

### 1.6 設計原則

1. **ネイティブ優先**: macOSフレームワークを最大限活用
2. **パフォーマンス重視**: GPUファーストの設計
3. **API互換性**: openFrameworks APIとの高い互換性
4. **シンプルさ**: 必要最小限の抽象化

---

## 2. 依存関係ポリシー

### 2.1 必須フレームワーク

| フレームワーク | 用途 | 最小バージョン |
|--------------|------|---------------|
| **Metal** | GPUレンダリング | macOS 13.0 |
| **MetalKit** | MTKView、テクスチャローダー | macOS 13.0 |
| **SwiftUI** | ウィンドウ、UI、イベント | macOS 13.0 |
| **Core Text** | フォント | macOS 13.0 |
| **Core Graphics** | 2Dパス、ビットマップ | macOS 13.0 |
| **ImageIO** | 画像I/O | macOS 13.0 |
| **Accelerate** | SIMD、画像処理 | macOS 13.0 |
| **QuartzCore** | CADisplayLink | macOS 13.0 |

### 2.2 禁止ライブラリ

| ライブラリ | 代替 | 例外 |
|-----------|------|------|
| FreeType | Core Text | なし |
| OpenGL | Metal | なし |
| SDL / GLFW | SwiftUI + MTKView | なし |
| stb_image (読込) | ImageIO / MTKTextureLoader | なし |
| stb_image_write | CGImageDestination | EXR/HDRのみ許可 |
| Dear ImGui | SwiftUI Overlay | なし |
| 外部SIMDライブラリ | simd/simd.h, Accelerate | なし |
| **AppKit (直接使用)** | SwiftUI | 低レベルブリッジのみ許可 |

### 2.3 AppKit 使用制限

```
禁止: NSWindow, NSView, NSEvent, NSResponder の直接使用
許可: MTKView (MetalKit経由)、NSApplication (起動のみ)
方針: 全てSwiftUI経由でラップし、AppKitを隠蔽
```

### 2.4 許可サードパーティ

| ライブラリ | 用途 | 状態 |
|-----------|------|------|
| tess2 | ポリゴン三角形分割 | 許可 |
| utf8-cpp | UTF-8処理 | 許可 |
| GLM | 行列計算 | **非推奨** (v0.5で廃止) |

### 2.5 数学ライブラリ移行

| フェーズ | バージョン | 状態 |
|---------|-----------|------|
| Phase 1 | 〜v0.1.0 | GLM許可 |
| Phase 2 | v0.1.0〜v0.5.0 | 併用（新規はsimd） |
| Phase 3 | v0.5.0〜 | **simd統一** |

#### 型対応表

| GLM (廃止予定) | simd (推奨) | oflike互換 |
|---------------|-------------|-----------|
| glm::vec2 | simd_float2 | ofVec2f |
| glm::vec3 | simd_float3 | ofVec3f |
| glm::vec4 | simd_float4 | ofVec4f |
| glm::mat4 | simd_float4x4 | ofMatrix4x4 |
| glm::quat | simd_quatf | ofQuaternion |

#### 新規コードでの使用ルール

```cpp
// ❌ 禁止: 新規コードでGLM直接使用
glm::vec3 pos = glm::vec3(1, 2, 3);

// ✅ 推奨: simd型を使用
float3 pos = simd_make_float3(1, 2, 3);

// ✅ 許可: oflike互換型を使用（内部でsimd）
ofVec3f pos(1, 2, 3);
```

---

## 3. アーキテクチャ

### 3.1 レイヤー構造

```
┌─────────────────────────────────────────┐
│           Application (User Code)        │
│              Pure C++ / oflike API       │
└─────────────────────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────┐
│         oflike/ (oF互換API)              │
│   ofDrawCircle, ofImage, ofCamera...    │
└─────────────────────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────┐
│         render/ (描画抽象化)             │
│      DrawList, DrawCommand, Types       │
└─────────────────────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────┐
│        render/metal/ (Metal実装)         │
│  MetalRenderer, MetalTexture, Shaders   │
└─────────────────────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────┐
│        platform/swiftui/ (UI層)          │
│   SwiftUI Window, MTKView, Events       │
└─────────────────────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────┐
│         macOS Frameworks                 │
│  Metal, MetalKit, SwiftUI, CoreText     │
└─────────────────────────────────────────┘
```

### 3.2 依存方向

```
oflike/ ──depends──► render/ ──depends──► render/metal/
   │                    │                      │
   │                    │                      ▼
   │                    │              platform/swiftui/
   │                    │                      │
   ▼                    ▼                      ▼
 math/               core/             macOS Frameworks
```

**禁止**: 下位レイヤーから上位レイヤーへの依存

### 3.3 ディレクトリ構造

```
oflike_metal/
├── docs/
│   ├── ARCHITECTURE.md
│   ├── IMPLEMENTATION.md
│   ├── THIRDPARTY.md
│   └── api/
│
├── src/
│   ├── core/
│   │   ├── Engine.h/.cpp
│   │   ├── AppBase.h/.cpp
│   │   ├── Context.h/.cpp
│   │   └── Time.h/.cpp
│   │
│   ├── math/
│   │   ├── Types.h               # simd エイリアス
│   │   ├── Functions.h
│   │   └── Transform.h
│   │
│   ├── platform/
│   │   ├── swiftui/
│   │   │   ├── App.swift          # @main エントリ
│   │   │   ├── MetalView.swift    # MTKView ラッパー
│   │   │   ├── EventBridge.swift  # イベント処理
│   │   │   └── DebugOverlay.swift # デバッグUI
│   │   └── bridge/
│   │       ├── SwiftBridge.h      # C++ から呼び出し用
│   │       └── SwiftBridge.mm
│   │
│   ├── render/
│   │   ├── IRenderer.h
│   │   ├── DrawList.h/.cpp
│   │   ├── DrawCommand.h
│   │   ├── RenderTypes.h
│   │   └── metal/
│   │       ├── MetalRenderer.h/.mm
│   │       ├── MetalTexture.h/.mm
│   │       ├── MetalBuffer.h/.mm
│   │       └── MetalShader.h/.mm
│   │
│   └── oflike/
│       ├── ofMain.h
│       ├── ofConstants.h
│       ├── app/
│       ├── graphics/
│       ├── 3d/
│       ├── image/
│       ├── lighting/
│       ├── math/
│       ├── types/
│       └── utils/
│
├── shaders/
│   ├── Common.h
│   ├── Basic2D.metal
│   ├── Basic3D.metal
│   └── Lighting.metal
│
├── third_party/
│   ├── tess2/
│   └── utf8/
│
├── examples/
├── tests/
├── resources/
└── references/
```

---

## 4. 実装パターン

### 4.1 pImpl パターン

#### ヘッダー (.h) - Pure C++

```cpp
// ofTexture.h
#pragma once
#include <memory>
#include <string>

class ofTexture {
public:
    ofTexture();
    ~ofTexture();
    ofTexture(ofTexture&&) noexcept;
    ofTexture& operator=(ofTexture&&) noexcept;
    
    ofTexture(const ofTexture&) = delete;
    ofTexture& operator=(const ofTexture&) = delete;
    
    bool load(const std::string& path);
    void bind(int unit = 0) const;
    int getWidth() const;
    int getHeight() const;
    
    void* getNativeHandle() const;
    
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
```

#### 実装 (.mm) - Objective-C++

```objc
// ofTexture.mm
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#include "ofTexture.h"
#include "Context.h"

struct ofTexture::Impl {
    id<MTLTexture> texture = nil;
    int width = 0;
    int height = 0;
    
    ~Impl() {
        texture = nil;  // ARC が解放
    }
};

ofTexture::ofTexture() : impl_(std::make_unique<Impl>()) {}
ofTexture::~ofTexture() = default;
ofTexture::ofTexture(ofTexture&&) noexcept = default;
ofTexture& ofTexture::operator=(ofTexture&&) noexcept = default;

bool ofTexture::load(const std::string& path) {
    @autoreleasepool {
        NSError* error = nil;
        NSURL* url = [NSURL fileURLWithPath:
            [NSString stringWithUTF8String:path.c_str()]];
        
        MTKTextureLoader* loader = ctx().renderer().textureLoader();
        NSDictionary* options = @{
            MTKTextureLoaderOptionSRGB: @NO,
            MTKTextureLoaderOptionGenerateMipmaps: @YES
        };
        
        impl_->texture = [loader newTextureWithContentsOfURL:url
                                                     options:options
                                                       error:&error];
        if (error) {
            ofLogError("ofTexture") << "Load failed: " 
                << error.localizedDescription.UTF8String;
            return false;
        }
        
        impl_->width = (int)impl_->texture.width;
        impl_->height = (int)impl_->texture.height;
        return true;
    }
}

void* ofTexture::getNativeHandle() const {
    return (__bridge void*)impl_->texture;
}
```

### 4.2 ブリッジキャスト規則

| キャスト | 所有権 | 用途 |
|---------|--------|------|
| `__bridge` | 変更なし | 一時参照 |
| `__bridge_retained` | ARC→手動 | C++で保持 |
| `__bridge_transfer` | 手動→ARC | ARCに戻す |

### 4.3 Autorelease Pool

```objc
// 必須: Objective-Cオブジェクト生成時
bool ofTexture::load(const std::string& path) {
    @autoreleasepool {
        // NSString, NSURL 等の生成
        return loadImpl(path);
    }
}
```

### 4.4 Swift-C++ ブリッジ

```
SwiftUI → Swift Callback → Obj-C++ Bridge → C++ Core
```

```swift
// Swift側
class RenderCoordinator: ObservableObject {
    private var bridge = OFLBridge()
    
    func setup() {
        bridge.setup()
    }
    
    func draw() {
        bridge.draw()
    }
}
```

```objc
// Obj-C++ Bridge
@interface OFLBridge : NSObject
- (void)setup;
- (void)draw;
@end

@implementation OFLBridge {
    std::unique_ptr<ofApp> app_;
}

- (void)setup {
    app_ = std::make_unique<ofApp>();
    app_->setup();
}

- (void)draw {
    app_->draw();
}
@end
```

---

## 5. 座標系・単位

### 5.1 2D座標系

```
(0,0) ────────────► X
  │
  │    原点: 左上
  │    Y軸: 下向き正
  │    単位: ピクセル
  ▼
  Y
```

### 5.2 3D座標系

```
      Y (上)
      │
      │     右手座標系
      │     (OpenGL/oF互換)
      └──────► X (右)
     ╱
    ╱
   Z (手前)
```

### 5.3 テクスチャ座標

```
(0,0) ────────────► U
  │
  │    正規化座標 (0.0〜1.0)
  │    原点: 左上 (Metal標準)
  ▼
  V
```

### 5.4 単位規則

| 項目 | 単位 | 備考 |
|------|------|------|
| 位置・サイズ | ピクセル (float) | Retina考慮 |
| 時間 | 秒 (double) | ミリ秒は使用しない |
| 角度 (内部) | ラジアン | |
| 角度 (API) | 度数も許可 | |
| 色 | 0.0〜1.0 | |

---

## 6. エラーハンドリング

### 6.1 方針

| エラー種別 | 対応 |
|-----------|------|
| 致命的 | `std::runtime_error` |
| 回復可能 | `bool` / `std::optional` |
| 警告 | `ofLogWarning()` |

### 6.2 Metal エラー

```objc
// ✅ 必須: エラーチェック
NSError* error = nil;
id<MTLTexture> tex = [loader newTextureWithContentsOfURL:url
                                                 options:options
                                                   error:&error];
if (error) {
    ofLogError("Metal") << error.localizedDescription.UTF8String;
    return nullptr;
}
```

### 6.3 ファイル操作

```cpp
// ✅ 推奨: 戻り値でチェック
ofPixels pix;
if (!ofLoadImage(pix, "image.png")) {
    ofLogError() << "Failed to load image";
}
```

---

## 7. スレッドルール

### 7.1 必須: メインスレッド

- MTKView 描画
- SwiftUI 更新
- UI操作全般

### 7.2 許可: 任意スレッド

- MTLDevice 作成
- MTLCommandBuffer 作成・コミット
- 画像デコード
- 計算処理

### 7.3 描画コマンド

```cpp
// ❌ 禁止: バックグラウンドスレッドから直接描画
DispatchQueue.global().async {
    ofDrawCircle(100, 100, 50)  // 危険！
}

// ✅ 推奨: メインスレッドで描画
DispatchQueue.main.async {
    ofDrawCircle(100, 100, 50)
}
```

---

## 8. パフォーマンス目標

### 8.1 フレームレート

| 解像度 | 目標 |
|--------|------|
| 1080p | 60fps安定 |
| 4K | 60fps (軽量シーン) |

### 8.2 リソース目標

| 項目 | 目標 | 警告閾値 |
|------|------|---------|
| フレーム時間 | <16ms | >14ms |
| ドローコール | <500 | >300 |
| 頂点数/フレーム | <1M | >500K |

### 8.3 バッファ戦略

| バッファ種類 | Storage Mode | 用途 |
|-------------|--------------|------|
| 頂点（静的） | Private | 変更しないメッシュ |
| 頂点（動的） | Shared | 毎フレーム更新 |
| Uniform | Shared | 定数バッファ |
| テクスチャ | Private | GPU専用 |

---

## 9. openFrameworks互換性

### 9.1 互換性レベル

| レベル | 説明 |
|--------|------|
| A | 完全互換 |
| B | API互換、軽微差異 |
| C | 類似API |
| D | 非対応 |

### 9.2 Graphics API

| oF API | レベル | 備考 |
|--------|--------|------|
| ofSetColor() | A | |
| ofSetBackgroundColor() | A | |
| ofBackground() | A | |
| ofClear() | A | |
| ofDrawCircle() | A | |
| ofDrawEllipse() | A | |
| ofDrawRectangle() | A | |
| ofDrawRectRounded() | A | |
| ofDrawTriangle() | A | |
| ofDrawLine() | A | |
| ofDrawBezier() | A | |
| ofDrawCurve() | A | |
| ofBeginShape() / ofEndShape() | A | |
| ofVertex() | A | |
| ofCurveVertex() | A | |
| ofBezierVertex() | A | |
| ofNextContour() | A | |
| ofPushMatrix() / ofPopMatrix() | A | |
| ofTranslate() | A | |
| ofRotate() | B | ラジアン推奨 |
| ofRotateX/Y/Z() | B | ラジアン推奨 |
| ofScale() | A | |
| ofSetLineWidth() | B | Metal制限 (最小1.0) |
| ofNoFill() / ofFill() | A | |
| ofSetCircleResolution() | A | |
| ofSetCurveResolution() | A | |
| ofEnableDepthTest() | A | |
| ofDisableDepthTest() | A | |
| ofEnableAlphaBlending() | A | |
| ofEnableBlendMode() | A | |
| ofEnableAntiAliasing() | B | MSAA設定に依存 |
| ofEnableSmoothing() | D | Metal非対応 |

### 9.3 3D API

| oF API | レベル | 備考 |
|--------|--------|------|
| ofDrawBox() | A | |
| ofDrawSphere() | A | |
| ofDrawCone() | A | |
| ofDrawCylinder() | A | |
| ofDrawPlane() | A | |
| ofDrawIcoSphere() | A | |
| ofDrawAxis() | A | |
| ofDrawGrid() | A | |
| ofDrawGridPlane() | A | |
| ofDrawArrow() | A | |
| ofDrawRotationAxes() | A | |

### 9.4 ofMesh

| oF API | レベル | 備考 |
|--------|--------|------|
| ofMesh() | A | |
| addVertex() | A | |
| addVertices() | A | |
| addNormal() | A | |
| addTexCoord() | A | |
| addColor() | A | |
| addIndex() | A | |
| addTriangle() | A | |
| draw() | A | |
| drawWireframe() | A | |
| drawVertices() | A | |
| drawFaces() | A | |
| setMode() | A | |
| getVertices() | A | |
| load() / save() | B | PLY/OBJのみ |
| mergeDuplicateVertices() | A | |
| setupIndicesAuto() | A | |
| smoothNormals() | A | |
| flatNormals() | A | |
| append() | A | |
| ofMesh::plane() | A | |
| ofMesh::sphere() | A | |
| ofMesh::box() | A | |
| ofMesh::cone() | A | |
| ofMesh::cylinder() | A | |
| ofMesh::icosphere() | A | |

### 9.5 ofCamera / ofEasyCam

| oF API | レベル | 備考 |
|--------|--------|------|
| ofCamera() | A | |
| begin() / end() | A | |
| setPosition() | A | |
| setOrientation() | A | |
| lookAt() | A | |
| setNearClip() | A | |
| setFarClip() | A | |
| setFov() | A | |
| setAspectRatio() | A | |
| getModelViewMatrix() | A | |
| getProjectionMatrix() | A | |
| worldToScreen() | A | |
| screenToWorld() | A | |
| ofEasyCam() | B | |
| setDistance() | A | |
| setTarget() | A | |
| enableMouseInput() | A | |
| disableMouseInput() | A | |
| setAutoDistance() | B | 軽微な差異 |
| enableInertia() | B | |

### 9.6 ofLight / ofMaterial

| oF API | レベル | 備考 |
|--------|--------|------|
| ofLight() | B | 最大8灯 |
| enable() / disable() | A | |
| setPointLight() | A | |
| setDirectional() | A | |
| setSpotlight() | A | |
| setAmbientColor() | A | |
| setDiffuseColor() | A | |
| setSpecularColor() | A | |
| setAttenuation() | A | |
| setSpotlightCutOff() | A | |
| ofMaterial() | B | |
| setAmbientColor() | A | |
| setDiffuseColor() | A | |
| setSpecularColor() | A | |
| setEmissiveColor() | A | |
| setShininess() | A | |
| setPBR*() | D | 将来対応予定 |

### 9.7 ofImage / ofPixels / ofTexture

| oF API | レベル | 備考 |
|--------|--------|------|
| ofImage() | A | |
| load() | A | |
| save() | B | PNG/JPEG/TIFFのみ |
| draw() | A | |
| getPixels() | A | |
| getTexture() | A | |
| setFromPixels() | A | |
| resize() | A | |
| crop() | A | |
| rotate90() | A | |
| mirror() | A | |
| setImageType() | A | |
| ofPixels() | A | |
| allocate() | A | |
| setColor() | A | |
| getColor() | A | |
| getData() | A | |
| getWidth() / getHeight() | A | |
| getNumChannels() | A | |
| getBytesPerPixel() | A | |
| ofTexture() | B | |
| allocate() | A | |
| loadData() | A | |
| draw() | A | |
| bind() / unbind() | A | |
| getWidth() / getHeight() | A | |
| setTextureWrap() | A | |
| setTextureMinMagFilter() | A | |
| getTextureData() | B | GL定数は変換 |
| texData.glInternalFormat | D | Metal形式を使用 |

### 9.8 ofFbo

| oF API | レベル | 備考 |
|--------|--------|------|
| ofFbo() | B | |
| allocate() | B | 設定項目に差異 |
| begin() / end() | A | |
| draw() | A | |
| getTexture() | A | |
| readToPixels() | A | |
| getWidth() / getHeight() | A | |
| setActiveDrawBuffer() | B | |
| getNumTextures() | A | |
| checkGLSupport() | D | 常にtrue返却 |

### 9.9 ofTrueTypeFont

| oF API | レベル | 備考 |
|--------|--------|------|
| ofTrueTypeFont() | A | Core Text実装 |
| load() | A | |
| drawString() | A | |
| drawStringAsShapes() | A | |
| getStringBoundingBox() | A | |
| stringWidth() | A | |
| stringHeight() | A | |
| getLineHeight() | A | |
| setLineHeight() | A | |
| getLetterSpacing() | A | |
| setLetterSpacing() | A | |
| getCharacterAsPoints() | A | |
| isLoaded() | A | |
| getFontSize() | A | |

### 9.10 ofPath / ofPolyline

| oF API | レベル | 備考 |
|--------|--------|------|
| ofPath() | A | |
| moveTo() | A | |
| lineTo() | A | |
| curveTo() | A | |
| bezierTo() | A | |
| arc() | A | |
| arcNegative() | A | |
| close() | A | |
| draw() | A | |
| setFilled() | A | |
| setStrokeWidth() | A | |
| setColor() | A | |
| setFillColor() | A | |
| setStrokeColor() | A | |
| getOutline() | A | |
| getTessellation() | A | |
| simplify() | A | |
| translate() | A | |
| rotate() | A | |
| scale() | A | |
| ofPolyline() | A | |
| addVertex() | A | |
| addVertices() | A | |
| lineTo() | A | |
| curveTo() | A | |
| bezierTo() | A | |
| arc() | A | |
| close() | A | |
| draw() | A | |
| getVertices() | A | |
| size() | A | |
| getPerimeter() | A | |
| getArea() | A | |
| getCentroid2D() | A | |
| getBoundingBox() | A | |
| getClosestPoint() | A | |
| inside() | A | |
| simplify() | A | |
| getResampledBySpacing() | A | |
| getResampledByCount() | A | |
| getSmoothed() | A | |

### 9.11 Math

| oF API | レベル | 備考 |
|--------|--------|------|
| ofVec2f | A | simd_float2ラッパー |
| ofVec3f | A | simd_float3ラッパー |
| ofVec4f | A | simd_float4ラッパー |
| ofMatrix4x4 | A | simd_float4x4ラッパー |
| ofQuaternion | A | simd_quatfラッパー |
| ofRandom() | A | |
| ofRandomf() | A | |
| ofRandomuf() | A | |
| ofSeedRandom() | A | |
| ofNoise() | A | |
| ofSignedNoise() | A | |
| ofMap() | A | |
| ofClamp() | A | |
| ofLerp() | A | |
| ofDist() | A | |
| ofDistSquared() | A | |
| ofSign() | A | |
| ofDegToRad() | A | |
| ofRadToDeg() | A | |
| ofWrap() | A | |
| ofWrapDegrees() | A | |
| ofWrapRadians() | A | |
| ofAngleDifferenceDegrees() | A | |
| ofAngleDifferenceRadians() | A | |

### 9.12 Color

| oF API | レベル | 備考 |
|--------|--------|------|
| ofColor | A | |
| ofFloatColor | A | |
| ofShortColor | A | |
| r, g, b, a | A | |
| set() | A | |
| setHex() | A | |
| getHex() | A | |
| setHsb() | A | |
| getHue() | A | |
| getSaturation() | A | |
| getBrightness() | A | |
| getLightness() | A | |
| lerp() | A | |
| getClamped() | A | |
| getInverted() | A | |
| getNormalized() | A | |
| limit() | A | |
| ofColor::fromHex() | A | |
| ofColor::fromHsb() | A | |

### 9.13 App / Events

| oF API | レベル | 備考 |
|--------|--------|------|
| ofBaseApp | A | |
| setup() | A | |
| update() | A | |
| draw() | A | |
| exit() | A | |
| keyPressed() | A | |
| keyReleased() | A | |
| mouseMoved() | A | |
| mouseDragged() | A | |
| mousePressed() | A | |
| mouseReleased() | A | |
| mouseScrolled() | A | |
| mouseEntered() | A | |
| mouseExited() | A | |
| windowResized() | A | |
| dragEvent() | B | |
| gotMessage() | D | |
| ofGetWidth() | A | |
| ofGetHeight() | A | |
| ofGetWindowWidth() | A | |
| ofGetWindowHeight() | A | |
| ofGetFrameRate() | A | |
| ofGetFrameNum() | A | |
| ofGetElapsedTimef() | A | |
| ofGetElapsedTimeMillis() | A | |
| ofSetFrameRate() | A | |
| ofSetWindowTitle() | A | |
| ofSetWindowShape() | A | |
| ofSetWindowPosition() | A | |
| ofSetFullscreen() | A | |
| ofToggleFullscreen() | A | |
| ofGetScreenWidth() | A | |
| ofGetScreenHeight() | A | |
| ofHideCursor() | A | |
| ofShowCursor() | A | |

### 9.14 Utils

| oF API | レベル | 備考 |
|--------|--------|------|
| ofToString() | A | |
| ofToInt() | A | |
| ofToFloat() | A | |
| ofToBool() | A | |
| ofToHex() | A | |
| ofHexToInt() | A | |
| ofSplitString() | A | |
| ofJoinString() | A | |
| ofToLower() | A | |
| ofToUpper() | A | |
| ofTrim() | A | |
| ofLog() | A | |
| ofLogVerbose() | A | |
| ofLogNotice() | A | |
| ofLogWarning() | A | |
| ofLogError() | A | |
| ofLogFatalError() | A | |
| ofSetLogLevel() | A | |
| ofFilePath::getAbsolutePath() | A | |
| ofFilePath::getFileName() | A | |
| ofFilePath::getBaseName() | A | |
| ofFilePath::getFileExt() | A | |
| ofFilePath::getEnclosingDirectory() | A | |
| ofFilePath::join() | A | |
| ofFile | A | |
| ofDirectory | A | |
| ofBuffer | A | |

### 9.15 非対応 (レベルD)

| oF API | 理由 | 代替案 |
|--------|------|--------|
| ofVideoPlayer | 将来対応予定 | AVFoundation直接使用 |
| ofVideoGrabber | 将来対応予定 | AVFoundation直接使用 |
| ofSoundPlayer | 将来対応予定 | AVAudioEngine直接使用 |
| ofSoundStream | 将来対応予定 | AudioUnit直接使用 |
| ofSerial | 将来対応予定 | IOKit直接使用 |
| ofArduino | 将来対応予定 | - |
| ofxiOS* | macOS専用 | - |
| ofxAndroid* | macOS専用 | - |
| ofxEmscripten* | macOS専用 | - |
| ofShader (GLSL) | Metal使用 | .metal シェーダー |
| ofVbo (GL) | Metal使用 | ofMesh / MTLBuffer |
| ofBufferObject | Metal使用 | MTLBuffer |

### 9.16 破壊的変更

| 項目 | oF (OpenGL) | oflike (Metal) | 移行方法 |
|------|-------------|----------------|---------|
| テクスチャ座標原点 | 左下 | **左上** | `v = 1.0 - v` |
| 深度範囲 | -1.0〜1.0 | **0.0〜1.0** | シェーダー修正 |
| 行列順序 | 列優先 | 列優先 | 変更なし |
| GLSL | サポート | **非対応** | MSLに書き換え |
| GL定数 | GL_* | 内部変換 | そのまま使用可 |

---

## 10. デバッグ機能方針

### 10.1 デバッグGUI

| 方式 | 優先度 | 用途 |
|------|--------|------|
| **SwiftUI Overlay** | 推奨 | 開発時パラメータ調整 |
| **OSC外部連携** | 推奨 | TouchOSC/iPad連携 |
| AppKit Debug Panel | **禁止** | SwiftUI統一方針 |
| Dear ImGui | **禁止** | ネイティブ方針違反 |

### 10.2 SwiftUI Debug Overlay 仕様

- NSHostingController + NSPanel（内部実装のみ）
- @Observable でパラメータ同期 (macOS 14+) または @ObservedObject
- C++ → Swift ブリッジは Objective-C++ 経由
- DEBUGビルドのみ有効化
- .ultraThinMaterial 背景

### 10.3 OSC連携 仕様

- 受信ポート: 8000（デフォルト）
- 送信ポート: 9000（デフォルト）
- 対応アプリ: TouchOSC, Open Stage Control, Protokol
- 実装は将来対応（優先度: 中）

### 10.4 プロファイリング

| ツール | 用途 |
|--------|------|
| Instruments | CPU/GPU プロファイリング |
| Metal System Trace | GPU詳細分析 |
| Metal GPU Capture | シェーダーデバッグ |
| os_signpost | カスタム計測ポイント |

### 10.5 ログ出力

```
ofLogVerbose()    → os_log (DEBUG レベル)
ofLogNotice()     → os_log (INFO レベル)
ofLogWarning()    → os_log (DEFAULT レベル)
ofLogError()      → os_log (ERROR レベル)
ofLogFatalError() → os_log (FAULT レベル) + abort()
```

- Console.app で統合閲覧可能
- `OSLog` / `os_signpost` を内部使用

---

## 11. ウィンドウ・イベント方針

### 11.1 ウィンドウ管理

```swift
// SwiftUI ベースのウィンドウ
@main
struct OFLikeApp: App {
    var body: some Scene {
        WindowGroup {
            MetalView()
                .frame(minWidth: 800, minHeight: 600)
        }
    }
}
```

### 11.2 MTKView統合

```swift
// SwiftUI でラップした MTKView
struct MetalView: NSViewRepresentable {
    func makeNSView(context: Context) -> MTKView { ... }
    func updateNSView(_ nsView: MTKView, context: Context) { ... }
}
```

### 11.3 イベント処理

| イベント | SwiftUI API |
|---------|------------|
| マウス移動 | `.onContinuousHover` |
| マウスクリック | `.onTapGesture`, `.gesture(DragGesture())` |
| キーボード | `.onKeyPress` (macOS 14+) / `.focusedValue` |
| スクロール | `.onScrollGeometryChange` / `ScrollView` |
| ドラッグ&ドロップ | `.dropDestination`, `.draggable` |
| ウィンドウリサイズ | `GeometryReader` / `.onChange(of: geometry)` |

### 11.4 C++へのイベント伝達

```
SwiftUI Event → Swift Callback → Obj-C++ Bridge → C++ EventDispatcher
```

---

## 12. 命名規則

### 12.1 ファイル名

| 種類 | 規則 | 例 |
|------|------|-----|
| C++ ヘッダー | PascalCase.h | `MetalRenderer.h` |
| C++ 実装 | PascalCase.cpp | `DrawList.cpp` |
| Objective-C++ | PascalCase.mm | `MetalRenderer.mm` |
| Swift | PascalCase.swift | `MetalView.swift` |
| Metal シェーダー | PascalCase.metal | `Basic2D.metal` |
| oflike 互換 | of + PascalCase | `ofGraphics.h` |

### 12.2 クラス・関数名

| 種類 | 規則 | 例 |
|------|------|-----|
| C++ クラス | PascalCase | `MetalRenderer` |
| C++ 関数 | camelCase | `loadTexture()` |
| C++ メンバ変数 | camelCase_ (末尾_) | `device_` |
| C++ 定数 | UPPER_SNAKE | `MAX_TEXTURES` |
| C++ 名前空間 | lowercase | `render::metal` |
| oflike関数 | of + PascalCase | `ofDrawCircle()` |
| Swift クラス | PascalCase | `RenderCoordinator` |
| Swift 関数 | camelCase | `setupMetal()` |
| Swift プロパティ | camelCase | `isRunning` |

### 12.3 Objective-C

| 種類 | 規則 | 例 |
|------|------|-----|
| クラス | プレフィックス + PascalCase | `OFLBridge` |
| メソッド | camelCase | `- (void)renderFrame` |
| プロパティ | camelCase | `@property device` |

---

## Appendix A: 画像読み込みフロー

```
ofLoadImage(ofTexture&, path)
  └─ MTKTextureLoader (直接GPU読み込み) ← 最速
      └─ 失敗時: ImageIO (CGImageSource) ← フォールバック

ofLoadImage(ofPixels&, path)
  └─ ImageIO (CGImageSource) ← ハードウェアアクセラレーション
```

---

## Appendix B: フォントレンダリングフロー

```
ofTrueTypeFont::load()
  └─ CTFontCreateWithName() ← Core Text

ofTrueTypeFont::drawString()
  └─ CTLineDraw() → CGBitmapContext → MTLTexture

ofTrueTypeFont::getCharacterAsPoints()
  └─ CTFontCreatePathForGlyph() → CGPath → ofPath
```

---

## Appendix C: 変更履歴

| 日付 | バージョン | 変更内容 |
|------|-----------|---------|
| 2025-01-22 | 1.0.0 | 初版作成 |
| 2025-01-23 | 1.1.0 | GLM移行計画、エラー処理、スレッドルール追加 |
| 2025-01-23 | 2.0.0 | SwiftUI統一、AppKit禁止、プロジェクト概要追加 |