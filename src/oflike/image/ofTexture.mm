#import "ofTexture.h"
#import "../../render/metal/MetalTexture.h"
#import "../../core/Context.h"
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#include <vector>

namespace oflike {

// ============================================================================
// Helper Functions
// ============================================================================

/// Convert ofTexwrapMode_t to render::metal::TextureWrap
static render::metal::TextureWrap ToMetalWrap(ofTexwrapMode_t mode) {
    switch (mode) {
        case OF_TEXTURE_WRAP_REPEAT:
            return render::metal::TextureWrap::Repeat;
        case OF_TEXTURE_WRAP_CLAMP_TO_EDGE:
            return render::metal::TextureWrap::Clamp;
        case OF_TEXTURE_WRAP_MIRRORED_REPEAT:
            return render::metal::TextureWrap::Mirror;
        default:
            return render::metal::TextureWrap::Clamp;
    }
}

/// Convert ofTexFilterMode_t to render::metal::TextureFilter
static render::metal::TextureFilter ToMetalFilter(ofTexFilterMode_t mode) {
    switch (mode) {
        case OF_TEXTURE_FILTER_NEAREST:
            return render::metal::TextureFilter::Nearest;
        case OF_TEXTURE_FILTER_LINEAR:
            return render::metal::TextureFilter::Linear;
        default:
            return render::metal::TextureFilter::Linear;
    }
}

/// Convert ofImageType to render::metal::TextureFormat
static render::metal::TextureFormat ImageTypeToTextureFormat(int imageType) {
    switch (imageType) {
        case OF_IMAGE_GRAYSCALE:
            return render::metal::TextureFormat::R8;
        case OF_IMAGE_COLOR:
            return render::metal::TextureFormat::RGBA8; // RGB stored as RGBA
        case OF_IMAGE_COLOR_ALPHA:
            return render::metal::TextureFormat::RGBA8;
        default:
            return render::metal::TextureFormat::RGBA8;
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

/// Convert RGB to RGBA (adds alpha channel)
static void ConvertRGBToRGBA(const unsigned char* src, unsigned char* dst,
                             size_t width, size_t height) {
    for (size_t i = 0; i < width * height; ++i) {
        dst[i * 4 + 0] = src[i * 3 + 0]; // R
        dst[i * 4 + 1] = src[i * 3 + 1]; // G
        dst[i * 4 + 2] = src[i * 3 + 2]; // B
        dst[i * 4 + 3] = 255;            // A (opaque)
    }
}

// ============================================================================
// ofTexture::Impl
// ============================================================================

struct ofTexture::Impl {
    std::unique_ptr<render::metal::MetalTexture> metalTexture;
    int width = 0;
    int height = 0;
    int internalFormat = OF_IMAGE_COLOR_ALPHA;
    bool bAllocated = false;

    Impl() {
        @autoreleasepool {
            // Get Metal device from Context
            // TODO: Update when Context has proper renderer access
            // For now, we'll create MetalTexture when needed
        }
    }

    ~Impl() {
        @autoreleasepool {
            metalTexture.reset();
        }
    }

    void ensureMetalTexture() {
        if (!metalTexture) {
            @autoreleasepool {
                // Get device from Context (Phase 7.1: Device ownership)
                void* devicePtr = Context::instance().getMetalDevice();
                if (devicePtr) {
                    metalTexture = std::make_unique<render::metal::MetalTexture>(devicePtr);
                }
            }
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
    impl_->ensureMetalTexture();

    if (!impl_->metalTexture) {
        return; // Failed to create Metal texture
    }

    render::metal::TextureFormat format = ImageTypeToTextureFormat(internalFormat);

    @autoreleasepool {
        bool success = impl_->metalTexture->create(
            static_cast<uint32_t>(w),
            static_cast<uint32_t>(h),
            format,
            nullptr // Empty texture
        );

        if (success) {
            impl_->width = w;
            impl_->height = h;
            impl_->internalFormat = internalFormat;
            impl_->bAllocated = true;
        }
    }
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
    if (impl_ && impl_->metalTexture) {
        @autoreleasepool {
            impl_->metalTexture->release();
            impl_->width = 0;
            impl_->height = 0;
            impl_->bAllocated = false;
        }
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
    // TODO: Support 16-bit pixels
    // For now, convert to 8-bit
    ofPixels pix8;
    pix8.allocate(pix.getWidth(), pix.getHeight(), pix.getNumChannels());

    const uint16_t* src = pix.getData();
    uint8_t* dst = pix8.getData();
    const size_t totalElements = pix.getWidth() * pix.getHeight() * pix.getNumChannels();

    for (size_t i = 0; i < totalElements; ++i) {
        dst[i] = static_cast<uint8_t>(src[i] >> 8); // Convert 16-bit to 8-bit
    }

    loadData(pix8);
}

void ofTexture::loadData(const ofFloatPixels& pix) {
    // TODO: Support float pixels
    // For now, convert to 8-bit
    ofPixels pix8;
    pix8.allocate(pix.getWidth(), pix.getHeight(), pix.getNumChannels());

    const float* src = pix.getData();
    uint8_t* dst = pix8.getData();
    const size_t totalElements = pix.getWidth() * pix.getHeight() * pix.getNumChannels();

    for (size_t i = 0; i < totalElements; ++i) {
        dst[i] = static_cast<uint8_t>(src[i] * 255.0f); // Convert float [0,1] to byte [0,255]
    }

    loadData(pix8);
}

void ofTexture::loadData(const void* data, int w, int h, int glFormat) {
    ensureImpl();

    if (!data || w <= 0 || h <= 0) {
        return;
    }

    // Allocate if not already allocated or dimensions changed
    if (!isAllocated() || impl_->width != w || impl_->height != h ||
        impl_->internalFormat != glFormat) {
        allocate(w, h, glFormat);
    }

    if (!impl_->metalTexture) {
        return;
    }

    @autoreleasepool {
        const size_t channels = GetChannelsFromImageType(glFormat);

        // Handle RGB -> RGBA conversion (Metal doesn't support RGB8)
        if (channels == 3) {
            const size_t rgbaSize = w * h * 4;
            std::vector<unsigned char> rgbaData(rgbaSize);
            ConvertRGBToRGBA(
                static_cast<const unsigned char*>(data),
                rgbaData.data(),
                w, h
            );
            impl_->metalTexture->updateData(rgbaData.data());
        } else {
            // Direct upload for R8 or RGBA8
            impl_->metalTexture->updateData(data);
        }
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
        // TODO: Integrate with oflike DrawList system
        // For now, this is a placeholder that will be connected to the renderer
        // when the graphics system is fully integrated.

        // The actual drawing will be done via:
        // 1. Push texture to current DrawList
        // 2. Add textured quad with this texture
        // 3. Renderer will bind texture and draw

        // Placeholder implementation would look like:
        // Context::instance().getDrawList().addTexturedQuad(
        //     x, y, w, h,
        //     getNativeHandle()
        // );
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
        // TODO: 3D textured quad drawing
        // Similar to 2D draw, but with z coordinate
    }
}

// ============================================================================
// Binding
// ============================================================================

void ofTexture::bind(int textureUnit) const {
    if (!isAllocated()) {
        return;
    }

    @autoreleasepool {
        // TODO: Integrate with Context to track bound textures
        // Metal doesn't have "bind" like OpenGL, but we need to track
        // which texture is active for subsequent draw calls

        // Context::instance().setActiveTexture(textureUnit, getNativeHandle());
    }
}

void ofTexture::unbind(int textureUnit) const {
    @autoreleasepool {
        // TODO: Integrate with Context to clear bound texture
        // Context::instance().setActiveTexture(textureUnit, nullptr);
    }
}

// ============================================================================
// Texture Settings
// ============================================================================

void ofTexture::setTextureWrap(ofTexwrapMode_t wrapModeHorizontal,
                                ofTexwrapMode_t wrapModeVertical) {
    if (!impl_ || !impl_->metalTexture) {
        return;
    }

    @autoreleasepool {
        render::metal::TextureWrap wrapS = ToMetalWrap(wrapModeHorizontal);
        render::metal::TextureWrap wrapT = ToMetalWrap(wrapModeVertical);
        impl_->metalTexture->setWrap(wrapS, wrapT);
    }
}

void ofTexture::setTextureMinMagFilter(ofTexFilterMode_t minFilter,
                                        ofTexFilterMode_t magFilter) {
    if (!impl_ || !impl_->metalTexture) {
        return;
    }

    @autoreleasepool {
        render::metal::TextureFilter minF = ToMetalFilter(minFilter);
        render::metal::TextureFilter magF = ToMetalFilter(magFilter);
        impl_->metalTexture->setFilter(minF, magF);
    }
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
// Native Handle Access
// ============================================================================

void* ofTexture::getNativeHandle() const {
    if (!impl_ || !impl_->metalTexture) {
        return nullptr;
    }

    @autoreleasepool {
        return impl_->metalTexture->getTexture();
    }
}

} // namespace oflike
