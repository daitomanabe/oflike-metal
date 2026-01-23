#pragma once

#include "../math/ofVec3f.h"
#include <vector>
#include <cstdint>

namespace oflike {

// Forward declarations
class ofRectangle;

/**
 * ofPolyline - A series of connected vertices forming a polyline.
 *
 * ofPolyline provides a way to create complex shapes by connecting vertices.
 * It supports both 2D and 3D coordinates, curves (Catmull-Rom and Bezier),
 * arcs, and various geometric operations.
 *
 * Compatible with openFrameworks ofPolyline API.
 */
class ofPolyline {
public:
    // ========================================================================
    // Constructors
    // ========================================================================

    /// Default constructor - creates an empty polyline
    ofPolyline();

    /// Construct from a vector of vertices
    explicit ofPolyline(const std::vector<ofVec3f>& vertices);

    // ========================================================================
    // Vertex Management
    // ========================================================================

    /// Add a single vertex to the polyline
    void addVertex(float x, float y, float z = 0.0f);

    /// Add a single vertex to the polyline
    void addVertex(const ofVec3f& p);

    /// Add multiple vertices to the polyline
    void addVertices(const std::vector<ofVec3f>& vertices);

    /// Add multiple vertices from raw float array (x,y,z triplets)
    void addVertices(const float* vertices, size_t count);

    // ========================================================================
    // Line and Curve Operations
    // ========================================================================

    /// Add a straight line from current position to the specified point
    void lineTo(float x, float y, float z = 0.0f);

    /// Add a straight line from current position to the specified point
    void lineTo(const ofVec3f& p);

    /// Add a Catmull-Rom curve to the specified point
    /// Uses the last point as start, next-to-last as control point
    void curveTo(float x, float y, float z = 0.0f);

    /// Add a Catmull-Rom curve to the specified point
    void curveTo(const ofVec3f& p);

    /// Add a cubic Bezier curve with two control points
    void bezierTo(float cx1, float cy1, float cz1,
                  float cx2, float cy2, float cz2,
                  float x, float y, float z = 0.0f);

    /// Add a cubic Bezier curve with two control points
    void bezierTo(const ofVec3f& cp1, const ofVec3f& cp2, const ofVec3f& p);

    // ========================================================================
    // Arc Operations
    // ========================================================================

    /// Add a circular arc (2D only, in XY plane)
    /// @param x Center X coordinate
    /// @param y Center Y coordinate
    /// @param radius Arc radius
    /// @param angleBegin Start angle in degrees
    /// @param angleEnd End angle in degrees
    void arc(float x, float y, float radius, float angleBegin, float angleEnd);

    /// Add a circular arc in the negative (clockwise) direction
    void arcNegative(float x, float y, float radius, float angleBegin, float angleEnd);

    // ========================================================================
    // Closing and State
    // ========================================================================

    /// Close the polyline by connecting the last vertex to the first
    void close();

    /// Check if the polyline is closed
    bool isClosed() const;

    /// Clear all vertices
    void clear();

    // ========================================================================
    // Drawing
    // ========================================================================

    /// Draw the polyline using current color and line width
    void draw() const;

    // ========================================================================
    // Access
    // ========================================================================

    /// Get all vertices
    const std::vector<ofVec3f>& getVertices() const;

    /// Get a reference to all vertices (non-const)
    std::vector<ofVec3f>& getVertices();

    /// Get number of vertices
    size_t size() const;

    /// Check if polyline is empty
    bool empty() const;

    /// Get vertex at index
    const ofVec3f& operator[](size_t index) const;

    /// Get vertex at index (non-const)
    ofVec3f& operator[](size_t index);

    // ========================================================================
    // Geometric Properties
    // ========================================================================

    /// Get the total length (perimeter) of the polyline
    float getPerimeter() const;

    /// Get the area enclosed by the polyline (2D only, assumes XY plane)
    /// Returns 0 if polyline is not closed
    float getArea() const;

    /// Get the 2D centroid of the polyline (average of all vertices in XY plane)
    ofVec3f getCentroid2D() const;

    /// Get the bounding box of the polyline
    ofRectangle getBoundingBox() const;

    // ========================================================================
    // Geometric Queries
    // ========================================================================

    /// Find the closest point on the polyline to the given point
    /// @param target The point to test
    /// @param nearestIndex Output parameter: index of the segment containing the closest point
    /// @return The closest point on the polyline
    ofVec3f getClosestPoint(const ofVec3f& target, size_t* nearestIndex = nullptr) const;

    /// Test if a 2D point is inside the polyline (2D only, XY plane)
    /// Only valid for closed polylines
    bool inside(float x, float y) const;

    /// Test if a 2D point is inside the polyline
    bool inside(const ofVec3f& p) const;

    // ========================================================================
    // Simplification and Resampling
    // ========================================================================

    /// Simplify the polyline using Ramer-Douglas-Peucker algorithm
    /// @param tolerance Maximum distance from original polyline
    void simplify(float tolerance = 0.3f);

    /// Get a resampled version with points evenly spaced by distance
    /// @param spacing Distance between points
    /// @return New resampled polyline
    ofPolyline getResampledBySpacing(float spacing) const;

    /// Get a resampled version with specified number of points
    /// @param count Number of points in resampled polyline
    /// @return New resampled polyline
    ofPolyline getResampledByCount(size_t count) const;

    /// Get a smoothed version of the polyline
    /// @param smoothingSize Number of vertices to average (higher = smoother)
    /// @param smoothingShape Smoothing factor (0.0 = no smoothing, 1.0 = maximum)
    /// @return New smoothed polyline
    ofPolyline getSmoothed(int smoothingSize, float smoothingShape = 0.0f) const;

private:
    std::vector<ofVec3f> vertices_;
    bool closed_;

    // Helper methods
    void addCurveVertex(const ofVec3f& p0, const ofVec3f& p1,
                       const ofVec3f& p2, const ofVec3f& p3);
    void addBezierVertex(const ofVec3f& p0, const ofVec3f& cp1,
                        const ofVec3f& cp2, const ofVec3f& p1);
    void addArcVertices(float x, float y, float radius,
                       float angleBegin, float angleEnd, bool clockwise);

    // Simplification helper (Ramer-Douglas-Peucker)
    void simplifyDP(const std::vector<ofVec3f>& points,
                   float tolerance,
                   size_t start, size_t end,
                   std::vector<bool>& keepFlags) const;
};

/**
 * ofRectangle - Axis-aligned bounding box
 * Simple structure used by ofPolyline::getBoundingBox()
 */
class ofRectangle {
public:
    float x, y;
    float width, height;

    ofRectangle() : x(0), y(0), width(0), height(0) {}
    ofRectangle(float x_, float y_, float w_, float h_)
        : x(x_), y(y_), width(w_), height(h_) {}

    float getMinX() const { return x; }
    float getMaxX() const { return x + width; }
    float getMinY() const { return y; }
    float getMaxY() const { return y + height; }
    float getWidth() const { return width; }
    float getHeight() const { return height; }
};

} // namespace oflike
