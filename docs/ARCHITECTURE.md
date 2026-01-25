# oflike-metal Architecture - Absolute Policies

**Version**: 3.3.0
**Last Updated**: 2026-01-25
**Status**: Active

> このドキュメントはプロジェクトの絶対的な方針を定義します。  
> 全ての実装はこの方針に従う必要があります。  
> 変更には全メンバーの合意が必要です。

---

## 1. プロジェクト概要

### 1.1 ミッション

**openFrameworks の macOS ネイティブ再実装 + Apple Native Addons**

- **外見**: openFrameworks API 互換
- **中身**: SwiftUI + Metal + Apple Native Frameworks
- **記述**: C++
- **拡張**: Apple Silicon / Neural Engine / Metal の能力を最大限活用

### 1.2 5つの柱

| 柱 | 内容 |
|---|------|
| **SwiftUI** | ウィンドウ、イベント、UI (デフォルト) |
| **Metal** | GPUレンダリング、シェーダー、Compute |
| **C++記述** | アプリケーションロジックはC++で記述 |
| **oF API互換** | openFrameworks の API シグネチャを維持 |
| **Apple Native** | Neural Engine, Core ML, Vision, PHASE 等を活用 |

### 1.3 エントリーポイント方針

**SwiftUI がデフォルト、ofMain は互換性のためのレガシーパス**

oflike-metal は2つのアプリケーションエントリーポイントを提供します:

#### 1.3.1 SwiftUI Entry (デフォルト・推奨)

**これがデフォルトです。** 新規プロジェクトは SwiftUI エントリーを使用してください。

```swift
// App.swift - SwiftUI Entry Point (推奨)
import SwiftUI

@main
struct MyApp: App {
    @StateObject private var appState = AppState()

    var body: some Scene {
        WindowGroup {
            MetalView(appState: appState)
                .frame(minWidth: 800, minHeight: 600)
        }
        .windowStyle(.titleBar)
        .windowToolbarStyle(.unified)
    }
}
```

**利点**:
- ✅ **モダンな macOS アプリ**: SwiftUI によるネイティブな UI/UX
- ✅ **フル機能**: 複数ウィンドウ、設定画面、メニューバー、ツールバー
- ✅ **統合**: SwiftUI コンポーネントと C++ レンダリングの組み合わせ
- ✅ **将来性**: Apple の推奨する UI フレームワーク

#### 1.3.2 ofMain Entry (レガシー・互換性)

**openFrameworks 互換性のみのために提供。** 既存の oF プロジェクトの移行用です。

```cpp
// main.mm - ofMain Entry Point (レガシー)
#include <oflike/ofMain.h>
#include "MyApp.h"

int main() {
    ofRunApp<MyApp>(1024, 768, "My App");
    return 0;
}
```

**制約**:
- ⚠️ **単一ウィンドウのみ**: 複数ウィンドウ不可
- ⚠️ **限定的なUI**: SwiftUI コンポーネント統合不可
- ⚠️ **レガシー**: 新機能は SwiftUI エントリーで優先実装
- ⚠️ **将来性**: メンテナンスモードのみ

**使用すべき場合**:
- 既存の openFrameworks プロジェクトを最小限の変更で移行する場合
- 単純なフルスクリーン描画アプリ (VJ, インスタレーション)
- 一時的な互換性ブリッジとして使用し、後で SwiftUI に移行予定

**使用すべきでない場合**:
- 新規プロジェクト → SwiftUI Entry を使用
- 複数ウィンドウが必要 → SwiftUI Entry を使用
- SwiftUI コンポーネント統合 → SwiftUI Entry を使用
- macOS ネイティブ UI/UX → SwiftUI Entry を使用

### 1.4 技術スタック

```
┌─────────────────────────────────────────────────────────────────────┐
│                      Application (User Code)                         │
│                         Pure C++ / oflike API                        │
└─────────────────────────────────────────────────────────────────────┘
                                    │
┌───────────────────────────────────┼───────────────────────────────────┐
│                                   ▼                                   │
│  ┌─────────────────────────────────────────────────────────────────┐ │
│  │                    oflike/ (oF互換API)                           │ │
│  │        ofDrawCircle, ofImage, ofCamera, ofMesh...               │ │
│  └─────────────────────────────────────────────────────────────────┘ │
│                                   │                                   │
│  ┌─────────────────────────────────────────────────────────────────┐ │
│  │                     Addons Layer                                 │ │
│  ├──────────────────────┬──────────────────────────────────────────┤ │
│  │   Core Addons        │      Apple Native Addons                 │ │
│  │   ────────────────   │      ────────────────────                │ │
│  │   ofxOsc             │      ofxSharp (3DGS)                     │ │
│  │   ofxGui             │      ofxNeuralEngine                     │ │
│  │   ofxXmlSettings     │      ofxMetalCompute                     │ │
│  │   ofxSvg             │      ofxMPS                              │ │
│  │   ofxNetwork         │      ofxVideoToolbox                     │ │
│  │   ofxOpenCv          │      ofxSpatialAudio                     │ │
│  │                      │      ofxMetalFX                          │ │
│  │                      │      ofxUnifiedMemory                    │ │
│  │                      │      ofxLiveText                         │ │
│  │                      │      ofxObjectCapture                    │ │
│  └──────────────────────┴──────────────────────────────────────────┘ │
│                                   │                                   │
│  ┌─────────────────────────────────────────────────────────────────┐ │
│  │                      render/ (描画抽象化)                         │ │
│  │           DrawList, DrawCommand, Types, Buffers                 │ │
│  └─────────────────────────────────────────────────────────────────┘ │
│                                   │                                   │
│  ┌─────────────────────────────────────────────────────────────────┐ │
│  │                     render/metal/ (Metal実装)                    │ │
│  │       MetalRenderer, MetalTexture, Shaders, Compute             │ │
│  └─────────────────────────────────────────────────────────────────┘ │
│                                   │                                   │
│  ┌─────────────────────────────────────────────────────────────────┐ │
│  │                    platform/swiftui/ (UI層)                      │ │
│  │            SwiftUI Window, MTKView, Events                      │ │
│  └─────────────────────────────────────────────────────────────────┘ │
└───────────────────────────────────┼───────────────────────────────────┘
                                    │
┌───────────────────────────────────┼───────────────────────────────────┐
│                                   ▼                                   │
│  ┌─────────────────────────────────────────────────────────────────┐ │
│  │                     Apple Frameworks                             │ │
│  │  Metal │ Core ML │ Vision │ PHASE │ VideoToolbox │ MetalFX      │ │
│  └─────────────────────────────────────────────────────────────────┘ │
│                                   │                                   │
│  ┌─────────────────────────────────────────────────────────────────┐ │
│  │                    Apple Silicon Hardware                        │ │
│  │      CPU Cores │ GPU Cores │ Neural Engine │ Media Engine       │ │
│  └─────────────────────────────────────────────────────────────────┘ │
└───────────────────────────────────────────────────────────────────────┘
```

### 1.5 プラットフォーム要件

- **macOS専用** (最小サポート: macOS 13.0 Ventura)
- **Apple Silicon最適化** (Intel互換は維持)
- **Neural Engine活用** (Apple Silicon のみ)
- **macOS 14 依存機能はオプション化** (VisionKit などは機能有効時のみ)

### 1.6 座標系とレイヤー責任

oflike-metal は openFrameworks 互換の座標系を維持しながら、Metal への変換は renderer 層で処理します。

#### 1.6.1 座標系定義

**2D 座標系** (oflike API):
- **原点**: 左上 (0, 0)
- **X軸**: 右方向が正 (+X)
- **Y軸**: 下方向が正 (+Y)
- **範囲**: (0, 0) から (width, height)

```cpp
// oflike API - 左上原点
ofDrawCircle(100, 100, 50);  // 左上から (100, 100) の位置
```

**3D 座標系** (oflike API):
- **座標系**: 右手座標系 (openFrameworks 互換)
- **X軸**: 右方向が正
- **Y軸**: 上方向が正
- **Z軸**: 手前方向が正 (右手系)

```cpp
// oflike API - 右手座標系
ofVec3f pos(1, 1, 1);  // X右, Y上, Z手前
camera.setPosition(0, 0, 10);  // カメラは Z+ 方向
camera.lookAt(ofVec3f(0, 0, 0));  // 原点を見る
```

**Metal NDC** (Metal 内部):
- **X軸**: -1 (左) ～ +1 (右)
- **Y軸**: -1 (下) ～ +1 (上) ※OpenGL と逆
- **Z軸**: 0 (near) ～ 1 (far) ※OpenGL は -1～+1

#### 1.6.2 レイヤー責任の分離

座標変換の責任は明確に分離されています:

```
┌─────────────────────────────────────────────────────────────────────┐
│  oflike Layer (User Code)                                            │
│  ─────────────────────────                                           │
│  • 2D: 左上原点 (0,0)～(width,height)                                 │
│  • 3D: 右手座標系                                                     │
│  • Metal の座標系を意識しない                                         │
│  • openFrameworks と同じコード                                        │
│                                                                      │
│  例:                                                                  │
│    ofDrawCircle(100, 100, 50);  // 左上から (100,100)                │
│    camera.setPosition(0, 0, 10);  // 右手系                          │
│                                                                      │
└───────────────────────────────┬──────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────────┐
│  Render Abstraction Layer                                            │
│  ─────────────────────────────                                       │
│  • DrawCommand にユーザー座標系のまま格納                              │
│  • 座標変換は実装しない                                               │
│  • API非依存の中間表現                                                │
│                                                                      │
└───────────────────────────────┬──────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────────┐
│  Metal Renderer Layer (Implementation)                               │
│  ──────────────────────────────────────                              │
│  ✅ ここで座標変換を実行 (全ての変換責任がここにある)                   │
│                                                                      │
│  【2D 変換】                                                          │
│  1. 左上原点 (0, height) → Metal NDC (-1, +1)                        │
│  2. Y軸反転: oF の Y↓ → Metal の Y↑                                  │
│                                                                      │
│     Projection Matrix (2D):                                          │
│     [  2/w    0      0   -1 ]                                        │
│     [   0   -2/h    0    1 ]  ← Y を反転して左上原点に               │
│     [   0     0     1    0 ]                                         │
│     [   0     0     0    1 ]                                         │
│                                                                      │
│  【3D 変換】                                                          │
│  1. 右手座標系 → Metal NDC                                            │
│  2. Y軸方向: oF (Y↑) → Metal (Y↑) ※3Dは同じ                         │
│  3. Z範囲変換: oF/OpenGL (-1～+1) → Metal (0～1)                     │
│                                                                      │
│     Projection Matrix (3D):                                          │
│     Metal の perspective projection を使用                            │
│     Z範囲を 0～1 に調整                                               │
│                                                                      │
│  【実装箇所】                                                          │
│  • MetalRenderer::setup2DProjection()                                │
│  • MetalRenderer::setup3DProjection(camera)                          │
│  • Vertex Shader で最終変換                                           │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

#### 1.6.3 実装パターン

**2D 座標変換 (MetalRenderer 実装)**:
```cpp
// src/render/metal/MetalRenderer.mm
void MetalRenderer::setup2DProjection(int width, int height) {
    // 左上原点 (0,0) を Metal NDC (-1, +1) に変換
    // Y軸を反転して左上原点にする
    float ortho[16] = {
        2.0f / width,  0.0f,           0.0f, -1.0f,
        0.0f,         -2.0f / height,  0.0f,  1.0f,  // Y反転
        0.0f,          0.0f,           1.0f,  0.0f,
        0.0f,          0.0f,           0.0f,  1.0f
    };
    // Uniform バッファに設定
}
```

**3D 座標変換 (MetalRenderer 実装)**:
```cpp
// src/render/metal/MetalRenderer.mm
void MetalRenderer::setup3DProjection(const ofCamera& camera) {
    // ofCamera の view/projection 行列を取得 (右手座標系)
    simd_float4x4 view = camera.getViewMatrix().toSimd();
    simd_float4x4 proj = camera.getProjectionMatrix().toSimd();

    // Metal 用に Z 範囲を 0～1 に調整
    simd_float4x4 metalProj = adjustZRangeForMetal(proj);

    // Uniform バッファに設定
}
```

**Vertex Shader (Metal 実装)**:
```metal
// src/render/metal/Shaders.metal
vertex VertexOut vertex2D(
    VertexIn in [[stage_in]],
    constant Uniforms& uniforms [[buffer(1)]]
) {
    VertexOut out;
    // oflike の左上原点座標をそのまま受け取る
    // Projection Matrix で Metal NDC に変換
    out.position = uniforms.projectionMatrix * float4(in.position, 1.0);
    return out;
}
```

#### 1.6.4 ユーザーコードへの影響

**ユーザーは座標変換を意識しない**:
```cpp
// ofApp.cpp - ユーザーコード
void ofApp::draw() {
    // 2D: 左上原点で描画 (openFrameworks と同じ)
    ofDrawCircle(100, 100, 50);

    // 3D: 右手座標系で描画 (openFrameworks と同じ)
    camera.setPosition(0, 0, 10);
    camera.begin();
    ofDrawBox(0, 0, 0, 2, 2, 2);
    camera.end();

    // Metal NDC/Y軸反転は renderer が自動処理
}
```

#### 1.6.5 テスト検証

座標系の正確性を検証するためのテスト:

```cpp
// 左上が (0,0) であることを確認
ofSetColor(255, 0, 0);
ofDrawCircle(0, 0, 10);  // 左上隅に赤い円

// 右下が (width, height) であることを確認
ofSetColor(0, 0, 255);
ofDrawCircle(ofGetWidth(), ofGetHeight(), 10);  // 右下隅に青い円

// 3D 右手座標系を確認
ofDrawAxis(100);  // X赤(右), Y緑(上), Z青(手前)
```

#### 1.6.6 まとめ

| 項目 | oflike Layer | Renderer Layer |
|------|-------------|----------------|
| **2D 座標系** | 左上原点 (0, 0) ～ (width, height) | Metal NDC + Y反転処理 |
| **3D 座標系** | 右手座標系 (Y↑, Z手前) | Metal NDC + Z範囲調整 |
| **責任** | oF互換APIを提供 | Metal変換を実装 |
| **ユーザーコード** | openFrameworks と同じ | 変換を意識しない |
| **変換処理** | なし | Projection Matrix + Shader |

---

## 2. レイヤー境界 (Layer Boundaries)

### 2.1 レイヤー分離の原則

oflike-metal は明確なレイヤー境界を持ち、各レイヤーは以下の制約に従います:

```
┌─────────────────────────────────────────────────────────────────────┐
│                      Layer Boundaries                                │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  Application Layer (User Code)                                       │
│  ├─ Depends on: oflike/*                                            │
│  └─ Cannot access: render/*, platform/*                             │
│                           ▼                                          │
│  oflike Layer (oF API Compatible)                                    │
│  ├─ Path: src/oflike/**/*                                           │
│  ├─ Depends on: render/DrawList, render/DrawCommand, render/Types   │
│  ├─ Cannot include: render/metal/*, platform/*                      │
│  └─ Metal code: ❌ FORBIDDEN                                         │
│                           ▼                                          │
│  Render Abstraction Layer                                            │
│  ├─ Path: src/render/ (excluding src/render/metal/)                │
│  ├─ Files: DrawList.h, DrawCommand.h, RenderTypes.h                │
│  ├─ Purpose: GPU API 非依存の描画コマンド抽象化                        │
│  └─ Metal code: ❌ FORBIDDEN                                         │
│                           ▼                                          │
│  Metal Implementation Layer                                          │
│  ├─ Path: src/render/metal/*                                        │
│  ├─ Files: MetalRenderer.h/.mm, MetalTexture.h/.mm, etc.           │
│  ├─ Purpose: Metal 固有の実装                                        │
│  └─ Metal code: ✅ ONLY HERE                                         │
│                           ▼                                          │
│  Platform Layer (SwiftUI + Bridge)                                   │
│  ├─ Path: src/platform/swiftui/*, src/platform/bridge/*            │
│  └─ Purpose: OS/UI 統合                                              │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### 2.2 具体的な制約

#### 2.2.1 oflike Layer

**許可される依存:**
- `src/render/DrawList.h` - 描画コマンドリスト
- `src/render/DrawCommand.h` - 描画コマンド定義
- `src/render/RenderTypes.h` - レンダリング型定義
- `src/core/Context.h` - グローバル状態
- `src/math/*` - 数学ライブラリ

**禁止される依存:**
- ❌ `src/render/metal/*` - Metal 実装への直接アクセス
- ❌ `#import <Metal/Metal.h>` - Metal フレームワークの直接使用
- ❌ `#import <MetalKit/MetalKit.h>` - MetalKit フレームワークの直接使用
- ❌ `id<MTLDevice>`, `id<MTLTexture>` 等の Metal 型の直接使用
- ❌ `src/platform/*` - プラットフォーム層への直接アクセス

**例外:**
- `src/oflike/image/ofTexture.mm` - テクスチャ作成時のみ Context 経由で device にアクセス可能
- `src/oflike/graphics/ofFbo.mm` - FBO 作成時のみ Context 経由で device にアクセス可能

#### 2.2.2 Render Abstraction Layer

**役割:**
- GPU API 非依存の描画コマンド定義
- DrawList によるコマンドバッファリング
- レンダリング状態の抽象化

**禁止:**
- ❌ Metal 固有の実装
- ❌ `#import <Metal/Metal.h>`

#### 2.2.3 Metal Implementation Layer

**役割:**
- Metal 固有の実装を集約
- DrawList の Metal への変換
- Metal リソース管理

**場所:**
- ✅ `src/render/metal/MetalRenderer.h/.mm`
- ✅ `src/render/metal/MetalTexture.h/.mm`
- ✅ `src/render/metal/MetalPipeline.h/.mm`
- ✅ `src/render/metal/Shaders.metal`

**これ以外の場所で Metal コードを書くことは禁止**

### 2.3 Context 経由のアクセスパターン

oflike レイヤーが Metal リソース (device, renderer) にアクセスする場合は、必ず Context を経由する:

```cpp
// ✅ CORRECT: Context 経由でアクセス
#include "ofTexture.h"
#include "Context.h"

void ofTexture::allocate(int w, int h) {
    auto* ctx = Context::Get();
    auto* renderer = ctx->renderer();
    // renderer 経由で Metal リソースを取得
}
```

```cpp
// ❌ WRONG: Metal を直接使用
#import <Metal/Metal.h>

void ofTexture::allocate(int w, int h) {
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();  // ❌ 禁止
}
```

### 2.4 検証方法

レイヤー境界違反を検出するための検証コマンド:

```bash
# oflike レイヤーで Metal ヘッダーを include していないか確認
grep -r "#import <Metal" src/oflike/

# oflike レイヤーで render/metal を include していないか確認
grep -r "#include \"render/metal" src/oflike/

# oflike レイヤーで Metal 型を使用していないか確認
grep -r "id<MTL" src/oflike/

# render/ (metal を除く) で Metal を使用していないか確認
grep -r "#import <Metal" src/render/*.h src/render/*.cpp
```

**期待される結果:** すべて 0 件

---

## 3. 依存関係ポリシー

### 3.1 必須フレームワーク (macOS 13.0)

| フレームワーク | 用途 | 最小バージョン |
|--------------|------|---------------|
| **Metal** | GPUレンダリング | macOS 13.0 |
| **MetalKit** | MTKView、テクスチャローダー | macOS 13.0 |
| **MetalFX** | アップスケーリング | macOS 13.0 |
| **Metal Performance Shaders** | 画像処理 | macOS 13.0 |
| **Core ML** | ML推論 | macOS 13.0 |
| **Vision** | 画像認識 | macOS 13.0 |
| **PHASE** | 空間オーディオ | macOS 13.0 |
| **VideoToolbox** | ビデオエンコード | macOS 13.0 |
| **SwiftUI** | ウィンドウ、UI、イベント | macOS 13.0 |
| **Core Text** | フォント | macOS 13.0 |
| **Core Graphics** | 2Dパス、ビットマップ | macOS 13.0 |
| **ImageIO** | 画像I/O | macOS 13.0 |
| **Accelerate** | SIMD、画像処理 | macOS 13.0 |
| **Network** | TCP/UDP | macOS 13.0 |
| **Model I/O** | 3Dモデル | macOS 13.0 |

### 3.1.1 条件付きフレームワーク (機能有効時)

| フレームワーク | 用途 | 最小バージョン | 備考 |
|--------------|------|---------------|------|
| **VisionKit** | OCR / LiveText | macOS 14.0 | ofxLiveText 有効時のみ |

### 3.2 禁止ライブラリ

| ライブラリ | 代替 | 例外 |
|-----------|------|------|
| FreeType | Core Text | なし |
| OpenGL | Metal | なし |
| SDL / GLFW | SwiftUI + MTKView | なし |
| stb_image (読込) | ImageIO / MTKTextureLoader | なし |
| stb_image_write | CGImageDestination | EXR/HDRのみ許可 |
| Dear ImGui | SwiftUI Overlay | Debug用のみ許可 |
| 外部SIMDライブラリ | simd/simd.h, Accelerate | なし |
| Poco | Foundation / Network.framework | なし |
| Cairo | Core Graphics | なし |
| Assimp | Model I/O | なし |
| **AppKit (直接使用)** | SwiftUI | 低レベルブリッジのみ許可 |

### 3.3 許可サードパーティ

| ライブラリ | 用途 | ライセンス | 状態 |
|-----------|------|-----------|------|
| tess2 | ポリゴン三角形分割 | SGI FSL B | ✅ Active |
| utf8-cpp | UTF-8処理 | Boost | ✅ Active |
| oscpack | OSC プロトコル | MIT | ✅ Active |
| nanosvg | SVG パース | Zlib | ✅ Active |
| pugixml | XML パース | MIT | ✅ Active |
| Dear ImGui | デバッグUI | MIT | ✅ Optional |
| OpenCV | 高度なCV | Apache/BSD | ✅ Optional |

---

## 4. Addons アーキテクチャ

### 4.1 Addon 分類

```
┌─────────────────────────────────────────────────────────────────────┐
│                         Addons                                       │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ┌─────────────────────────────────────────────────────────────────┐│
│  │  Core Addons (oF互換)                                           ││
│  │  ─────────────────────                                          ││
│  │  oF addon を Mac Native に移植                                   ││
│  │                                                                  ││
│  │  • ofxOsc          - OSC 通信 (oscpack)                         ││
│  │  • ofxGui          - GUI (SwiftUI)                              ││
│  │  • ofxXmlSettings  - XML 設定 (pugixml)                         ││
│  │  • ofxSvg          - SVG 読み込み (nanosvg + Core Graphics)     ││
│  │  • ofxNetwork      - ネットワーク (Network.framework)            ││
│  │  • ofxOpenCv       - CV (Vision.framework + OpenCV)             ││
│  └─────────────────────────────────────────────────────────────────┘│
│                                                                      │
│  ┌─────────────────────────────────────────────────────────────────┐│
│  │  Apple Native Addons (新規)                                     ││
│  │  ───────────────────────────                                    ││
│  │  Apple Silicon / macOS の能力を最大限活用                        ││
│  │                                                                  ││
│  │  ⭐ ofxSharp         - 3D Gaussian Splatting (Core ML)          ││
│  │  • ofxNeuralEngine   - ML推論 (Core ML / Vision)                ││
│  │  • ofxMetalCompute   - GPU Compute (Metal)                      ││
│  │  • ofxMPS            - 画像処理 (Metal Performance Shaders)     ││
│  │  • ofxVideoToolbox   - 動画エンコード (VideoToolbox)            ││
│  │  • ofxSpatialAudio   - 空間オーディオ (PHASE)                   ││
│  │  • ofxMetalFX        - AI アップスケール (MetalFX)              ││
│  │  • ofxUnifiedMemory  - ゼロコピーバッファ (Metal)               ││
│  │  • ofxLiveText       - OCR (VisionKit)                          ││
│  │  • ofxObjectCapture  - 写真→3D (Object Capture API)             ││
│  └─────────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────────┘
```

### 4.2 ofxSharp アーキテクチャ (⭐ 最優先)

**目的**: 単一画像から3D Gaussian Splatting を 1秒以内で生成

**Reference**: https://github.com/apple/ml-sharp

```
┌─────────────────────────────────────────────────────────────────────┐
│                      ofxSharp Pipeline                               │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ┌──────────┐    ┌───────────────┐    ┌──────────────┐    ┌───────┐│
│  │  Single  │───▶│  SHARP Model  │───▶│  Gaussian    │───▶│ Metal ││
│  │  Image   │    │  (Core ML)    │    │  Cloud       │    │Render ││
│  └──────────┘    └───────────────┘    └──────────────┘    └───────┘│
│                          │                     │                    │
│                   ┌──────┴──────┐       ┌──────┴──────┐            │
│                   │ Neural      │       │ GPU Sorted  │            │
│                   │ Engine      │       │ Alpha Blend │            │
│                   └─────────────┘       └─────────────┘            │
│                                                                      │
│  Classes:                                                            │
│  ─────────                                                          │
│  • Sharp::GaussianCloud   - 3DGS データコンテナ                     │
│  • Sharp::SharpModel      - Core ML 推論                            │
│  • Sharp::GaussianRenderer- Metal レンダリング                      │
│  • Sharp::SharpScene      - シーン管理                              │
│  • Sharp::CameraPath      - カメラ軌道                              │
│  • Sharp::VideoExporter   - 動画出力                                │
│  • ofxSharp               - ラッパークラス                          │
│                                                                      │
│  Performance Target:                                                 │
│  ───────────────────                                                │
│  • Inference: < 1 second (Neural Engine)                            │
│  • Rendering: 60+ FPS (Metal)                                       │
│  • Memory: Unified Memory (Zero-copy)                               │
└─────────────────────────────────────────────────────────────────────┘
```

### 4.3 Addon ディレクトリ構造

```
src/
├── addons/
│   ├── core/                    # Core Addons
│   │   ├── ofxOsc/
│   │   │   ├── ofxOsc.h
│   │   │   ├── ofxOsc.mm
│   │   │   └── README.md
│   │   ├── ofxGui/
│   │   ├── ofxXmlSettings/
│   │   ├── ofxSvg/
│   │   ├── ofxNetwork/
│   │   └── ofxOpenCv/
│   │
│   └── native/                  # Apple Native Addons
│       ├── ofxSharp/
│       │   ├── ofxSharp.h
│       │   ├── ofxSharp.mm
│       │   ├── Sharp/
│       │   │   ├── GaussianCloud.h/.mm
│       │   │   ├── SharpModel.h/.mm
│       │   │   ├── GaussianRenderer.h/.mm
│       │   │   ├── SharpScene.h/.mm
│       │   │   ├── CameraPath.h/.mm
│       │   │   └── VideoExporter.h/.mm
│       │   ├── shaders/
│       │   │   └── GaussianSplat.metal
│       │   ├── models/
│       │   │   └── sharp.mlmodelc
│       │   └── README.md
│       ├── ofxNeuralEngine/
│       ├── ofxMetalCompute/
│       ├── ofxMPS/
│       ├── ofxVideoToolbox/
│       ├── ofxSpatialAudio/
│       ├── ofxMetalFX/
│       ├── ofxUnifiedMemory/
│       ├── ofxLiveText/
│       └── ofxObjectCapture/
```

### 4.4 Core Addons 移植方針

| oF Addon | 元の依存 | Mac Native 置換 |
|----------|---------|-----------------|
| ofxOsc | oscpack | oscpack (継続) |
| ofxGui | 独自実装 | SwiftUI |
| ofxXmlSettings | Poco XML | pugixml |
| ofxSvg | svgTiny | nanosvg + Core Graphics |
| ofxNetwork | Poco | Network.framework |
| ofxOpenCv | OpenCV | Vision.framework + OpenCV |
| ofxAssimpModelLoader | Assimp | Model I/O |
| ofxThreadedImageLoader | FreeImage | GCD + ImageIO |
| ofxVectorGraphics | Cairo | Core Graphics (PDF) |

---

## 5. 実装パターン

### 5.1 pImpl パターン (Pure C++ Header)

```cpp
// ofxSharp.h - Pure C++ header
#pragma once
#include <memory>
#include <string>
#include "ofPixels.h"
#include "ofCamera.h"

namespace Sharp {
    class GaussianCloud;
}

class ofxSharp {
public:
    ofxSharp();
    ~ofxSharp();
    
    void setup(int width, int height);
    Sharp::GaussianCloud generateFromImage(const ofPixels& image);
    void drawCloud(const Sharp::GaussianCloud& cloud, const ofCamera& camera);
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
```

### 5.2 Objective-C++ 実装

```objc
// ofxSharp.mm
#import <Metal/Metal.h>
#import <CoreML/CoreML.h>
#include "ofxSharp.h"

class ofxSharp::Impl {
    id<MTLDevice> device_;
    MLModel* sharpModel_;
    // ...
    
public:
    void setup(int width, int height) {
        @autoreleasepool {
            device_ = MTLCreateSystemDefaultDevice();
            // ...
        }
    }
};
```

### 5.3 Core ML 統合パターン

```objc
// NeuralEngine optimized loading
MLModelConfiguration* config = [[MLModelConfiguration alloc] init];
config.computeUnits = MLComputeUnitsAll;  // Prefer Neural Engine

NSError* error = nil;
MLModel* model = [MLModel modelWithContentsOfURL:url 
                                   configuration:config 
                                           error:&error];
```

### 5.4 Metal Compute パターン

```objc
// Compute shader dispatch
id<MTLCommandBuffer> buffer = [queue_ commandBuffer];
id<MTLComputeCommandEncoder> encoder = [buffer computeCommandEncoder];

[encoder setComputePipelineState:pipeline_];
[encoder setBuffer:dataBuffer_ offset:0 atIndex:0];

MTLSize threadgroups = MTLSizeMake(count / 256, 1, 1);
MTLSize threadsPerGroup = MTLSizeMake(256, 1, 1);
[encoder dispatchThreadgroups:threadgroups threadsPerThreadgroup:threadsPerGroup];

[encoder endEncoding];
[buffer commit];
```

---

## 6. Apple Silicon 最適化

### 6.1 ハードウェア活用

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Apple Silicon Utilization                         │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ┌─────────────┐   ┌─────────────┐   ┌─────────────┐   ┌──────────┐│
│  │   CPU       │   │   GPU       │   │  Neural     │   │  Media   ││
│  │   Cores     │   │   Cores     │   │  Engine     │   │  Engine  ││
│  └──────┬──────┘   └──────┬──────┘   └──────┬──────┘   └────┬─────┘│
│         │                 │                 │                │      │
│    App Logic         Metal Render      Core ML           VideoToolbox
│    File I/O          Compute Shader    Vision            ProRes
│    Audio DSP         MPS Filters       ofxSharp          4K/8K
│                      ofxMetalCompute   ofxNeuralEngine             │
│         │                 │                 │                │      │
│         └─────────────────┴─────────────────┴────────────────┘      │
│                                    │                                 │
│                         ┌─────────┴─────────┐                       │
│                         │  Unified Memory   │                       │
│                         │  (Zero-Copy)      │                       │
│                         └───────────────────┘                       │
└─────────────────────────────────────────────────────────────────────┘
```

### 6.2 Unified Memory 活用

```cpp
// ofxUnifiedBuffer - CPU/GPU Zero-copy
template<typename T>
class ofxUnifiedBuffer {
public:
    void allocate(size_t count) {
        buffer_ = [device_ newBufferWithLength:count * sizeof(T)
                                       options:MTLResourceStorageModeShared];
    }
    
    T* data() {
        return static_cast<T*>(buffer_.contents);  // Direct CPU access
    }
    
    id<MTLBuffer> getMetalBuffer() {
        return buffer_;  // Direct GPU access, no copy
    }
};
```

### 6.3 Neural Engine 最適化

```cpp
// Core ML model with Neural Engine preference
MLModelConfiguration* config = [[MLModelConfiguration alloc] init];
config.computeUnits = MLComputeUnitsAll;  // Includes Neural Engine

// Float16 for better Neural Engine performance
// Model should be converted with float16 weights
```

---

## 7. API 互換性

### 7.1 Core Addon API 互換

Core Addons は openFrameworks の API を維持:

```cpp
// oF 互換
ofxOscSender sender;
sender.setup("localhost", 8000);

ofxOscMessage msg;
msg.setAddress("/test");
msg.addFloatArg(0.5f);
sender.sendMessage(msg);
```

### 7.2 Apple Native Addon API 設計

Apple Native Addons は新規設計だが、oF スタイルを踏襲:

```cpp
// oF スタイルを踏襲した新規 API
ofxSharp sharp;
sharp.setup(1920, 1080);

auto cloud = sharp.generateFromImage(myImage);
sharp.drawCloud(cloud, camera);
```

---

## 8. ビルド設定

### 8.1 CMake 構成

```cmake
# Addons
option(OFLIKE_ADDON_OSC "Build ofxOsc" ON)
option(OFLIKE_ADDON_GUI "Build ofxGui" ON)
option(OFLIKE_ADDON_SHARP "Build ofxSharp" ON)
option(OFLIKE_ADDON_NEURAL "Build ofxNeuralEngine" ON)
option(OFLIKE_ADDON_COMPUTE "Build ofxMetalCompute" ON)

# Framework linking
target_link_libraries(oflike-metal
    "-framework Metal"
    "-framework MetalKit"
    "-framework MetalPerformanceShaders"
    "-framework MetalFX"
    "-framework CoreML"
    "-framework Vision"
    "-framework VisionKit"        # Optional (macOS 14+, ofxLiveText)
    "-framework PHASE"
    "-framework VideoToolbox"
    "-framework AVFoundation"
    "-framework Network"
    "-framework ModelIO"
)
```

### 8.2 Xcode 設定

```
Build Settings (推奨):
  - C++ Language Dialect: C++20 (最小: C++17)
  - Enable Modules: YES
  - Metal Compiler:
    - Language Revision: Metal 3.1 (推奨)
    - Fast Math: YES
```

---

## 9. 命名規則

### 9.1 Addon 命名

| 種類 | 規則 | 例 |
|------|------|-----|
| Core Addon | ofx + PascalCase | `ofxOsc`, `ofxGui` |
| Native Addon | ofx + Feature | `ofxSharp`, `ofxNeuralEngine` |
| 内部名前空間 | PascalCase | `Sharp::`, `Neural::` |
| クラス | PascalCase | `GaussianCloud`, `SharpModel` |

### 9.2 ファイル命名

| 種類 | 規則 | 例 |
|------|------|-----|
| Addon ヘッダー | ofx + Name.h | `ofxSharp.h` |
| Addon 実装 | ofx + Name.mm | `ofxSharp.mm` |
| 内部クラス | Name.h/.mm | `GaussianRenderer.mm` |
| Metal シェーダー | Name.metal | `GaussianSplat.metal` |

---

## 10. デバッグ・プロファイリング

### 10.1 Metal デバッグ

```cpp
// Metal GPU Capture
MTLCaptureManager* capture = [MTLCaptureManager sharedCaptureManager];
MTLCaptureDescriptor* desc = [[MTLCaptureDescriptor alloc] init];
desc.captureObject = device_;
[capture startCaptureWithDescriptor:desc error:nil];
```

### 10.2 Core ML プロファイリング

```cpp
// Instruments で Core ML Trace を使用
// Neural Engine 使用率を確認
```

### 10.3 ログ出力

```cpp
// os_log 統合
#import <os/log.h>

os_log_debug(OS_LOG_DEFAULT, "ofxSharp: Inference time: %.2f ms", inferenceTime);
os_log_error(OS_LOG_DEFAULT, "ofxSharp: Failed to load model");
```

---

## 11. 変更履歴

| 日付 | バージョン | 変更内容 |
|------|-----------|---------|
| 2025-01-22 | 1.0.0 | 初版作成 |
| 2025-01-23 | 2.0.0 | SwiftUI統一、AppKit禁止 |
| 2026-01-23 | 3.0.0 | Apple Native Addons追加、ofxSharp設計 |
| 2026-01-25 | 3.1.0 | レイヤー境界の明確化 (Phase 0.1) |
| 2026-01-25 | 3.2.0 | SwiftUIデフォルト、ofMainレガシー明記 (Phase 0.2) |
| 2026-01-25 | 3.3.0 | 座標系責任の明確化、Metal変換はrenderer層 (Phase 0.3) |

---

## Appendix A: Addon 依存関係図

```
oflike-metal (core)
    │
    ├── Core Addons
    │   ├── ofxOsc ───────────── oscpack (MIT)
    │   ├── ofxGui ───────────── SwiftUI
    │   ├── ofxXmlSettings ───── pugixml (MIT)
    │   ├── ofxSvg ───────────── nanosvg (Zlib)
    │   ├── ofxNetwork ───────── Network.framework
    │   └── ofxOpenCv ────────── Vision.framework + OpenCV (optional)
    │
    └── Apple Native Addons
        ├── ofxSharp ─────────── Core ML + Metal
        ├── ofxNeuralEngine ──── Core ML + Vision
        ├── ofxMetalCompute ──── Metal
        ├── ofxMPS ───────────── MetalPerformanceShaders
        ├── ofxVideoToolbox ──── VideoToolbox + AVFoundation
        ├── ofxSpatialAudio ──── PHASE
        ├── ofxMetalFX ───────── MetalFX
        ├── ofxUnifiedMemory ─── Metal (Unified Memory)
        ├── ofxLiveText ──────── VisionKit
        └── ofxObjectCapture ─── Object Capture API
```

---

## Appendix B: macOS バージョン互換性

| Addon | macOS 13 | macOS 14 | macOS 15 |
|-------|----------|----------|----------|
| ofxOsc | ✅ | ✅ | ✅ |
| ofxGui | ✅ | ✅ | ✅ |
| ofxSharp | ✅ | ✅ | ✅ |
| ofxNeuralEngine | ✅ | ✅ | ✅ |
| ofxMetalCompute | ✅ | ✅ | ✅ |
| ofxMPS | ✅ | ✅ | ✅ |
| ofxVideoToolbox | ✅ | ✅ | ✅ |
| ofxSpatialAudio | ✅ | ✅ | ✅ |
| ofxMetalFX | ✅ | ✅ | ✅ |
| ofxLiveText | ❌ | ✅ | ✅ |
| ofxObjectCapture | ✅ | ✅ | ✅ |
