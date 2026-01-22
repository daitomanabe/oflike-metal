# Implementation Status

実装の進捗状況を追跡するファイルです。

---

## 実装ステータス一覧

### 2D グラフィックス

| API | 状態 | ファイル |
|-----|------|----------|
| ofMesh | ✅ 完了 | `src/oflike/3d/ofMesh.h` |
| ofPath | ✅ 完了 | `src/oflike/graphics/ofPath.h` |
| ofPolyline | ✅ 完了 | `src/oflike/types/ofPolyline.h` |
| ofRectangle | ✅ 完了 | `src/oflike/types/ofRectangle.h` |
| ofBackgroundGradient | ✅ 完了 | `src/oflike/graphics/ofGraphics.h` |
| ofEnableDepthTest | ✅ 完了 | `src/oflike/graphics/ofGraphics.h` |
| ofRotateXRad/YRad/ZRad | ✅ 完了 (2D) | `src/oflike/graphics/ofGraphics.h` |

### 画像・テクスチャ

| API | 状態 | ファイル | 備考 |
|-----|------|----------|------|
| ofTexture | ✅ 完了 | `src/oflike/image/ofTexture.h` | Metal実装 |
| ofPixels | ✅ 完了 | `src/oflike/image/ofTexture.h` | |
| ofLoadImage(ofTexture&) | ✅ 完了 | `src/render/metal/MetalTexture.mm` | MTKTextureLoader使用 |
| ofLoadImage(ofPixels&) | ✅ 完了 | `src/render/metal/MetalTexture.mm` | ImageIO使用 |
| ofSetTextureWrap | ✅ 完了 | `src/oflike/image/ofTexture.h` | |

### フォント

| API | 状態 | ファイル | 備考 |
|-----|------|----------|------|
| ofTrueTypeFont | ✅ 完了 | `src/oflike/graphics/ofTrueTypeFont.h` | Core Text使用 |
| drawString | ✅ 完了 | | テクスチャベース |
| drawStringAsShapes | ✅ 完了 | | ベクター描画 |
| getStringAsPoints | ✅ 完了 | | パス取得 |
| getStringBoundingBox | ✅ 完了 | | |

### 色

| API | 状態 | ファイル |
|-----|------|----------|
| ofColor | ✅ 完了 | `src/oflike/graphics/ofColor.h` |
| ofFloatColor | ✅ 完了 | `src/oflike/graphics/ofColor.h` |

---

## 3D API（未実装）

### 基盤

| API | 状態 | 詳細 |
|-----|------|------|
| ofNode | ⬜ 未実装 | 3D変換ノード |
| ofCamera | ⬜ 未実装 | View/Projection管理 |
| ofEasyCam | ⬜ 未実装 | マウス操作カメラ |
| 3D行列スタック | ⬜ 未実装 | View/Projection管理 |

### 3Dプリミティブ

| API | 状態 |
|-----|------|
| of3dPrimitive | ⬜ 未実装 |
| ofBoxPrimitive | ⬜ 未実装 |
| ofSpherePrimitive | ⬜ 未実装 |
| ofCylinderPrimitive | ⬜ 未実装 |
| ofConePrimitive | ⬜ 未実装 |
| ofPlanePrimitive | ⬜ 未実装 |
| ofIcoSpherePrimitive | ⬜ 未実装 |

### ライティング

| API | 状態 |
|-----|------|
| ofLight | ⬜ 未実装 |
| ofMaterial | ⬜ 未実装 |
| ofEnableLighting | ⬜ 未実装 |

### 3D描画関数

| API | 状態 |
|-----|------|
| ofDrawSphere | ⬜ 未実装 |
| ofDrawBox | ⬜ 未実装 |
| ofDrawCone | ⬜ 未実装 |
| ofDrawCylinder | ⬜ 未実装 |
| ofDrawPlane | ⬜ 未実装 |
| ofDrawGrid | ⬜ 未実装 |
| ofDrawAxis | ⬜ 未実装 |

---

## 実装フェーズ

### Phase 1: 2D基盤 ✅ 完了
- ofRectangle
- ofPrimitiveMode enum
- 深度テスト関数
- 回転関数（Rad版）
- ofBackgroundGradient

### Phase 2: ofMesh ✅ 完了
- 頂点・色・インデックス管理
- LINE_STRIP / LINE_LOOP描画
- Metal統合

### Phase 3: ofPath ✅ 完了
- コマンド記録
- ポリライン生成
- 描画（DrawList利用）

### Phase 4: テクスチャ ✅ 完了
- ofPixels拡張
- ofTexture（Metal実装）
- ofLoadImage（ImageIO/MTKTextureLoader）

### Phase 5: フォント ✅ 完了
- Core Text統合
- テクスチャアトラス生成
- コンターモード

### Phase 6: 3D基盤 ⬜ 未着手
1. ofNode
2. 3D行列スタック
3. ofCamera
4. ofEasyCam

### Phase 7: 3Dプリミティブ ⬜ 未着手
1. of3dPrimitive
2. ofBoxPrimitive, ofSpherePrimitive
3. その他プリミティブ

### Phase 8: ライティング ⬜ 未着手
1. ofLight
2. ofMaterial
3. Phongシェーダー（Metal）

---

## 実装済みファイル一覧

### ソースファイル

```
src/
├── core/
│   ├── Context.h
│   └── Types.h
├── render/
│   ├── DrawList.h
│   ├── DrawList.cpp
│   └── metal/
│       ├── MetalRenderer.h
│       ├── MetalRenderer.mm
│       ├── MetalTexture.mm
│       └── MetalImage.mm
└── oflike/
    ├── ofMain.h
    ├── graphics/
    │   ├── ofGraphics.h
    │   ├── ofColor.h
    │   ├── ofPath.h
    │   └── ofTrueTypeFont.h/.mm
    ├── image/
    │   └── ofTexture.h
    ├── types/
    │   ├── ofRectangle.h
    │   └── ofPolyline.h
    └── 3d/
        └── ofMesh.h
```

### サンプル

```
examples/
├── 00_basic/
├── 01_particles/
├── 02_polylines/
├── 03_image/
├── 04_typography/
├── 05_3d_primitives/
├── 06_graphics/
├── 07_color/
├── 08_polygon/
├── 09_lines/
└── 10_blending/
```

---

## 凡例

| 記号 | 意味 |
|------|------|
| ✅ | 完了 |
| 🔄 | 進行中 |
| ⬜ | 未着手 |
