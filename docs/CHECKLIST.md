# oflike-metal 開発チェックリスト

**Version**: 1.0.0  
**Last Updated**: 2025-01-23

> AIはこのチェックリストを参照しながら開発を進めます。  
> 完了したタスクには `[x]` を付けてください。

---

## Phase 0: プロジェクト基盤

### 0.1 プロジェクト構成
- [x] Xcodeプロジェクト作成（Swift + Objective-C++ 混在）
- [x] ディレクトリ構造作成
- [x] CMakeLists.txt または Makefile 作成
- [x] .clang-format 設定
- [x] .gitignore 設定
- [x] README.md 作成

### 0.2 ドキュメント
- [x] ARCHITECTURE.md 配置
- [x] IMPLEMENTATION.md 作成
- [x] THIRDPARTY.md 作成
- [x] API設計書ディレクトリ作成

### 0.3 サードパーティ
- [x] tess2 導入
- [x] utf8-cpp 導入
- [x] GLM 導入（暫定）

---

## Phase 1: SwiftUI + Metal 基盤

### 1.1 SwiftUI アプリケーション
- [x] @main App 構造体作成
- [x] WindowGroup 設定
- [x] アプリケーションライフサイクル実装

### 1.2 MTKView 統合
- [x] NSViewRepresentable でラップ
- [x] MTKViewDelegate 実装
- [x] フレームループ確立（update/draw）

### 1.3 Metal 初期化
- [x] MTLDevice 取得
- [x] MTLCommandQueue 作成
- [x] MTLLibrary（シェーダー）読み込み
- [x] 基本 RenderPipelineState 作成

### 1.4 Swift-C++ ブリッジ
- [x] OFLBridge クラス作成（Obj-C++）
- [x] Swift → C++ 呼び出し確認
- [x] C++ → Swift コールバック確認

### 1.5 基本描画確認
- [x] 画面クリア（単色）動作確認
- [x] 三角形描画テスト
- [x] フレームレート表示

---

## Phase 2: Core システム

### 2.1 Context
- [x] Context シングルトン設計
- [x] Renderer アクセス
- [x] 現在時刻 / フレーム番号
- [x] ウィンドウサイズ取得

### 2.2 Engine
- [x] Engine クラス設計
- [x] 初期化 / シャットダウン
- [x] メインループ管理

### 2.3 AppBase
- [x] ofBaseApp 互換クラス
- [x] setup() / update() / draw() / exit()
- [x] イベントコールバック（空実装）

### 2.4 Time
- [x] ofGetElapsedTimef() 実装
- [x] ofGetElapsedTimeMillis() 実装
- [x] ofGetFrameNum() 実装
- [x] ofGetFrameRate() 実装
- [x] ofSetFrameRate() 実装

---

## Phase 3: 数学ライブラリ

### 3.1 基本型（simd ベース）
- [x] Types.h 作成（simd エイリアス）
- [x] float2 / float3 / float4
- [x] float2x2 / float3x3 / float4x4
- [x] quatf

### 3.2 ofVec2f
- [x] コンストラクタ
- [x] 演算子（+, -, *, /）
- [x] length() / normalize() / dot()
- [x] simd_float2 との相互変換

### 3.3 ofVec3f
- [x] コンストラクタ
- [x] 演算子（+, -, *, /）
- [x] length() / normalize() / dot() / cross()
- [x] simd_float3 との相互変換

### 3.4 ofVec4f
- [x] コンストラクタ
- [x] 演算子
- [x] simd_float4 との相互変換

### 3.5 ofMatrix4x4
- [x] コンストラクタ
- [x] 単位行列 / ゼロ行列
- [x] 乗算
- [x] translate / rotate / scale
- [x] inverse / transpose
- [x] lookAt / perspective / ortho
- [x] simd_float4x4 との相互変換

### 3.6 ofQuaternion
- [x] コンストラクタ
- [x] 軸角度から生成
- [x] オイラー角から生成
- [x] slerp
- [x] simd_quatf との相互変換

### 3.7 数学関数
- [x] ofRandom() / ofRandomf() / ofRandomuf()
- [x] ofSeedRandom()
- [x] ofNoise() / ofSignedNoise()
- [x] ofMap()
- [x] ofClamp()
- [x] ofLerp()
- [x] ofDist() / ofDistSquared()
- [x] ofDegToRad() / ofRadToDeg()
- [x] ofWrap() / ofWrapDegrees() / ofWrapRadians()

---

## Phase 4: レンダリング基盤

### 4.1 RenderTypes
- [x] Vertex2D 構造体
- [x] Vertex3D 構造体
- [x] PrimitiveType enum
- [x] BlendMode enum

### 4.2 DrawCommand
- [x] DrawCommand2D 構造体
- [x] DrawCommand3D 構造体
- [x] SetViewport / SetScissor
- [x] Clear コマンド
- [x] SetRenderTarget コマンド

### 4.3 DrawList
- [x] コマンドバッファ管理
- [x] 頂点バッファ管理
- [x] インデックスバッファ管理
- [x] コマンド追加 API
- [x] フレーム終了時リセット

### 4.4 MetalRenderer
- [x] IRenderer インターフェース定義
- [x] MetalRenderer 実装
- [x] DrawList 実行
- [x] 状態管理（ブレンド、深度等）
- [x] Triple Buffering

### 4.5 MetalBuffer
- [x] 動的バッファ管理
- [x] バッファプール

### 4.6 MetalTexture
- [x] MTKTextureLoader ラッパー
- [x] テクスチャキャッシュ
- [x] サンプラー管理

### 4.7 シェーダー
- [x] Common.h（共通定義）
- [x] Basic2D.metal
- [x] Basic3D.metal
- [x] Textured.metal

---

## Phase 5: 2D 描画 API

### 5.1 描画状態
- [x] ofSetColor()
- [x] ofSetBackgroundColor()
- [x] ofBackground()
- [x] ofClear()
- [x] ofNoFill() / ofFill()
- [x] ofSetLineWidth()
- [x] ofSetCircleResolution()
- [x] ofSetCurveResolution()

### 5.2 行列スタック
- [x] ofPushMatrix() / ofPopMatrix()
- [x] ofTranslate()
- [x] ofRotate() / ofRotateX() / ofRotateY() / ofRotateZ()
- [x] ofScale()
- [x] ofLoadIdentityMatrix()
- [x] ofLoadMatrix()
- [x] ofMultMatrix()

### 5.3 基本図形
- [x] ofDrawLine()
- [x] ofDrawRectangle()
- [x] ofDrawRectRounded()
- [x] ofDrawCircle()
- [x] ofDrawEllipse()
- [x] ofDrawTriangle()

### 5.4 曲線
- [x] ofDrawCurve()
- [x] ofDrawBezier()

### 5.5 ofPolyline
- [x] コンストラクタ
- [x] addVertex() / addVertices()
- [x] lineTo() / curveTo() / bezierTo()
- [x] arc() / arcNegative()
- [x] close()
- [x] draw()
- [x] getVertices() / size()
- [x] getPerimeter() / getArea()
- [x] getCentroid2D() / getBoundingBox()
- [x] getClosestPoint() / inside()
- [x] simplify()
- [x] getResampledBySpacing() / getResampledByCount()
- [x] getSmoothed()

### 5.6 ofPath
- [x] moveTo() / lineTo()
- [x] curveTo() / bezierTo()
- [x] arc() / arcNegative()
- [x] close()
- [x] setFilled() / setStrokeWidth()
- [x] setColor() / setFillColor() / setStrokeColor()
- [x] draw()
- [x] getOutline()
- [x] getTessellation()（tess2 使用）
- [x] simplify()
- [x] translate() / rotate() / scale()

### 5.7 Shape API
- [x] ofBeginShape() / ofEndShape()
- [x] ofVertex()
- [x] ofCurveVertex()
- [x] ofBezierVertex()
- [x] ofNextContour()

### 5.8 ブレンドモード
- [x] ofEnableAlphaBlending()
- [x] ofDisableAlphaBlending()
- [x] ofEnableBlendMode()

---

## Phase 6: 色

### 6.1 ofColor
- [x] コンストラクタ（r,g,b,a）
- [x] set() / setHex()
- [x] getHex()
- [x] setHsb() / getHue() / getSaturation() / getBrightness()
- [x] lerp()
- [x] getClamped() / getInverted() / getNormalized()
- [x] limit()
- [x] ofColor::fromHex() / ofColor::fromHsb()

### 6.2 ofFloatColor
- [x] 上記と同等（float 0.0-1.0）

### 6.3 ofShortColor
- [x] 上記と同等（unsigned short）

---

## Phase 7: 画像・テクスチャ

### 7.1 ofPixels
- [x] allocate()
- [x] setColor() / getColor()
- [x] getData()
- [x] getWidth() / getHeight()
- [x] getNumChannels() / getBytesPerPixel()
- [x] setFromPixels() / setFromExternalPixels()

### 7.2 ofTexture
- [x] allocate()
- [x] loadData()
- [x] draw()
- [x] bind() / unbind()
- [x] getWidth() / getHeight()
- [x] setTextureWrap()
- [x] setTextureMinMagFilter()
- [x] getNativeHandle()

### 7.3 ofImage
- [x] load()（MTKTextureLoader / ImageIO）
- [x] save()（CGImageDestination）
- [x] draw()
- [x] getPixels() / getTexture()
- [x] setFromPixels()
- [x] resize()
- [x] crop()
- [x] rotate90()
- [x] mirror()
- [x] setImageType()

### 7.4 画像読み込み関数
- [x] ofLoadImage(ofPixels&, path)
- [x] ofLoadImage(ofTexture&, path)
- [x] ofSaveImage(ofPixels&, path)

---

## Phase 8: 3D 描画

### 8.1 3D プリミティブ
- [x] ofDrawBox()
- [x] ofDrawSphere()
- [x] ofDrawCone()
- [x] ofDrawCylinder()
- [x] ofDrawPlane()
- [x] ofDrawIcoSphere()

### 8.2 3D ヘルパー
- [x] ofDrawAxis()
- [x] ofDrawGrid()
- [x] ofDrawGridPlane()
- [x] ofDrawArrow()
- [x] ofDrawRotationAxes()

### 8.3 深度・カリング
- [x] ofEnableDepthTest() / ofDisableDepthTest()
- [x] ofSetDepthWrite()
- [x] ofEnableCulling() / ofDisableCulling()

### 8.4 ofMesh
- [x] コンストラクタ
- [x] addVertex() / addVertices()
- [x] addNormal() / addNormals()
- [x] addTexCoord() / addTexCoords()
- [x] addColor() / addColors()
- [x] addIndex() / addIndices()
- [x] addTriangle()
- [x] setMode()
- [x] draw() / drawWireframe() / drawVertices() / drawFaces()
- [x] getVertices() / getNormals() / getTexCoords() / getColors()
- [x] getIndices()
- [x] clear()
- [x] mergeDuplicateVertices()
- [x] setupIndicesAuto()
- [x] smoothNormals() / flatNormals()
- [x] append()

### 8.5 ofMesh ジェネレータ
- [x] ofMesh::plane()
- [x] ofMesh::sphere()
- [x] ofMesh::box()
- [x] ofMesh::cone()
- [x] ofMesh::cylinder()
- [x] ofMesh::icosphere()

### 8.6 ofMesh I/O
- [x] load()（PLY / OBJ）
- [x] save()

### 8.7 3D シェーダー
- [x] Basic3D.metal（頂点色）
- [x] Textured3D.metal
- [~] Lighting.metal（Phong） → Phase 10.4で実装

---

## Phase 9: カメラ

### 9.1 ofCamera
- [x] コンストラクタ
- [x] begin() / end()
- [x] setPosition() / getPosition()
- [x] setOrientation() / getOrientation()
- [x] lookAt()
- [x] setNearClip() / setFarClip()
- [x] setFov() / getFov()
- [x] setAspectRatio()
- [x] getModelViewMatrix()
- [x] getProjectionMatrix()
- [x] getModelViewProjectionMatrix()
- [x] worldToScreen()
- [x] screenToWorld()

### 9.2 ofEasyCam
- [ ] コンストラクタ
- [ ] setDistance() / getDistance()
- [ ] setTarget() / getTarget()
- [ ] enableMouseInput() / disableMouseInput()
- [ ] enableInertia() / disableInertia()
- [ ] setAutoDistance()
- [ ] マウスドラッグ回転
- [ ] マウスホイールズーム
- [ ] 慣性実装

---

## Phase 10: ライティング

### 10.1 ofLight
- [ ] コンストラクタ
- [ ] enable() / disable()
- [ ] setPointLight()
- [ ] setDirectional()
- [ ] setSpotlight()
- [ ] setPosition()
- [ ] setAmbientColor() / setDiffuseColor() / setSpecularColor()
- [ ] setAttenuation()
- [ ] setSpotlightCutOff() / setSpotConcentration()

### 10.2 ofMaterial
- [ ] コンストラクタ
- [ ] begin() / end()
- [ ] setAmbientColor()
- [ ] setDiffuseColor()
- [ ] setSpecularColor()
- [ ] setEmissiveColor()
- [ ] setShininess()

### 10.3 ライティングシステム
- [ ] グローバルライト配列管理（最大8灯）
- [ ] ライト Uniform Buffer
- [ ] ofEnableLighting() / ofDisableLighting()
- [ ] ofSetSmoothLighting()

### 10.4 ライティングシェーダー
- [ ] Lighting.metal 完成版
- [ ] Phong シェーディング
- [ ] 複数ライト対応

---

## Phase 11: FBO

### 11.1 ofFbo
- [ ] allocate()
- [ ] allocateWithSettings()
- [ ] begin() / end()
- [ ] draw()
- [ ] getTexture()
- [ ] getDepthTexture()
- [ ] readToPixels()
- [ ] getWidth() / getHeight()
- [ ] setActiveDrawBuffer()
- [ ] getNumTextures()
- [x] clear()

### 11.2 FBO 設定
- [ ] ofFboSettings 構造体
- [ ] マルチサンプル対応
- [ ] 深度バッファ設定
- [ ] 複数カラーアタッチメント

---

## Phase 12: フォント

### 12.1 ofTrueTypeFont
- [ ] load()（Core Text）
- [ ] isLoaded()
- [ ] drawString()
- [ ] drawStringAsShapes()
- [ ] getStringBoundingBox()
- [ ] stringWidth() / stringHeight()
- [ ] getLineHeight() / setLineHeight()
- [ ] getLetterSpacing() / setLetterSpacing()
- [ ] getCharacterAsPoints()
- [ ] getFontSize()

### 12.2 フォントアトラス
- [ ] グリフキャッシュ
- [ ] アトラステクスチャ生成
- [ ] バッチ描画最適化

### 12.3 日本語対応
- [ ] UTF-8 処理
- [ ] 日本語フォント読み込みテスト

---

## Phase 13: イベントシステム

### 13.1 マウスイベント
- [ ] mouseMoved()
- [ ] mouseDragged()
- [ ] mousePressed()
- [ ] mouseReleased()
- [ ] mouseScrolled()
- [ ] mouseEntered()
- [ ] mouseExited()

### 13.2 キーボードイベント
- [ ] keyPressed()
- [ ] keyReleased()
- [ ] ofGetKeyPressed()

### 13.3 ウィンドウイベント
- [ ] windowResized()
- [ ] dragEvent()

### 13.4 イベントディスパッチャー
- [ ] EventDispatcher クラス
- [ ] SwiftUI → C++ イベント伝達

### 13.5 マウス・キーボード状態
- [ ] ofGetMouseX() / ofGetMouseY()
- [ ] ofGetPreviousMouseX() / ofGetPreviousMouseY()
- [ ] ofGetMousePressed()

---

## Phase 14: ウィンドウ管理

### 14.1 ウィンドウ関数
- [ ] ofGetWidth() / ofGetHeight()
- [ ] ofGetWindowWidth() / ofGetWindowHeight()
- [ ] ofSetWindowShape()
- [ ] ofSetWindowPosition()
- [ ] ofSetWindowTitle()
- [ ] ofSetFullscreen()
- [ ] ofToggleFullscreen()
- [ ] ofGetScreenWidth() / ofGetScreenHeight()

### 14.2 カーソル
- [ ] ofHideCursor()
- [ ] ofShowCursor()

---

## Phase 15: ユーティリティ

### 15.1 文字列
- [ ] ofToString()
- [ ] ofToInt() / ofToFloat() / ofToBool()
- [ ] ofToHex() / ofHexToInt()
- [ ] ofSplitString() / ofJoinString()
- [ ] ofToLower() / ofToUpper()
- [ ] ofTrim()

### 15.2 ログ
- [ ] ofLog()
- [ ] ofLogVerbose() / ofLogNotice() / ofLogWarning()
- [ ] ofLogError() / ofLogFatalError()
- [ ] ofSetLogLevel()
- [ ] os_log 統合

### 15.3 ファイル
- [ ] ofFile クラス
- [ ] ofDirectory クラス
- [ ] ofBuffer クラス
- [ ] ofFilePath 関数群

### 15.4 システム
- [ ] ofSystem()
- [ ] ofLaunchBrowser()
- [ ] ofGetTimestampString()

---

## Phase 16: デバッグ機能

### 16.1 SwiftUI Debug Overlay
- [ ] DebugOverlay.swift 作成
- [ ] パラメータバインディング
- [ ] グループ表示
- [ ] スライダー / トグル / カラーピッカー
- [ ] DEBUG ビルドのみ有効化

### 16.2 パフォーマンス表示
- [ ] FPS 表示
- [ ] ドローコール数
- [ ] 頂点数
- [ ] GPU 時間

### 16.3 OSC 連携（将来）
- [ ] OSC 受信
- [ ] OSC 送信
- [ ] パラメータマッピング

---

## Phase 17: サンプル・テスト

### 17.1 Examples
- [ ] 01_basics（基本描画）
- [ ] 02_shapes（図形描画）
- [ ] 03_color（色操作）
- [ ] 04_image（画像読み込み）
- [ ] 05_typography（フォント描画）
- [ ] 06_3d_primitives（3Dプリミティブ）
- [ ] 07_camera（カメラ操作）
- [ ] 08_lighting（ライティング）
- [ ] 09_mesh（メッシュ操作）
- [ ] 10_fbo（オフスクリーン描画）
- [ ] 11_events（イベント処理）
- [ ] 12_easycam（EasyCam操作）

### 17.2 テスト
- [ ] 数学ライブラリテスト
- [ ] 描画テスト（ビジュアル比較）
- [ ] パフォーマンステスト

---

## Phase 18: 最適化・仕上げ

### 18.1 パフォーマンス最適化
- [ ] バッチング最適化
- [ ] バッファプール最適化
- [ ] シェーダー最適化

### 18.2 GLM 除去
- [ ] GLM 依存コード特定
- [ ] simd 完全移行
- [ ] GLM 削除

### 18.3 ドキュメント
- [ ] API ドキュメント完成
- [ ] 移行ガイド作成
- [ ] サンプルコード充実

### 18.4 リリース準備
- [ ] バージョン 0.1.0 タグ
- [ ] リリースノート作成

---

## 進捗サマリー

| Phase | 名称 | 状態 | 完了率 |
|-------|------|------|--------|
| 0 | プロジェクト基盤 | 完了 | 100% |
| 1 | SwiftUI + Metal 基盤 | 完了 | 100% |
| 2 | Core システム | 完了 | 100% |
| 3 | 数学ライブラリ | 完了 | 100% |
| 4 | レンダリング基盤 | 完了 | 100% |
| 5 | 2D 描画 API | 完了 | 100% |
| 6 | 色 | 完了 | 100% |
| 7 | 画像・テクスチャ | 完了 | 100% |
| 8 | 3D 描画 | 完了 | 100% |
| 9 | カメラ | 進行中 | 54% (13/24) |
| 10 | ライティング | 未着手 | 0% |
| 11 | FBO | 未着手 | 0% |
| 12 | フォント | 未着手 | 0% |
| 13 | イベントシステム | 未着手 | 0% |
| 14 | ウィンドウ管理 | 未着手 | 0% |
| 15 | ユーティリティ | 未着手 | 0% |
| 16 | デバッグ機能 | 未着手 | 0% |
| 17 | サンプル・テスト | 未着手 | 0% |
| 18 | 最適化・仕上げ | 未着手 | 0% |

---

**総タスク数**: 399 項目
**完了**: 254 項目
**全体進捗**: 63.7%

---

次にどの Phase から着手しますか？Phase 0 → Phase 1 の順で進めることを推奨します。
