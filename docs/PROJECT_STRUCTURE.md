# oflike-metal プロジェクト構造

## 概要

oflike-metalは、openFrameworksのAPIをmacOS Metal GPUで再実装したフレームワークです。

---

## ディレクトリ構造

```
oflike_metal_skeleton/
├── src/                      # ソースコード
│   ├── core/                 # コアシステム
│   ├── math/                 # 数学ライブラリ
│   ├── platform/             # プラットフォーム固有
│   ├── render/               # レンダリングエンジン
│   ├── oflike/               # openFrameworks互換API
│   └── addons/               # アドオン
├── shaders/                  # Metal シェーダー
├── examples/                 # サンプルプロジェクト
├── third_party/              # サードパーティライブラリ
├── tests/                    # テスト
├── tools/                    # ツール
├── scripts/                  # スクリプト
├── build/                    # CMakeビルド出力
└── docs/                     # ドキュメント
```

---

## ソースコード詳細 (`src/`)

### `src/core/` - コアシステム

| ファイル | 説明 |
|----------|------|
| `AppBase.h` | アプリケーション基底クラス（`ofBaseApp`） |
| `Context.h/.mm` | グローバルコンテキスト（ウィンドウサイズ、フレームレート等） |
| `Engine.h/.mm` | メインエンジン |
| `EventDispatcher.h/.mm` | イベントディスパッチャー |

### `src/math/` - 数学ライブラリ

| ファイル | 説明 |
|----------|------|
| `Types.h` | 基本型定義（simd使用） |

### `src/platform/` - プラットフォーム

```
platform/
├── bridge/
│   ├── SwiftBridge.h/.mm     # Swift連携ブリッジ
│   └── oflike-metal-Bridging-Header.h
└── swiftui/                  # SwiftUI統合（将来用）
```

### `src/render/` - レンダリングエンジン

```
render/
├── IRenderer.h               # レンダラーインターフェース
├── DrawCommand.h/.cpp        # 描画コマンド
├── DrawList.h/.cpp           # 描画リスト
├── RenderTypes.h/.mm         # レンダリング型定義
└── metal/
    ├── MetalRenderer.h/.mm   # Metal レンダラー実装
    ├── MetalBuffer.h/.mm     # Metal バッファ管理
    └── MetalTexture.h/.mm    # Metal テクスチャ管理
```

### `src/oflike/` - openFrameworks互換API

```
oflike/
├── app/
│   └── ofMain.h              # メインエントリポイント（Cocoa/Metal boilerplate）
├── graphics/
│   ├── ofGraphics.h/.cpp     # 描画関数（ofDrawCircle, ofDrawRect等）
│   ├── ofFbo.h/.mm           # フレームバッファオブジェクト
│   ├── ofPath.h/.cpp         # パス描画
│   ├── ofPolyline.h/.cpp     # ポリライン
│   └── ofTrueTypeFont.h/.mm  # フォント描画
├── image/
│   ├── ofImage.h/.mm         # 画像読み込み・描画
│   ├── ofTexture.h/.mm       # テクスチャ
│   └── ofPixels.h            # ピクセルデータ
├── 3d/
│   ├── ofCamera.h/.cpp       # カメラ
│   ├── ofEasyCam.h/.cpp      # イージーカメラ
│   └── ofMesh.h/.cpp         # メッシュ
├── lighting/
│   ├── ofLight.h/.cpp        # ライト
│   ├── ofMaterial.h/.cpp     # マテリアル
│   └── ofLightingSystem.h/.cpp # ライティングシステム
├── math/
│   ├── ofVec2f.h             # 2Dベクトル
│   ├── ofVec3f.h             # 3Dベクトル
│   ├── ofVec4f.h             # 4Dベクトル
│   ├── ofMatrix4x4.h         # 4x4行列
│   ├── ofQuaternion.h        # クォータニオン
│   └── ofMath.h              # 数学関数
├── types/
│   ├── ofColor.h             # 色（ofColor, ofFloatColor）
│   └── ofParameter.h/.cpp    # パラメーター
└── utils/
    ├── ofUtils.h/.cpp        # ユーティリティ関数
    ├── ofFile.h/.mm          # ファイル操作
    ├── ofDirectory.h/.mm     # ディレクトリ操作
    ├── ofFilePath.h/.mm      # パス操作
    ├── ofBuffer.h/.mm        # バッファ
    ├── ofLog.h/.mm           # ログ出力
    └── ofOSC.h/.mm           # OSC（内部用）
```

### `src/addons/` - アドオン

```
addons/
├── core/                     # コアアドオン（常に含む）
│   ├── ofxGui/               # GUI
│   ├── ofxOsc/               # OSC通信
│   ├── ofxNetwork/           # TCP/UDP通信
│   ├── ofxXmlSettings/       # XML設定
│   ├── ofxSvg/               # SVG読み込み
│   └── ofxOpenCv/            # OpenCV（Vision.framework使用）
└── apple_native/             # Apple専用アドオン
    └── ofxSharp/             # 3D Gaussian Splatting（開発中）
```

---

## シェーダー (`shaders/`)

| ファイル | 説明 |
|----------|------|
| `Common.h` | 共通構造体・定数 |
| `Basic2D.metal` | 2D描画シェーダー |
| `Basic3D.metal` | 3D描画シェーダー |
| `Textured.metal` | テクスチャ描画シェーダー |
| `Lighting.metal` | ライティングシェーダー |

---

## サードパーティライブラリ (`third_party/`)

| ライブラリ | 説明 |
|------------|------|
| `tess2` | ポリゴンテセレーション |
| `utf8` | UTF-8文字列処理 |
| `oscpack` | OSCプロトコル |
| `pugixml` | XML解析 |
| `nanosvg` | SVG解析 |

---

## Examples (`examples/`)

| Example | 説明 |
|---------|------|
| `01_basics` | 基本描画 |
| `02_shapes` | 図形描画 |
| `03_color` | 色 |
| `04_image` | 画像 |
| `05_typography` | フォント |
| `06_3d_primitives` | 3Dプリミティブ |
| `07_camera` | カメラ |
| `08_lighting` | ライティング |
| `09_mesh` | メッシュ |
| `10_fbo` | FBO |
| `11_events` | イベント |
| `12_easycam` | EasyCam |
| `13_osc_sender` | OSC送信 |
| `14_osc_receiver` | OSC受信 |
| `15_gui_basic` | GUI基本 |
| `16_xml_settings` | XML設定 |
| `17_svg_loader` | SVG読み込み |
| `18_tcp_server` | TCPサーバー |

---

## ビルド構造

### CMakeビルド (`build/`)
```bash
mkdir build && cd build
cmake ..
make -j8
```

### Xcodeビルド (`xcode/`)
```bash
mkdir xcode && cd xcode
cmake -G Xcode ..
open oflike-metal.xcodeproj
```

### Example単独Xcode (`examples/*/`)
```bash
open examples/01_basics/01_basics.xcodeproj
```

---

## 主要API

### 描画関数
```cpp
ofBackground(r, g, b);
ofSetColor(r, g, b, a);
ofDrawCircle(x, y, radius);
ofDrawRectangle(x, y, w, h);
ofDrawTriangle(x1, y1, x2, y2, x3, y3);
ofDrawLine(x1, y1, x2, y2);
ofFill();
ofNoFill();
```

### ユーティリティ
```cpp
ofGetWidth();
ofGetHeight();
ofGetFrameRate();
ofSetFrameRate(fps);
ofRandom(min, max);
ofMap(value, inMin, inMax, outMin, outMax);
```

### アプリケーション
```cpp
class ofApp : public ofBaseApp {
    void setup() override;
    void update() override;
    void draw() override;
    void keyPressed(int key) override;
    void mousePressed(int x, int y, int button) override;
};
```

---

## 依存関係

```
                    ┌─────────────────┐
                    │   Application   │
                    │   (ofApp)       │
                    └────────┬────────┘
                             │
                    ┌────────▼────────┐
                    │  oflike API     │
                    │  (ofGraphics,   │
                    │   ofImage...)   │
                    └────────┬────────┘
                             │
              ┌──────────────┼──────────────┐
              │              │              │
     ┌────────▼────────┐ ┌───▼───┐ ┌───────▼───────┐
     │  Core           │ │ Math  │ │   Addons      │
     │  (Context,      │ │       │ │  (ofxOsc,     │
     │   AppBase)      │ │       │ │   ofxGui...)  │
     └────────┬────────┘ └───────┘ └───────────────┘
              │
     ┌────────▼────────┐
     │  Render         │
     │  (MetalRenderer)│
     └────────┬────────┘
              │
     ┌────────▼────────┐
     │  Metal API      │
     │  (GPU)          │
     └─────────────────┘
```
