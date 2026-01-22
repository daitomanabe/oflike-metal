# oflike-metal Documentation Index

このファイルは全てのドキュメントの索引です。新しいセッションでは必ずこのファイルを最初に参照してください。

---

## ドキュメント構成

```
docs/
├── INDEX.md              ← 現在のファイル（全体の索引）
├── ARCHITECTURE.md       ← 絶対的な方針・禁止事項
├── IMPLEMENTATION.md     ← 実装ステータス一覧
├── CHANGELOG.md          ← 変更履歴
├── api/                  ← API設計書
│   ├── CORE.md           ← ofNode, ofCamera, ofEasyCam
│   ├── GRAPHICS_2D.md    ← ofPath, ofPolyline, ofGraphics関数
│   ├── GRAPHICS_3D.md    ← of3dPrimitive, 3D描画関数
│   ├── MESH.md           ← ofMesh, ofVboMesh
│   ├── LIGHTING.md       ← ofLight, ofMaterial
│   ├── IMAGE.md          ← ofTexture, ofPixels, ofLoadImage
│   └── TYPOGRAPHY.md     ← ofTrueTypeFont
└── metal/                ← Metal実装詳細
    ├── PIPELINE.md       ← レンダリングパイプライン設計
    └── SHADERS.md        ← シェーダー設計・構造体
```

---

## 各ファイルの役割

### ARCHITECTURE.md
**必読** - プロジェクトの絶対的な方針を定義

- macOS専用の方針
- 禁止されているライブラリ（FreeType, stb_image, OpenGL等）
- 使用すべきmacOSネイティブフレームワーク

### IMPLEMENTATION.md
実装の進捗状況を追跡

- 各APIの実装ステータス（✅完了 / ⬜未実装）
- 実装フェーズの定義
- 実装済みファイル一覧

### api/*.md
各API領域の詳細設計

| ファイル | 内容 |
|---------|------|
| CORE.md | 3D空間の基盤（ofNode, ofCamera, ofEasyCam） |
| GRAPHICS_2D.md | 2Dグラフィックス（ofPath, ofPolyline, 描画関数） |
| GRAPHICS_3D.md | 3Dプリミティブ（of3dPrimitive, ofDrawBox等） |
| MESH.md | メッシュ管理（ofMesh, ofVboMesh） |
| LIGHTING.md | ライティング（ofLight, ofMaterial） |
| IMAGE.md | 画像処理（ofTexture, ofPixels, ofLoadImage） |
| TYPOGRAPHY.md | フォント（ofTrueTypeFont） |

### metal/*.md
Metal実装の技術詳細

| ファイル | 内容 |
|---------|------|
| PIPELINE.md | レンダリングパイプライン、深度バッファ、ブレンディング |
| SHADERS.md | シェーダー構造体、Uniforms、ライティング計算 |

---

## クイックリファレンス

### 絶対的なルール（ARCHITECTURE.mdより）

```
✅ 使用する                    ❌ 禁止
Metal                         OpenGL
Core Text                     FreeType
ImageIO/MTKTextureLoader      stb_image
Cocoa/AppKit                  SDL/GLFW
Accelerate                    外部SIMD
```

### 実装ワークフロー

1. **openFrameworks exampleを確認** - 必要なAPIを把握
2. **ARCHITECTURE.md を確認** - 方針に従っているか確認
3. **api/*.md に設計を記述** - API仕様を定義
4. **macOS最適実装を検討** - ネイティブAPIを最大活用
5. **実装** - 設計に従って実装
6. **IMPLEMENTATION.md を更新** - ステータスを更新

---

## 関連ファイル（ソースコード）

### コア
- `src/core/Context.h` - グローバルエンジンコンテキスト
- `src/core/Types.h` - 基本型定義

### レンダリング
- `src/render/DrawList.h` - 描画コマンドリスト
- `src/render/metal/MetalRenderer.mm` - Metalレンダラー
- `src/render/metal/MetalTexture.mm` - テクスチャ実装

### グラフィックス
- `src/oflike/graphics/ofGraphics.h` - 描画関数
- `src/oflike/graphics/ofPath.h` - パスクラス
- `src/oflike/graphics/ofColor.h` - 色クラス
- `src/oflike/graphics/ofTrueTypeFont.h` - フォントクラス

### 画像
- `src/oflike/image/ofTexture.h` - テクスチャクラス

### 3D
- `src/oflike/3d/ofMesh.h` - メッシュクラス

### 型
- `src/oflike/types/ofRectangle.h` - 矩形クラス
- `src/oflike/types/ofPolyline.h` - ポリラインクラス
