#import "ofFbo.h"
#import "../../core/Context.h"
#import "../../render/DrawList.h"
#import "../../render/DrawCommand.h"
#import "../../render/metal/MetalTexture.h"
#import "../../render/metal/MetalRenderer.h"  // Phase 7.3: For viewport/render target queries
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#include <vector>
#include <stdexcept>

// TODO(Phase 7.3): Remove Metal imports once FBO creation is delegated to renderer
// Currently ofFbo::Impl directly creates Metal textures with MTLTextureDescriptor
// which requires Metal types. This needs refactoring to use renderer abstraction.

namespace oflike {

// ============================================================================
// Helper Functions
// ============================================================================

/// Convert ofImageType to Metal pixel format
static MTLPixelFormat ImageTypeToMetalFormat(int imageType) {
    switch (imageType) {
        case OF_IMAGE_GRAYSCALE:
            return MTLPixelFormatR8Unorm;
        case OF_IMAGE_COLOR:
        case OF_IMAGE_COLOR_ALPHA:
            return MTLPixelFormatRGBA8Unorm;
        default:
            return MTLPixelFormatRGBA8Unorm;
    }
}

/// Convert Metal pixel format to ofImageType
static int MetalFormatToImageType(MTLPixelFormat format) {
    switch (format) {
        case MTLPixelFormatR8Unorm:
            return OF_IMAGE_GRAYSCALE;
        case MTLPixelFormatRGBA8Unorm:
        case MTLPixelFormatBGRA8Unorm:
            return OF_IMAGE_COLOR_ALPHA;
        default:
            return OF_IMAGE_COLOR_ALPHA;
    }
}

// ============================================================================
// ofFbo::Impl - Implementation Details
// ============================================================================

struct ofFbo::Impl {
    // Configuration
    int width = 0;
    int height = 0;
    int numColorAttachments = 1;
    int internalFormat = OF_IMAGE_COLOR_ALPHA;
    bool useDepth = false;
    bool useStencil = false;
    int numSamples = 0;
    bool bAllocated = false;

    // Metal resources
    std::vector<id<MTLTexture>> colorTextures;
    id<MTLTexture> depthTexture = nil;
    id<MTLTexture> stencilTexture = nil;
    id<MTLTexture> msaaTexture = nil; // For MSAA resolve

    // ofTexture wrappers
    std::vector<ofTexture> textureWrappers;
    ofTexture depthTextureWrapper;

    // State tracking for begin/end restore
    bool isRendering = false;
    render::Rect previousViewport{0, 0, 0, 0};
    void* previousRenderTarget = nullptr;

    // MRT (Multiple Render Targets) state
    int activeDrawBuffer = 0;
    std::vector<int> activeDrawBuffers;  // For simultaneous MRT rendering

    // Helper: Get Metal device from Context (Phase 7.1)
    id<MTLDevice> getDevice() const {
        void* devicePtr = Context::instance().getMetalDevice();
        return devicePtr ? (__bridge id<MTLDevice>)devicePtr : nil;
    }

    Impl() = default;

    ~Impl() {
        @autoreleasepool {
            release();
        }
    }

    void release() {
        colorTextures.clear();
        depthTexture = nil;
        stencilTexture = nil;
        msaaTexture = nil;
        textureWrappers.clear();
        bAllocated = false;
    }

    bool allocate(const ofFboSettings& settings) {
        @autoreleasepool {
            id<MTLDevice> device = getDevice();
            if (!device) {
                return false;
            }

            // Store settings
            width = settings.width;
            height = settings.height;
            numColorAttachments = settings.numColorAttachments;
            internalFormat = settings.internalFormat;
            useDepth = settings.useDepth;
            useStencil = settings.useStencil;
            numSamples = settings.numSamples;

            // Validate settings
            if (width <= 0 || height <= 0) {
                return false;
            }
            if (numColorAttachments < 1 || numColorAttachments > 4) {
                numColorAttachments = 1;
            }

            // Create color textures
            MTLPixelFormat colorFormat = ImageTypeToMetalFormat(internalFormat);
            MTLTextureDescriptor* colorDesc = [[MTLTextureDescriptor alloc] init];
            colorDesc.pixelFormat = colorFormat;
            colorDesc.width = width;
            colorDesc.height = height;
            colorDesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
            colorDesc.storageMode = MTLStorageModePrivate;

            colorTextures.clear();
            textureWrappers.clear();

            for (int i = 0; i < numColorAttachments; ++i) {
                id<MTLTexture> texture = [device newTextureWithDescriptor:colorDesc];
                if (!texture) {
                    release();
                    return false;
                }
                colorTextures.push_back(texture);

                // Create ofTexture wrapper
                ofTexture wrapper;
                // TODO: Wrap the Metal texture properly
                // For now, the wrapper is empty but we keep it for API compatibility
                textureWrappers.push_back(std::move(wrapper));
            }

            // Create depth texture if requested
            if (useDepth) {
                MTLTextureDescriptor* depthDesc = [[MTLTextureDescriptor alloc] init];
                depthDesc.pixelFormat = MTLPixelFormatDepth32Float;
                depthDesc.width = width;
                depthDesc.height = height;
                depthDesc.usage = MTLTextureUsageRenderTarget;
                depthDesc.storageMode = MTLStorageModePrivate;

                depthTexture = [device newTextureWithDescriptor:depthDesc];
                if (!depthTexture) {
                    release();
                    return false;
                }
            }

            // Create stencil texture if requested
            if (useStencil) {
                MTLTextureDescriptor* stencilDesc = [[MTLTextureDescriptor alloc] init];
                stencilDesc.pixelFormat = MTLPixelFormatStencil8;
                stencilDesc.width = width;
                stencilDesc.height = height;
                stencilDesc.usage = MTLTextureUsageRenderTarget;
                stencilDesc.storageMode = MTLStorageModePrivate;

                stencilTexture = [device newTextureWithDescriptor:stencilDesc];
                if (!stencilTexture) {
                    release();
                    return false;
                }
            }

            // Create MSAA texture if requested
            if (numSamples > 1) {
                MTLTextureDescriptor* msaaDesc = [[MTLTextureDescriptor alloc] init];
                msaaDesc.pixelFormat = colorFormat;
                msaaDesc.width = width;
                msaaDesc.height = height;
                msaaDesc.textureType = MTLTextureType2DMultisample;
                msaaDesc.sampleCount = numSamples;
                msaaDesc.usage = MTLTextureUsageRenderTarget;
                msaaDesc.storageMode = MTLStorageModePrivate;

                msaaTexture = [device newTextureWithDescriptor:msaaDesc];
                if (!msaaTexture) {
                    release();
                    return false;
                }
            }

            bAllocated = true;
            return true;
        }
    }

    void begin() {
        if (!bAllocated || isRendering) {
            return;
        }

        // Get current context and renderer
        auto& ctx = Context::instance();
        auto* renderer = ctx.renderer();
        auto& drawList = ctx.getDrawList();

        if (!renderer) {
            return;
        }

        // Phase 7.3: Save current render target state for restore
        previousRenderTarget = renderer->getDefaultRenderTarget();
        previousViewport = render::Rect(
            0, 0,
            (float)renderer->getViewportWidth(),
            (float)renderer->getViewportHeight()
        );

        // Determine which texture to use (MRT support)
        void* targetTexture = nullptr;
        if (!activeDrawBuffers.empty()) {
            // Use first buffer in active set (Metal limitation: single active attachment)
            int index = activeDrawBuffers[0];
            if (index >= 0 && index < (int)colorTextures.size()) {
                targetTexture = (__bridge void*)colorTextures[index];
            }
        } else {
            // Use active draw buffer
            if (activeDrawBuffer >= 0 && activeDrawBuffer < (int)colorTextures.size()) {
                targetTexture = (__bridge void*)colorTextures[activeDrawBuffer];
            }
        }

        // Fallback to first texture
        if (!targetTexture && !colorTextures.empty()) {
            targetTexture = (__bridge void*)colorTextures[0];
        }

        // Set FBO as render target
        render::SetRenderTargetCommand rtCmd;
        rtCmd.renderTarget = targetTexture;
        drawList.addCommand(rtCmd);

        // Set viewport to FBO dimensions
        render::SetViewportCommand vpCmd;
        vpCmd.viewport = render::Rect(0, 0, (float)width, (float)height);
        drawList.addCommand(vpCmd);

        isRendering = true;
    }

    void end() {
        if (!bAllocated || !isRendering) {
            return;
        }

        // Get current context and draw list
        auto& ctx = Context::instance();
        auto& drawList = ctx.getDrawList();

        // Phase 7.3: Restore previous render target state
        render::SetRenderTargetCommand rtCmd;
        rtCmd.renderTarget = previousRenderTarget;
        drawList.addCommand(rtCmd);

        // Phase 7.3: Restore previous viewport
        render::SetViewportCommand vpCmd;
        vpCmd.viewport = previousViewport;
        drawList.addCommand(vpCmd);

        isRendering = false;
    }

    void draw(float x, float y, float w, float h) const {
        if (!bAllocated || colorTextures.empty()) {
            return;
        }

        // Get current context
        auto& ctx = Context::instance();
        auto& drawList = ctx.getDrawList();

        // Create vertices for textured quad
        std::vector<render::Vertex2D> vertices(6);

        // Triangle 1: Top-left, top-right, bottom-left
        vertices[0].position = simd_make_float2(x, y);
        vertices[0].texCoord = simd_make_float2(0, 0);
        vertices[0].color = simd_make_float4(1, 1, 1, 1);

        vertices[1].position = simd_make_float2(x + w, y);
        vertices[1].texCoord = simd_make_float2(1, 0);
        vertices[1].color = simd_make_float4(1, 1, 1, 1);

        vertices[2].position = simd_make_float2(x, y + h);
        vertices[2].texCoord = simd_make_float2(0, 1);
        vertices[2].color = simd_make_float4(1, 1, 1, 1);

        // Triangle 2: Top-right, bottom-right, bottom-left
        vertices[3].position = simd_make_float2(x + w, y);
        vertices[3].texCoord = simd_make_float2(1, 0);
        vertices[3].color = simd_make_float4(1, 1, 1, 1);

        vertices[4].position = simd_make_float2(x + w, y + h);
        vertices[4].texCoord = simd_make_float2(1, 1);
        vertices[4].color = simd_make_float4(1, 1, 1, 1);

        vertices[5].position = simd_make_float2(x, y + h);
        vertices[5].texCoord = simd_make_float2(0, 1);
        vertices[5].color = simd_make_float4(1, 1, 1, 1);

        // Add vertices to draw list
        uint32_t vtxOffset = drawList.addVertices2D(vertices);

        // Create draw command
        render::DrawCommand2D cmd;
        cmd.vertexOffset = vtxOffset;
        cmd.vertexCount = 6;
        cmd.primitiveType = render::PrimitiveType::Triangle;
        cmd.blendMode = render::BlendMode::Alpha;
        cmd.texture = (__bridge void*)colorTextures[0];

        // Get current transform from context
        cmd.transform = ctx.getCurrentMatrix();

        drawList.addCommand(cmd);
    }

    void readToPixels(ofPixels& pixels, int attachmentIndex) const {
        @autoreleasepool {
            if (!bAllocated || attachmentIndex >= (int)colorTextures.size()) {
                return;
            }

            id<MTLTexture> texture = colorTextures[attachmentIndex];
            if (!texture) {
                return;
            }

            // Allocate pixel buffer
            int imageType = MetalFormatToImageType(texture.pixelFormat);
            pixels.allocate(width, height, imageType);

            // Calculate bytes per row
            size_t bytesPerPixel = pixels.getBytesPerPixel();
            size_t bytesPerRow = bytesPerPixel * width;

            // Read texture data
            MTLRegion region = MTLRegionMake2D(0, 0, width, height);
            [texture getBytes:pixels.getData()
                  bytesPerRow:bytesPerRow
                   fromRegion:region
                  mipmapLevel:0];
        }
    }
};

// ============================================================================
// ofFbo Implementation
// ============================================================================

ofFbo::ofFbo()
    : impl_(std::make_unique<Impl>()) {
}

ofFbo::~ofFbo() = default;

ofFbo::ofFbo(ofFbo&& other) noexcept = default;

ofFbo& ofFbo::operator=(ofFbo&& other) noexcept = default;

void ofFbo::ensureImpl() {
    if (!impl_) {
        impl_ = std::make_unique<Impl>();
    }
}

// ============================================================================
// Allocation
// ============================================================================

void ofFbo::allocate(int width, int height, int internalFormat, int numSamples) {
    ofFboSettings settings;
    settings.width = width;
    settings.height = height;
    settings.internalFormat = internalFormat;
    settings.numSamples = numSamples;
    settings.numColorAttachments = 1;
    settings.useDepth = false;
    settings.useStencil = false;

    allocateWithSettings(settings);
}

void ofFbo::allocateWithSettings(const ofFboSettings& settings) {
    ensureImpl();
    impl_->allocate(settings);
}

bool ofFbo::isAllocated() const {
    return impl_ && impl_->bAllocated;
}

void ofFbo::clear() {
    if (impl_) {
        impl_->release();
    }
}

// ============================================================================
// Rendering
// ============================================================================

void ofFbo::begin() {
    ensureImpl();
    impl_->begin();
}

void ofFbo::end() {
    if (impl_) {
        impl_->end();
    }
}

// ============================================================================
// Drawing
// ============================================================================

void ofFbo::draw(float x, float y) const {
    if (impl_ && impl_->bAllocated) {
        impl_->draw(x, y, (float)impl_->width, (float)impl_->height);
    }
}

void ofFbo::draw(float x, float y, float width, float height) const {
    if (impl_ && impl_->bAllocated) {
        impl_->draw(x, y, width, height);
    }
}

// ============================================================================
// Texture Access
// ============================================================================

ofTexture& ofFbo::getTexture() {
    ensureImpl();
    if (impl_->textureWrappers.empty()) {
        throw std::runtime_error("ofFbo::getTexture() - FBO not allocated");
    }
    return impl_->textureWrappers[0];
}

const ofTexture& ofFbo::getTexture() const {
    if (!impl_ || impl_->textureWrappers.empty()) {
        throw std::runtime_error("ofFbo::getTexture() - FBO not allocated");
    }
    return impl_->textureWrappers[0];
}

ofTexture& ofFbo::getTexture(int attachmentIndex) {
    ensureImpl();
    if (attachmentIndex < 0 || attachmentIndex >= (int)impl_->textureWrappers.size()) {
        throw std::runtime_error("ofFbo::getTexture() - Invalid attachment index");
    }
    return impl_->textureWrappers[attachmentIndex];
}

ofTexture& ofFbo::getDepthTexture() {
    ensureImpl();
    if (!impl_->useDepth) {
        throw std::runtime_error("ofFbo::getDepthTexture() - Depth buffer not enabled");
    }
    return impl_->depthTextureWrapper;
}

const ofTexture& ofFbo::getDepthTexture() const {
    if (!impl_ || !impl_->useDepth) {
        throw std::runtime_error("ofFbo::getDepthTexture() - Depth buffer not enabled");
    }
    return impl_->depthTextureWrapper;
}

// ============================================================================
// Readback
// ============================================================================

void ofFbo::readToPixels(ofPixels& pixels) const {
    readToPixels(pixels, 0);
}

void ofFbo::readToPixels(ofPixels& pixels, int attachmentIndex) const {
    if (impl_ && impl_->bAllocated) {
        impl_->readToPixels(pixels, attachmentIndex);
    }
}

// ============================================================================
// Properties
// ============================================================================

int ofFbo::getWidth() const {
    return impl_ ? impl_->width : 0;
}

int ofFbo::getHeight() const {
    return impl_ ? impl_->height : 0;
}

int ofFbo::getNumTextures() const {
    return impl_ ? impl_->numColorAttachments : 0;
}

bool ofFbo::hasDepthBuffer() const {
    return impl_ && impl_->useDepth;
}

bool ofFbo::hasStencilBuffer() const {
    return impl_ && impl_->useStencil;
}

// ============================================================================
// Multi-Attachment Control (MRT)
// ============================================================================

void ofFbo::setActiveDrawBuffer(int attachmentIndex) {
    if (!impl_ || !impl_->bAllocated) {
        return;
    }

    if (attachmentIndex < 0 || attachmentIndex >= impl_->numColorAttachments) {
        return;
    }

    // Phase 7.3: Implement MRT buffer selection
    impl_->activeDrawBuffer = attachmentIndex;
    impl_->activeDrawBuffers.clear();  // Clear simultaneous MRT mode

    // If currently rendering, update render target immediately
    if (impl_->isRendering) {
        auto& ctx = Context::instance();
        auto& drawList = ctx.getDrawList();

        render::SetRenderTargetCommand rtCmd;
        rtCmd.renderTarget = (__bridge void*)impl_->colorTextures[attachmentIndex];
        drawList.addCommand(rtCmd);
    }
}

void ofFbo::setActiveDrawBuffers(const std::vector<int>& attachmentIndices) {
    if (!impl_ || !impl_->bAllocated) {
        return;
    }

    if (attachmentIndices.empty()) {
        return;
    }

    // Phase 7.3: Implement MRT buffer selection
    // Note: Metal supports multiple render targets, but our current DrawCommand
    // architecture only supports a single active target. This implementation
    // stores the request and uses the first valid attachment.
    // TODO(Future): Extend DrawCommand to support multiple simultaneous color attachments

    impl_->activeDrawBuffers.clear();
    for (int index : attachmentIndices) {
        if (index >= 0 && index < impl_->numColorAttachments) {
            impl_->activeDrawBuffers.push_back(index);
        }
    }

    // If currently rendering, update to first valid attachment
    if (impl_->isRendering && !impl_->activeDrawBuffers.empty()) {
        auto& ctx = Context::instance();
        auto& drawList = ctx.getDrawList();

        int firstAttachment = impl_->activeDrawBuffers[0];
        render::SetRenderTargetCommand rtCmd;
        rtCmd.renderTarget = (__bridge void*)impl_->colorTextures[firstAttachment];
        drawList.addCommand(rtCmd);
    }
}

// ============================================================================
// Native Handle Access
// ============================================================================

void* ofFbo::getNativeTextureHandle(int attachmentIndex) const {
    if (!impl_ || !impl_->bAllocated) {
        return nullptr;
    }

    if (attachmentIndex < 0 || attachmentIndex >= (int)impl_->colorTextures.size()) {
        return nullptr;
    }

    return (__bridge void*)impl_->colorTextures[attachmentIndex];
}

void* ofFbo::getNativeDepthHandle() const {
    if (!impl_ || !impl_->bAllocated || !impl_->useDepth) {
        return nullptr;
    }

    return (__bridge void*)impl_->depthTexture;
}

} // namespace oflike
