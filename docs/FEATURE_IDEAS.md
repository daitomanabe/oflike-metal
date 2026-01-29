# oflike-metal 機能拡張アイデア

## 現在の実装状況

### 実装済み ✅
- **3D**: ofEasyCam, ofMesh, ofDrawBox/Sphere/Cone/Cylinder/Plane/IcoSphere
- **Graphics**: ofGraphics (基本描画), ofPath, ofCoreText, ofPolyline
- **Image**: ofImage, ofPixels, ofTexture
- **Math**: ofVec2f/3f/4f, ofMatrix4x4, ofQuaternion, ofMath
- **Lighting**: ofLight, ofMaterial (API実装済み、シェーダー未統合)

---

## 3D モジュール拡張

### 高優先度 🔴

#### ofNode
シーングラフの基本ノード。親子関係による階層的な変換を実現。
```cpp
class ofNode {
    void setParent(ofNode& parent);
    void clearParent();
    ofVec3f getGlobalPosition();
    ofQuaternion getGlobalOrientation();
    ofMatrix4x4 getGlobalTransformMatrix();
    void lookAt(const ofVec3f& target);
};
```

#### ofCamera
汎用カメラクラス（ofEasyCamの基底クラス）
```cpp
class ofCamera : public ofNode {
    void setFov(float fov);
    void setNearClip(float near);
    void setFarClip(float far);
    void setOrtho(float left, float right, float bottom, float top);
    ofVec3f worldToScreen(const ofVec3f& world);
    ofVec3f screenToWorld(const ofVec3f& screen);
};
```

#### ofVbo / ofVboMesh
GPU側で頂点データを効率的に管理
```cpp
class ofVbo {
    void setVertexData(const float* data, int total, int usage);
    void setNormalData(const float* data, int total, int usage);
    void setTexCoordData(const float* data, int total, int usage);
    void setIndexData(const unsigned int* data, int total, int usage);
    void draw(int drawMode, int first, int total);
    void drawElements(int drawMode, int count);
};
```

### 中優先度 🟡

#### of3dPrimitive
3Dプリミティブの基底クラス
```cpp
class of3dPrimitive : public ofNode {
    ofMesh& getMesh();
    void draw();
    void drawWireframe();
    void setResolution(int res);
};

class ofBoxPrimitive : public of3dPrimitive { ... };
class ofSpherePrimitive : public of3dPrimitive { ... };
class ofCylinderPrimitive : public of3dPrimitive { ... };
class ofConePrimitive : public of3dPrimitive { ... };
class ofPlanePrimitive : public of3dPrimitive { ... };
```

#### ofModelLoader (3Dモデル読み込み)
```cpp
class ofxAssimpModelLoader {
    bool load(const string& path);  // OBJ, FBX, GLTF対応
    void draw();
    ofMesh getMesh(int index);
    int getNumMeshes();
    void setAnimation(int index);
    void update();
};
```

### 低優先度 🟢

#### スケルタルアニメーション
```cpp
class ofSkeleton {
    void addBone(const string& name, int parentIndex);
    void setPose(const vector<ofMatrix4x4>& boneTransforms);
    ofMatrix4x4 getBoneTransform(int index);
};
```

#### パーティクルシステム
```cpp
class ofParticleSystem {
    void emit(int count);
    void update(float dt);
    void draw();
    void setEmitterPosition(const ofVec3f& pos);
    void setLifetime(float min, float max);
    void setVelocity(const ofVec3f& min, const ofVec3f& max);
};
```

---

## Graphics モジュール拡張

### 高優先度 🔴

#### ofFbo (フレームバッファオブジェクト)
オフスクリーンレンダリング、ポストプロセス用
```cpp
class ofFbo {
    void allocate(int width, int height, int internalFormat = GL_RGBA);
    void begin();
    void end();
    void draw(float x, float y, float w, float h);
    ofTexture& getTexture();
    void readToPixels(ofPixels& pixels);

    // マルチサンプリング
    void allocate(int width, int height, int internalFormat, int numSamples);

    // 複数カラーアタッチメント (MRT)
    void createAndAttachTexture(int attachmentPoint);
};
```

#### ofShader
カスタムシェーダー管理
```cpp
class ofShader {
    bool load(const string& vertPath, const string& fragPath);
    bool load(const string& shaderPath);  // .metal ファイル
    void begin();
    void end();

    void setUniform1f(const string& name, float v);
    void setUniform2f(const string& name, float v1, float v2);
    void setUniform3f(const string& name, const ofVec3f& v);
    void setUniform4f(const string& name, const ofVec4f& v);
    void setUniformMatrix4f(const string& name, const ofMatrix4x4& m);
    void setUniformTexture(const string& name, const ofTexture& tex, int location);
};
```

#### ofTrueTypeFont 拡張
```cpp
class ofTrueTypeFont {
    // 既存機能に追加
    void drawStringAsShapes(const string& text, float x, float y);
    ofPath getCharacterAsPath(int character);
    vector<ofPath> getStringAsPath(const string& text);
    float getKerning(int c1, int c2);

    // マルチライン対応
    void drawMultiLine(const string& text, float x, float y, float lineHeight);
    ofRectangle getMultiLineBoundingBox(const string& text, float x, float y);
};
```

### 中優先度 🟡

#### ブレンドモード拡張
```cpp
enum ofBlendMode {
    OF_BLENDMODE_DISABLED,
    OF_BLENDMODE_ALPHA,
    OF_BLENDMODE_ADD,
    OF_BLENDMODE_SUBTRACT,
    OF_BLENDMODE_MULTIPLY,
    OF_BLENDMODE_SCREEN,
    OF_BLENDMODE_OVERLAY,    // 追加
    OF_BLENDMODE_SOFT_LIGHT, // 追加
    OF_BLENDMODE_HARD_LIGHT, // 追加
    OF_BLENDMODE_DIFFERENCE, // 追加
};

void ofEnableBlendMode(ofBlendMode mode);
```

#### ofGraphics 追加関数
```cpp
// グラデーション
void ofDrawGradient(const ofColor& start, const ofColor& end, ofGradientMode mode);

// 点線・破線
void ofSetLineStipple(int factor, unsigned short pattern);

// アンチエイリアス制御
void ofEnableAntiAliasing();
void ofDisableAntiAliasing();
void ofSetSampleCount(int samples);

// クリッピング
void ofEnableScissor(float x, float y, float w, float h);
void ofDisableScissor();
```

### 低優先度 🟢

#### ポストプロセスエフェクト
```cpp
class ofPostProcess {
    void setup(int width, int height);
    void begin();
    void end();

    void addEffect(shared_ptr<ofPostEffect> effect);
    void removeEffect(int index);

    void draw();
};

class ofBloomEffect : public ofPostEffect { ... };
class ofBlurEffect : public ofPostEffect { ... };
class ofColorCorrectionEffect : public ofPostEffect { ... };
class ofVignetteEffect : public ofPostEffect { ... };
```

---

## Image モジュール拡張

### 高優先度 🔴

#### ofTexture 拡張
```cpp
class ofTexture {
    // 既存機能に追加
    void setTextureWrap(GLint wrapS, GLint wrapT);
    void setTextureMinMagFilter(GLint minFilter, GLint magFilter);
    void generateMipmap();

    // キューブマップ
    void loadCubeMap(const string paths[6]);

    // 3Dテクスチャ
    void allocate3D(int width, int height, int depth, int format);
};
```

#### ofImage 拡張
```cpp
class ofImage {
    // 既存機能に追加

    // フィルタ
    void blur(int radius);
    void sharpen(float amount);
    void contrast(float amount);
    void brightness(float amount);
    void saturation(float amount);

    // 色変換
    void convertToGrayscale();
    void invertColors();
    void applyColorMatrix(const float matrix[16]);

    // リサイズ品質
    void resize(int w, int h, ofInterpolationMethod method);
    enum ofInterpolationMethod {
        OF_INTERPOLATE_NEAREST_NEIGHBOR,
        OF_INTERPOLATE_BILINEAR,
        OF_INTERPOLATE_BICUBIC,
        OF_INTERPOLATE_LANCZOS
    };
};
```

### 中優先度 🟡

#### ofVideoPlayer
```cpp
class ofVideoPlayer {
    bool load(const string& path);
    void play();
    void pause();
    void stop();
    void setPosition(float pct);  // 0.0 - 1.0
    void setSpeed(float speed);
    void setLoopState(ofLoopType state);

    void update();
    void draw(float x, float y, float w, float h);

    float getPosition();
    float getDuration();
    bool isPlaying();
    bool isLoaded();

    ofPixels& getPixels();
    ofTexture& getTexture();
};
```

#### ofVideoGrabber
```cpp
class ofVideoGrabber {
    vector<ofVideoDevice> listDevices();
    bool setup(int width, int height);
    bool setup(int width, int height, int deviceId);

    void update();
    bool isFrameNew();
    void draw(float x, float y, float w, float h);

    ofPixels& getPixels();
    ofTexture& getTexture();

    void setDesiredFrameRate(int fps);
    void close();
};
```

### 低優先度 🟢

#### HDR / 高精度ピクセル
```cpp
class ofFloatImage {
    void allocate(int width, int height, ofImageType type);
    void load(const string& path);  // EXR, HDR対応
    void save(const string& path);

    ofFloatPixels& getPixels();
    void setFromPixels(const ofFloatPixels& pixels);

    // トーンマッピング
    ofImage tonemapReinhard(float exposure);
    ofImage tonemapACES(float exposure);
};
```

#### 画像フォーマット拡張
```cpp
// 追加フォーマット対応
// - WebP (読み書き)
// - HEIC/HEIF (読み込み)
// - RAW (読み込み)
// - DDS (読み込み, GPUテクスチャ圧縮)
```

---

## その他のモジュール案

### Audio
```cpp
class ofSoundPlayer {
    bool load(const string& path);
    void play();
    void stop();
    void setVolume(float vol);
    void setPan(float pan);
    void setSpeed(float speed);
    void setLoop(bool loop);
    bool isPlaying();
};

class ofSoundStream {
    void setup(int outChannels, int inChannels, int sampleRate, int bufferSize);
    void setOutput(ofBaseSoundOutput* output);
    void setInput(ofBaseSoundInput* input);
};
```

### Events / Communication
```cpp
class ofOscReceiver {
    void setup(int port);
    bool hasWaitingMessages();
    bool getNextMessage(ofxOscMessage& msg);
};

class ofOscSender {
    void setup(const string& host, int port);
    void sendMessage(const ofxOscMessage& msg);
};
```

---

## 実装優先順位まとめ

### Phase 1 (基盤強化)
1. ofFbo - オフスクリーンレンダリング必須
2. ofShader - カスタムシェーダー対応
3. ofNode/ofCamera - シーングラフ基盤

### Phase 2 (表現力向上)
4. ofVbo/ofVboMesh - パフォーマンス最適化
5. Shader-based Lighting - リアルなライティング
6. ofTrueTypeFont拡張 - テキスト表現

### Phase 3 (メディア対応)
7. ofVideoPlayer - 動画再生
8. ofVideoGrabber - カメラ入力
9. ofSoundPlayer - 音声再生

### Phase 4 (高度な機能)
10. ofModelLoader - 3Dモデル読み込み
11. ポストプロセス - 視覚エフェクト
12. パーティクルシステム

---

*最終更新: 2026-01-29*
