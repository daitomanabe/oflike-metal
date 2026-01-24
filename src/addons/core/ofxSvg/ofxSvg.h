#pragma once

#include "../../../oflike/graphics/ofPath.h"
#include <string>
#include <vector>
#include <memory>

/// \brief SVG loader using nanosvg
///
/// ofxSvg provides a simple interface for loading SVG files and converting
/// them to ofPath objects for rendering. It uses nanosvg for parsing.
///
/// Example usage:
/// \code
/// ofxSvg svg;
/// svg.load("logo.svg");
///
/// for (int i = 0; i < svg.getNumPaths(); ++i) {
///     ofPath& path = svg.getPathAt(i);
///     path.draw();
/// }
/// \endcode
class ofxSvg {
public:
    ofxSvg();
    ~ofxSvg();

    // Copy constructor and assignment
    ofxSvg(const ofxSvg& other);
    ofxSvg& operator=(const ofxSvg& other);

    // File I/O
    /// \brief Load SVG from file
    /// \param path Path to SVG file
    /// \param units Units for coordinate conversion (default "px")
    /// \param dpi DPI for unit conversion (default 96)
    /// \return true if successful
    bool load(const std::string& path, const std::string& units = "px", float dpi = 96.0f);

    /// \brief Load SVG from string
    /// \param svgStr SVG content as string
    /// \param units Units for coordinate conversion (default "px")
    /// \param dpi DPI for unit conversion (default 96)
    /// \return true if successful
    bool loadFromString(const std::string& svgStr, const std::string& units = "px", float dpi = 96.0f);

    /// \brief Check if SVG is loaded
    /// \return true if loaded
    bool isLoaded() const;

    /// \brief Clear all data
    void clear();

    // Path access
    /// \brief Get number of paths in the SVG
    /// \return Number of paths
    int getNumPaths() const;

    /// \brief Get path at index
    /// \param index Path index (0-based)
    /// \return Reference to ofPath
    oflike::ofPath& getPathAt(int index);

    /// \brief Get path at index (const version)
    /// \param index Path index (0-based)
    /// \return Const reference to ofPath
    const oflike::ofPath& getPathAt(int index) const;

    /// \brief Get all paths
    /// \return Vector of all paths
    std::vector<oflike::ofPath>& getPaths();

    /// \brief Get all paths (const version)
    /// \return Vector of all paths
    const std::vector<oflike::ofPath>& getPaths() const;

    // SVG dimensions
    /// \brief Get SVG width
    /// \return Width in pixels
    float getWidth() const;

    /// \brief Get SVG height
    /// \return Height in pixels
    float getHeight() const;

    // Drawing
    /// \brief Draw all paths
    void draw();

    /// \brief Draw at specified position
    /// \param x X position
    /// \param y Y position
    void draw(float x, float y);

    /// \brief Draw at specified position with size
    /// \param x X position
    /// \param y Y position
    /// \param width Width to draw
    /// \param height Height to draw
    void draw(float x, float y, float width, float height);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
