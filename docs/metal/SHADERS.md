# Metal Shader Design

Metal シェーダーの設計です。

---

## シェーダーファイル構成

```
src/render/metal/
├── Shaders.metal       ← 2D シェーダー
└── Shaders3D.metal     ← 3D シェーダー（予定）
```

---

## Uniform 構造体

### 2D Uniforms

```metal
struct Uniforms2D {
    float4x4 projectionMatrix;
    float4x4 modelViewMatrix;
};
```

### 3D Uniforms

```metal
struct Uniforms3D {
    float4x4 modelMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 normalMatrix;  // transpose(inverse(modelView))
    float3 cameraPosition;
    float pad0;
};
```

### C++ 対応構造体

```cpp
// 16バイトアライメント必須
struct Uniforms3D {
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::mat4 normalMatrix;
    glm::vec3 cameraPosition;
    float pad0;
};
```

---

## ライト構造体

### Metal

```metal
struct Light {
    float3 position;
    float pad0;
    float3 direction;
    float pad1;
    float3 diffuseColor;
    float pad2;
    float3 specularColor;
    float pad3;
    float3 ambientColor;
    float pad4;
    float3 attenuation;  // constant, linear, quadratic
    int type;            // 0=point, 1=directional, 2=spot
    float spotCutoff;
    float spotConcentration;
    float pad5[2];
};
```

### C++ 対応構造体

```cpp
struct LightData {
    glm::vec3 position;
    float pad0;
    glm::vec3 direction;
    float pad1;
    glm::vec3 diffuseColor;
    float pad2;
    glm::vec3 specularColor;
    float pad3;
    glm::vec3 ambientColor;
    float pad4;
    glm::vec3 attenuation;
    int type;
    float spotCutoff;
    float spotConcentration;
    float pad5[2];
};
```

---

## マテリアル構造体

### Metal

```metal
struct Material {
    float3 diffuse;
    float pad0;
    float3 specular;
    float pad1;
    float3 ambient;
    float pad2;
    float3 emissive;
    float shininess;
};
```

### C++ 対応構造体

```cpp
struct MaterialData {
    glm::vec3 diffuse;
    float pad0;
    glm::vec3 specular;
    float pad1;
    glm::vec3 ambient;
    float pad2;
    glm::vec3 emissive;
    float shininess;
};
```

---

## 頂点シェーダー

### 2D 頂点シェーダー

```metal
struct VertexIn2D {
    float2 position [[attribute(0)]];
    float4 color [[attribute(1)]];
    float2 texCoord [[attribute(2)]];
};

struct VertexOut2D {
    float4 position [[position]];
    float4 color;
    float2 texCoord;
};

vertex VertexOut2D vertex_2d(VertexIn2D in [[stage_in]],
                              constant Uniforms2D& uniforms [[buffer(1)]]) {
    VertexOut2D out;
    float4 pos = float4(in.position, 0.0, 1.0);
    out.position = uniforms.projectionMatrix * uniforms.modelViewMatrix * pos;
    out.color = in.color;
    out.texCoord = in.texCoord;
    return out;
}
```

### 3D 頂点シェーダー

```metal
struct VertexIn3D {
    float3 position [[attribute(0)]];
    float3 normal [[attribute(1)]];
    float2 texCoord [[attribute(2)]];
    float4 color [[attribute(3)]];
};

struct VertexOut3D {
    float4 position [[position]];
    float3 worldPosition;
    float3 worldNormal;
    float2 texCoord;
    float4 color;
};

vertex VertexOut3D vertex_3d(VertexIn3D in [[stage_in]],
                              constant Uniforms3D& uniforms [[buffer(1)]]) {
    float4 worldPos = uniforms.modelMatrix * float4(in.position, 1.0);
    float4 viewPos = uniforms.viewMatrix * worldPos;
    float4 clipPos = uniforms.projectionMatrix * viewPos;

    VertexOut3D out;
    out.position = clipPos;
    out.worldPosition = worldPos.xyz;
    out.worldNormal = (uniforms.normalMatrix * float4(in.normal, 0.0)).xyz;
    out.texCoord = in.texCoord;
    out.color = in.color;
    return out;
}
```

---

## フラグメントシェーダー

### 2D フラグメントシェーダー（色のみ）

```metal
fragment float4 fragment_color(VertexOut2D in [[stage_in]]) {
    return in.color;
}
```

### 2D フラグメントシェーダー（テクスチャ）

```metal
fragment float4 fragment_texture(VertexOut2D in [[stage_in]],
                                  texture2d<float> tex [[texture(0)]],
                                  sampler samp [[sampler(0)]]) {
    float4 texColor = tex.sample(samp, in.texCoord);
    return texColor * in.color;
}
```

### 3D Phong ライティングシェーダー

```metal
fragment float4 fragment_phong(VertexOut3D in [[stage_in]],
                                constant Light* lights [[buffer(0)]],
                                constant Material& material [[buffer(1)]],
                                constant int& numLights [[buffer(2)]],
                                constant float3& cameraPosition [[buffer(3)]]) {

    float3 normal = normalize(in.worldNormal);
    float3 viewDir = normalize(cameraPosition - in.worldPosition);

    // Ambient + Emissive
    float3 result = material.ambient + material.emissive;

    for (int i = 0; i < numLights; i++) {
        Light light = lights[i];

        float3 lightDir;
        float attenuation = 1.0;

        if (light.type == 0) {
            // Point Light
            float3 lightVec = light.position - in.worldPosition;
            float distance = length(lightVec);
            lightDir = normalize(lightVec);

            // 減衰
            attenuation = 1.0 / (light.attenuation.x +
                                 light.attenuation.y * distance +
                                 light.attenuation.z * distance * distance);
        } else if (light.type == 1) {
            // Directional Light
            lightDir = normalize(-light.direction);
        } else {
            // Spot Light
            float3 lightVec = light.position - in.worldPosition;
            lightDir = normalize(lightVec);

            float theta = dot(lightDir, normalize(-light.direction));
            float epsilon = light.spotCutoff - light.spotConcentration;
            float intensity = clamp((theta - light.spotConcentration) / epsilon, 0.0, 1.0);
            attenuation *= intensity;
        }

        // Diffuse
        float diff = max(dot(normal, lightDir), 0.0);

        // Specular (Blinn-Phong)
        float3 halfDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfDir), 0.0), material.shininess);

        // 加算
        result += attenuation * diff * light.diffuseColor * material.diffuse;
        result += attenuation * spec * light.specularColor * material.specular;
    }

    return float4(result, 1.0) * in.color;
}
```

---

## バッファインデックス

### 頂点シェーダー

| Index | 用途 |
|-------|------|
| 0 | 頂点バッファ (stage_in) |
| 1 | Uniforms |

### フラグメントシェーダー

| Index | 用途 |
|-------|------|
| 0 | ライト配列 |
| 1 | マテリアル |
| 2 | ライト数 |
| 3 | カメラ位置 |

### テクスチャ/サンプラー

| Index | 用途 |
|-------|------|
| 0 | ディフューズテクスチャ |
| 1 | ノーマルマップ（予定） |
| 2 | スペキュラーマップ（予定） |

---

## シェーダーバリアント

### フラグベースの切り替え

```metal
// 頂点カラー使用
#ifdef USE_VERTEX_COLORS
    out.color = in.color;
#else
    out.color = float4(1.0);
#endif

// テクスチャ使用
#ifdef USE_TEXTURE
    float4 texColor = tex.sample(samp, in.texCoord);
#else
    float4 texColor = float4(1.0);
#endif

// ライティング使用
#ifdef USE_LIGHTING
    // Phong 計算
#else
    // Unlit
#endif
```

### 関数コンスタント（推奨）

```metal
constant bool useVertexColors [[function_constant(0)]];
constant bool useTexture [[function_constant(1)]];
constant bool useLighting [[function_constant(2)]];
constant int maxLights [[function_constant(3)]];
```

---

## 関連ファイル

- `src/render/metal/Shaders.metal` - シェーダー実装
- `src/render/metal/MetalRenderer.mm` - パイプライン設定
