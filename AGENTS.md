```markdown
# CLAUDE.md - AI開発コンテキスト

このファイルはAIがプロジェクトを理解し、開発を進めるためのコンテキストです。

---

## プロジェクト概要

**oflike-metal**: openFrameworks の macOS ネイティブ再実装

| 柱 | 内容 |
|---|------|
| SwiftUI | ウィンドウ、イベント、UI |
| Metal | GPUレンダリング |
| C++記述 | アプリロジックはC++ |
| oF API互換 | openFrameworks API維持 |

---

## 技術スタック

```
ユーザーコード (C++) → oflike API → render → Metal → SwiftUI → macOS
```

---

## 重要ファイル

| ファイル | 役割 |
|---------|------|
| docs/ARCHITECTURE.md | 絶対方針（必読） |
| docs/CHECKLIST.md | 開発進捗チェックリスト |
| src/oflike/ | oF互換API実装 |
| src/render/metal/ | Metal実装 |
| src/platform/swiftui/ | SwiftUI実装 |

---

## 絶対禁止事項

- AppKit 直接使用（SwiftUI経由のみ）
- OpenGL / GLSL
- FreeType（Core Text使用）
- Dear ImGui（SwiftUI Overlay使用）
- クロスプラットフォームライブラリ

---

## 推奨事項

- simd/simd.h を使用（GLMは非推奨）
- pImplパターンでObj-C++を隠蔽
- MTKTextureLoaderで画像読み込み
- Core Textでフォント処理

---

## 開発の進め方

1. docs/CHECKLIST.md を確認
2. 現在のPhaseのタスクを実行
3. 完了したら `[x]` を付ける
4. 次のタスクへ

---

## 座標系

- 2D: 左上原点（Metal標準）
- 3D: 右手座標系（oF互換）
- テクスチャ: 左上原点

---

## 命名規則

| 種類 | 規則 | 例 |
|------|------|-----|
| C++クラス | PascalCase | MetalRenderer |
| C++関数 | camelCase | loadTexture() |
| メンバ変数 | camelCase_ | device_ |
| oflike関数 | ofPascalCase | ofDrawCircle() |
| Swift | PascalCase | MetalView |

---

```
