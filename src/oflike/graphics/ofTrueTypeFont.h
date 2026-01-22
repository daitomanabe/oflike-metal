#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "ofPath.h"
#include "../types/ofRectangle.h"
#include "../image/ofTexture.h"

// ofTrueTypeFont - Core Text based font rendering for Metal/macOS
// NOTE: This implementation uses macOS Core Text instead of FreeType.
// See ARCHITECTURE.md for details on this decision.

class ofTrueTypeFont {
public:
  ofTrueTypeFont();
  ~ofTrueTypeFont();

  // No copy
  ofTrueTypeFont(const ofTrueTypeFont&) = delete;
  ofTrueTypeFont& operator=(const ofTrueTypeFont&) = delete;

  // Move
  ofTrueTypeFont(ofTrueTypeFont&& other) noexcept;
  ofTrueTypeFont& operator=(ofTrueTypeFont&& other) noexcept;

  // Load font
  // fontName: system font name (e.g., "Helvetica", "Arial") or path to .ttf file
  // fontSize: font size in points
  // antialiased: enable anti-aliasing (always true for Core Text)
  // fullCharacterSet: load extended character set
  // makeContours: enable getStringAsPoints() functionality
  bool load(const std::string& fontName, int fontSize,
            bool antialiased = true, bool fullCharacterSet = true,
            bool makeContours = false, float simplifyAmt = 0.0f, int dpi = 0);

  bool isLoaded() const { return loaded_; }
  void unload();

  // Drawing
  void drawString(const std::string& str, float x, float y) const;
  void drawStringAsShapes(const std::string& str, float x, float y) const;

  // Measurement
  float stringWidth(const std::string& str) const;
  float stringHeight(const std::string& str) const;
  ofRectangle getStringBoundingBox(const std::string& str, float x, float y, bool vflip = true) const;

  // Metrics
  float getLineHeight() const { return lineHeight_; }
  float getAscenderHeight() const { return ascenderHeight_; }
  float getDescenderHeight() const { return descenderHeight_; }

  void setLineHeight(float height) { lineHeight_ = height; }
  void setLetterSpacing(float spacing) { letterSpacing_ = spacing; }
  float getLetterSpacing() const { return letterSpacing_; }

  // Contour mode (requires makeContours=true in load())
  std::vector<ofPath> getStringAsPoints(const std::string& str, bool vflip = true, bool filled = true) const;
  ofPath getCharacterAsPoints(uint32_t character, bool vflip = true, bool filled = true) const;

  // Character info
  bool isValidGlyph(uint32_t codepoint) const;
  float getCharWidth(uint32_t codepoint) const;

  // Font info
  int getSize() const { return fontSize_; }
  std::string getFontName() const { return fontName_; }

private:
  struct GlyphInfo {
    float width;
    float height;
    float bearingX;
    float bearingY;
    float advance;
    float texX, texY;     // Position in texture atlas
    float texW, texH;     // Size in texture atlas
  };

  void createTextureAtlas();
  void renderGlyphToAtlas(uint32_t codepoint);
  GlyphInfo getGlyphInfo(uint32_t codepoint) const;

  // Core Text font handle (CTFontRef, stored as void* for C++ header)
  void* ctFont_ = nullptr;

  // Font properties
  std::string fontName_;
  int fontSize_ = 12;
  float lineHeight_ = 0;
  float ascenderHeight_ = 0;
  float descenderHeight_ = 0;
  float letterSpacing_ = 0;
  bool loaded_ = false;
  bool hasContours_ = false;

  // Texture atlas for bitmap rendering
  ofTexture atlasTexture_;
  int atlasWidth_ = 0;
  int atlasHeight_ = 0;
  std::unordered_map<uint32_t, GlyphInfo> glyphCache_;

  // Contour cache for vector rendering
  mutable std::unordered_map<uint32_t, ofPath> contourCache_;
};
