#import "ofTexture.h"
#import "../../render/DrawList.h"
#import "../../render/DrawCommand.h"
#import "../../render/RenderTypes.h"
#import "../../render/IRenderer.h"
#import "../../core/Context.h"
#import "../graphics/ofGraphics.h"
#import "../math/ofMatrix4x4.h"
#include <vector>

namespace oflike {

// ============================================================================
// Helper Functions
// ============================================================================

/// Convert ofTexwrapMode_t to render::TextureWrap
static render::TextureWrap ToRenderWrap(ofTexwrapMode_t mode) {
    switch (mode) {
        case OF_TEXTURE_WRAP_REPEAT:
            return render::TextureWrap::Repeat;
        case OF_TEXTURE_WRAP_CLAMP_TO_EDGE:
            return render::TextureWrap::Clamp;
        case OF_TEXTURE_WRAP_MIRRORED_REPEAT:
            return render::TextureWrap::Mirror;
        default:
            return render::TextureWrap::Clamp;
    }
}

/// Convert ofTexFilterMode_t to render::TextureFilter
static render::TextureFilter ToRenderFilter(ofTexFilterMode_t mode) {
    switch (mode) {
        case OF_TEXTURE_FILTER_NEAREST:
            return render::TextureFilter::Nearest;
        case OF_TEXTURE_FILTER_LINEAR:
            return render::TextureFilter::Linear;
        default:
            return render::TextureFilter::Linear;
    }
}

/// Convert ofImageType to render::TextureFormat (8-bit)
static render::TextureFormat ImageTypeToTextureFormat(int imageType) {
    switch (imageType) {
        case OF_IMAGE_GRAYSCALE:
            return render::TextureFormat::R8;
        case OF_IMAGE_COLOR:
            return render::TextureFormat::RGBA8; // RGB stored as RGBA
        case OF_IMAGE_COLOR_ALPHA:
            return render::TextureFormat::RGBA8;
        default:
            return render::TextureFormat::RGBA8;
    }
}

/// Convert ofImageType to render::TextureFormat (16-bit unsigned integer)
static render::TextureFormat ImageTypeToTextureFormat16(int imageType) {
    switch (imageType) {
        case OF_IMAGE_GRAYSCALE:
            return render::TextureFormat::R16;
        case OF_IMAGE_COLOR:
            return render::TextureFormat::RGBA16; // RGB stored as RGBA
        case OF_IMAGE_COLOR_ALPHA:
            return render::TextureFormat::RGBA16;
        default:
            return render::TextureFormat::RGBA16;
    }
}

/// Convert ofImageType to render::TextureFormat (32-bit float)
static render::TextureFormat ImageTypeToTextureFormat32F(int imageType) {
    switch (imageType) {
        case OF_IMAGE_GRAYSCALE:
            return render::TextureFormat::R32F;
        case OF_IMAGE_COLOR:
            return render::TextureFormat::RGBA32F; // RGB stored as RGBA
        case OF_IMAGE_COLOR_ALPHA:
            return render::TextureFormat::RGBA32F;
        default:
            return render::TextureFormat::RGBA32F;
    }
}

/// Get channels from image type
static size_t GetChannelsFromImageType(int imageType) {
    switch (imageType) {
        case OF_IMAGE_GRAYSCALE:
            return 1;
        case OF_IMAGE_COLOR:
            return 3;
        case OF_IMAGE_COLOR_ALPHA:
            return 4;
        default:
            return 4;
    }
}

/// Convert RGB to RGBA (adds alpha channel) - 8-bit
static void ConvertRGBToRGBA(const unsigned char* src, unsigned char* dst,
                             size_t width, size_t height) {
    for (size_t i = 0; i < width * height; ++i) {
        dst[i * 4 + 0] = src[i * 3 + 0]; // R
        dst[i * 4 + 1] = src[i * 3 + 1]; // G
        dst[i * 4 + 2] = src[i * 3 + 2]; // B
        dst[i * 4 + 3] = 255;            // A (opaque)
    }
}

/// Convert RGB to RGBA (adds alpha channel) - 16-bit
static void ConvertRGBToRGBA16(const uint16_t* src, uint16_t* dst,
                               size_t width, size_t height) {
    for (size_t i = 0; i < width * height; ++i) {
        dst[i * 4 + 0] = src[i * 3 + 0]; // R
        dst[i * 4 + 1] = src[i * 3 + 1]; // G
        dst[i * 4 + 2] = src[i * 3 + 2]; // B
        dst[i * 4 + 3] = 65535;          // A (opaque, max 16-bit)
    }
}

/// Convert RGB to RGBA (adds alpha channel) - float
static void ConvertRGBToRGBAFloat(const float* src, float* dst,
                                  size_t width, size_t height) {
    for (size_t i = 0; i < width * height; ++i) {
        dst[i * 4 + 0] = src[i * 3 + 0]; // R
        dst[i * 4 + 1] = src[i * 3 + 1]; // G
        dst[i * 4 + 2] = src[i * 3 + 2]; // B
        dst[i * 4 + 3] = 1.0f;           // A (opaque)
    }
}

// ============================================================================
// ofTexture::Impl
// ============================================================================

struct ofTexture::Impl {
    void* textureHandle = nullptr;  // Opaque texture handle (no Metal types)
    int width = 0;
    int height = 0;
    int internalFormat = OF_IMAGE_COLOR_ALPHA;
    bool bAllocated = false;
    render::TextureWrap wrapS = render::TextureWrap::Clamp;
    render::TextureWrap wrapT = render::TextureWrap::Clamp;
    render::TextureFilter minFilter = render::TextureFilter::Linear;
    render::TextureFilter magFilter = render::TextureFilter::Linear;

    // Phase 2: Mipmap support
    bool mipmapEnabled = false;
    render::TextureFilter mipmapFilter = render::TextureFilter::Linear;
    int numMipmapLevels = 1;
    int maxAnisotropy = 1;

    Impl() = default;

    ~Impl() {
        if (textureHandle) {
            // Release texture through Context/Renderer
            // TODO: Add proper texture release path through Context
            textureHandle = nullptr;
        }
    }
};

// ============================================================================
// ofTexture Implementation
// ============================================================================

ofTexture::ofTexture()
    : impl_(std::make_unique<Impl>()) {
}

ofTexture::~ofTexture() = default;

ofTexture::ofTexture(ofTexture&& other) noexcept = default;

ofTexture& ofTexture::operator=(ofTexture&& other) noexcept = default;

void ofTexture::ensureImpl() {
    if (!impl_) {
        impl_ = std::make_unique<Impl>();
    }
}

// ============================================================================
// Allocation
// ============================================================================

void ofTexture::allocate(int w, int h, int internalFormat) {
    ensureImpl();

    render::TextureFormat format = ImageTypeToTextureFormat(internalFormat);

    // Create texture through Context/Renderer (respecting layer boundaries)
    // TODO: Implement Context::createTexture() API for proper texture allocation
    // For now, we mark as allocated and defer actual creation to draw/bind time
    impl_->width = w;
    impl_->height = h;
    impl_->internalFormat = internalFormat;
    impl_->bAllocated = true;
}

void ofTexture::allocate(const ofPixels& pix) {
    allocate(
        static_cast<int>(pix.getWidth()),
        static_cast<int>(pix.getHeight()),
        static_cast<int>(pix.getImageType())
    );
}

bool ofTexture::isAllocated() const {
    return impl_ && impl_->bAllocated;
}

void ofTexture::clear() {
    if (impl_) {
        // Release texture through Context/Renderer
        // TODO: Implement proper texture release through Context
        impl_->textureHandle = nullptr;
        impl_->width = 0;
        impl_->height = 0;
        impl_->bAllocated = false;
    }
}

// ============================================================================
// Data Upload
// ============================================================================

void ofTexture::loadData(const ofPixels& pix) {
    const int w = static_cast<int>(pix.getWidth());
    const int h = static_cast<int>(pix.getHeight());
    const int format = static_cast<int>(pix.getImageType());

    loadData(pix.getData(), w, h, format);
}

void ofTexture::loadData(const ofShortPixels& pix) {
    ensureImpl();

    const int w = static_cast<int>(pix.getWidth());
    const int h = static_cast<int>(pix.getHeight());
    const int channels = static_cast<int>(pix.getNumChannels());

    if (w <= 0 || h <= 0 || !pix.getData()) {
        return;
    }

    // Determine image type from channels
    int imageType = OF_IMAGE_COLOR_ALPHA;
    if (channels == 1) {
        imageType = OF_IMAGE_GRAYSCALE;
    } else if (channels == 3) {
        imageType = OF_IMAGE_COLOR;
    } else if (channels == 4) {
        imageType = OF_IMAGE_COLOR_ALPHA;
    }

    // Convert 16-bit to 8-bit RGBA for Metal
    std::vector<unsigned char> rgbaData(w * h * 4);
    const uint16_t* srcData = pix.getData();

    if (channels == 1) {
        // Grayscale 16-bit to RGBA 8-bit
        for (int i = 0; i < w * h; ++i) {
            unsigned char v = static_cast<unsigned char>(srcData[i] >> 8);
            rgbaData[i * 4 + 0] = v;
            rgbaData[i * 4 + 1] = v;
            rgbaData[i * 4 + 2] = v;
            rgbaData[i * 4 + 3] = 255;
        }
    } else if (channels == 3) {
        // RGB 16-bit to RGBA 8-bit
        for (int i = 0; i < w * h; ++i) {
            rgbaData[i * 4 + 0] = static_cast<unsigned char>(srcData[i * 3 + 0] >> 8);
            rgbaData[i * 4 + 1] = static_cast<unsigned char>(srcData[i * 3 + 1] >> 8);
            rgbaData[i * 4 + 2] = static_cast<unsigned char>(srcData[i * 3 + 2] >> 8);
            rgbaData[i * 4 + 3] = 255;
        }
    } else {
        // RGBA 16-bit to RGBA 8-bit
        for (int i = 0; i < w * h; ++i) {
            rgbaData[i * 4 + 0] = static_cast<unsigned char>(srcData[i * 4 + 0] >> 8);
            rgbaData[i * 4 + 1] = static_cast<unsigned char>(srcData[i * 4 + 1] >> 8);
            rgbaData[i * 4 + 2] = static_cast<unsigned char>(srcData[i * 4 + 2] >> 8);
            rgbaData[i * 4 + 3] = static_cast<unsigned char>(srcData[i * 4 + 3] >> 8);
        }
    }

    // Use the main loadData to upload
    loadData(rgbaData.data(), w, h, OF_IMAGE_COLOR_ALPHA);
}

void ofTexture::loadData(const ofFloatPixels& pix) {
    ensureImpl();

    const int w = static_cast<int>(pix.getWidth());
    const int h = static_cast<int>(pix.getHeight());
    const int channels = static_cast<int>(pix.getNumChannels());

    if (w <= 0 || h <= 0 || !pix.getData()) {
        return;
    }

    // Determine image type from channels
    int imageType = OF_IMAGE_COLOR_ALPHA;
    if (channels == 1) {
        imageType = OF_IMAGE_GRAYSCALE;
    } else if (channels == 3) {
        imageType = OF_IMAGE_COLOR;
    } else if (channels == 4) {
        imageType = OF_IMAGE_COLOR_ALPHA;
    }

    // Convert float to 8-bit RGBA for Metal
    std::vector<unsigned char> rgbaData(w * h * 4);
    const float* srcData = pix.getData();

    auto clampToByte = [](float v) -> unsigned char {
        return static_cast<unsigned char>(std::max(0.0f, std::min(1.0f, v)) * 255.0f);
    };

    if (channels == 1) {
        // Grayscale float to RGBA 8-bit
        for (int i = 0; i < w * h; ++i) {
            unsigned char v = clampToByte(srcData[i]);
            rgbaData[i * 4 + 0] = v;
            rgbaData[i * 4 + 1] = v;
            rgbaData[i * 4 + 2] = v;
            rgbaData[i * 4 + 3] = 255;
        }
    } else if (channels == 3) {
        // RGB float to RGBA 8-bit
        for (int i = 0; i < w * h; ++i) {
            rgbaData[i * 4 + 0] = clampToByte(srcData[i * 3 + 0]);
            rgbaData[i * 4 + 1] = clampToByte(srcData[i * 3 + 1]);
            rgbaData[i * 4 + 2] = clampToByte(srcData[i * 3 + 2]);
            rgbaData[i * 4 + 3] = 255;
        }
    } else {
        // RGBA float to RGBA 8-bit
        for (int i = 0; i < w * h; ++i) {
            rgbaData[i * 4 + 0] = clampToByte(srcData[i * 4 + 0]);
            rgbaData[i * 4 + 1] = clampToByte(srcData[i * 4 + 1]);
            rgbaData[i * 4 + 2] = clampToByte(srcData[i * 4 + 2]);
            rgbaData[i * 4 + 3] = clampToByte(srcData[i * 4 + 3]);
        }
    }

    // Use the main loadData to upload
    loadData(rgbaData.data(), w, h, OF_IMAGE_COLOR_ALPHA);
}

void ofTexture::loadData(const void* data, int w, int h, int glFormat) {
    ensureImpl();

    if (!data || w <= 0 || h <= 0) {
        return;
    }

    // Release old texture if exists
    if (impl_->textureHandle) {
        auto* renderer = Context::instance().renderer();
        if (renderer) {
            renderer->destroyTexture(impl_->textureHandle);
        }
        impl_->textureHandle = nullptr;
    }

    // Update dimensions and format
    impl_->width = w;
    impl_->height = h;
    impl_->internalFormat = glFormat;

    // Determine number of channels from format
    size_t channels = 4;  // Default to RGBA
    if (glFormat == OF_IMAGE_GRAYSCALE || glFormat == 1) {
        channels = 1;
    } else if (glFormat == OF_IMAGE_COLOR || glFormat == 3) {
        channels = 3;
    } else if (glFormat == OF_IMAGE_COLOR_ALPHA || glFormat == 4) {
        channels = 4;
    }

    // Prepare RGBA data for Metal (Metal requires RGBA8)
    std::vector<unsigned char> rgbaData;
    const unsigned char* uploadData = static_cast<const unsigned char*>(data);

    if (channels == 1) {
        // Convert grayscale to RGBA
        rgbaData.resize(w * h * 4);
        for (int i = 0; i < w * h; ++i) {
            unsigned char v = uploadData[i];
            rgbaData[i * 4 + 0] = v;
            rgbaData[i * 4 + 1] = v;
            rgbaData[i * 4 + 2] = v;
            rgbaData[i * 4 + 3] = 255;
        }
        uploadData = rgbaData.data();
    } else if (channels == 3) {
        // Convert RGB to RGBA
        rgbaData.resize(w * h * 4);
        ConvertRGBToRGBA(static_cast<const unsigned char*>(data), rgbaData.data(), w, h);
        uploadData = rgbaData.data();
    }
    // For channels == 4, use data directly

    // Create texture through renderer
    auto* renderer = Context::instance().renderer();
    if (renderer) {
        impl_->textureHandle = renderer->createTexture(w, h, uploadData);
        impl_->bAllocated = (impl_->textureHandle != nullptr);
    }
}

// ============================================================================
// Drawing
// ============================================================================

void ofTexture::draw(float x, float y) const {
    if (!isAllocated()) {
        return;
    }
    draw(x, y, static_cast<float>(impl_->width), static_cast<float>(impl_->height));
}

void ofTexture::draw(float x, float y, float w, float h) const {
    if (!isAllocated()) {
        return;
    }

    @autoreleasepool {
        // Create textured quad vertices
        uint8_t r, g, b, a;
        ::ofGetColor(r, g, b, a);
        simd_float4 color = simd_make_float4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);

        // Define quad vertices (2 triangles)
        std::vector<render::Vertex2D> vertices = {
            render::Vertex2D(x,     y,     0.0f, 0.0f, color.x, color.y, color.z, color.w),  // Top-left
            render::Vertex2D(x + w, y,     1.0f, 0.0f, color.x, color.y, color.z, color.w),  // Top-right
            render::Vertex2D(x + w, y + h, 1.0f, 1.0f, color.x, color.y, color.z, color.w),  // Bottom-right
            render::Vertex2D(x,     y + h, 0.0f, 1.0f, color.x, color.y, color.z, color.w),  // Bottom-left
        };

        // Define quad indices (2 triangles)
        std::vector<uint32_t> indices = {
            0, 1, 2,  // First triangle
            0, 2, 3   // Second triangle
        };

        // Add vertices and indices to DrawList
        auto& drawList = Context::instance().getDrawList();
        uint32_t vtxOffset = drawList.addVertices2D(vertices);
        uint32_t idxOffset = drawList.addIndices(indices);

        // Create draw command with texture
        render::DrawCommand2D cmd;
        cmd.vertexOffset = vtxOffset;
        cmd.vertexCount = static_cast<uint32_t>(vertices.size());
        cmd.indexOffset = idxOffset;
        cmd.indexCount = static_cast<uint32_t>(indices.size());
        cmd.primitiveType = render::PrimitiveType::Triangle;
        cmd.blendMode = render::BlendMode::Alpha;
        cmd.texture = getNativeHandle();  // Texture handle

        // Get current transform matrix
        auto m = ::ofGetCurrentMatrix();
        cmd.transform = simd_matrix(
            simd_make_float4(m(0,0), m(1,0), m(2,0), m(3,0)),
            simd_make_float4(m(0,1), m(1,1), m(2,1), m(3,1)),
            simd_make_float4(m(0,2), m(1,2), m(2,2), m(3,2)),
            simd_make_float4(m(0,3), m(1,3), m(2,3), m(3,3))
        );

        drawList.addCommand(cmd);
    }
}

void ofTexture::draw(float x, float y, float z) const {
    draw(x, y, z, static_cast<float>(impl_->width), static_cast<float>(impl_->height));
}

void ofTexture::draw(float x, float y, float z, float w, float h) const {
    if (!isAllocated()) {
        return;
    }

    @autoreleasepool {
        // Create textured quad vertices in 3D
        uint8_t r, g, b, a;
        ::ofGetColor(r, g, b, a);
        simd_float4 color = simd_make_float4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);

        // Define quad vertices (2 triangles) in 3D space
        std::vector<render::Vertex3D> vertices;
        vertices.reserve(4);

        simd_float3 normal = simd_make_float3(0.0f, 0.0f, 1.0f);  // Facing forward

        vertices.push_back(render::Vertex3D(
            simd_make_float3(x, y, z),           // Position
            normal,                              // Normal
            simd_make_float2(0.0f, 0.0f),       // UV
            color                                // Color
        ));
        vertices.push_back(render::Vertex3D(
            simd_make_float3(x + w, y, z),
            normal,
            simd_make_float2(1.0f, 0.0f),
            color
        ));
        vertices.push_back(render::Vertex3D(
            simd_make_float3(x + w, y + h, z),
            normal,
            simd_make_float2(1.0f, 1.0f),
            color
        ));
        vertices.push_back(render::Vertex3D(
            simd_make_float3(x, y + h, z),
            normal,
            simd_make_float2(0.0f, 1.0f),
            color
        ));

        // Define quad indices (2 triangles)
        std::vector<uint32_t> indices = {
            0, 1, 2,  // First triangle
            0, 2, 3   // Second triangle
        };

        // Add vertices and indices to DrawList
        auto& drawList = Context::instance().getDrawList();
        uint32_t vtxOffset = drawList.addVertices3D(vertices);
        uint32_t idxOffset = drawList.addIndices(indices);

        // Create 3D draw command with texture
        render::DrawCommand3D cmd;
        cmd.vertexOffset = vtxOffset;
        cmd.vertexCount = static_cast<uint32_t>(vertices.size());
        cmd.indexOffset = idxOffset;
        cmd.indexCount = static_cast<uint32_t>(indices.size());
        cmd.primitiveType = render::PrimitiveType::Triangle;
        cmd.blendMode = render::BlendMode::Alpha;
        cmd.texture = getNativeHandle();

        // Get current model-view matrix and projection
        auto m = ::ofGetCurrentMatrix();
        cmd.modelViewMatrix = simd_matrix(
            simd_make_float4(m(0,0), m(1,0), m(2,0), m(3,0)),
            simd_make_float4(m(0,1), m(1,1), m(2,1), m(3,1)),
            simd_make_float4(m(0,2), m(1,2), m(2,2), m(3,2)),
            simd_make_float4(m(0,3), m(1,3), m(2,3), m(3,3))
        );

        // TODO: Get proper projection matrix from camera system when available
        // For now, use identity (will be set by renderer)
        cmd.projectionMatrix = matrix_identity_float4x4;
        cmd.normalMatrix = matrix_identity_float3x3;

        cmd.depthTestEnabled = false;  // Match 2D behavior by default
        cmd.depthWriteEnabled = false;
        cmd.cullBackFace = false;

        drawList.addCommand(cmd);
    }
}

// ============================================================================
// Binding
// ============================================================================

void ofTexture::bind(int textureUnit) const {
    (void)textureUnit;  // Metal doesn't use texture units like OpenGL

    if (!isAllocated()) {
        return;
    }

    // Track active texture in graphics state
    // Used by ofMesh and other primitives that rely on bound textures
    ::ofSetActiveTexture(getNativeHandle());
}

void ofTexture::unbind(int textureUnit) const {
    (void)textureUnit;  // Metal doesn't use texture units like OpenGL

    // Clear active texture in graphics state
    ::ofSetActiveTexture(nullptr);
}

// ============================================================================
// Texture Settings
// ============================================================================

void ofTexture::setTextureWrap(ofTexwrapMode_t wrapModeHorizontal,
                                ofTexwrapMode_t wrapModeVertical) {
    if (!impl_) {
        return;
    }

    // Store texture settings for later application
    impl_->wrapS = ToRenderWrap(wrapModeHorizontal);
    impl_->wrapT = ToRenderWrap(wrapModeVertical);

    // TODO: Apply texture wrap settings through Context/Renderer
}

void ofTexture::setTextureMinMagFilter(ofTexFilterMode_t minFilter,
                                        ofTexFilterMode_t magFilter) {
    if (!impl_) {
        return;
    }

    // Store filter settings for later application
    impl_->minFilter = ToRenderFilter(minFilter);
    impl_->magFilter = ToRenderFilter(magFilter);

    // TODO: Apply texture filter settings through Context/Renderer
}

// ============================================================================
// Properties
// ============================================================================

int ofTexture::getWidth() const {
    return impl_ ? impl_->width : 0;
}

int ofTexture::getHeight() const {
    return impl_ ? impl_->height : 0;
}

// ============================================================================
// Data Readback
// ============================================================================

bool ofTexture::readToPixels(ofPixels& pix) const {
    if (!impl_ || !impl_->textureHandle || !impl_->bAllocated) {
        return false;
    }

    const int w = impl_->width;
    const int h = impl_->height;
    const size_t channels = GetChannelsFromImageType(impl_->internalFormat);

    // Allocate pixel buffer
    pix.allocate(w, h, channels);

    // Calculate bytes per row
    const size_t bytesPerRow = w * channels;

    // Read pixels from GPU texture through Context (respecting layer boundaries)
    auto& ctx = Context::instance();
    return ctx.readTexturePixels(
        impl_->textureHandle,
        pix.getData(),
        w,
        h,
        bytesPerRow
    );
}

// ============================================================================
// Native Handle Access
// ============================================================================

void* ofTexture::getNativeHandle() const {
    if (!impl_) {
        return nullptr;
    }

    // Return opaque texture handle (no Metal types exposed)
    return impl_->textureHandle;
}

// ============================================================================
// Mipmap (Phase 2)
// ============================================================================

void ofTexture::enableMipmap() {
    ensureImpl();
    impl_->mipmapEnabled = true;
}

void ofTexture::disableMipmap() {
    if (impl_) {
        impl_->mipmapEnabled = false;
    }
}

bool ofTexture::hasMipmap() const {
    return impl_ && impl_->mipmapEnabled;
}

void ofTexture::generateMipmap() {
    if (!impl_ || !impl_->textureHandle || !impl_->bAllocated) {
        return;
    }

    @autoreleasepool {
        // Get Metal texture from handle
        id<MTLTexture> texture = (__bridge id<MTLTexture>)impl_->textureHandle;
        if (!texture) {
            return;
        }

        // Check if texture supports mipmaps
        if (texture.mipmapLevelCount <= 1) {
            // Texture was not allocated with mipmap storage
            // Note: Metal textures must be allocated with mipmap support at creation time
            NSLog(@"ofTexture: Cannot generate mipmaps - texture not allocated with mipmap support");
            return;
        }

        // Get command queue from context
        void* devicePtr = Context::instance().getMetalDevice();
        if (!devicePtr) {
            return;
        }

        id<MTLDevice> device = (__bridge id<MTLDevice>)devicePtr;
        id<MTLCommandQueue> commandQueue = [device newCommandQueue];
        if (!commandQueue) {
            return;
        }

        // Create command buffer and blit encoder for mipmap generation
        id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
        if (!commandBuffer) {
            return;
        }

        id<MTLBlitCommandEncoder> blitEncoder = [commandBuffer blitCommandEncoder];
        if (!blitEncoder) {
            return;
        }

        // Generate mipmaps
        [blitEncoder generateMipmapsForTexture:texture];
        [blitEncoder endEncoding];

        // Commit and wait
        [commandBuffer commit];
        [commandBuffer waitUntilCompleted];

        // Update mipmap level count
        impl_->numMipmapLevels = static_cast<int>(texture.mipmapLevelCount);
    }
}

void ofTexture::setMipmapFilter(ofTexFilterMode_t filter) {
    ensureImpl();
    impl_->mipmapFilter = ToRenderFilter(filter);

    // TODO: Apply mipmap filter through Context/Renderer sampler state
}

int ofTexture::getNumMipmapLevels() const {
    return impl_ ? impl_->numMipmapLevels : 1;
}

// ============================================================================
// Anisotropic Filtering (Phase 2)
// ============================================================================

void ofTexture::setMaxAnisotropy(int level) {
    ensureImpl();
    // Clamp to valid range (1-16)
    impl_->maxAnisotropy = std::max(1, std::min(16, level));

    // TODO: Apply anisotropic filtering through Context/Renderer sampler state
}

int ofTexture::getMaxAnisotropy() const {
    return impl_ ? impl_->maxAnisotropy : 1;
}

} // namespace oflike
