#pragma once

#include <memory>
#include <string>
#include <vector>

class ofPath;

// Simple rectangle structure (ofRectangle will be implemented in Phase 14)
struct ofRectangle {
    float x = 0, y = 0, width = 0, height = 0;
    ofRectangle() = default;
    ofRectangle(float x_, float y_, float w_, float h_) : x(x_), y(y_), width(w_), height(h_) {}
};

/// \brief openFrameworks-compatible TrueType font class using Core Text
/// \note Uses Core Text for font loading and rendering (not FreeType)
/// \note Implements glyph atlas for efficient batch rendering
class ofTrueTypeFont {
public:
    /// \brief Font rendering modes
    enum class RenderMode {
        Texture,    ///< Bitmap texture (default, fast)
        Shapes      ///< Vector shapes (slow, scalable)
    };

    ofTrueTypeFont();
    ~ofTrueTypeFont();

    // Non-copyable, movable
    ofTrueTypeFont(const ofTrueTypeFont&) = delete;
    ofTrueTypeFont& operator=(const ofTrueTypeFont&) = delete;
    ofTrueTypeFont(ofTrueTypeFont&&) noexcept;
    ofTrueTypeFont& operator=(ofTrueTypeFont&&) noexcept;

    // ---- Loading ----

    /// \brief Load a TrueType font from file or system font name
    /// \param fontPath Path to .ttf/.otf file or system font name (e.g., "Helvetica")
    /// \param fontSize Font size in points
    /// \param antialiased Enable antialiasing (default: true)
    /// \param fullCharacterSet Load full Unicode set (default: false, ASCII only)
    /// \param makeContours Pre-generate vector contours (default: false)
    /// \param dpi Dots per inch (default: 72)
    /// \return true on success
    bool load(const std::string& fontPath,
              int fontSize,
              bool antialiased = true,
              bool fullCharacterSet = false,
              bool makeContours = false,
              float dpi = 72.0f);

    /// \brief Check if font is loaded
    bool isLoaded() const;

    // ---- Drawing ----

    /// \brief Draw text at position (bitmap mode)
    /// \param text UTF-8 string
    /// \param x X position
    /// \param y Y position (baseline)
    void drawString(const std::string& text, float x, float y) const;

    /// \brief Draw text as vector shapes (slow, scalable)
    void drawStringAsShapes(const std::string& text, float x, float y) const;

    // ---- Measurements ----

    /// \brief Get bounding box for text
    /// \param text UTF-8 string
    /// \param x X position
    /// \param y Y position
    /// \return Rectangle containing the text
    ofRectangle getStringBoundingBox(const std::string& text, float x, float y) const;

    /// \brief Get text width in pixels
    float stringWidth(const std::string& text) const;

    /// \brief Get text height in pixels (ascent + descent)
    float stringHeight(const std::string& text) const;

    /// \brief Get line height (baseline to baseline spacing)
    float getLineHeight() const;

    /// \brief Set line height multiplier (default: 1.0)
    void setLineHeight(float lineHeight);

    /// \brief Get letter spacing in pixels
    float getLetterSpacing() const;

    /// \brief Set letter spacing in pixels (default: 0)
    void setLetterSpacing(float spacing);

    /// \brief Get font size
    float getFontSize() const;

    // ---- Advanced ----

    /// \brief Get character outline as vector points
    /// \param character UTF-8 character
    /// \param vflip Flip vertically (default: false)
    /// \return Vector of ofPath objects (may contain multiple contours)
    std::vector<ofPath> getCharacterAsPoints(char32_t character, bool vflip = false) const;

    /// \brief Get character outline as single path
    std::vector<ofPath> getCharacterAsPoints(const std::string& character, bool vflip = false) const;

    /// \brief Set rendering mode
    void setRenderMode(RenderMode mode);

    /// \brief Get rendering mode
    RenderMode getRenderMode() const;

    /// \brief Get atlas texture for debugging (null if shapes mode)
    void* getAtlasTexture() const;

    /// \brief Get number of cached glyphs
    int getNumGlyphsCached() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    friend class ofGraphics;  // For batch rendering optimization
};
