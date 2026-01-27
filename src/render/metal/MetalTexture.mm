#import "MetalTexture.h"
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <Foundation/Foundation.h>
#import <unordered_map>
#import <string>

namespace render {
namespace metal {

// ============================================================================
// Helper Functions
// ============================================================================

static MTLPixelFormat ToMTLPixelFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::RGBA8:      return MTLPixelFormatRGBA8Unorm;
        case TextureFormat::BGRA8:      return MTLPixelFormatBGRA8Unorm;
        case TextureFormat::RGB8:       return MTLPixelFormatRGBA8Unorm; // No RGB8, use RGBA8
        case TextureFormat::R8:         return MTLPixelFormatR8Unorm;
        case TextureFormat::RG8:        return MTLPixelFormatRG8Unorm;
        case TextureFormat::RGBA16F:    return MTLPixelFormatRGBA16Float;
        case TextureFormat::RGBA32F:    return MTLPixelFormatRGBA32Float;
        case TextureFormat::Depth32F:   return MTLPixelFormatDepth32Float;
        case TextureFormat::Stencil8:   return MTLPixelFormatStencil8;
        default:                        return MTLPixelFormatRGBA8Unorm;
    }
}

static MTLSamplerAddressMode ToMTLAddressMode(TextureWrap wrap) {
    switch (wrap) {
        case TextureWrap::Clamp:    return MTLSamplerAddressModeClampToEdge;
        case TextureWrap::Repeat:   return MTLSamplerAddressModeRepeat;
        case TextureWrap::Mirror:   return MTLSamplerAddressModeMirrorRepeat;
        default:                    return MTLSamplerAddressModeClampToEdge;
    }
}

static MTLSamplerMinMagFilter ToMTLFilter(TextureFilter filter) {
    switch (filter) {
        case TextureFilter::Nearest:    return MTLSamplerMinMagFilterNearest;
        case TextureFilter::Linear:     return MTLSamplerMinMagFilterLinear;
        default:                        return MTLSamplerMinMagFilterLinear;
    }
}

static size_t GetBytesPerPixel(TextureFormat format) {
    switch (format) {
        case TextureFormat::RGBA8:      return 4;
        case TextureFormat::BGRA8:      return 4;
        case TextureFormat::RGB8:       return 4; // Converted to RGBA8
        case TextureFormat::R8:         return 1;
        case TextureFormat::RG8:        return 2;
        case TextureFormat::RGBA16F:    return 8;
        case TextureFormat::RGBA32F:    return 16;
        case TextureFormat::Depth32F:   return 4;
        case TextureFormat::Stencil8:   return 1;
        default:                        return 4;
    }
}

// ============================================================================
// MetalTexture Implementation
// ============================================================================

struct MetalTexture::Impl {
    id<MTLDevice> device = nil;
    id<MTLTexture> texture = nil;
    id<MTLSamplerState> samplerState = nil;

    uint32_t width = 0;
    uint32_t height = 0;
    TextureFormat format = TextureFormat::RGBA8;

    TextureWrap wrapS = TextureWrap::Clamp;
    TextureWrap wrapT = TextureWrap::Clamp;
    TextureFilter minFilter = TextureFilter::Linear;
    TextureFilter magFilter = TextureFilter::Linear;

    bool samplerDirty = true;

    Impl(id<MTLDevice> dev) : device(dev) {}

    ~Impl() {
        @autoreleasepool {
            texture = nil;
            samplerState = nil;
        }
    }

    void updateSamplerState() {
        if (!samplerDirty || !device) {
            return;
        }

        @autoreleasepool {
            MTLSamplerDescriptor* desc = [MTLSamplerDescriptor new];
            desc.sAddressMode = ToMTLAddressMode(wrapS);
            desc.tAddressMode = ToMTLAddressMode(wrapT);
            desc.minFilter = ToMTLFilter(minFilter);
            desc.magFilter = ToMTLFilter(magFilter);
            desc.mipFilter = MTLSamplerMipFilterNotMipmapped;

            samplerState = [device newSamplerStateWithDescriptor:desc];
            samplerDirty = false;
        }
    }
};

MetalTexture::MetalTexture(void* device)
    : impl_(std::make_unique<Impl>((__bridge id<MTLDevice>)device)) {
}

MetalTexture::~MetalTexture() = default;

bool MetalTexture::create(uint32_t width, uint32_t height, TextureFormat format, const void* data) {
    if (!impl_ || !impl_->device || width == 0 || height == 0) {
        return false;
    }

    @autoreleasepool {
        MTLTextureDescriptor* desc = [MTLTextureDescriptor
            texture2DDescriptorWithPixelFormat:ToMTLPixelFormat(format)
                                         width:width
                                        height:height
                                     mipmapped:NO];
        desc.usage = MTLTextureUsageShaderRead;
        desc.storageMode = MTLStorageModeShared;

        impl_->texture = [impl_->device newTextureWithDescriptor:desc];
        if (!impl_->texture) {
            NSLog(@"MetalTexture: Failed to create texture %ux%u", width, height);
            return false;
        }

        impl_->width = width;
        impl_->height = height;
        impl_->format = format;

        // Upload data if provided
        if (data) {
            MTLRegion region = MTLRegionMake2D(0, 0, width, height);
            size_t bytesPerRow = width * GetBytesPerPixel(format);
            [impl_->texture replaceRegion:region
                              mipmapLevel:0
                                withBytes:data
                              bytesPerRow:bytesPerRow];
        }

        impl_->samplerDirty = true;
        return true;
    }
}

bool MetalTexture::loadFromFile(const char* path) {
    if (!impl_ || !impl_->device || !path) {
        return false;
    }

    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:path];
        NSURL* url = [NSURL fileURLWithPath:nsPath];

        MTKTextureLoader* loader = [[MTKTextureLoader alloc] initWithDevice:impl_->device];

        NSDictionary* options = @{
            MTKTextureLoaderOptionTextureUsage: @(MTLTextureUsageShaderRead),
            MTKTextureLoaderOptionTextureStorageMode: @(MTLStorageModeShared)
        };

        NSError* error = nil;
        impl_->texture = [loader newTextureWithContentsOfURL:url
                                                     options:options
                                                       error:&error];

        if (!impl_->texture || error) {
            NSLog(@"MetalTexture: Failed to load texture from %s: %@", path, error);
            return false;
        }

        impl_->width = (uint32_t)impl_->texture.width;
        impl_->height = (uint32_t)impl_->texture.height;

        // Detect format (simplified, assuming common formats)
        MTLPixelFormat pixelFormat = impl_->texture.pixelFormat;
        if (pixelFormat == MTLPixelFormatRGBA8Unorm) {
            impl_->format = TextureFormat::RGBA8;
        } else if (pixelFormat == MTLPixelFormatBGRA8Unorm) {
            impl_->format = TextureFormat::BGRA8;
        } else {
            impl_->format = TextureFormat::RGBA8; // Default
        }

        impl_->samplerDirty = true;
        return true;
    }
}

bool MetalTexture::updateData(const void* data, const void* region) {
    if (!impl_ || !impl_->texture || !data) {
        return false;
    }

    @autoreleasepool {
        MTLRegion mtlRegion;
        if (region) {
            // Custom region (cast from user-provided format)
            // For simplicity, assume entire texture for now
            mtlRegion = MTLRegionMake2D(0, 0, impl_->width, impl_->height);
        } else {
            mtlRegion = MTLRegionMake2D(0, 0, impl_->width, impl_->height);
        }

        size_t bytesPerRow = impl_->width * GetBytesPerPixel(impl_->format);
        [impl_->texture replaceRegion:mtlRegion
                          mipmapLevel:0
                            withBytes:data
                          bytesPerRow:bytesPerRow];
        return true;
    }
}

bool MetalTexture::readPixels(void* data, const void* region) const {
    if (!impl_ || !impl_->texture || !data) {
        return false;
    }

    @autoreleasepool {
        MTLRegion mtlRegion;
        if (region) {
            // Custom region (cast from user-provided format)
            // For simplicity, assume entire texture for now
            mtlRegion = MTLRegionMake2D(0, 0, impl_->width, impl_->height);
        } else {
            mtlRegion = MTLRegionMake2D(0, 0, impl_->width, impl_->height);
        }

        size_t bytesPerRow = impl_->width * GetBytesPerPixel(impl_->format);

        // Read pixels from GPU texture to CPU memory
        // This works because we use MTLStorageModeShared for textures
        [impl_->texture getBytes:data
                     bytesPerRow:bytesPerRow
                      fromRegion:mtlRegion
                     mipmapLevel:0];

        return true;
    }
}

void* MetalTexture::getTexture() const {
    return impl_ ? (__bridge void*)impl_->texture : nullptr;
}

uint32_t MetalTexture::getWidth() const {
    return impl_ ? impl_->width : 0;
}

uint32_t MetalTexture::getHeight() const {
    return impl_ ? impl_->height : 0;
}

TextureFormat MetalTexture::getFormat() const {
    return impl_ ? impl_->format : TextureFormat::RGBA8;
}

bool MetalTexture::isValid() const {
    return impl_ && impl_->texture != nil;
}

void MetalTexture::setWrap(TextureWrap wrapS, TextureWrap wrapT) {
    if (!impl_) return;
    impl_->wrapS = wrapS;
    impl_->wrapT = wrapT;
    impl_->samplerDirty = true;
}

void MetalTexture::setFilter(TextureFilter minFilter, TextureFilter magFilter) {
    if (!impl_) return;
    impl_->minFilter = minFilter;
    impl_->magFilter = magFilter;
    impl_->samplerDirty = true;
}

void* MetalTexture::getSamplerState() {
    if (!impl_) return nullptr;
    impl_->updateSamplerState();
    return (__bridge void*)impl_->samplerState;
}

void MetalTexture::release() {
    if (!impl_) return;

    @autoreleasepool {
        impl_->texture = nil;
        impl_->samplerState = nil;
        impl_->width = 0;
        impl_->height = 0;
        impl_->samplerDirty = true;
    }
}

// ============================================================================
// MetalTextureCache Implementation
// ============================================================================

struct MetalTextureCache::Impl {
    id<MTLDevice> device = nil;
    std::unordered_map<std::string, std::unique_ptr<MetalTexture>> cache;

    Impl(id<MTLDevice> dev) : device(dev) {}

    ~Impl() {
        @autoreleasepool {
            cache.clear();
        }
    }
};

MetalTextureCache::MetalTextureCache(void* device)
    : impl_(std::make_unique<Impl>((__bridge id<MTLDevice>)device)) {
}

MetalTextureCache::~MetalTextureCache() = default;

MetalTexture* MetalTextureCache::load(const char* path) {
    if (!impl_ || !path) {
        return nullptr;
    }

    std::string pathStr(path);

    @autoreleasepool {
        // Check cache first
        auto it = impl_->cache.find(pathStr);
        if (it != impl_->cache.end()) {
            return it->second.get();
        }

        // Load new texture
        auto texture = std::make_unique<MetalTexture>((__bridge void*)impl_->device);
        if (!texture->loadFromFile(path)) {
            NSLog(@"MetalTextureCache: Failed to load texture: %s", path);
            return nullptr;
        }

        MetalTexture* texturePtr = texture.get();
        impl_->cache[pathStr] = std::move(texture);
        return texturePtr;
    }
}

MetalTexture* MetalTextureCache::create(uint32_t width, uint32_t height,
                                        TextureFormat format, const void* data) {
    if (!impl_) {
        return nullptr;
    }

    @autoreleasepool {
        // Create non-cached texture
        auto texture = std::make_unique<MetalTexture>((__bridge void*)impl_->device);
        if (!texture->create(width, height, format, data)) {
            NSLog(@"MetalTextureCache: Failed to create texture %ux%u", width, height);
            return nullptr;
        }

        // Store with generated key (could use pointer address)
        std::string key = "__dynamic_" + std::to_string(width) + "x" +
                         std::to_string(height) + "_" +
                         std::to_string((uintptr_t)texture.get());

        MetalTexture* texturePtr = texture.get();
        impl_->cache[key] = std::move(texture);
        return texturePtr;
    }
}

void MetalTextureCache::remove(const char* path) {
    if (!impl_ || !path) {
        return;
    }

    std::string pathStr(path);

    @autoreleasepool {
        impl_->cache.erase(pathStr);
    }
}

void MetalTextureCache::clear() {
    if (!impl_) {
        return;
    }

    @autoreleasepool {
        impl_->cache.clear();
    }
}

size_t MetalTextureCache::getCacheSize() const {
    return impl_ ? impl_->cache.size() : 0;
}

bool MetalTextureCache::isCached(const char* path) const {
    if (!impl_ || !path) {
        return false;
    }

    std::string pathStr(path);
    return impl_->cache.find(pathStr) != impl_->cache.end();
}

} // namespace metal
} // namespace render
