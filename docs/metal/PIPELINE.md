# Metal Rendering Pipeline Design

Metal レンダリングパイプラインの設計です。

---

## パイプライン概要

```
┌─────────────────────────────────────────────────────┐
│                  Application                         │
│  ofCamera, ofNode, of3dPrimitive, ofLight, etc.     │
└────────────────────────┬────────────────────────────┘
                         │
┌────────────────────────▼────────────────────────────┐
│              Uniform Buffer 管理                     │
│  - Model/View/Projection 行列                        │
│  - ライトパラメータ (位置、色、減衰)                   │
│  - マテリアルパラメータ (Diffuse/Specular/Shininess)  │
└────────────────────────┬────────────────────────────┘
                         │
┌────────────────────────▼────────────────────────────┐
│              Metal Render Pipeline                   │
│  - Vertex Shader: MVP変換、法線変換                   │
│  - Fragment Shader: Phong ライティング               │
│  - Depth Stencil State: 深度テスト                   │
└─────────────────────────────────────────────────────┘
```

---

## レンダリングステート管理

### RenderState3D 構造体

```cpp
struct RenderState3D {
    // 変換行列
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 projectionMatrix = glm::mat4(1.0f);

    // 行列スタック
    std::stack<glm::mat4> modelMatrixStack;

    // ライト（最大8個）
    std::array<LightData, 8> lights;
    int numActiveLights = 0;

    // マテリアル
    MaterialData currentMaterial;

    // カメラ
    glm::vec3 cameraPosition;
};
```

### Engine 統合

```cpp
class Engine {
    // ...
    RenderState3D renderState3D_;

    // 行列操作
    void pushMatrix();
    void popMatrix();
    void setModelMatrix(const glm::mat4& m);
    void setViewMatrix(const glm::mat4& m);
    void setProjectionMatrix(const glm::mat4& m);

    // ライト管理
    void enableLight(int index, const LightData& data);
    void disableLight(int index);

    // マテリアル管理
    void setMaterial(const MaterialData& data);
};
```

---

## 深度バッファ

### 深度ステート設定

```objc
MTLDepthStencilDescriptor* depthDesc = [[MTLDepthStencilDescriptor alloc] init];
depthDesc.depthCompareFunction = MTLCompareFunctionLess;
depthDesc.depthWriteEnabled = YES;
id<MTLDepthStencilState> depthState = [device newDepthStencilStateWithDescriptor:depthDesc];
```

### 深度テクスチャ作成

```objc
MTLTextureDescriptor* depthDesc = [MTLTextureDescriptor
    texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                 width:width
                                height:height
                              mipmapped:NO];
depthDesc.storageMode = MTLStorageModePrivate;
depthDesc.usage = MTLTextureUsageRenderTarget;
id<MTLTexture> depthTexture = [device newTextureWithDescriptor:depthDesc];
```

### レンダーパスへの設定

```objc
MTLRenderPassDescriptor* passDesc = [MTLRenderPassDescriptor renderPassDescriptor];
passDesc.depthAttachment.texture = depthTexture;
passDesc.depthAttachment.loadAction = MTLLoadActionClear;
passDesc.depthAttachment.storeAction = MTLStoreActionDontCare;
passDesc.depthAttachment.clearDepth = 1.0;
```

---

## ブレンディング

### ブレンドステート設定

```objc
MTLRenderPipelineDescriptor* pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
pipelineDesc.colorAttachments[0].blendingEnabled = YES;

// Alpha ブレンディング（標準）
pipelineDesc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
pipelineDesc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
pipelineDesc.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;

pipelineDesc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
pipelineDesc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
pipelineDesc.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
```

### ブレンドモードマッピング

| openFrameworks | MTLBlendFactor (src/dst) |
|----------------|--------------------------|
| OF_BLENDMODE_ALPHA | SourceAlpha / OneMinusSourceAlpha |
| OF_BLENDMODE_ADD | One / One |
| OF_BLENDMODE_SUBTRACT | One / One (operation: Subtract) |
| OF_BLENDMODE_MULTIPLY | DestinationColor / Zero |
| OF_BLENDMODE_SCREEN | One / OneMinusSourceColor |

---

## 頂点フォーマット

### 2D 頂点

```cpp
struct Vertex2D {
    glm::vec2 position;
    glm::vec4 color;
    glm::vec2 texCoord;
};
```

### 3D 頂点

```cpp
struct Vertex3D {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec4 color;
};
```

### Metal 頂点ディスクリプタ

```objc
MTLVertexDescriptor* vertexDesc = [[MTLVertexDescriptor alloc] init];

// Position (float3)
vertexDesc.attributes[0].format = MTLVertexFormatFloat3;
vertexDesc.attributes[0].offset = 0;
vertexDesc.attributes[0].bufferIndex = 0;

// Normal (float3)
vertexDesc.attributes[1].format = MTLVertexFormatFloat3;
vertexDesc.attributes[1].offset = sizeof(float) * 3;
vertexDesc.attributes[1].bufferIndex = 0;

// TexCoord (float2)
vertexDesc.attributes[2].format = MTLVertexFormatFloat2;
vertexDesc.attributes[2].offset = sizeof(float) * 6;
vertexDesc.attributes[2].bufferIndex = 0;

// Color (float4)
vertexDesc.attributes[3].format = MTLVertexFormatFloat4;
vertexDesc.attributes[3].offset = sizeof(float) * 8;
vertexDesc.attributes[3].bufferIndex = 0;

vertexDesc.layouts[0].stride = sizeof(Vertex3D);
vertexDesc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
```

---

## プリミティブモードマッピング

| ofPrimitiveMode | MTLPrimitiveType | 備考 |
|-----------------|------------------|------|
| OF_PRIMITIVE_TRIANGLES | Triangle | |
| OF_PRIMITIVE_TRIANGLE_STRIP | TriangleStrip | |
| OF_PRIMITIVE_TRIANGLE_FAN | (インデックスでエミュレート) | |
| OF_PRIMITIVE_LINES | Line | |
| OF_PRIMITIVE_LINE_STRIP | LineStrip | |
| OF_PRIMITIVE_LINE_LOOP | LineStrip | 最後に先頭へ戻る |
| OF_PRIMITIVE_POINTS | Point | |

---

## パイプラインステートキャッシュ

同一設定のパイプラインを再利用してオーバーヘッドを削減。

```cpp
struct PipelineKey {
    MTLPixelFormat colorFormat;
    MTLPixelFormat depthFormat;
    bool blendingEnabled;
    MTLBlendFactor srcFactor;
    MTLBlendFactor dstFactor;
    // ...
};

class PipelineCache {
public:
    id<MTLRenderPipelineState> getOrCreate(const PipelineKey& key);
private:
    std::unordered_map<PipelineKey, id<MTLRenderPipelineState>> cache_;
};
```

---

## バッファ管理

### トリプルバッファリング

フレーム間の競合を防ぐため、3フレーム分のバッファを用意。

```cpp
static const int MAX_FRAMES_IN_FLIGHT = 3;

class BufferPool {
    std::array<id<MTLBuffer>, MAX_FRAMES_IN_FLIGHT> uniformBuffers_;
    int currentFrame_ = 0;
    dispatch_semaphore_t frameSemaphore_;
};
```

### 動的バッファ更新

```objc
// setVertexBytes を使用（小さいデータ向け、4KB以下）
[encoder setVertexBytes:&uniforms length:sizeof(uniforms) atIndex:1];

// バッファを使用（大きいデータ向け）
[encoder setVertexBuffer:uniformBuffer offset:0 atIndex:1];
```

---

## テクスチャバインディング

### テクスチャスロット

```objc
// テクスチャ
[encoder setFragmentTexture:texture atIndex:textureLocation];

// サンプラー
[encoder setFragmentSamplerState:sampler atIndex:textureLocation];
```

### サンプラーオプション

```objc
MTLSamplerDescriptor* samplerDesc = [[MTLSamplerDescriptor alloc] init];
samplerDesc.minFilter = MTLSamplerMinMagFilterLinear;
samplerDesc.magFilter = MTLSamplerMinMagFilterLinear;
samplerDesc.sAddressMode = MTLSamplerAddressModeRepeat;
samplerDesc.tAddressMode = MTLSamplerAddressModeRepeat;
```

---

## 関連ファイル

- `src/render/metal/MetalRenderer.h`
- `src/render/metal/MetalRenderer.mm`
- `src/render/DrawList.h`
