# oflike-metal Architecture - Absolute Policies

**このファイルはプロジェクトの絶対的な方針を定義します。**
**全ての実装はこの方針に従う必要があります。例外はありません。**

---

## 絶対的な方針: macOS専用・ネイティブAPI優先

**決定日**: 2025-01-22

このプロジェクトは**macOS専用**です。全てのAPIはmacOSに特化して設計・実装してください。

---

## 禁止事項

### 1. クロスプラットフォームライブラリは使用禁止

| 禁止 | 代替（必須） |
|------|------------|
| FreeType | Core Text |
| stb_image | ImageIO (CGImageSource) / MTKTextureLoader |
| OpenGL | Metal |
| SDL / GLFW | Cocoa / AppKit |
| 外部SIMDライブラリ | Accelerate |

### 2. 外部依存は最小限に

- macOSネイティブフレームワークで実現可能な機能に外部ライブラリを使用しない
- glmは行列計算のみ許可（simdへの移行を検討中）

---

## 必須フレームワーク

| フレームワーク | 用途 | 優先度 |
|--------------|------|--------|
| **Metal** | GPU描画、シェーダー | 最高 |
| **MetalKit** | テクスチャローダー、ビュー | 最高 |
| **Core Text** | フォント読み込み・レンダリング | 最高 |
| **Core Graphics** | パス、ビットマップコンテキスト | 高 |
| **ImageIO** | 画像ファイル読み込み | 高 |
| **Accelerate** | SIMD演算、画像処理 (vImage, vDSP) | 中 |
| **QuartzCore** | Core Animation | 中 |
| **Cocoa** | ウィンドウ、イベント | 高 |

---

## パフォーマンス最適化の優先順位

### GPU処理
```
Metal GPU処理 > Accelerate SIMD > ソフトウェア実装
```

### 画像読み込み
```
MTKTextureLoader (最速) > ImageIO > NSImage
```

### フォントレンダリング
```
Core Text (必須) - FreeTypeは禁止
```

---

## 画像読み込みの実装方針

```
ofLoadImage(ofTexture&, path)
  └─ MTKTextureLoader (直接GPU読み込み、CPUコピーなし) ← 最速
      └─ 失敗時: ImageIO (CGImageSource) ← フォールバック

ofLoadImage(ofPixels&, path)
  └─ ImageIO (CGImageSource) ← ハードウェアアクセラレーション
```

---

## フォントレンダリングの実装方針

### Core Text使用（FreeType禁止）

```objc
// テキスト描画
CTFontRef font = CTFontCreateWithName(...);
CTLineDraw(line, cgContext);
// テクスチャにアップロードしてMetal描画

// コンター取得
CGPathRef path = CTFontCreatePathForGlyph(font, glyph, NULL);
// CGPathをofPathに変換
```

### 警告
- **ofxTrueTypeFontは使用禁止** - 代わりにofTrueTypeFont（内部でCore Text使用）を使用すること
- FreeType系のコードが出てきた場合はCore Textベースの実装に置き換えること

---

## レイヤー責務境界

### 概要図

```
┌─────────────────────────────────────────────────────────────────┐
│                      User Application                           │
│                   (examples/, ofApp.h)                          │
└─────────────────────────────────────────────────────────────────┘
                              │ 呼び出し
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                    oflike/ (oF互換API層)                        │
│  責務: openFrameworks互換のAPIを提供                            │
│  禁止: Metal/GPU直接操作、レンダリングロジック                   │
├─────────────────────────────────────────────────────────────────┤
│  oflike/graphics/ofGraphics.h                                   │
│    - ofDrawCircle(), ofSetColor(), ofPushMatrix() 等            │
│    - DrawListへコマンド発行のみ                                 │
│                                                                 │
│  oflike/3d/ofCamera.h                                           │
│    - View/Projection行列の計算                                  │
│    - begin()/end()でDrawListに行列をpush/pop                    │
│                                                                 │
│  oflike/image/ofTexture.h                                       │
│    - テクスチャの論理的管理                                     │
│    - 内部でMetalTexture.mmを呼び出し（pImpl）                   │
└─────────────────────────────────────────────────────────────────┘
                              │ コマンド発行
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                  render/DrawList.h (抽象化層)                   │
│  責務: 描画コマンドのバッファリング・状態管理                    │
│  禁止: Metal API直接呼び出し、プラットフォーム固有コード         │
├─────────────────────────────────────────────────────────────────┤
│  DrawCommand構造体                                              │
│    - 頂点データ (positions, colors, texCoords, normals)         │
│    - インデックスデータ                                         │
│    - プリミティブタイプ (triangles, lines, points)              │
│    - テクスチャバインド情報                                     │
│                                                                 │
│  状態スタック                                                   │
│    - 行列スタック (Model, View, Projection)                     │
│    - スタイルスタック (fillColor, strokeColor, lineWidth)       │
│    - ライティング状態 (lights[], materials[])                   │
│    - ブレンドモード、深度テスト状態                             │
│                                                                 │
│  バッチング                                                     │
│    - 同一テクスチャ・同一状態のコマンドをマージ                 │
│    - 描画順序の最適化（不透明→半透明）                          │
└─────────────────────────────────────────────────────────────────┘
                              │ 実行
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│              render/metal/MetalRenderer.mm (実装層)             │
│  責務: DrawListをMetal APIで実行                                │
│  許可: Metal API、Objective-C++、GPUリソース管理                │
├─────────────────────────────────────────────────────────────────┤
│  GPUリソース管理                                                │
│    - MTLDevice, MTLCommandQueue                                 │
│    - MTLRenderPipelineState (シェーダー)                        │
│    - MTLDepthStencilState                                       │
│    - MTLBuffer (頂点/インデックス/Uniform)                      │
│                                                                 │
│  描画実行                                                       │
│    - DrawList → MTLRenderCommandEncoder                         │
│    - setVertexBuffer, setFragmentTexture                        │
│    - drawPrimitives, drawIndexedPrimitives                      │
│                                                                 │
│  テクスチャ管理 (MetalTexture.mm)                               │
│    - MTLTexture生成・破棄                                       │
│    - MTKTextureLoader経由の読み込み                             │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                    macOS Native Frameworks                      │
│          Metal, MetalKit, Core Graphics, Core Text, etc.        │
└─────────────────────────────────────────────────────────────────┘
```

### 各層の責務詳細

#### oflike/ 層
| やること | やらないこと |
|----------|--------------|
| oF互換APIの提供 | Metal API呼び出し |
| DrawListへのコマンド発行 | GPUバッファ管理 |
| 行列計算（GLM使用） | シェーダー操作 |
| パス→ポリライン変換 | ピクセルフォーマット変換 |
| ユーザー入力の正規化 | プラットフォーム固有処理 |

#### render/DrawList 層
| やること | やらないこと |
|----------|--------------|
| 描画コマンドの蓄積 | Metal/GL API呼び出し |
| 状態スタック管理 | テクスチャ読み込み |
| バッチング・ソート | ウィンドウ管理 |
| 頂点データの集約 | シェーダーコンパイル |

#### render/metal/ 層
| やること | やらないこと |
|----------|--------------|
| Metal API呼び出し | oF API定義 |
| GPUリソース管理 | 描画ロジック（何を描くか） |
| シェーダーバインド | 行列計算 |
| テクスチャアップロード | パス処理 |

### コード例

```cpp
// ❌ 悪い例: oflike/でMetal直接操作
// oflike/graphics/ofGraphics.h
void ofDrawCircle(float x, float y, float r) {
    id<MTLBuffer> buffer = ...;  // NG: Metal直接操作
    [encoder drawPrimitives:...]; // NG
}

// ✅ 良い例: oflike/からDrawListへ発行
// oflike/graphics/ofGraphics.h
void ofDrawCircle(float x, float y, float r) {
    auto& dl = getDrawList();
    dl.addCircle(x, y, r, dl.getCurrentStyle());
}

// render/DrawList.cpp
void DrawList::addCircle(float x, float y, float r, const Style& style) {
    // 頂点生成してコマンドに追加
    DrawCommand cmd;
    cmd.primitiveType = PrimitiveType::TriangleFan;
    cmd.vertices = generateCircleVertices(x, y, r, style.circleResolution);
    cmd.style = style;
    commands_.push_back(cmd);
}

// render/metal/MetalRenderer.mm
void MetalRenderer::executeDrawList(const DrawList& dl) {
    for (const auto& cmd : dl.getCommands()) {
        // Metal APIで実際に描画
        [encoder setVertexBytes:cmd.vertices.data() ...];
        [encoder drawPrimitives:toMTLPrimitiveType(cmd.primitiveType) ...];
    }
}
```

### 依存方向の原則

```
     依存OK          依存NG
        │               │
        ▼               ▼
  oflike/ ───────> render/DrawList ───────> render/metal/
        │                    │
        │                    │
        └──────> third_party/glm
                     │
                     ▼
               (header-only)

  ※ render/metal/ から oflike/ への依存は禁止
  ※ DrawList から Metal への依存は禁止
  ※ 循環依存は禁止
```

---

## 実装パターン

### pImplパターン
Objective-C++をパブリックヘッダーから隔離する

```cpp
// ヘッダー (.h)
class MyClass {
private:
    void* impl_;  // Objective-C型を隠蔽
};

// 実装 (.mm)
id<MTLTexture> tex = (__bridge id<MTLTexture>)impl_;
```

### メモリ管理
- ARC互換の`__bridge`、`__bridge_retained`、`__bridge_transfer`を使用
- `CFRelease`で参照カウントを管理

---

## openFrameworks API互換性

### 方針
- openFrameworksのAPIシグネチャを可能な限り維持
- 内部実装はMetal/macOSに最適化
- 完全互換ではなく、必要な機能のサブセットを実装

### 変更点
- GL定数は内部でMetalに変換
- テクスチャ座標は正規化座標を使用

---

## 変更履歴

| 日付 | 決定事項 |
|------|----------|
| 2025-01-22 | macOS専用方針を絶対的なルールとして制定 |
| 2025-01-22 | Core Text使用、FreeType禁止 |
| 2025-01-22 | MTKTextureLoader優先使用（直接GPU読み込み） |
| 2025-01-22 | ImageIO使用（NSImageより高速） |
