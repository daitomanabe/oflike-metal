#pragma once

#include <memory>
#include <string>

/// \brief Core Text based text rendering for Apple platforms
/// \note Uses Core Text + Core Graphics for high-quality text rendering
/// \note Renders directly to RGBA texture for Metal compatibility
class ofCoreText {
public:
    ofCoreText();
    ~ofCoreText();

    // Non-copyable, movable
    ofCoreText(const ofCoreText&) = delete;
    ofCoreText& operator=(const ofCoreText&) = delete;
    ofCoreText(ofCoreText&&) noexcept;
    ofCoreText& operator=(ofCoreText&&) noexcept;

    // ---- Loading ----

    /// \brief Load a font by name or path
    /// \param fontNameOrPath System font name (e.g., "Helvetica") or path to .ttf/.otf file
    /// \param size Font size in points
    /// \param antialiased Enable antialiasing (default: true)
    /// \return true if successful
    bool load(const std::string& fontNameOrPath, float size, bool antialiased = true);

    /// \brief Check if font is loaded
    bool isLoaded() const;

    /// \brief Unload the font and free resources
    void unload();

    // ---- Drawing ----

    /// \brief Draw text at specified position
    /// \param text UTF-8 encoded string
    /// \param x X position
    /// \param y Y position (baseline)
    void drawString(const std::string& text, float x, float y) const;

    // ---- Measurements ----

    /// \brief Get the width of a string
    /// \param text UTF-8 encoded string
    /// \return Width in pixels
    float stringWidth(const std::string& text) const;

    /// \brief Get the height of a string
    /// \param text UTF-8 encoded string
    /// \return Height in pixels
    float stringHeight(const std::string& text) const;

    /// \brief Get bounding box for text
    /// \param text UTF-8 encoded string
    /// \param x X position
    /// \param y Y position
    /// \return Bounding rectangle (x, y, width, height)
    struct Rectangle {
        float x, y, width, height;
    };
    Rectangle getStringBoundingBox(const std::string& text, float x, float y) const;

    // ---- Font Properties ----

    /// \brief Get font size
    float getSize() const;

    /// \brief Get line height
    float getLineHeight() const;

    /// \brief Get ascender height
    float getAscenderHeight() const;

    /// \brief Get descender height (negative value)
    float getDescenderHeight() const;

    /// \brief Set letter spacing
    void setLetterSpacing(float spacing);

    /// \brief Get letter spacing
    float getLetterSpacing() const;

    /// \brief Set line height multiplier
    void setLineHeight(float height);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
