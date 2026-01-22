# Implementation Status

openFrameworks v0.12.1 からの移植進捗を追跡するファイルです。

参照元: `references/of_v0.12.1_osx_release/libs/openFrameworks/`

---

## ⚠️ 移植ワークフロー（必ず従うこと）

### Step 1: oFソースを読む
```bash
# 例: ofMath.h を移植する場合
cat references/of_v0.12.1_osx_release/libs/openFrameworks/math/ofMath.h
cat references/of_v0.12.1_osx_release/libs/openFrameworks/math/ofMath.cpp
```

### Step 2: ARCHITECTURE.md を確認
```bash
cat docs/ARCHITECTURE.md
```
→ 禁止ライブラリを使っていないか確認
→ 代替フレームワークを特定

### Step 3: 変換ルールを適用

#### グラフィックス・レンダリング
| oFで使用 | Metal版で使用 | 備考 |
|----------|---------------|------|
| OpenGL (gl*, GL_*) | Metal (MTL*, id<MTL*>) | レンダリングAPI |
| GLシェーダー (GLSL) | Metal Shading Language (.metal) | シェーダー言語 |
| GL FBO | MTLTexture (renderTarget) | オフスクリーン |
| GL VBO/VAO | MTLBuffer | 頂点バッファ |
| GL テクスチャ | MTLTexture | テクスチャ |

#### フォント・テキスト
| oFで使用 | Metal版で使用 | 備考 |
|----------|---------------|------|
| FreeType | Core Text (CTFont) | フォント読み込み |
| - | Core Graphics (CGPath) | グリフパス取得 |

#### 画像
| oFで使用 | Metal版で使用 | 備考 |
|----------|---------------|------|
| stb_image | ImageIO (CGImageSource) | CPU読み込み |
| FreeImage | MTKTextureLoader | GPU直接読み込み（推奨） |
| - | Core Graphics (CGBitmapContext) | ピクセル操作 |

#### ウィンドウ・UI
| oFで使用 | Metal版で使用 | 備考 |
|----------|---------------|------|
| GLFW | AppKit (NSWindow) | ウィンドウ管理 |
| SDL | AppKit (NSView) | ビュー管理 |
| - | MetalKit (MTKView) | Metal描画ビュー |
| システムダイアログ | AppKit (NSOpenPanel, NSSavePanel, NSAlert) | ファイル選択等 |

#### ビデオ・カメラ
| oFで使用 | Metal版で使用 | 備考 |
|----------|---------------|------|
| GStreamer | AVFoundation (AVPlayer) | ビデオ再生 |
| QuickTime | AVFoundation (AVAsset) | メディア管理 |
| - | AVFoundation (AVCaptureSession) | カメラキャプチャ |
| - | Core Video (CVPixelBuffer) | フレームバッファ |
| - | Core Media (CMSampleBuffer) | サンプルデータ |

#### サウンド
| oFで使用 | Metal版で使用 | 備考 |
|----------|---------------|------|
| FMOD | AVFoundation (AVAudioEngine) | オーディオエンジン |
| OpenAL | AVFoundation (AVAudioPlayer) | サウンド再生 |
| RtAudio | Core Audio (AudioUnit) | 低レベルオーディオ |
| - | AudioToolbox | オーディオファイル読み込み |

#### ファイル・システム
| oFで使用 | Metal版で使用 | 備考 |
|----------|---------------|------|
| Poco (FileSystem) | Foundation (NSFileManager) | ファイル操作 |
| boost::filesystem | Foundation (NSURL, NSBundle) | パス操作 |
| - | std::filesystem (C++17) | クロスプラットフォーム代替 |

#### ネットワーク
| oFで使用 | Metal版で使用 | 備考 |
|----------|---------------|------|
| Poco (HTTP) | Foundation (NSURLSession) | HTTP通信 |
| curl | Foundation (NSURLRequest) | URL読み込み |

#### XML/JSON
| oFで使用 | Metal版で使用 | 備考 |
|----------|---------------|------|
| Poco (XML) | Foundation (NSXMLParser) | XML解析 |
| nlohmann/json | Foundation (NSJSONSerialization) | JSON解析 |
| - | std::optional + 手動パース | 軽量代替 |

#### 数学・SIMD
| oFで使用 | Metal版で使用 | 備考 |
|----------|---------------|------|
| 外部SIMD | Accelerate (vDSP, vImage) | ベクトル演算 |
| - | simd/simd.h | Apple SIMD型 |
| GLM | simd/simd.h | Apple SIMD型 |

#### スレッド・並行処理
| oFで使用 | Metal版で使用 | 備考 |
|----------|---------------|------|
| Poco (Thread) | std::thread | 標準スレッド |
| - | Grand Central Dispatch (GCD) | 非同期処理 |
| - | dispatch_queue_t | キュー管理 |
| - | NSOperationQueue | 高レベル並行処理 |

#### 時間
| oFで使用 | Metal版で使用 | 備考 |
|----------|---------------|------|
| - | mach_absolute_time() | 高精度時間 |
| - | CACurrentMediaTime() | メディア時間 |
| - | std::chrono | 標準時間 |
| - | Foundation (NSDate) | 日付・時刻 |

#### シリアル通信
| oFで使用 | Metal版で使用 | 備考 |
|----------|---------------|------|
| - | IOKit (IOSerialPort) | シリアルポート |
| - | termios (POSIX) | 低レベルシリアル |

### Step 4: API設計 → docs/api/*.md に記述
```bash
# 設計書を作成または更新
# docs/api/MATH.md など
```

### Step 5: 実装
- パブリックヘッダーは純粋C++ (.h)
- 実装でObjective-C必要なら .mm

### Step 6: このファイルを更新
- ⬜ → ✅ に変更
- 移植先パスを記入

---

## API設計ドキュメント (docs/api/*.md)

**ルール:** 実装前に必ずAPI設計ドキュメントを作成/確認すること

| ドキュメント | 状態 | 対応oFカテゴリ | 内容 |
|-------------|------|----------------|------|
| CORE.md | ✅ | 3d/ | ofNode, ofCamera, ofEasyCam |
| GRAPHICS_2D.md | ✅ | graphics/ | ofPath, ofPolyline, 2D描画関数 |
| GRAPHICS_3D.md | ✅ | 3d/, graphics/ | of3dPrimitive, 3D描画関数 |
| IMAGE.md | ✅ | gl/, graphics/ | ofTexture, ofPixels, ofImage, ofLoadImage |
| LIGHTING.md | ✅ | gl/ | ofLight, ofMaterial |
| MESH.md | ✅ | 3d/, gl/ | ofMesh, ofVboMesh |
| TYPOGRAPHY.md | ✅ | graphics/ | ofTrueTypeFont |
| COLOR.md | ✅ | types/ | ofColor, ofFloatColor |
| MATH.md | ✅ | math/ | ofVec*, ofMatrix*, ofQuaternion, ofMath |
| APP.md | ⬜ | app/ | ofBaseApp, ofAppRunner, ofMainLoop |
| EVENTS.md | ⬜ | events/ | ofEvent, ofEvents |
| UTILS.md | ⬜ | utils/ | ofUtils, ofFileUtils, ofLog, ofNoise |
| FBO.md | ⬜ | gl/ | ofFbo, ofShader, ofVbo, ofBufferObject |
| SOUND.md | ⬜ | sound/ | ofSoundPlayer, ofSoundStream |
| VIDEO.md | ⬜ | video/ | ofVideoPlayer, ofVideoGrabber |
| COMMUNICATION.md | ⬜ | communication/ | ofSerial, ofArduino |
| TYPES.md | ⬜ | types/ | ofRectangle, ofParameter, ofBaseTypes |

### API設計ワークフロー

1. **oFソースを読む** → 該当カテゴリの全ファイルを把握
2. **docs/api/<NAME>.md を作成/更新** → クラス定義、メソッド一覧、使用例
3. **ARCHITECTURE.md の変換ルール適用** → OpenGL→Metal等
4. **このチェックリストを ✅ に更新**

---

## 凡例

| 記号 | 意味 |
|------|------|
| ✅ | 完了 |
| 🔄 | 進行中 |
| ⬜ | 未着手 |
| ➖ | 移植不要（Metal非対応/プラットフォーム固有） |

---

## 1. 3d/ ディレクトリ

| ファイル | 状態 | 移植先 | 備考 |
|----------|------|--------|------|
| ofCamera.h | ✅ | `src/oflike/3d/ofCamera.h` | View/Projection管理 |
| ofCamera.cpp | ✅ | `src/oflike/3d/ofCamera.cpp` | |
| ofEasyCam.h | ✅ | `src/oflike/3d/ofEasyCam.h` | マウス操作カメラ |
| ofEasyCam.cpp | ✅ | `src/oflike/3d/ofEasyCam.cpp` | |
| ofMesh.h | ✅ | `src/oflike/3d/ofMesh.h` | 頂点データ管理 |
| ofNode.h | ✅ | `src/oflike/3d/ofNode.h` | 3D変換ノード |
| ofNode.cpp | ✅ | `src/oflike/3d/ofNode.cpp` | |
| of3dPrimitives.h | ✅ | `src/oflike/3d/of3dPrimitive.h` 他 | 各プリミティブに分割 |
| of3dPrimitives.cpp | ✅ | `src/oflike/3d/of*Primitive.cpp` | Box,Sphere,Cylinder,Cone,Plane,IcoSphere |
| of3dUtils.h | ⬜ | `src/oflike/3d/of3dUtils.h` | 3Dユーティリティ関数 |
| of3dUtils.cpp | ⬜ | `src/oflike/3d/of3dUtils.cpp` | |

---

## 2. app/ ディレクトリ

| ファイル | 状態 | 移植先 | 備考 |
|----------|------|--------|------|
| ofAppBaseWindow.h | ⬜ | `src/oflike/app/ofAppBaseWindow.h` | ウィンドウ基底クラス |
| ofAppRunner.h | ⬜ | `src/oflike/app/ofAppRunner.h` | アプリ起動関数 |
| ofAppRunner.cpp | ⬜ | `src/oflike/app/ofAppRunner.cpp` | ofRunApp, ofSetupOpenGL等 |
| ofBaseApp.h | ⬜ | `src/oflike/app/ofBaseApp.h` | setup/update/draw等 |
| ofBaseApp.cpp | ⬜ | `src/oflike/app/ofBaseApp.cpp` | |
| ofMainLoop.h | ⬜ | `src/oflike/app/ofMainLoop.h` | メインループ管理 |
| ofMainLoop.cpp | ⬜ | `src/oflike/app/ofMainLoop.cpp` | |
| ofWindowSettings.h | ⬜ | `src/oflike/app/ofWindowSettings.h` | ウィンドウ設定 |
| ofAppGLFWWindow.h | ➖ | - | GLFW固有（Metal版は別実装） |
| ofAppGLFWWindow.cpp | ➖ | - | |
| ofAppEGLWindow.h | ➖ | - | EGL固有 |
| ofAppEGLWindow.cpp | ➖ | - | |
| ofAppNoWindow.h | ⬜ | `src/oflike/app/ofAppNoWindow.h` | ヘッドレス実行 |
| ofAppNoWindow.cpp | ⬜ | `src/oflike/app/ofAppNoWindow.cpp` | |
| ofIcon.h | ➖ | - | アイコンデータ（必要時のみ） |

---

## 3. communication/ ディレクトリ

| ファイル | 状態 | 移植先 | 備考 |
|----------|------|--------|------|
| ofSerial.h | ⬜ | `src/oflike/communication/ofSerial.h` | シリアル通信 |
| ofSerial.cpp | ⬜ | `src/oflike/communication/ofSerial.cpp` | |
| ofArduino.h | ⬜ | `src/oflike/communication/ofArduino.h` | Arduino通信 |
| ofArduino.cpp | ⬜ | `src/oflike/communication/ofArduino.cpp` | Firmataプロトコル |

---

## 4. events/ ディレクトリ

| ファイル | 状態 | 移植先 | 備考 |
|----------|------|--------|------|
| ofEvent.h | ⬜ | `src/oflike/events/ofEvent.h` | イベントシステム基盤 |
| ofEventUtils.h | ⬜ | `src/oflike/events/ofEventUtils.h` | イベントユーティリティ |
| ofEvents.h | ⬜ | `src/oflike/events/ofEvents.h` | マウス/キーボード等イベント |
| ofEvents.cpp | ⬜ | `src/oflike/events/ofEvents.cpp` | |

---

## 5. gl/ ディレクトリ（→ Metal移植）

| ファイル | 状態 | 移植先 | 備考 |
|----------|------|--------|------|
| ofTexture.h | ✅ | `src/oflike/image/ofTexture.h` | Metal実装済み |
| ofTexture.cpp | ✅ | `src/render/metal/MetalTexture.mm` | |
| ofLight.h | ✅ | `src/oflike/lighting/ofLight.h` | |
| ofLight.cpp | ✅ | `src/oflike/lighting/ofLight.cpp` | |
| ofMaterial.h | ✅ | `src/oflike/lighting/ofMaterial.h` | |
| ofMaterial.cpp | ✅ | `src/oflike/lighting/ofMaterial.cpp` | |
| ofMaterialBaseTypes.h | ⬜ | `src/oflike/lighting/ofMaterialBaseTypes.h` | マテリアル基底型 |
| ofFbo.h | ⬜ | `src/oflike/gl/ofFbo.h` | フレームバッファ→MTLTexture |
| ofFbo.cpp | ⬜ | `src/oflike/gl/ofFbo.cpp` | Metal実装必要 |
| ofShader.h | ⬜ | `src/oflike/gl/ofShader.h` | →Metal Shader Library |
| ofShader.cpp | ⬜ | `src/oflike/gl/ofShader.cpp` | |
| ofVbo.h | ⬜ | `src/oflike/gl/ofVbo.h` | →MTLBuffer |
| ofVbo.cpp | ⬜ | `src/oflike/gl/ofVbo.cpp` | |
| ofVboMesh.h | ⬜ | `src/oflike/gl/ofVboMesh.h` | VBO版Mesh |
| ofVboMesh.cpp | ⬜ | `src/oflike/gl/ofVboMesh.cpp` | |
| ofBufferObject.h | ⬜ | `src/oflike/gl/ofBufferObject.h` | バッファオブジェクト |
| ofBufferObject.cpp | ⬜ | `src/oflike/gl/ofBufferObject.cpp` | |
| ofCubeMap.h | ⬜ | `src/oflike/gl/ofCubeMap.h` | キューブマップ |
| ofCubeMap.cpp | ⬜ | `src/oflike/gl/ofCubeMap.cpp` | |
| ofCubeMapShaders.h | ⬜ | `src/oflike/gl/ofCubeMapShaders.h` | キューブマップシェーダー |
| ofShadow.h | ⬜ | `src/oflike/gl/ofShadow.h` | シャドウマッピング |
| ofShadow.cpp | ⬜ | `src/oflike/gl/ofShadow.cpp` | |
| ofGLBaseTypes.h | ⬜ | `src/oflike/gl/ofGLBaseTypes.h` | 基底型定義 |
| ofGLUtils.h | ⬜ | `src/oflike/gl/ofGLUtils.h` | GLユーティリティ |
| ofGLUtils.cpp | ⬜ | `src/oflike/gl/ofGLUtils.cpp` | |
| ofGLRenderer.h | ➖ | - | GL固有（Metal版は別） |
| ofGLRenderer.cpp | ➖ | - | |
| ofGLProgrammableRenderer.h | ➖ | - | GL固有（参照用） |
| ofGLProgrammableRenderer.cpp | ➖ | - | |

---

## 6. graphics/ ディレクトリ

| ファイル | 状態 | 移植先 | 備考 |
|----------|------|--------|------|
| ofGraphics.h | ✅ | `src/oflike/graphics/ofGraphics.h` | 2D描画関数 |
| ofGraphics.cpp | ✅ | - | DrawList経由で実装 |
| of3dGraphics.h | ✅ | `src/oflike/graphics/ofGraphics3D.h` | 3D描画関数 |
| of3dGraphics.cpp | ✅ | - | |
| ofGraphicsConstants.h | ⬜ | `src/oflike/graphics/ofGraphicsConstants.h` | 定数定義 |
| ofGraphicsBaseTypes.h | ⬜ | `src/oflike/graphics/ofGraphicsBaseTypes.h` | 基底型 |
| ofGraphicsBaseTypes.cpp | ⬜ | `src/oflike/graphics/ofGraphicsBaseTypes.cpp` | |
| ofPath.h | ✅ | `src/oflike/graphics/ofPath.h` | パス描画 |
| ofPath.cpp | ✅ | - | |
| ofPolyline.h | ✅ | `src/oflike/types/ofPolyline.h` | ポリライン |
| ofPixels.h | ✅ | `src/oflike/image/ofTexture.h` | ofPixels含む |
| ofPixels.cpp | ⬜ | `src/oflike/graphics/ofPixels.cpp` | 拡張実装必要 |
| ofImage.h | ⬜ | `src/oflike/graphics/ofImage.h` | 画像読み込み/操作 |
| ofImage.cpp | ⬜ | `src/oflike/graphics/ofImage.cpp` | |
| ofTrueTypeFont.h | ✅ | `src/oflike/graphics/ofTrueTypeFont.h` | Core Text実装 |
| ofTrueTypeFont.cpp | ✅ | `src/oflike/graphics/ofTrueTypeFont.mm` | |
| ofBitmapFont.h | ⬜ | `src/oflike/graphics/ofBitmapFont.h` | ビットマップフォント |
| ofBitmapFont.cpp | ⬜ | `src/oflike/graphics/ofBitmapFont.cpp` | |
| ofTessellator.h | ⬜ | `src/oflike/graphics/ofTessellator.h` | ポリゴンテセレーション |
| ofTessellator.cpp | ⬜ | `src/oflike/graphics/ofTessellator.cpp` | |
| ofCairoRenderer.h | ➖ | - | Cairo固有 |
| ofCairoRenderer.cpp | ➖ | - | |
| ofGraphicsCairo.h | ➖ | - | Cairo固有 |
| ofGraphicsCairo.cpp | ➖ | - | |
| ofRendererCollection.h | ⬜ | `src/oflike/graphics/ofRendererCollection.h` | レンダラー管理 |
| ofRendererCollection.cpp | ⬜ | `src/oflike/graphics/ofRendererCollection.cpp` | |

---

## 7. math/ ディレクトリ

| ファイル | 状態 | 移植先 | 備考 |
|----------|------|--------|------|
| ofVec2f.h | ✅ | `src/oflike/math/ofVectorMath.h` | glm::vec2エイリアス |
| ofVec2f.cpp | ✅ | - | |
| ofVec3f.h | ✅ | `src/oflike/math/ofVectorMath.h` | glm::vec3エイリアス |
| ofVec4f.h | ✅ | `src/oflike/math/ofVectorMath.h` | glm::vec4エイリアス |
| ofVec4f.cpp | ✅ | - | |
| ofMatrix3x3.h | ⬜ | `src/oflike/math/ofMatrix3x3.h` | 3x3行列 |
| ofMatrix3x3.cpp | ⬜ | `src/oflike/math/ofMatrix3x3.cpp` | |
| ofMatrix4x4.h | ⬜ | `src/oflike/math/ofMatrix4x4.h` | 4x4行列 |
| ofMatrix4x4.cpp | ⬜ | `src/oflike/math/ofMatrix4x4.cpp` | |
| ofQuaternion.h | ⬜ | `src/oflike/math/ofQuaternion.h` | クォータニオン |
| ofQuaternion.cpp | ⬜ | `src/oflike/math/ofQuaternion.cpp` | |
| ofMath.h | ⬜ | `src/oflike/math/ofMath.h` | 数学関数 |
| ofMath.cpp | ⬜ | `src/oflike/math/ofMath.cpp` | |
| ofMathConstants.h | ⬜ | `src/oflike/math/ofMathConstants.h` | 数学定数 |
| ofVectorMath.h | ✅ | `src/oflike/math/ofVectorMath.h` | 統合ヘッダー |

---

## 8. sound/ ディレクトリ

| ファイル | 状態 | 移植先 | 備考 |
|----------|------|--------|------|
| ofSoundPlayer.h | ⬜ | `src/oflike/sound/ofSoundPlayer.h` | サウンド再生 |
| ofSoundPlayer.cpp | ⬜ | `src/oflike/sound/ofSoundPlayer.cpp` | |
| ofSoundStream.h | ⬜ | `src/oflike/sound/ofSoundStream.h` | オーディオストリーム |
| ofSoundStream.cpp | ⬜ | `src/oflike/sound/ofSoundStream.cpp` | |
| ofSoundBuffer.h | ⬜ | `src/oflike/sound/ofSoundBuffer.h` | サウンドバッファ |
| ofSoundBuffer.cpp | ⬜ | `src/oflike/sound/ofSoundBuffer.cpp` | |
| ofSoundBaseTypes.h | ⬜ | `src/oflike/sound/ofSoundBaseTypes.h` | 基底型 |
| ofSoundBaseTypes.cpp | ⬜ | `src/oflike/sound/ofSoundBaseTypes.cpp` | |
| ofSoundUtils.h | ⬜ | `src/oflike/sound/ofSoundUtils.h` | サウンドユーティリティ |
| ofAVEngineSoundPlayer.h | ⬜ | `src/oflike/sound/ofAVEngineSoundPlayer.h` | macOS AVEngine |
| ofAVEngineSoundPlayer.mm | ⬜ | `src/oflike/sound/ofAVEngineSoundPlayer.mm` | |
| ofFmodSoundPlayer.h | ➖ | - | FMOD固有 |
| ofFmodSoundPlayer.cpp | ➖ | - | |
| ofMediaFoundationSoundPlayer.h | ➖ | - | Windows固有 |
| ofMediaFoundationSoundPlayer.cpp | ➖ | - | |
| ofOpenALSoundPlayer.h | ➖ | - | OpenAL固有 |
| ofOpenALSoundPlayer.cpp | ➖ | - | |
| ofRtAudioSoundStream.h | ➖ | - | RtAudio固有 |
| ofRtAudioSoundStream.cpp | ➖ | - | |

---

## 9. types/ ディレクトリ

| ファイル | 状態 | 移植先 | 備考 |
|----------|------|--------|------|
| ofColor.h | ✅ | `src/oflike/graphics/ofColor.h` | 色クラス |
| ofColor.cpp | ✅ | - | |
| ofRectangle.h | ✅ | `src/oflike/types/ofRectangle.h` | 矩形クラス |
| ofRectangle.cpp | ⬜ | `src/oflike/types/ofRectangle.cpp` | 追加メソッド |
| ofPoint.h | ✅ | - | ofVec3fエイリアス |
| ofTypes.h | ⬜ | `src/oflike/types/ofTypes.h` | 型定義 |
| ofBaseTypes.h | ⬜ | `src/oflike/types/ofBaseTypes.h` | 基底型 |
| ofBaseTypes.cpp | ⬜ | `src/oflike/types/ofBaseTypes.cpp` | |
| ofParameter.h | ⬜ | `src/oflike/types/ofParameter.h` | パラメーター |
| ofParameter.cpp | ⬜ | `src/oflike/types/ofParameter.cpp` | |
| ofParameterGroup.cpp | ⬜ | `src/oflike/types/ofParameterGroup.cpp` | パラメーターグループ |

---

## 10. utils/ ディレクトリ

| ファイル | 状態 | 移植先 | 備考 |
|----------|------|--------|------|
| ofConstants.h | ⬜ | `src/oflike/utils/ofConstants.h` | 定数定義 |
| ofUtils.h | ⬜ | `src/oflike/utils/ofUtils.h` | 汎用ユーティリティ |
| ofUtils.cpp | ⬜ | `src/oflike/utils/ofUtils.cpp` | ofToString, ofRandom等 |
| ofFileUtils.h | ⬜ | `src/oflike/utils/ofFileUtils.h` | ファイル操作 |
| ofFileUtils.cpp | ⬜ | `src/oflike/utils/ofFileUtils.cpp` | ofFile, ofDirectory等 |
| ofLog.h | ⬜ | `src/oflike/utils/ofLog.h` | ロギング |
| ofLog.cpp | ⬜ | `src/oflike/utils/ofLog.cpp` | |
| ofNoise.h | ⬜ | `src/oflike/utils/ofNoise.h` | パーリンノイズ等 |
| ofRandomDistributions.h | ⬜ | `src/oflike/utils/ofRandomDistributions.h` | 乱数分布 |
| ofRandomEngine.h | ⬜ | `src/oflike/utils/ofRandomEngine.h` | 乱数エンジン |
| ofJson.h | ⬜ | `src/oflike/utils/ofJson.h` | JSON操作 |
| ofXml.h | ⬜ | `src/oflike/utils/ofXml.h` | XML操作 |
| ofXml.cpp | ⬜ | `src/oflike/utils/ofXml.cpp` | |
| ofThread.h | ⬜ | `src/oflike/utils/ofThread.h` | スレッド |
| ofThread.cpp | ⬜ | `src/oflike/utils/ofThread.cpp` | |
| ofThreadChannel.h | ⬜ | `src/oflike/utils/ofThreadChannel.h` | スレッド間通信 |
| ofFpsCounter.h | ⬜ | `src/oflike/utils/ofFpsCounter.h` | FPSカウンター |
| ofFpsCounter.cpp | ⬜ | `src/oflike/utils/ofFpsCounter.cpp` | |
| ofTimer.h | ⬜ | `src/oflike/utils/ofTimer.h` | タイマー |
| ofTimer.cpp | ⬜ | `src/oflike/utils/ofTimer.cpp` | |
| ofTimerFps.h | ⬜ | `src/oflike/utils/ofTimerFps.h` | FPSタイマー |
| ofTimerFps.cpp | ⬜ | `src/oflike/utils/ofTimerFps.cpp` | |
| ofMatrixStack.h | ⬜ | `src/oflike/utils/ofMatrixStack.h` | 行列スタック |
| ofMatrixStack.cpp | ⬜ | `src/oflike/utils/ofMatrixStack.cpp` | |
| ofSystemUtils.h | ⬜ | `src/oflike/utils/ofSystemUtils.h` | システムダイアログ等 |
| ofSystemUtils.cpp | ⬜ | `src/oflike/utils/ofSystemUtils.cpp` | |
| ofURLFileLoader.h | ⬜ | `src/oflike/utils/ofURLFileLoader.h` | URL読み込み |
| ofURLFileLoader.cpp | ⬜ | `src/oflike/utils/ofURLFileLoader.cpp` | |
| ofSingleton.h | ⬜ | `src/oflike/utils/ofSingleton.h` | シングルトン |

---

## 11. video/ ディレクトリ

| ファイル | 状態 | 移植先 | 備考 |
|----------|------|--------|------|
| ofVideoBaseTypes.h | ⬜ | `src/oflike/video/ofVideoBaseTypes.h` | ビデオ基底型 |
| ofVideoBaseTypes.cpp | ⬜ | `src/oflike/video/ofVideoBaseTypes.cpp` | |
| ofVideoPlayer.h | ⬜ | `src/oflike/video/ofVideoPlayer.h` | ビデオ再生 |
| ofVideoPlayer.cpp | ⬜ | `src/oflike/video/ofVideoPlayer.cpp` | |
| ofVideoGrabber.h | ⬜ | `src/oflike/video/ofVideoGrabber.h` | カメラキャプチャ |
| ofVideoGrabber.cpp | ⬜ | `src/oflike/video/ofVideoGrabber.cpp` | |
| ofAVFoundationPlayer.h | ⬜ | `src/oflike/video/ofAVFoundationPlayer.h` | macOS AVFoundation |
| ofAVFoundationPlayer.mm | ⬜ | `src/oflike/video/ofAVFoundationPlayer.mm` | |
| ofAVFoundationGrabber.h | ⬜ | `src/oflike/video/ofAVFoundationGrabber.h` | macOSカメラ |
| ofAVFoundationGrabber.mm | ⬜ | `src/oflike/video/ofAVFoundationGrabber.mm` | |
| ofAVFoundationVideoPlayer.h | ⬜ | `src/oflike/video/ofAVFoundationVideoPlayer.h` | |
| ofDirectShowGrabber.h | ➖ | - | Windows固有 |
| ofDirectShowGrabber.cpp | ➖ | - | |
| ofDirectShowPlayer.h | ➖ | - | Windows固有 |
| ofDirectShowPlayer.cpp | ➖ | - | |
| ofMediaFoundationPlayer.h | ➖ | - | Windows固有 |
| ofMediaFoundationPlayer.cpp | ➖ | - | |
| ofGstUtils.h | ➖ | - | GStreamer固有 |
| ofGstUtils.cpp | ➖ | - | |
| ofGstVideoGrabber.h | ➖ | - | GStreamer固有 |
| ofGstVideoGrabber.cpp | ➖ | - | |
| ofGstVideoPlayer.h | ➖ | - | GStreamer固有 |
| ofGstVideoPlayer.cpp | ➖ | - | |

---

## 12. ルートファイル

| ファイル | 状態 | 移植先 | 備考 |
|----------|------|--------|------|
| ofMain.h | ✅ | `src/oflike/ofMain.h` | 統合インクルード |

---

## 移植統計

### カテゴリ別進捗

| カテゴリ | 完了 | 進行中 | 未着手 | 移植不要 | 合計 |
|----------|------|--------|--------|----------|------|
| 3d | 9 | 0 | 2 | 0 | 11 |
| app | 0 | 0 | 9 | 6 | 15 |
| communication | 0 | 0 | 4 | 0 | 4 |
| events | 0 | 0 | 4 | 0 | 4 |
| gl (→Metal) | 5 | 0 | 20 | 4 | 29 |
| graphics | 8 | 0 | 11 | 4 | 23 |
| math | 5 | 0 | 9 | 0 | 14 |
| sound | 0 | 0 | 9 | 7 | 16 |
| types | 3 | 0 | 8 | 0 | 11 |
| utils | 0 | 0 | 26 | 0 | 26 |
| video | 0 | 0 | 10 | 12 | 22 |
| root | 1 | 0 | 0 | 0 | 1 |
| **合計** | **31** | **0** | **112** | **33** | **176** |

### 進捗率

- 完了: 31/176 (17.6%)
- 移植対象: 143/176 (81.3%)
- 移植不要: 33/176 (18.8%)

---

## 実装フェーズ（優先順位）

### Phase 1: Core基盤 ✅ 完了
- [x] ofMesh
- [x] ofPath
- [x] ofPolyline
- [x] ofRectangle
- [x] ofColor
- [x] ofGraphics (2D)
- [x] ofVectorMath

### Phase 2: テクスチャ・画像 ✅ 完了
- [x] ofTexture
- [x] ofPixels (基本)
- [x] ofLoadImage

### Phase 3: フォント ✅ 完了
- [x] ofTrueTypeFont (Core Text)

### Phase 4: 3D基盤 ✅ 完了
- [x] ofNode
- [x] ofCamera
- [x] ofEasyCam
- [x] of3dPrimitives (Box, Sphere, Cylinder, Cone, Plane, IcoSphere)

### Phase 5: ライティング ✅ 完了
- [x] ofLight
- [x] ofMaterial

### Phase 6: Math拡張 ⬜ 未着手
- [ ] ofMatrix3x3
- [ ] ofMatrix4x4
- [ ] ofQuaternion
- [ ] ofMath
- [ ] ofMathConstants

### Phase 7: ユーティリティ ⬜ 未着手
- [ ] ofUtils
- [ ] ofFileUtils
- [ ] ofLog
- [ ] ofConstants
- [ ] ofNoise
- [ ] ofRandom系

### Phase 8: イベントシステム ⬜ 未着手
- [ ] ofEvent
- [ ] ofEvents
- [ ] ofEventUtils

### Phase 9: アプリケーション基盤 ⬜ 未着手
- [ ] ofBaseApp
- [ ] ofAppRunner
- [ ] ofMainLoop
- [ ] ofWindowSettings
- [ ] ofAppBaseWindow

### Phase 10: FBO・シェーダー ⬜ 未着手
- [ ] ofFbo (Metal実装)
- [ ] ofShader (Metal Shader Library)
- [ ] ofVbo
- [ ] ofVboMesh
- [ ] ofBufferObject

### Phase 11: 高度なグラフィックス ⬜ 未着手
- [ ] ofImage (フル実装)
- [ ] ofPixels (フル実装)
- [ ] ofBitmapFont
- [ ] ofTessellator
- [ ] ofCubeMap
- [ ] ofShadow

### Phase 12: サウンド ⬜ 未着手
- [ ] ofSoundPlayer
- [ ] ofSoundStream
- [ ] ofSoundBuffer
- [ ] ofAVEngineSoundPlayer (macOS)

### Phase 13: ビデオ ⬜ 未着手
- [ ] ofVideoPlayer
- [ ] ofVideoGrabber
- [ ] ofAVFoundationPlayer
- [ ] ofAVFoundationGrabber

### Phase 14: 通信 ⬜ 未着手
- [ ] ofSerial
- [ ] ofArduino

### Phase 15: 拡張型 ⬜ 未着手
- [ ] ofParameter
- [ ] ofParameterGroup
- [ ] ofTypes
- [ ] ofBaseTypes

---

## 現在の実装ファイル一覧

```
src/
├── core/
│   ├── Context.h
│   └── Types.h
├── render/
│   ├── DrawList.h
│   ├── DrawList.cpp
│   ├── Types.h
│   └── metal/
│       ├── MetalRenderer.h
│       ├── MetalRenderer.mm
│       ├── MetalTexture.mm
│       └── MetalImage.mm
└── oflike/
    ├── ofMain.h
    ├── 3d/
    │   ├── ofMesh.h
    │   ├── ofNode.h / .cpp
    │   ├── ofCamera.h / .cpp
    │   ├── ofEasyCam.h / .cpp
    │   ├── of3dPrimitive.h / .cpp
    │   ├── ofBoxPrimitive.h / .cpp
    │   ├── ofSpherePrimitive.h / .cpp
    │   ├── ofCylinderPrimitive.h / .cpp
    │   ├── ofConePrimitive.h / .cpp
    │   ├── ofPlanePrimitive.h / .cpp
    │   └── ofIcoSpherePrimitive.h / .cpp
    ├── app/
    │   └── (準備中)
    ├── graphics/
    │   ├── ofGraphics.h
    │   ├── ofGraphics3D.h
    │   ├── ofColor.h
    │   ├── ofPath.h
    │   └── ofTrueTypeFont.h / .mm
    ├── image/
    │   └── ofTexture.h
    ├── lighting/
    │   ├── ofLight.h / .cpp
    │   └── ofMaterial.h / .cpp
    ├── math/
    │   └── ofVectorMath.h
    ├── types/
    │   ├── ofRectangle.h
    │   └── ofPolyline.h
    └── utils/
        └── (準備中)
```

---

## 備考

### Metal移植のポイント

1. **GL→Metal変換**
   - GLテクスチャ → MTLTexture
   - GLシェーダー → Metal Shader Library
   - GLVBO → MTLBuffer
   - GLFBO → MTLTexture (Render Target)

2. **プラットフォーム固有**
   - macOS: AVFoundation, Core Text
   - iOS: Metal Kit
   - Windows/Linux: 移植対象外

3. **依存ライブラリ**
   - GLM (数学ライブラリ)
   - stb_image (画像読み込み、オプション)

### 参照資料

- openFrameworks v0.12.1 ソースコード: `references/of_v0.12.1_osx_release/libs/openFrameworks/`
- Metal Programming Guide
- Core Text Programming Guide
- AVFoundation Programming Guide
