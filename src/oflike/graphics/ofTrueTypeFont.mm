#include "ofTrueTypeFont.h"
#include "ofPath.h"
#include "ofTexture.h"
#include "ofPixels.h"
#include "../../core/Context.h"
#include "../../utils/ofLog.h"
#include "../../utils/ofUtils.h"
#include <CoreText/CoreText.h>
#include <CoreGraphics/CoreGraphics.h>
#include <Foundation/Foundation.h>
#include <unordered_map>
#include <codecvt>
#include <locale>

namespace {
    // UTF-8 to UTF-32 conversion
    std::u32string utf8ToUtf32(const std::string& utf8) {
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
        try {
            return converter.from_bytes(utf8);
        } catch (...) {
            ofLogError("ofTrueTypeFont") << "UTF-8 conversion failed";
            return U"";
        }
    }

    // Check if path is a file or system font name
    bool isSystemFont(const std::string& fontPath) {
        return fontPath.find('.') == std::string::npos &&
               fontPath.find('/') == std::string::npos;
    }
}

// ============================================================
// Glyph Cache Structure
// ============================================================

struct GlyphInfo {
    ofRectangle texCoords;  // UV coordinates in atlas (0-1)
    ofRectangle bounds;     // Glyph bounds in pixels
    float advance;          // Horizontal advance
    bool cached = false;
};

// ============================================================
// Implementation
// ============================================================

struct ofTrueTypeFont::Impl {
    CTFontRef ctFont = nullptr;
    float fontSize = 12.0f;
    float lineHeight = 1.0f;
    float letterSpacing = 0.0f;
    bool antialiased = true;
    bool loaded = false;
    RenderMode renderMode = RenderMode::Texture;

    // Atlas
    std::unique_ptr<ofTexture> atlasTexture;
    int atlasWidth = 1024;
    int atlasHeight = 1024;
    int atlasCursorX = 0;
    int atlasCursorY = 0;
    int atlasRowHeight = 0;

    // Glyph cache
    std::unordered_map<char32_t, GlyphInfo> glyphCache;

    ~Impl() {
        if (ctFont) {
            CFRelease(ctFont);
            ctFont = nullptr;
        }
    }

    // Load font using Core Text
    bool loadFont(const std::string& fontPath, float size, float dpi);

    // Cache a single glyph
    bool cacheGlyph(char32_t ch);

    // Get or create glyph info
    const GlyphInfo* getGlyphInfo(char32_t ch);

    // Render glyph to bitmap
    ofPixels renderGlyphBitmap(char32_t ch, ofRectangle& bounds, float& advance);

    // Get glyph path (vector)
    ofPath getGlyphPath(char32_t ch);
};

// ============================================================
// Load Font
// ============================================================

bool ofTrueTypeFont::Impl::loadFont(const std::string& fontPath, float size, float dpi) {
    @autoreleasepool {
        NSString* fontName = [NSString stringWithUTF8String:fontPath.c_str()];

        if (isSystemFont(fontPath)) {
            // System font by name
            ctFont = CTFontCreateWithName((__bridge CFStringRef)fontName, size, nullptr);
        } else {
            // Font from file
            NSURL* fontURL = [NSURL fileURLWithPath:fontName];

            CFArrayRef descriptors = CTFontManagerCreateFontDescriptorsFromURL((__bridge CFURLRef)fontURL);
            if (!descriptors || CFArrayGetCount(descriptors) == 0) {
                if (descriptors) CFRelease(descriptors);
                ofLogError("ofTrueTypeFont") << "Failed to load font: " << fontPath;
                return false;
            }

            CTFontDescriptorRef descriptor = (CTFontDescriptorRef)CFArrayGetValueAtIndex(descriptors, 0);
            ctFont = CTFontCreateWithFontDescriptor(descriptor, size, nullptr);
            CFRelease(descriptors);
        }

        if (!ctFont) {
            ofLogError("ofTrueTypeFont") << "Failed to create CTFont: " << fontPath;
            return false;
        }

        fontSize = size;
        loaded = true;

        // Initialize atlas texture
        atlasTexture = std::make_unique<ofTexture>();
        atlasTexture->allocate(atlasWidth, atlasHeight, 1);  // Single channel (alpha)

        ofLogVerbose("ofTrueTypeFont") << "Loaded font: " << fontPath << " (" << size << "pt)";
        return true;
    }
}

// ============================================================
// Render Glyph to Bitmap
// ============================================================

ofPixels ofTrueTypeFont::Impl::renderGlyphBitmap(char32_t ch, ofRectangle& bounds, float& advance) {
    @autoreleasepool {
        CGGlyph glyph;
        UniChar unichar = (UniChar)ch;
        if (!CTFontGetGlyphsForCharacters(ctFont, &unichar, &glyph, 1)) {
            ofLogWarning("ofTrueTypeFont") << "Glyph not found for character: " << (int)ch;
            return ofPixels();
        }

        // Get glyph metrics
        CGSize glyphAdvance;
        CTFontGetAdvancesForGlyphs(ctFont, kCTFontOrientationHorizontal, &glyph, &glyphAdvance, 1);
        advance = glyphAdvance.width;

        CGRect glyphBounds;
        CTFontGetBoundingRectsForGlyphs(ctFont, kCTFontOrientationHorizontal, &glyph, &glyphBounds, 1);

        // Add padding
        int padding = 2;
        int width = (int)std::ceil(glyphBounds.size.width) + padding * 2;
        int height = (int)std::ceil(glyphBounds.size.height) + padding * 2;

        if (width <= 0 || height <= 0) {
            // Space or empty glyph
            bounds = ofRectangle(0, 0, 0, 0);
            return ofPixels();
        }

        // Create bitmap context
        ofPixels pixels;
        pixels.allocate(width, height, 1);  // Grayscale
        pixels.set(0);  // Clear to transparent

        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceGray();
        CGContextRef context = CGBitmapContextCreate(
            pixels.getData(),
            width,
            height,
            8,  // bits per component
            width,  // bytes per row
            colorSpace,
            kCGImageAlphaNone
        );
        CGColorSpaceRelease(colorSpace);

        if (!context) {
            ofLogError("ofTrueTypeFont") << "Failed to create CGContext";
            return ofPixels();
        }

        // Set rendering quality
        if (antialiased) {
            CGContextSetShouldAntialias(context, true);
            CGContextSetShouldSmoothFonts(context, true);
        } else {
            CGContextSetShouldAntialias(context, false);
        }

        // Draw glyph
        CGContextSetGrayFillColor(context, 1.0, 1.0);  // White

        CGPoint position = CGPointMake(
            padding - glyphBounds.origin.x,
            padding - glyphBounds.origin.y
        );

        CTFontDrawGlyphs(ctFont, &glyph, &position, 1, context);
        CGContextRelease(context);

        // Store bounds
        bounds = ofRectangle(
            glyphBounds.origin.x,
            glyphBounds.origin.y,
            glyphBounds.size.width,
            glyphBounds.size.height
        );

        return pixels;
    }
}

// ============================================================
// Cache Glyph
// ============================================================

bool ofTrueTypeFont::Impl::cacheGlyph(char32_t ch) {
    if (glyphCache.find(ch) != glyphCache.end()) {
        return true;  // Already cached
    }

    ofRectangle bounds;
    float advance;
    ofPixels glyphPixels = renderGlyphBitmap(ch, bounds, advance);

    if (glyphPixels.getWidth() == 0 || glyphPixels.getHeight() == 0) {
        // Space or empty glyph
        GlyphInfo info;
        info.bounds = bounds;
        info.advance = advance;
        info.cached = true;
        glyphCache[ch] = info;
        return true;
    }

    int glyphWidth = glyphPixels.getWidth();
    int glyphHeight = glyphPixels.getHeight();

    // Check if we need a new row
    if (atlasCursorX + glyphWidth > atlasWidth) {
        atlasCursorX = 0;
        atlasCursorY += atlasRowHeight + 1;  // +1 for spacing
        atlasRowHeight = 0;
    }

    // Check if atlas is full
    if (atlasCursorY + glyphHeight > atlasHeight) {
        ofLogWarning("ofTrueTypeFont") << "Atlas texture full, cannot cache more glyphs";
        return false;
    }

    // Upload glyph to atlas (TODO: implement texture subimage update)
    // For now, we'll mark it as cached with UV coordinates
    GlyphInfo info;
    info.texCoords = ofRectangle(
        (float)atlasCursorX / atlasWidth,
        (float)atlasCursorY / atlasHeight,
        (float)glyphWidth / atlasWidth,
        (float)glyphHeight / atlasHeight
    );
    info.bounds = bounds;
    info.advance = advance;
    info.cached = true;

    glyphCache[ch] = info;

    // Update cursor
    atlasCursorX += glyphWidth + 1;  // +1 for spacing
    atlasRowHeight = std::max(atlasRowHeight, glyphHeight);

    return true;
}

// ============================================================
// Get Glyph Info
// ============================================================

const GlyphInfo* ofTrueTypeFont::Impl::getGlyphInfo(char32_t ch) {
    auto it = glyphCache.find(ch);
    if (it != glyphCache.end()) {
        return &it->second;
    }

    // Cache on demand
    if (cacheGlyph(ch)) {
        return &glyphCache[ch];
    }

    return nullptr;
}

// ============================================================
// Get Glyph Path (Vector)
// ============================================================

ofPath ofTrueTypeFont::Impl::getGlyphPath(char32_t ch) {
    @autoreleasepool {
        ofPath path;

        CGGlyph glyph;
        UniChar unichar = (UniChar)ch;
        if (!CTFontGetGlyphsForCharacters(ctFont, &unichar, &glyph, 1)) {
            return path;
        }

        CGPathRef cgPath = CTFontCreatePathForGlyph(ctFont, glyph, nullptr);
        if (!cgPath) {
            return path;
        }

        // Convert CGPath to ofPath (TODO: implement path conversion)
        // For now, return empty path
        CGPathRelease(cgPath);

        return path;
    }
}

// ============================================================
// Public API
// ============================================================

ofTrueTypeFont::ofTrueTypeFont()
    : impl_(std::make_unique<Impl>()) {}

ofTrueTypeFont::~ofTrueTypeFont() = default;

ofTrueTypeFont::ofTrueTypeFont(ofTrueTypeFont&&) noexcept = default;
ofTrueTypeFont& ofTrueTypeFont::operator=(ofTrueTypeFont&&) noexcept = default;

bool ofTrueTypeFont::load(const std::string& fontPath,
                           int fontSize,
                           bool antialiased,
                           bool fullCharacterSet,
                           bool makeContours,
                           float dpi) {
    impl_->antialiased = antialiased;
    bool success = impl_->loadFont(fontPath, (float)fontSize, dpi);

    if (success && !fullCharacterSet) {
        // Pre-cache ASCII printable characters (32-126)
        for (char32_t ch = 32; ch <= 126; ++ch) {
            impl_->cacheGlyph(ch);
        }
    }

    return success;
}

bool ofTrueTypeFont::isLoaded() const {
    return impl_->loaded;
}

void ofTrueTypeFont::drawString(const std::string& text, float x, float y) const {
    if (!impl_->loaded) {
        ofLogWarning("ofTrueTypeFont") << "Font not loaded";
        return;
    }

    // Convert to UTF-32
    std::u32string utf32 = utf8ToUtf32(text);

    float cursorX = x;
    float cursorY = y;

    for (char32_t ch : utf32) {
        const GlyphInfo* info = impl_->getGlyphInfo(ch);
        if (!info) continue;

        // TODO: Render glyph quad with atlas texture
        // For now, just advance cursor
        cursorX += info->advance + impl_->letterSpacing;
    }
}

void ofTrueTypeFont::drawStringAsShapes(const std::string& text, float x, float y) const {
    if (!impl_->loaded) {
        ofLogWarning("ofTrueTypeFont") << "Font not loaded";
        return;
    }

    // TODO: Implement vector rendering
    ofLogWarning("ofTrueTypeFont") << "drawStringAsShapes not yet implemented";
}

ofRectangle ofTrueTypeFont::getStringBoundingBox(const std::string& text, float x, float y) const {
    if (!impl_->loaded) {
        return ofRectangle(x, y, 0, 0);
    }

    std::u32string utf32 = utf8ToUtf32(text);

    float minX = x;
    float minY = y;
    float maxX = x;
    float maxY = y;
    float cursorX = x;

    for (char32_t ch : utf32) {
        const GlyphInfo* info = impl_->getGlyphInfo(ch);
        if (!info) continue;

        float glyphMinX = cursorX + info->bounds.x;
        float glyphMinY = y + info->bounds.y;
        float glyphMaxX = glyphMinX + info->bounds.width;
        float glyphMaxY = glyphMinY + info->bounds.height;

        minX = std::min(minX, glyphMinX);
        minY = std::min(minY, glyphMinY);
        maxX = std::max(maxX, glyphMaxX);
        maxY = std::max(maxY, glyphMaxY);

        cursorX += info->advance + impl_->letterSpacing;
    }

    return ofRectangle(minX, minY, maxX - minX, maxY - minY);
}

float ofTrueTypeFont::stringWidth(const std::string& text) const {
    if (!impl_->loaded) {
        return 0.0f;
    }

    std::u32string utf32 = utf8ToUtf32(text);
    float width = 0.0f;

    for (size_t i = 0; i < utf32.size(); ++i) {
        const GlyphInfo* info = impl_->getGlyphInfo(utf32[i]);
        if (!info) continue;

        width += info->advance;
        if (i < utf32.size() - 1) {
            width += impl_->letterSpacing;
        }
    }

    return width;
}

float ofTrueTypeFont::stringHeight(const std::string& text) const {
    @autoreleasepool {
        if (!impl_->loaded) {
            return 0.0f;
        }

        CGFloat ascent = CTFontGetAscent(impl_->ctFont);
        CGFloat descent = CTFontGetDescent(impl_->ctFont);
        return (float)(ascent + descent);
    }
}

float ofTrueTypeFont::getLineHeight() const {
    return stringHeight("") * impl_->lineHeight;
}

void ofTrueTypeFont::setLineHeight(float lineHeight) {
    impl_->lineHeight = lineHeight;
}

float ofTrueTypeFont::getLetterSpacing() const {
    return impl_->letterSpacing;
}

void ofTrueTypeFont::setLetterSpacing(float spacing) {
    impl_->letterSpacing = spacing;
}

float ofTrueTypeFont::getFontSize() const {
    return impl_->fontSize;
}

std::vector<ofPath> ofTrueTypeFont::getCharacterAsPoints(char32_t character, bool vflip) const {
    std::vector<ofPath> paths;
    if (!impl_->loaded) {
        return paths;
    }

    ofPath path = impl_->getGlyphPath(character);
    paths.push_back(std::move(path));
    return paths;
}

std::vector<ofPath> ofTrueTypeFont::getCharacterAsPoints(const std::string& character, bool vflip) const {
    std::u32string utf32 = utf8ToUtf32(character);
    if (utf32.empty()) {
        return {};
    }
    return getCharacterAsPoints(utf32[0], vflip);
}

void ofTrueTypeFont::setRenderMode(RenderMode mode) {
    impl_->renderMode = mode;
}

ofTrueTypeFont::RenderMode ofTrueTypeFont::getRenderMode() const {
    return impl_->renderMode;
}

void* ofTrueTypeFont::getAtlasTexture() const {
    if (impl_->atlasTexture) {
        return impl_->atlasTexture->getNativeHandle();
    }
    return nullptr;
}

int ofTrueTypeFont::getNumGlyphsCached() const {
    return (int)impl_->glyphCache.size();
}
