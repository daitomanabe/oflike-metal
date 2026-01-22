// ofTrueTypeFont implementation using macOS Core Text
// NOTE: FreeType is NOT used. See ARCHITECTURE.md for this decision.

#import <CoreText/CoreText.h>
#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>

#include "ofTrueTypeFont.h"
#include "../../core/Context.h"

ofTrueTypeFont::ofTrueTypeFont() = default;

ofTrueTypeFont::~ofTrueTypeFont() {
  unload();
}

ofTrueTypeFont::ofTrueTypeFont(ofTrueTypeFont&& other) noexcept
  : ctFont_(other.ctFont_),
    fontName_(std::move(other.fontName_)),
    fontSize_(other.fontSize_),
    lineHeight_(other.lineHeight_),
    ascenderHeight_(other.ascenderHeight_),
    descenderHeight_(other.descenderHeight_),
    letterSpacing_(other.letterSpacing_),
    loaded_(other.loaded_),
    hasContours_(other.hasContours_),
    atlasTexture_(std::move(other.atlasTexture_)),
    atlasWidth_(other.atlasWidth_),
    atlasHeight_(other.atlasHeight_),
    glyphCache_(std::move(other.glyphCache_)),
    contourCache_(std::move(other.contourCache_)) {
  other.ctFont_ = nullptr;
  other.loaded_ = false;
}

ofTrueTypeFont& ofTrueTypeFont::operator=(ofTrueTypeFont&& other) noexcept {
  if (this != &other) {
    unload();
    ctFont_ = other.ctFont_;
    fontName_ = std::move(other.fontName_);
    fontSize_ = other.fontSize_;
    lineHeight_ = other.lineHeight_;
    ascenderHeight_ = other.ascenderHeight_;
    descenderHeight_ = other.descenderHeight_;
    letterSpacing_ = other.letterSpacing_;
    loaded_ = other.loaded_;
    hasContours_ = other.hasContours_;
    atlasTexture_ = std::move(other.atlasTexture_);
    atlasWidth_ = other.atlasWidth_;
    atlasHeight_ = other.atlasHeight_;
    glyphCache_ = std::move(other.glyphCache_);
    contourCache_ = std::move(other.contourCache_);
    other.ctFont_ = nullptr;
    other.loaded_ = false;
  }
  return *this;
}

bool ofTrueTypeFont::load(const std::string& fontName, int fontSize,
                          bool antialiased, bool fullCharacterSet,
                          bool makeContours, float simplifyAmt, int dpi) {
  (void)antialiased;  // Core Text always antialiases
  (void)fullCharacterSet;
  (void)simplifyAmt;
  (void)dpi;

  unload();

  @autoreleasepool {
    NSString* name = [NSString stringWithUTF8String:fontName.c_str()];
    CTFontRef font = nullptr;

    // Check if it's a file path
    if (fontName.find(".ttf") != std::string::npos ||
        fontName.find(".otf") != std::string::npos ||
        fontName.find("/") != std::string::npos) {
      // Load from file
      NSURL* fontURL = [NSURL fileURLWithPath:name];
      CGDataProviderRef dataProvider = CGDataProviderCreateWithURL((__bridge CFURLRef)fontURL);
      if (dataProvider) {
        CGFontRef cgFont = CGFontCreateWithDataProvider(dataProvider);
        if (cgFont) {
          font = CTFontCreateWithGraphicsFont(cgFont, fontSize, NULL, NULL);
          CGFontRelease(cgFont);
        }
        CGDataProviderRelease(dataProvider);
      }
    }

    // If file loading failed or it's a font name, try system font
    if (!font) {
      font = CTFontCreateWithName((__bridge CFStringRef)name, fontSize, NULL);
    }

    // Fallback to Helvetica
    if (!font) {
      font = CTFontCreateWithName(CFSTR("Helvetica"), fontSize, NULL);
    }

    if (!font) {
      return false;
    }

    ctFont_ = (void*)font;
    fontName_ = fontName;
    fontSize_ = fontSize;
    hasContours_ = makeContours;
    loaded_ = true;

    // Get font metrics
    ascenderHeight_ = CTFontGetAscent(font);
    descenderHeight_ = CTFontGetDescent(font);
    lineHeight_ = ascenderHeight_ + descenderHeight_ + CTFontGetLeading(font);

    // Create texture atlas for common ASCII characters
    createTextureAtlas();

    return true;
  }
}

void ofTrueTypeFont::unload() {
  if (ctFont_) {
    CFRelease((CTFontRef)ctFont_);
    ctFont_ = nullptr;
  }
  loaded_ = false;
  glyphCache_.clear();
  contourCache_.clear();
  atlasTexture_.clear();
}

void ofTrueTypeFont::createTextureAtlas() {
  if (!ctFont_) return;

  @autoreleasepool {
    CTFontRef font = (CTFontRef)ctFont_;

    // Calculate atlas size based on font size
    int charsPerRow = 16;
    int numRows = 8;  // ASCII 32-127 = 96 characters
    int cellWidth = fontSize_ + 4;
    int cellHeight = static_cast<int>(lineHeight_) + 4;

    atlasWidth_ = charsPerRow * cellWidth;
    atlasHeight_ = numRows * cellHeight;

    // Round up to power of 2
    int pw = 1;
    while (pw < atlasWidth_) pw *= 2;
    atlasWidth_ = pw;
    pw = 1;
    while (pw < atlasHeight_) pw *= 2;
    atlasHeight_ = pw;

    // Create bitmap context
    size_t bytesPerRow = atlasWidth_ * 4;
    std::vector<uint8_t> pixels(atlasWidth_ * atlasHeight_ * 4, 0);

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx = CGBitmapContextCreate(
      pixels.data(),
      atlasWidth_,
      atlasHeight_,
      8,
      bytesPerRow,
      colorSpace,
      kCGImageAlphaPremultipliedLast
    );
    CGColorSpaceRelease(colorSpace);

    if (!ctx) return;

    // Set up text rendering
    CGContextSetRGBFillColor(ctx, 1.0, 1.0, 1.0, 1.0);
    CGContextSetTextMatrix(ctx, CGAffineTransformIdentity);

    // Render ASCII characters 32-127
    for (int i = 32; i < 128; i++) {
      int col = (i - 32) % charsPerRow;
      int row = (i - 32) / charsPerRow;

      float cellX = col * cellWidth;
      float cellY = atlasHeight_ - (row + 1) * cellHeight;  // Flip Y

      UniChar ch = (UniChar)i;
      CGGlyph glyph;
      CTFontGetGlyphsForCharacters(font, &ch, &glyph, 1);

      // Get glyph metrics
      CGSize advance;
      CTFontGetAdvancesForGlyphs(font, kCTFontOrientationHorizontal, &glyph, &advance, 1);

      CGRect boundingRect;
      CTFontGetBoundingRectsForGlyphs(font, kCTFontOrientationHorizontal, &glyph, &boundingRect, 1);

      // Store glyph info
      GlyphInfo info;
      info.width = boundingRect.size.width;
      info.height = boundingRect.size.height;
      info.bearingX = boundingRect.origin.x;
      info.bearingY = boundingRect.origin.y;
      info.advance = advance.width;
      info.texX = (cellX + 2) / atlasWidth_;
      info.texY = (cellY + 2) / atlasHeight_;
      info.texW = (cellWidth - 4) / (float)atlasWidth_;
      info.texH = (cellHeight - 4) / (float)atlasHeight_;
      glyphCache_[(uint32_t)i] = info;

      // Draw glyph
      CGPoint position = CGPointMake(cellX + 2 - boundingRect.origin.x,
                                     cellY + 2 + descenderHeight_);
      CTFontDrawGlyphs(font, &glyph, &position, 1, ctx);
    }

    CGContextRelease(ctx);

    // Upload to texture
    atlasTexture_.loadData(pixels.data(), atlasWidth_, atlasHeight_, 4);
  }
}

ofTrueTypeFont::GlyphInfo ofTrueTypeFont::getGlyphInfo(uint32_t codepoint) const {
  auto it = glyphCache_.find(codepoint);
  if (it != glyphCache_.end()) {
    return it->second;
  }

  // Return default for unknown characters
  GlyphInfo info = {};
  info.advance = fontSize_ * 0.5f;
  return info;
}

void ofTrueTypeFont::drawString(const std::string& str, float x, float y) const {
  if (!loaded_ || str.empty()) return;

  auto& dl = oflike::engine().drawList();
  auto savedColor = oflike::engine().getStyle().color;

  // Use texture atlas for rendering
  if (atlasTexture_.isAllocated()) {
    atlasTexture_.bind();
  }

  float curX = x;
  float curY = y;

  for (size_t i = 0; i < str.size(); i++) {
    char c = str[i];

    if (c == '\n') {
      curX = x;
      curY += lineHeight_;
      continue;
    }

    if (c == '\t') {
      curX += fontSize_ * 4;
      continue;
    }

    uint32_t codepoint = static_cast<uint8_t>(c);
    GlyphInfo info = getGlyphInfo(codepoint);

    if (codepoint >= 32 && codepoint < 128) {
      // Calculate quad positions
      float gx = curX + info.bearingX;
      float gy = curY - info.bearingY - info.height + descenderHeight_;
      float gw = info.texW * atlasWidth_;
      float gh = info.texH * atlasHeight_;

      // Draw textured quad
      float u0 = info.texX;
      float v0 = info.texY;
      float u1 = info.texX + info.texW;
      float v1 = info.texY + info.texH;

      // Add vertices for textured quad (using DrawList's text rendering)
      dl.addText(std::string(1, c), curX, curY);
    }

    curX += info.advance + letterSpacing_;
  }

  if (atlasTexture_.isAllocated()) {
    atlasTexture_.unbind();
  }
}

void ofTrueTypeFont::drawStringAsShapes(const std::string& str, float x, float y) const {
  if (!loaded_ || !hasContours_) return;

  std::vector<ofPath> paths = getStringAsPoints(str, true, true);

  for (auto& path : paths) {
    path.draw(x, y);
  }
}

float ofTrueTypeFont::stringWidth(const std::string& str) const {
  if (!loaded_ || str.empty()) return 0;

  float width = 0;
  float maxWidth = 0;

  for (char c : str) {
    if (c == '\n') {
      maxWidth = std::max(maxWidth, width);
      width = 0;
      continue;
    }

    uint32_t codepoint = static_cast<uint8_t>(c);
    GlyphInfo info = getGlyphInfo(codepoint);
    width += info.advance + letterSpacing_;
  }

  return std::max(maxWidth, width);
}

float ofTrueTypeFont::stringHeight(const std::string& str) const {
  if (!loaded_ || str.empty()) return lineHeight_;

  int lines = 1;
  for (char c : str) {
    if (c == '\n') lines++;
  }

  return lines * lineHeight_;
}

ofRectangle ofTrueTypeFont::getStringBoundingBox(const std::string& str, float x, float y, bool vflip) const {
  (void)vflip;

  float w = stringWidth(str);
  float h = stringHeight(str);

  return ofRectangle(x, y - ascenderHeight_, w, h);
}

std::vector<ofPath> ofTrueTypeFont::getStringAsPoints(const std::string& str, bool vflip, bool filled) const {
  std::vector<ofPath> paths;

  if (!loaded_ || !hasContours_ || str.empty()) return paths;

  float curX = 0;
  float curY = 0;

  for (char c : str) {
    if (c == '\n') {
      curX = 0;
      curY += lineHeight_;
      continue;
    }

    if (c == '\t') {
      curX += fontSize_ * 4;
      continue;
    }

    if (c == ' ') {
      curX += fontSize_ * 0.3f + letterSpacing_;
      continue;
    }

    uint32_t codepoint = static_cast<uint8_t>(c);
    ofPath charPath = getCharacterAsPoints(codepoint, vflip, filled);

    // Translate path to current position
    // Note: ofPath doesn't have a translate method that modifies in place,
    // so we need to rebuild the path with offset
    ofPath translatedPath;
    translatedPath.setFilled(charPath.isFilled());
    translatedPath.setFillColor(charPath.getFillColor());
    translatedPath.setStrokeColor(charPath.getStrokeColor());
    translatedPath.setStrokeWidth(charPath.getStrokeWidth());

    // Get the outline and add with offset
    const auto& outlines = const_cast<ofPath&>(charPath).getOutline();
    for (const auto& polyline : outlines) {
      const auto& verts = polyline.getVertices();
      if (verts.empty()) continue;

      translatedPath.moveTo(verts[0].x + curX, verts[0].y + curY);
      for (size_t i = 1; i < verts.size(); i++) {
        translatedPath.lineTo(verts[i].x + curX, verts[i].y + curY);
      }
      if (polyline.isClosed()) {
        translatedPath.close();
      }
    }

    paths.push_back(translatedPath);

    GlyphInfo info = getGlyphInfo(codepoint);
    curX += info.advance + letterSpacing_;
  }

  return paths;
}

ofPath ofTrueTypeFont::getCharacterAsPoints(uint32_t character, bool vflip, bool filled) const {
  // Check cache
  auto it = contourCache_.find(character);
  if (it != contourCache_.end()) {
    ofPath path = it->second;
    path.setFilled(filled);
    return path;
  }

  ofPath path;

  if (!loaded_ || !hasContours_ || !ctFont_) {
    return path;
  }

  @autoreleasepool {
    CTFontRef font = (CTFontRef)ctFont_;

    UniChar ch = (UniChar)character;
    CGGlyph glyph;
    if (!CTFontGetGlyphsForCharacters(font, &ch, &glyph, 1)) {
      return path;
    }

    // Get glyph path
    CGPathRef cgPath = CTFontCreatePathForGlyph(font, glyph, NULL);
    if (!cgPath) {
      return path;
    }

    // Convert CGPath to ofPath
    struct PathContext {
      ofPath* path;
      bool vflip;
      float height;
    };

    PathContext ctx;
    ctx.path = &path;
    ctx.vflip = vflip;
    ctx.height = lineHeight_;

    CGPathApply(cgPath, &ctx, [](void* info, const CGPathElement* element) {
      PathContext* ctx = (PathContext*)info;
      ofPath* path = ctx->path;
      float yMult = ctx->vflip ? -1.0f : 1.0f;
      float yOffset = ctx->vflip ? ctx->height : 0;

      switch (element->type) {
        case kCGPathElementMoveToPoint:
          path->moveTo(element->points[0].x,
                       yOffset + element->points[0].y * yMult);
          break;

        case kCGPathElementAddLineToPoint:
          path->lineTo(element->points[0].x,
                       yOffset + element->points[0].y * yMult);
          break;

        case kCGPathElementAddQuadCurveToPoint:
          // Quadratic to cubic bezier conversion
          path->quadBezierTo(
            element->points[0].x, yOffset + element->points[0].y * yMult,
            element->points[0].x, yOffset + element->points[0].y * yMult,
            element->points[1].x, yOffset + element->points[1].y * yMult
          );
          break;

        case kCGPathElementAddCurveToPoint:
          path->bezierTo(
            element->points[0].x, yOffset + element->points[0].y * yMult,
            element->points[1].x, yOffset + element->points[1].y * yMult,
            element->points[2].x, yOffset + element->points[2].y * yMult
          );
          break;

        case kCGPathElementCloseSubpath:
          path->close();
          break;
      }
    });

    CGPathRelease(cgPath);
  }

  path.setFilled(filled);

  // Cache the result
  contourCache_[character] = path;

  return path;
}

bool ofTrueTypeFont::isValidGlyph(uint32_t codepoint) const {
  if (!loaded_ || !ctFont_) return false;

  @autoreleasepool {
    CTFontRef font = (CTFontRef)ctFont_;
    UniChar ch = (UniChar)codepoint;
    CGGlyph glyph;
    return CTFontGetGlyphsForCharacters(font, &ch, &glyph, 1);
  }
}

float ofTrueTypeFont::getCharWidth(uint32_t codepoint) const {
  GlyphInfo info = getGlyphInfo(codepoint);
  return info.advance;
}
