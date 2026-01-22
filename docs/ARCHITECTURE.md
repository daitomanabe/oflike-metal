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
