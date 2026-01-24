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
- [x] コンストラクタ
- [x] setDistance() / getDistance()
- [x] setTarget() / getTarget()
- [x] enableMouseInput() / disableMouseInput()
- [x] enableInertia() / disableInertia()
- [x] setAutoDistance()
- [x] マウスドラッグ回転
- [x] マウスホイールズーム
- [x] 慣性実装

---

## Phase 10: ライティング

### 10.1 ofLight
- [x] コンストラクタ
- [x] enable() / disable()
- [x] setPointLight()
- [x] setDirectional()
- [x] setSpotlight()
- [x] setPosition()
- [x] setAmbientColor() / setDiffuseColor() / setSpecularColor()
- [x] setAttenuation()
- [x] setSpotlightCutOff() / setSpotConcentration()

### 10.2 ofMaterial
- [x] コンストラクタ
- [x] begin() / end()
- [x] setAmbientColor()
- [x] setDiffuseColor()
- [x] setSpecularColor()
- [x] setEmissiveColor()
- [x] setShininess()

### 10.3 ライティングシステム
- [x] グローバルライト配列管理（最大8灯）
- [x] ライト Uniform Buffer
- [x] ofEnableLighting() / ofDisableLighting()
- [x] ofSetSmoothLighting()

### 10.4 ライティングシェーダー
- [x] Lighting.metal 完成版
- [x] Phong シェーディング
- [x] 複数ライト対応

---

## Phase 11: FBO

### 11.1 ofFbo
- [x] allocate()
- [x] allocateWithSettings()
- [x] begin() / end()
- [x] draw()
- [x] getTexture()
- [x] getDepthTexture()
- [x] readToPixels()
- [x] getWidth() / getHeight()
- [x] setActiveDrawBuffer()
- [x] getNumTextures()
- [x] clear()

### 11.2 FBO 設定
- [x] ofFboSettings 構造体
- [x] マルチサンプル対応
- [x] 深度バッファ設定
- [x] 複数カラーアタッチメント

---

## Phase 12: フォント

### 12.1 ofTrueTypeFont
- [x] load()（Core Text）
- [x] isLoaded()
- [x] drawString()
- [x] drawStringAsShapes()
- [x] getStringBoundingBox()
- [x] stringWidth() / stringHeight()
- [x] getLineHeight() / setLineHeight()
- [x] getLetterSpacing() / setLetterSpacing()
- [x] getCharacterAsPoints()
- [x] getFontSize()

### 12.2 フォントアトラス
- [x] グリフキャッシュ
- [x] アトラステクスチャ生成
- [x] バッチ描画最適化

### 12.3 日本語対応
- [x] UTF-8 処理
- [x] 日本語フォント読み込みテスト

---

## Phase 13: イベントシステム

### 13.1 マウスイベント
- [x] mouseMoved()
- [x] mouseDragged()
- [x] mousePressed()
- [x] mouseReleased()
- [x] mouseScrolled()
- [x] mouseEntered()
- [x] mouseExited()

### 13.2 キーボードイベント
- [x] keyPressed()
- [x] keyReleased()
- [x] ofGetKeyPressed()

### 13.3 ウィンドウイベント
- [x] windowResized()
- [x] dragEvent()

### 13.4 イベントディスパッチャー
- [x] EventDispatcher クラス
- [x] SwiftUI → C++ イベント伝達

### 13.5 マウス・キーボード状態
- [x] ofGetMouseX() / ofGetMouseY()
- [x] ofGetPreviousMouseX() / ofGetPreviousMouseY()
- [x] ofGetMousePressed()

---

## Phase 14: ウィンドウ管理

### 14.1 ウィンドウ関数
- [x] ofGetWidth() / ofGetHeight()
- [x] ofGetWindowWidth() / ofGetWindowHeight()
- [x] ofSetWindowShape()
- [x] ofSetWindowPosition()
- [x] ofSetWindowTitle()
- [x] ofSetFullscreen()
- [x] ofToggleFullscreen()
- [x] ofGetScreenWidth() / ofGetScreenHeight()

### 14.2 カーソル
- [x] ofHideCursor()
- [x] ofShowCursor()

---

## Phase 15: ユーティリティ

### 15.1 文字列
- [x] ofToString()
- [x] ofToInt() / ofToFloat() / ofToBool()
- [x] ofToHex() / ofHexToInt()
- [x] ofSplitString() / ofJoinString()
- [x] ofToLower() / ofToUpper()
- [x] ofTrim()

### 15.2 ログ
- [x] ofLog()
- [x] ofLogVerbose() / ofLogNotice() / ofLogWarning()
- [x] ofLogError() / ofLogFatalError()
- [x] ofSetLogLevel()
- [x] os_log 統合

### 15.3 ファイル
- [x] ofFile クラス
- [x] ofDirectory クラス
- [x] ofBuffer クラス
- [x] ofFilePath 関数群

### 15.4 システム
- [x] ofSystem()
- [x] ofLaunchBrowser()
- [x] ofGetTimestampString()

---

## Phase 16: デバッグ機能

### 16.1 SwiftUI Debug Overlay
- [x] DebugOverlay.swift 作成
- [x] パラメータバインディング
- [x] グループ表示
- [x] スライダー / トグル / カラーピッカー
- [x] DEBUG ビルドのみ有効化

### 16.2 パフォーマンス表示
- [x] FPS 表示
- [x] ドローコール数
- [x] 頂点数
- [x] GPU 時間

### 16.3 OSC 連携
- [x] OSC 受信
- [x] OSC 送信
- [x] パラメータマッピング

---

## Phase 17: サンプル・テスト

### 17.1 Examples
- [x] 01_basics（基本描画）
- [x] 02_shapes（図形描画）
- [x] 03_color（色操作）
- [x] 04_image（画像読み込み）
- [x] 05_typography（フォント描画）
- [x] 06_3d_primitives（3Dプリミティブ）
- [x] 07_camera（カメラ操作）
- [x] 08_lighting（ライティング）
- [x] 09_mesh（メッシュ操作）
- [x] 10_fbo（オフスクリーン描画）
- [x] 11_events（イベント処理）
- [x] 12_easycam（EasyCam操作）

### 17.2 テスト
- [x] 数学ライブラリテスト
- [x] 描画テスト（ビジュアル比較）
- [x] パフォーマンステスト

---

## Phase 18: 最適化・仕上げ

### 18.1 パフォーマンス最適化
- [x] バッチング最適化
- [x] バッファプール最適化
- [x] シェーダー最適化

### 18.2 GLM 除去
- [x] GLM 依存コード特定
- [x] simd 完全移行
- [x] GLM 削除

### 18.3 ドキュメント
- [x] API ドキュメント完成
- [x] 移行ガイド作成
- [x] サンプルコード充実

### 18.4 リリース準備
- [x] バージョン 0.1.0 タグ
- [x] リリースノート作成

---

## Phase 19: ofxOsc

**Dependencies**: oscpack (MIT)

### 19.1 Core Classes
- [x] ofxOscMessage 実装
- [x] ofxOscBundle 実装
- [x] ofxOscSender 実装
- [x] ofxOscReceiver 実装

### 19.2 Message Types
- [x] Int32 引数
- [x] Float 引数
- [x] String 引数
- [x] Blob 引数
- [x] Int64 / Double / Char / Bool

### 19.3 Features
- [x] マルチキャスト対応
- [x] Bundle タイムスタンプ
- [x] エラーハンドリング

### 19.4 Parameter Mapping
- [x] ofxOscParameterSync 実装
- [x] ofParameter との連携
- [x] 双方向同期

### 19.5 Examples
- [x] osc_sender example
- [x] osc_receiver example
- [x] osc_parameter_sync example

---

## Phase 20: ofxGui

**Dependencies**: SwiftUI (primary), Dear ImGui (optional)

### 20.1 SwiftUI Implementation
- [x] OFLGuiPanel.swift 作成
- [x] C++ → Swift ブリッジ
- [x] パラメータバインディング

### 20.2 Widgets
- [x] Slider (float, int)
- [x] Toggle (bool)
- [x] Button
- [x] Label
- [x] TextField
- [x] ColorPicker
- [x] Dropdown / Picker

### 20.3 Layout
- [x] ofxGuiGroup 実装
- [x] ofxPanel 実装
- [x] ネスト対応
- [x] 折りたたみ

### 20.4 Styling
- [x] .ultraThinMaterial 背景
- [x] ダークモード対応
- [x] カスタムカラー

### 20.5 ImGui Backend (Optional)
- [ ] Metal バックエンド統合
- [ ] DEBUG ビルドのみ有効化

### 20.6 Examples
- [x] gui_basic example
- [x] gui_groups example

---

## Phase 21: ofxXmlSettings

**Dependencies**: pugixml (MIT)

### 21.1 Core
- [x] ofxXmlSettings クラス実装
- [x] load() / save()
- [x] pushTag() / popTag()

### 21.2 Value Types
- [x] getValue() - int, float, string
- [x] setValue() - int, float, string
- [x] getAttribute()
- [x] addTag() / removeTag()

### 21.3 Navigation
- [x] getNumTags()
- [x] tagExists()
- [x] getCurrentTag()

### 21.4 Examples
- [x] xml_settings example

---

## Phase 22: ofxSvg

**Dependencies**: nanosvg (Zlib), Core Graphics

### 22.1 Core
- [x] ofxSvg クラス実装
- [x] load() - nanosvg パース
- [x] getNumPaths()
- [x] getPathAt()

### 22.2 Path Conversion
- [x] NSVGpath → ofPath 変換
- [x] 色・スタイル変換
- [~] 変換行列対応 (TODO: Future enhancement for draw(x,y,w,h))

### 22.3 Rendering
- [x] draw()
- [x] 個別パス描画

### 22.4 Examples
- [x] svg_loader example

---

## Phase 23: ofxNetwork

**Dependencies**: Network.framework (Apple)

### 23.1 TCP
- [x] ofxTcpClient 実装
- [x] ofxTcpServer 実装
- [x] 非同期接続
- [x] 送受信

### 23.2 UDP
- [x] ofxUdpManager 実装
- [x] bind() / connect()
- [x] send() / receive()
- [x] マルチキャスト

### 23.3 Features
- [x] 非ブロッキングモード
- [x] タイムアウト設定
- [x] エラーハンドリング

### 23.4 Examples
- [x] tcp_client example
- [x] tcp_server example
- [x] udp_sender example
- [x] udp_receiver example

---

## Phase 24: ofxOpenCv

**Dependencies**: Vision.framework (primary), OpenCV (optional)

### 24.1 Vision.framework Implementation
- [x] 顔検出 (VNDetectFaceRectanglesRequest)
- [x] 人体検出 (VNDetectHumanRectanglesRequest)
- [x] テキスト検出 (VNRecognizeTextRequest)
- [x] バーコード検出

### 24.2 Image Conversion
- [x] ofPixels → CVPixelBuffer
- [x] CVPixelBuffer → ofPixels
- [x] ofPixels → cv::Mat (optional)

### 24.3 OpenCV Features (Optional)
- [ ] ContourFinder
- [ ] Optical Flow
- [ ] Background Subtraction

### 24.4 Wrapper Classes
- [x] ofxCv namespace
- [x] toCv() / toOf() 変換関数

### 24.5 Examples
- [x] face_detection example
- [ ] contour_finder example (skipped - 24.3 not implemented)

---

## Phase 25: ofxSharp ⭐ (Priority: HIGH)

**Dependencies**: Core ML, Metal, Neural Engine  
**Reference**: https://github.com/apple/ml-sharp

### 25.1 Core Classes
- [x] Sharp::Gaussian 構造体
- [x] Sharp::GaussianCloud クラス
- [x] Sharp::SharpModel クラス
- [x] Sharp::GaussianRenderer クラス

### 25.2 Model Integration
- [x] PyTorch → Core ML 変換スクリプト
- [x] .mlmodelc バンドル
- [x] Neural Engine 最適化

### 25.3 Inference
- [x] predict(ofPixels&)
- [x] predict(ofTexture&)
- [x] predictAsync()
- [x] バッチ推論

### 25.4 Gaussian Cloud
- [x] PLY 読み込み / 書き出し
- [x] Metal Buffer 生成
- [x] 変換 (translate, rotate, scale)
- [x] フィルタリング

### 25.5 Renderer
- [x] 深度ソート (GPU)
- [x] Alpha blending
- [x] Covariance → 2D projection
- [x] Anti-aliasing

### 25.6 Scene Management
- [x] Sharp::SharpScene クラス
- [x] 複数オブジェクト管理
- [x] シーン保存 / 読み込み

### 25.7 Camera Path
- [x] Sharp::CameraPath クラス
- [x] Orbit / Dolly / Spiral
- [x] キーフレーム補間

### 25.8 Video Export
- [x] Sharp::VideoExporter クラス
- [x] H.265 / ProRes 出力
- [x] 進捗コールバック

### 25.9 ofxSharp Wrapper
- [ ] ofxSharp クラス実装
- [ ] setup()
- [ ] generateFromImage()
- [ ] drawCloud()

### 25.10 Examples
- [ ] sharp_basic example
- [ ] sharp_scene example
- [ ] sharp_video_export example
- [ ] sharp_realtime_camera example

---

## Phase 26: ofxNeuralEngine

**Dependencies**: Core ML, Vision

### 26.1 Image Classification
- [ ] ImageClassifier クラス
- [ ] load() / classify()
- [ ] Top-K 結果

### 26.2 Pose Estimation
- [ ] PoseEstimator クラス
- [ ] VNDetectHumanBodyPoseRequest 統合
- [ ] スケルトン描画

### 26.3 Depth Estimation
- [ ] DepthEstimator クラス
- [ ] estimate(ofPixels&)
- [ ] GPU 直接処理

### 26.4 Style Transfer
- [ ] StyleTransfer クラス
- [ ] load() / transfer()
- [ ] GPU バージョン

### 26.5 Person Segmentation
- [ ] PersonSegmentation クラス
- [ ] segment() - マスク生成

### 26.6 Generic Model
- [ ] GenericModel クラス
- [ ] setInput() / predict() / getOutput()
- [ ] 任意の Core ML モデル対応

### 26.7 Examples
- [ ] neural_classify example
- [ ] neural_pose example
- [ ] neural_depth example
- [ ] neural_style example

---

## Phase 27: ofxMetalCompute

**Dependencies**: Metal

### 27.1 Core
- [ ] ofxMetalCompute クラス
- [ ] load() / loadFromSource()
- [ ] dispatch() variants

### 27.2 Buffer Management
- [ ] setBuffer(index, ofBufferObject&)
- [ ] setBuffer(index, vector<float>&)
- [ ] readBuffer()

### 27.3 Texture Support
- [ ] setTexture(index, ofTexture&, write)
- [ ] 読み書きテクスチャ

### 27.4 Uniforms
- [ ] setUniform(name, float)
- [ ] setUniform(name, int)
- [ ] setUniform(name, vec3/vec4)

### 27.5 Synchronization
- [ ] waitUntilCompleted()
- [ ] 非同期実行

### 27.6 Helpers
- [ ] ofxComputeFilter クラス
- [ ] blur / sobel / threshold

### 27.7 Examples
- [ ] compute_basic example
- [ ] compute_particles example (100万パーティクル)
- [ ] compute_image_filter example

---

## Phase 28: ofxMPS

**Dependencies**: MetalPerformanceShaders.framework

### 28.1 Image Filters
- [ ] gaussianBlur()
- [ ] boxBlur()
- [ ] tentBlur()

### 28.2 Edge Detection
- [ ] sobel()
- [ ] laplacian()
- [ ] canny()

### 28.3 Morphology
- [ ] erode()
- [ ] dilate()

### 28.4 Color
- [ ] colorConvert()
- [ ] histogram()
- [ ] histogramEqualization()

### 28.5 Transform
- [ ] scale()
- [ ] lanczosScale()

### 28.6 Compositing
- [ ] add() / subtract() / multiply()

### 28.7 Examples
- [ ] mps_filters example
- [ ] mps_histogram example

---

## Phase 29: ofxVideoToolbox

**Dependencies**: VideoToolbox, AVFoundation

### 29.1 Encoder
- [ ] ofxVideoEncoder クラス
- [ ] H.264 / H.265 対応
- [ ] ProRes 422 / 4444 対応

### 29.2 Settings
- [ ] 解像度 (4K, 8K)
- [ ] フレームレート
- [ ] ビットレート
- [ ] Hardware/Software 切り替え

### 29.3 Input
- [ ] addFrame(ofPixels&)
- [ ] addFrame(ofTexture&) - Zero-copy

### 29.4 Decoder
- [ ] ofxVideoDecoder クラス
- [ ] load() / seekTo()
- [ ] getFrame()

### 29.5 High-Res Player
- [ ] ofxHighResVideoPlayer クラス
- [ ] 4K/8K リアルタイム再生

### 29.6 Examples
- [ ] video_encode_h265 example
- [ ] video_encode_prores example
- [ ] video_decode example

---

## Phase 30: ofxSpatialAudio

**Dependencies**: PHASE.framework

### 30.1 Engine
- [ ] ofxSpatialAudio クラス
- [ ] setup() / update()

### 30.2 Listener
- [ ] setListenerPosition()
- [ ] setListenerOrientation()

### 30.3 Sources
- [ ] Source クラス
- [ ] load() / play() / stop()
- [ ] setPosition() / setVelocity()
- [ ] setDirectivity() / setRolloff()

### 30.4 Environment
- [ ] setReverb(preset)
- [ ] setReverbFromMesh()

### 30.5 Head Tracking
- [ ] enableHeadTracking()
- [ ] isHeadTrackingAvailable()
- [ ] getHeadOrientation()

### 30.6 Examples
- [ ] spatial_audio_basic example
- [ ] spatial_audio_3d example

---

## Phase 31: Additional Apple Native Addons

### 31.1 ofxMetalFX
- [ ] setup(inputW, inputH, outputW, outputH)
- [ ] Spatial / Temporal モード
- [ ] upscale()

### 31.2 ofxUnifiedMemory
- [ ] ofxUnifiedBuffer<T> テンプレート
- [ ] allocate() / data()
- [ ] getMetalBuffer()

### 31.3 ofxLiveText
- [ ] recognize(ofPixels&)
- [ ] recognizeBarcodes()
- [ ] setLanguages()

### 31.4 ofxObjectCapture
- [ ] processImages(folderPath)
- [ ] getMesh() / getTexture()
- [ ] exportUSDZ()

---

## Phase 32: Examples & Documentation

### 32.1 Addon Examples
- [ ] 全 addon の example 完成
- [ ] README.md 各 addon

### 32.2 Migration Guide
- [ ] oF → oflike 移行ガイド
- [ ] API 差分一覧

### 32.3 API Documentation
- [ ] Doxygen 設定
- [ ] 全 public API ドキュメント

---

## 進捗サマリー

### Core Framework (Phase 0-18)

| Phase | 名称 | 完了率 |
|-------|------|--------|
| 0-16 | Core | 100% |
| 17-18 | Samples & Polish | 65% |

**Core 全体**: 97.5% 完了

### Core Addons (Phase 19-24)

| Phase | Addon | 完了率 |
|-------|-------|--------|
| 19 | ofxOsc | 0% |
| 20 | ofxGui | 0% |
| 21 | ofxXmlSettings | 0% |
| 22 | ofxSvg | 0% |
| 23 | ofxNetwork | 0% |
| 24 | ofxOpenCv | 0% |

**Core Addons 全体**: 0% 完了

### Apple Native Addons (Phase 25-31)

| Phase | Addon | Priority | 完了率 |
|-------|-------|----------|--------|
| 25 | **ofxSharp** | ⭐ HIGH | 0% |
| 26 | ofxNeuralEngine | HIGH | 0% |
| 27 | ofxMetalCompute | HIGH | 0% |
| 28 | ofxMPS | MEDIUM | 0% |
| 29 | ofxVideoToolbox | MEDIUM | 0% |
| 30 | ofxSpatialAudio | MEDIUM | 0% |
| 31 | Additional | LOW | 0% |

**Apple Native Addons 全体**: 0% 完了

---

## 優先順位

### 最優先 (Phase 25)
1. **ofxSharp** - Apple SHARP 3DGS統合

### 高優先 (Phase 19, 26-27)
2. **ofxOsc** - クリエイティブコーディング必須
3. **ofxNeuralEngine** - ML推論
4. **ofxMetalCompute** - GPU計算

### 中優先 (Phase 20-24, 28-30)
5. **ofxGui** - パラメータ調整
6. **ofxMPS** - 画像処理
7. **ofxVideoToolbox** - ビデオ出力
8. その他
---

