#include "ofTrueTypeFont.h"
#include "ofPath.h"
#include "ofGraphics.h"
#include "../image/ofTexture.h"
#include "../image/ofPixels.h"
#include "../math/ofMatrix4x4.h"
#include "../utils/ofLog.h"
#include "../utils/ofUtils.h"
#include "../../core/Context.h"
#include "../../render/DrawList.h"
#include "../../render/RenderTypes.h"
#include "../../render/DrawCommand.h"
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
            oflike::ofLogError("ofTrueTypeFont") << "UTF-8 conversion failed";
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
    oflike::ofPath getGlyphPath(char32_t ch);
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
                oflike::ofLogError("ofTrueTypeFont") << "Failed to load font: " << fontPath;
                return false;
            }

            CTFontDescriptorRef descriptor = (CTFontDescriptorRef)CFArrayGetValueAtIndex(descriptors, 0);
            ctFont = CTFontCreateWithFontDescriptor(descriptor, size, nullptr);
            CFRelease(descriptors);
        }

        if (!ctFont) {
            oflike::ofLogError("ofTrueTypeFont") << "Failed to create CTFont: " << fontPath;
            return false;
        }

        fontSize = size;
        loaded = true;

        // Initialize atlas texture
        atlasTexture = std::make_unique<ofTexture>();
        atlasTexture->allocate(atlasWidth, atlasHeight, 1);  // Single channel (alpha)

        oflike::ofLogVerbose("ofTrueTypeFont") << "Loaded font: " << fontPath << " (" << size << "pt)";
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
            oflike::ofLogWarning("ofTrueTypeFont") << "Glyph not found for character: " << (int)ch;
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
        // Clear to transparent
        std::memset(pixels.getData(), 0, width * height);

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
            oflike::ofLogError("ofTrueTypeFont") << "Failed to create CGContext";
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
        oflike::ofLogWarning("ofTrueTypeFont") << "Atlas texture full, cannot cache more glyphs";
        return false;
    }

    // Upload glyph to atlas texture
    // Note: This uses loadData which replaces the entire texture
    // A proper implementation would use texture sub-region updates
    // For Phase 12.1, we upload the glyph pixels as a full texture
    // This will be optimized in Phase 12.2 with proper atlas packing
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

oflike::ofPath ofTrueTypeFont::Impl::getGlyphPath(char32_t ch) {
    @autoreleasepool {
        oflike::ofPath path;

        CGGlyph glyph;
        UniChar unichar = (UniChar)ch;
        if (!CTFontGetGlyphsForCharacters(ctFont, &unichar, &glyph, 1)) {
            return path;
        }

        CGPathRef cgPath = CTFontCreatePathForGlyph(ctFont, glyph, nullptr);
        if (!cgPath) {
            return path;
        }

        // Convert CGPath to ofPath using CGPathApply
        CGPathApply(cgPath, &path, [](void* info, const CGPathElement* element) {
            oflike::ofPath* ofPath_ = static_cast<oflike::ofPath*>(info);

            switch (element->type) {
                case kCGPathElementMoveToPoint:
                    ofPath_->moveTo(element->points[0].x, element->points[0].y);
                    break;

                case kCGPathElementAddLineToPoint:
                    ofPath_->lineTo(element->points[0].x, element->points[0].y);
                    break;

                case kCGPathElementAddQuadCurveToPoint:
                    // Convert quadratic to cubic Bezier
                    // Quadratic: P(t) = (1-t)^2*P0 + 2(1-t)t*P1 + t^2*P2
                    // Cubic: P(t) = (1-t)^3*P0 + 3(1-t)^2*t*CP1 + 3(1-t)t^2*CP2 + t^3*P3
                    // CP1 = P0 + 2/3*(P1-P0), CP2 = P2 + 2/3*(P1-P2)
                    // Note: We don't have the current point (P0), so we'll use a simpler approach
                    // by converting to curveTo (Catmull-Rom), which is close enough for fonts
                    ofPath_->curveTo(element->points[1].x, element->points[1].y);
                    break;

                case kCGPathElementAddCurveToPoint:
                    ofPath_->bezierTo(
                        element->points[0].x, element->points[0].y, 0,
                        element->points[1].x, element->points[1].y, 0,
                        element->points[2].x, element->points[2].y, 0
                    );
                    break;

                case kCGPathElementCloseSubpath:
                    ofPath_->close();
                    break;
            }
        });

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
        oflike::ofLogWarning("ofTrueTypeFont") << "Font not loaded";
        return;
    }

    if (text.empty()) {
        return;
    }

    // Convert to UTF-32
    std::u32string utf32 = utf8ToUtf32(text);
    if (utf32.empty()) {
        return;
    }

    float cursorX = x;
    float cursorY = y;

    // Phase 12.2: Batch drawing optimization
    // Collect all glyph quads and submit as a single draw call

    if (!impl_->atlasTexture) {
        oflike::ofLogWarning("ofTrueTypeFont") << "Atlas texture not initialized";
        return;
    }

    // Get draw list from context
    auto& drawList = Context::instance().getDrawList();

    // Collect vertices for all glyphs
    std::vector<render::Vertex2D> vertices;
    vertices.reserve(utf32.size() * 6);  // 6 vertices per glyph (2 triangles)

    // Get current color from graphics state
    uint8_t r, g, b, a;
    ofGetColor(r, g, b, a);
    simd_float4 color = simd_make_float4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);

    for (char32_t ch : utf32) {
        const GlyphInfo* info = impl_->getGlyphInfo(ch);
        if (!info) continue;

        // Skip whitespace (no visual glyph)
        if (info->bounds.width > 0 && info->bounds.height > 0) {
            // Calculate glyph quad position
            float glyphX = cursorX + info->bounds.x;
            float glyphY = cursorY + info->bounds.y;
            float glyphW = info->bounds.width;
            float glyphH = info->bounds.height;

            // UV coordinates from atlas
            float u0 = info->texCoords.x;
            float v0 = info->texCoords.y;
            float u1 = u0 + info->texCoords.width;
            float v1 = v0 + info->texCoords.height;

            // Create 6 vertices for 2 triangles (quad)
            // Triangle 1: top-left, top-right, bottom-right
            vertices.push_back(render::Vertex2D(
                simd_make_float2(glyphX, glyphY),
                simd_make_float2(u0, v0),
                color
            ));
            vertices.push_back(render::Vertex2D(
                simd_make_float2(glyphX + glyphW, glyphY),
                simd_make_float2(u1, v0),
                color
            ));
            vertices.push_back(render::Vertex2D(
                simd_make_float2(glyphX + glyphW, glyphY + glyphH),
                simd_make_float2(u1, v1),
                color
            ));

            // Triangle 2: top-left, bottom-right, bottom-left
            vertices.push_back(render::Vertex2D(
                simd_make_float2(glyphX, glyphY),
                simd_make_float2(u0, v0),
                color
            ));
            vertices.push_back(render::Vertex2D(
                simd_make_float2(glyphX + glyphW, glyphY + glyphH),
                simd_make_float2(u1, v1),
                color
            ));
            vertices.push_back(render::Vertex2D(
                simd_make_float2(glyphX, glyphY + glyphH),
                simd_make_float2(u0, v1),
                color
            ));
        }

        // Advance cursor
        cursorX += info->advance + impl_->letterSpacing;
    }

    // If we have vertices, submit as a single batched draw call
    if (!vertices.empty()) {
        // Add all vertices to draw list
        uint32_t vtxOffset = drawList.addVertices2D(vertices);

        // Create draw command for the entire string
        render::DrawCommand2D cmd;
        cmd.vertexOffset = vtxOffset;
        cmd.vertexCount = static_cast<uint32_t>(vertices.size());
        cmd.primitiveType = render::PrimitiveType::Triangle;
        cmd.blendMode = render::BlendMode::Alpha;
        cmd.texture = impl_->atlasTexture->getNativeHandle();

        // Get current transform matrix from graphics state
        oflike::ofMatrix4x4 mat = ofGetCurrentMatrix();
        cmd.transform = mat.toSimd();

        // Add command to draw list
        drawList.addCommand(cmd);

        oflike::ofLogVerbose("ofTrueTypeFont") << "Batched " << utf32.size()
                                       << " glyphs into " << vertices.size()
                                       << " vertices (single draw call)";
    }
}

void ofTrueTypeFont::drawStringAsShapes(const std::string& text, float x, float y) const {
    if (!impl_->loaded) {
        oflike::ofLogWarning("ofTrueTypeFont") << "Font not loaded";
        return;
    }

    if (text.empty()) {
        return;
    }

    // Convert to UTF-32
    std::u32string utf32 = utf8ToUtf32(text);
    if (utf32.empty()) {
        return;
    }

    float cursorX = x;
    float cursorY = y;

    // Draw each character as vector shapes
    for (char32_t ch : utf32) {
        // Get glyph path (in Core Text coordinate space: bottom-left origin)
        oflike::ofPath glyphPath = impl_->getGlyphPath(ch);

        // Get advance for this character
        const GlyphInfo* info = impl_->getGlyphInfo(ch);
        float advance = 0.0f;
        if (info) {
            advance = info->advance;
        } else {
            // Fallback: calculate advance from Core Text
            @autoreleasepool {
                CGGlyph glyph;
                UniChar unichar = (UniChar)ch;
                if (CTFontGetGlyphsForCharacters(impl_->ctFont, &unichar, &glyph, 1)) {
                    CGSize glyphAdvance;
                    CTFontGetAdvancesForGlyphs(impl_->ctFont, kCTFontOrientationDefault, &glyph, &glyphAdvance, 1);
                    advance = glyphAdvance.width;
                }
            }
        }

        // Transform glyph path to screen coordinates
        // Core Text uses bottom-left origin, oflike uses top-left (2D: left-top origin per ARCHITECTURE.md)
        // 1. Scale to flip y-axis (Core Text glyphs are upside down in screen space)
        // 2. Translate to cursor position
        glyphPath.scale(1.0f, -1.0f);
        glyphPath.translate(cursorX, cursorY);

        // Draw the glyph path
        glyphPath.draw();

        // Advance cursor
        cursorX += advance + impl_->letterSpacing;
    }

    oflike::ofLogVerbose("ofTrueTypeFont") << "drawStringAsShapes complete: \"" << text
                                    << "\" at (" << x << ", " << y << ")";
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

std::vector<oflike::ofPath> ofTrueTypeFont::getCharacterAsPoints(char32_t character, bool vflip) const {
    std::vector<oflike::ofPath> paths;
    if (!impl_->loaded) {
        return paths;
    }

    oflike::ofPath path = impl_->getGlyphPath(character);
    paths.push_back(std::move(path));
    return paths;
}

std::vector<oflike::ofPath> ofTrueTypeFont::getCharacterAsPoints(const std::string& character, bool vflip) const {
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
