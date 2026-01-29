#include "ofCoreText.h"
#include "ofGraphics.h"
#include "../image/ofTexture.h"
#include "../../core/Context.h"
#include "../../render/DrawList.h"
#include "../../render/DrawCommand.h"
#include "../math/ofMatrix4x4.h"

#import <Foundation/Foundation.h>
#import <CoreText/CoreText.h>
#import <CoreGraphics/CoreGraphics.h>

#include <unordered_map>

// ============================================================
// Implementation
// ============================================================

struct ofCoreText::Impl {
    CTFontRef ctFont = nullptr;
    float fontSize = 12.0f;
    float letterSpacing = 0.0f;
    float lineHeightMultiplier = 1.0f;
    bool antialiased = true;
    bool loaded = false;

    // Cache for rendered strings
    struct CachedString {
        std::unique_ptr<oflike::ofTexture> texture;
        float width;
        float height;
    };
    mutable std::unordered_map<std::string, CachedString> stringCache;
    mutable size_t cacheSize = 0;
    static constexpr size_t maxCacheSize = 100;

    ~Impl() {
        if (ctFont) {
            CFRelease(ctFont);
            ctFont = nullptr;
        }
    }

    bool isSystemFont(const std::string& fontPath) {
        // Check if it's a system font name (no path separators, no extension)
        if (fontPath.find('/') != std::string::npos) return false;
        if (fontPath.find('.') != std::string::npos) {
            std::string ext = fontPath.substr(fontPath.rfind('.'));
            if (ext == ".ttf" || ext == ".otf" || ext == ".ttc") return false;
        }
        return true;
    }

    void clearCache() {
        stringCache.clear();
        cacheSize = 0;
    }

    // Render text to RGBA texture
    oflike::ofTexture* renderStringToTexture(const std::string& text) const {
        @autoreleasepool {
            if (!ctFont || text.empty()) return nullptr;

            NSString* nsText = [NSString stringWithUTF8String:text.c_str()];
            if (!nsText) return nullptr;

            // Create attributed string
            NSDictionary* attributes = @{
                (__bridge NSString*)kCTFontAttributeName: (__bridge id)ctFont,
                (__bridge NSString*)kCTForegroundColorFromContextAttributeName: @YES
            };

            if (letterSpacing != 0.0f) {
                NSMutableDictionary* mutableAttrs = [attributes mutableCopy];
                mutableAttrs[(__bridge NSString*)kCTKernAttributeName] = @(letterSpacing);
                attributes = mutableAttrs;
            }

            NSAttributedString* attrString = [[NSAttributedString alloc]
                initWithString:nsText
                attributes:attributes];

            // Create line
            CTLineRef line = CTLineCreateWithAttributedString((__bridge CFAttributedStringRef)attrString);
            if (!line) return nullptr;

            // Get bounds
            CGFloat ascent, descent, leading;
            double lineWidth = CTLineGetTypographicBounds(line, &ascent, &descent, &leading);

            int width = (int)std::ceil(lineWidth) + 4;  // Add padding
            int height = (int)std::ceil(ascent + descent) + 4;

            if (width <= 0 || height <= 0) {
                CFRelease(line);
                return nullptr;
            }

            // Create RGBA bitmap context
            size_t bytesPerRow = width * 4;
            std::vector<uint8_t> bitmapData(bytesPerRow * height, 0);

            CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
            CGContextRef context = CGBitmapContextCreate(
                bitmapData.data(),
                width,
                height,
                8,  // bits per component
                bytesPerRow,
                colorSpace,
                kCGImageAlphaPremultipliedLast  // RGBA
            );
            CGColorSpaceRelease(colorSpace);

            if (!context) {
                CFRelease(line);
                return nullptr;
            }

            // Set rendering quality
            if (antialiased) {
                CGContextSetShouldAntialias(context, true);
                CGContextSetShouldSmoothFonts(context, true);
                CGContextSetAllowsFontSmoothing(context, true);
            } else {
                CGContextSetShouldAntialias(context, false);
            }

            // Clear background (transparent)
            CGContextClearRect(context, CGRectMake(0, 0, width, height));

            // Set text color to white (will be tinted by vertex color)
            CGContextSetRGBFillColor(context, 1.0, 1.0, 1.0, 1.0);

            // Draw text
            // Core Graphics has origin at bottom-left, so we flip
            CGContextTranslateCTM(context, 0, height);
            CGContextScaleCTM(context, 1.0, -1.0);

            CGContextSetTextPosition(context, 2, descent + 2);
            CTLineDraw(line, context);

            CGContextRelease(context);
            CFRelease(line);

            // Create texture from bitmap
            auto texture = new oflike::ofTexture();
            texture->loadData(bitmapData.data(), width, height, 4);

            return texture;
        }
    }

    const CachedString* getCachedString(const std::string& text) const {
        auto it = stringCache.find(text);
        if (it != stringCache.end()) {
            return &it->second;
        }

        // Render and cache
        oflike::ofTexture* tex = renderStringToTexture(text);
        if (!tex) return nullptr;

        // Evict old entries if cache is full
        if (cacheSize >= maxCacheSize) {
            stringCache.clear();
            cacheSize = 0;
        }

        CachedString cached;
        cached.texture.reset(tex);
        cached.width = tex->getWidth();
        cached.height = tex->getHeight();

        auto result = stringCache.emplace(text, std::move(cached));
        cacheSize++;

        return &result.first->second;
    }
};

// ============================================================
// Public API
// ============================================================

ofCoreText::ofCoreText() : impl_(std::make_unique<Impl>()) {}

ofCoreText::~ofCoreText() = default;

ofCoreText::ofCoreText(ofCoreText&&) noexcept = default;
ofCoreText& ofCoreText::operator=(ofCoreText&&) noexcept = default;

bool ofCoreText::load(const std::string& fontNameOrPath, float size, bool antialiased) {
    @autoreleasepool {
        unload();

        NSString* fontName = [NSString stringWithUTF8String:fontNameOrPath.c_str()];

        if (impl_->isSystemFont(fontNameOrPath)) {
            // System font by name
            impl_->ctFont = CTFontCreateWithName((__bridge CFStringRef)fontName, size, nullptr);
        } else {
            // Font from file
            NSURL* fontURL = [NSURL fileURLWithPath:fontName];
            CFArrayRef descriptors = CTFontManagerCreateFontDescriptorsFromURL((__bridge CFURLRef)fontURL);

            if (!descriptors || CFArrayGetCount(descriptors) == 0) {
                if (descriptors) CFRelease(descriptors);
                return false;
            }

            CTFontDescriptorRef descriptor = (CTFontDescriptorRef)CFArrayGetValueAtIndex(descriptors, 0);
            impl_->ctFont = CTFontCreateWithFontDescriptor(descriptor, size, nullptr);
            CFRelease(descriptors);
        }

        if (!impl_->ctFont) {
            return false;
        }

        impl_->fontSize = size;
        impl_->antialiased = antialiased;
        impl_->loaded = true;

        return true;
    }
}

bool ofCoreText::isLoaded() const {
    return impl_->loaded;
}

void ofCoreText::unload() {
    if (impl_->ctFont) {
        CFRelease(impl_->ctFont);
        impl_->ctFont = nullptr;
    }
    impl_->loaded = false;
    impl_->clearCache();
}

void ofCoreText::drawString(const std::string& text, float x, float y) const {
    if (!impl_->loaded || text.empty()) return;

    const auto* cached = impl_->getCachedString(text);
    if (!cached || !cached->texture) return;

    // Get current color
    uint8_t r, g, b, a;
    ofGetColor(r, g, b, a);
    simd_float4 color = simd_make_float4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);

    // Calculate position (y is baseline, texture origin is top-left)
    float drawX = x;
    float drawY = y - getAscenderHeight();
    float w = cached->width;
    float h = cached->height;

    // Create quad vertices
    // Note: UV coordinates are flipped vertically (V: 1->0) because Core Graphics
    // renders with Y-up and we flip to match oF's Y-down coordinate system
    render::Vertex2D vertices[6];

    // Triangle 1
    vertices[0] = render::Vertex2D(drawX, drawY, 0, 1, color.x, color.y, color.z, color.w);
    vertices[1] = render::Vertex2D(drawX + w, drawY, 1, 1, color.x, color.y, color.z, color.w);
    vertices[2] = render::Vertex2D(drawX + w, drawY + h, 1, 0, color.x, color.y, color.z, color.w);

    // Triangle 2
    vertices[3] = render::Vertex2D(drawX, drawY, 0, 1, color.x, color.y, color.z, color.w);
    vertices[4] = render::Vertex2D(drawX + w, drawY + h, 1, 0, color.x, color.y, color.z, color.w);
    vertices[5] = render::Vertex2D(drawX, drawY + h, 0, 0, color.x, color.y, color.z, color.w);

    // Add to draw list
    auto& drawList = Context::instance().getDrawList();
    uint32_t vtxOffset = drawList.addVertices2D(vertices, 6);

    // Create draw command
    render::DrawCommand2D cmd;
    cmd.vertexOffset = vtxOffset;
    cmd.vertexCount = 6;
    cmd.primitiveType = render::PrimitiveType::Triangle;
    cmd.blendMode = render::BlendMode::Alpha;
    cmd.texture = cached->texture->getNativeHandle();

    // Get current transform
    oflike::ofMatrix4x4 mat = ofGetCurrentMatrix();
    cmd.transform = mat.toSimd();

    drawList.addCommand(cmd);
}

float ofCoreText::stringWidth(const std::string& text) const {
    if (!impl_->loaded || text.empty()) return 0;

    const auto* cached = impl_->getCachedString(text);
    return cached ? cached->width : 0;
}

float ofCoreText::stringHeight(const std::string& text) const {
    if (!impl_->loaded || text.empty()) return 0;

    const auto* cached = impl_->getCachedString(text);
    return cached ? cached->height : 0;
}

ofCoreText::Rectangle ofCoreText::getStringBoundingBox(const std::string& text, float x, float y) const {
    Rectangle rect = {x, y - getAscenderHeight(), 0, 0};

    if (!impl_->loaded || text.empty()) return rect;

    const auto* cached = impl_->getCachedString(text);
    if (cached) {
        rect.width = cached->width;
        rect.height = cached->height;
    }

    return rect;
}

float ofCoreText::getSize() const {
    return impl_->fontSize;
}

float ofCoreText::getLineHeight() const {
    if (!impl_->ctFont) return impl_->fontSize;
    return CTFontGetAscent(impl_->ctFont) + CTFontGetDescent(impl_->ctFont) + CTFontGetLeading(impl_->ctFont);
}

float ofCoreText::getAscenderHeight() const {
    if (!impl_->ctFont) return impl_->fontSize * 0.8f;
    return CTFontGetAscent(impl_->ctFont);
}

float ofCoreText::getDescenderHeight() const {
    if (!impl_->ctFont) return impl_->fontSize * 0.2f;
    return -CTFontGetDescent(impl_->ctFont);
}

void ofCoreText::setLetterSpacing(float spacing) {
    impl_->letterSpacing = spacing;
    impl_->clearCache();  // Invalidate cache
}

float ofCoreText::getLetterSpacing() const {
    return impl_->letterSpacing;
}

void ofCoreText::setLineHeight(float height) {
    impl_->lineHeightMultiplier = height;
}
