#pragma once

#include "../math/ofVec3f.h"
#include "ofPolyline.h"
#include <vector>
#include <cstdint>
#include <memory>

namespace oflike {

/**
 * ofPath - A path object for creating complex 2D shapes.
 *
 * ofPath provides a high-level interface for creating complex 2D shapes
 * using path commands (moveTo, lineTo, curveTo, bezierTo, arc).
 * It supports both filled and stroked rendering, and uses tess2 for
 * polygon tessellation of filled shapes.
 *
 * Compatible with openFrameworks ofPath API.
 *
 * Usage:
 * @code
 * ofPath path;
 * path.setFilled(true);
 * path.setFillColor(255, 0, 0);
 * path.moveTo(100, 100);
 * path.lineTo(200, 100);
 * path.lineTo(150, 200);
 * path.close();
 * path.draw();
 * @endcode
 */
class ofPath {
public:
    // ========================================================================
    // Constructors
    // ========================================================================

    /// Default constructor - creates an empty path
    ofPath();

    /// Destructor
    ~ofPath();

    // Copy/move constructors and operators
    ofPath(const ofPath& other);
    ofPath& operator=(const ofPath& other);
    ofPath(ofPath&& other) noexcept;
    ofPath& operator=(ofPath&& other) noexcept;

    // ========================================================================
    // Path Construction
    // ========================================================================

    /// Move the current drawing position without creating a line
    /// This starts a new subpath
    void moveTo(float x, float y, float z = 0.0f);

    /// Move the current drawing position
    void moveTo(const ofVec3f& p);

    /// Add a straight line from current position to the specified point
    void lineTo(float x, float y, float z = 0.0f);

    /// Add a straight line from current position to the specified point
    void lineTo(const ofVec3f& p);

    /// Add a Catmull-Rom curve to the specified point
    void curveTo(float x, float y, float z = 0.0f);

    /// Add a Catmull-Rom curve to the specified point
    void curveTo(const ofVec3f& p);

    /// Add a cubic Bezier curve with two control points
    void bezierTo(float cx1, float cy1, float cz1,
                  float cx2, float cy2, float cz2,
                  float x, float y, float z = 0.0f);

    /// Add a cubic Bezier curve with two control points
    void bezierTo(const ofVec3f& cp1, const ofVec3f& cp2, const ofVec3f& p);

    /// Add a circular arc (2D only)
    /// @param x Center X coordinate
    /// @param y Center Y coordinate
    /// @param radius Arc radius
    /// @param angleBegin Start angle in degrees
    /// @param angleEnd End angle in degrees
    void arc(float x, float y, float radius, float angleBegin, float angleEnd);

    /// Add a circular arc in the negative (clockwise) direction
    void arcNegative(float x, float y, float radius, float angleBegin, float angleEnd);

    /// Close the current subpath by connecting back to its starting point
    void close();

    /// Clear all path data
    void clear();

    // ========================================================================
    // Rendering Properties
    // ========================================================================

    /// Set whether the path should be filled
    void setFilled(bool filled);

    /// Check if the path is filled
    bool isFilled() const;

    /// Set the stroke width for outline rendering
    void setStrokeWidth(float width);

    /// Get the stroke width
    float getStrokeWidth() const;

    /// Set the fill color (0-255 range)
    void setFillColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

    /// Set the fill color (grayscale)
    void setFillColor(uint8_t gray, uint8_t a = 255);

    /// Set the stroke color (0-255 range)
    void setStrokeColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

    /// Set the stroke color (grayscale)
    void setStrokeColor(uint8_t gray, uint8_t a = 255);

    /// Set both fill and stroke color
    void setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

    /// Set both fill and stroke color (grayscale)
    void setColor(uint8_t gray, uint8_t a = 255);

    /// Get the fill color
    void getFillColor(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) const;

    /// Get the stroke color
    void getStrokeColor(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) const;

    // ========================================================================
    // Drawing
    // ========================================================================

    /// Draw the path (filled and/or stroked based on current settings)
    void draw() const;

    /// Draw the path at a specified position
    void draw(float x, float y) const;

    // ========================================================================
    // Access to Geometry
    // ========================================================================

    /// Get the outline as a vector of polylines (one per subpath)
    const std::vector<ofPolyline>& getOutline() const;

    /// Get the tessellated geometry (triangles) for filled rendering
    /// Returns a flat array of vertex coordinates (x,y,z triplets)
    /// Only valid if the path is filled
    std::vector<ofVec3f> getTessellation() const;

    // ========================================================================
    // Simplification
    // ========================================================================

    /// Simplify the path using Ramer-Douglas-Peucker algorithm
    /// @param tolerance Maximum distance from original path
    void simplify(float tolerance = 0.3f);

    // ========================================================================
    // Transformations
    // ========================================================================

    /// Translate the entire path
    void translate(float x, float y, float z = 0.0f);

    /// Translate the entire path
    void translate(const ofVec3f& offset);

    /// Rotate the entire path around the Z axis (2D rotation)
    /// @param angle Rotation angle in degrees
    void rotate(float angle);

    /// Rotate the entire path around an arbitrary axis
    /// @param angle Rotation angle in degrees
    /// @param axis Rotation axis (must be normalized)
    void rotate(float angle, const ofVec3f& axis);

    /// Scale the entire path
    void scale(float sx, float sy, float sz = 1.0f);

    /// Scale the entire path uniformly
    void scale(float s);

    // ========================================================================
    // State Queries
    // ========================================================================

    /// Check if the path is empty
    bool empty() const;

    /// Get the number of subpaths
    size_t getNumSubpaths() const;

    /// Get the current drawing position
    ofVec3f getCurrentPosition() const;

private:
    // Command types for path construction
    enum class Command {
        MoveTo,
        LineTo,
        Close
    };

    // A single path command with position
    struct PathCommand {
        Command type;
        ofVec3f position;
    };

    // Internal state
    std::vector<ofPolyline> polylines_;      // One polyline per subpath
    ofVec3f currentPosition_;                 // Current drawing position
    bool hasCurrentPosition_;                 // Whether currentPosition_ is valid

    // Rendering state
    bool filled_;
    float strokeWidth_;
    uint8_t fillColor_[4];                    // RGBA
    uint8_t strokeColor_[4];                  // RGBA

    // Cached tessellation (mutable for lazy evaluation)
    mutable std::vector<ofVec3f> tessellationCache_;
    mutable bool tessellationDirty_;

    // Helper methods
    void ensurePolyline();                    // Ensure we have a current polyline
    ofPolyline& getCurrentPolyline();         // Get or create current polyline
    const ofPolyline& getCurrentPolyline() const;
    void tessellate() const;                  // Perform tessellation using tess2
    void invalidateTessellation();            // Mark tessellation as dirty
};

} // namespace oflike
