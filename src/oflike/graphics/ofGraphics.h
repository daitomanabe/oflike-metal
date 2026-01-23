#pragma once

#include <cstdint>
#include <cstddef>  // for size_t

// Forward declarations
namespace oflike {
    class ofVec3f;
}

// ============================================================================
// Blend Mode Constants
// ============================================================================

/// Blend mode constants (compatible with openFrameworks)
enum {
    OF_BLENDMODE_DISABLED = 0,      ///< No blending
    OF_BLENDMODE_ALPHA = 1,         ///< Standard alpha blending (default)
    OF_BLENDMODE_ADD = 2,           ///< Additive blending
    OF_BLENDMODE_SUBTRACT = 3,      ///< Subtractive blending
    OF_BLENDMODE_MULTIPLY = 4,      ///< Multiply blending
    OF_BLENDMODE_SCREEN = 5,        ///< Screen blending
};

// ============================================================================
// ofGraphics - openFrameworks Compatible Graphics API
// ============================================================================

/**
 * Graphics state management and drawing functions.
 *
 * This module provides oF-compatible drawing state functions including:
 * - Color management (ofSetColor, ofSetBackgroundColor)
 * - Background clearing (ofBackground, ofClear)
 * - Fill/stroke mode (ofFill, ofNoFill)
 * - Line width and shape resolution
 *
 * All functions are thread-safe when called from the main thread only.
 */

// ============================================================================
// Color Management
// ============================================================================

/**
 * Set the current drawing color (0-255 range).
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @param a Alpha component (0-255), default: 255
 */
void ofSetColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

/**
 * Set the current drawing color (0-255 range, grayscale).
 * @param gray Grayscale value (0-255)
 * @param a Alpha component (0-255), default: 255
 */
void ofSetColor(uint8_t gray, uint8_t a = 255);

/**
 * Set the current drawing color from hex (0xRRGGBB or 0xAARRGGBB).
 * @param hex Color as hexadecimal (0xRRGGBB or 0xAARRGGBB)
 * @param a Alpha component (0-255), default: 255 (ignored if hex includes alpha)
 */
void ofSetHexColor(uint32_t hex, uint8_t a = 255);

/**
 * Set the background color (0-255 range).
 * Does not clear immediately, use ofBackground() to clear.
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @param a Alpha component (0-255), default: 255
 */
void ofSetBackgroundColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

/**
 * Set the background color (0-255 range, grayscale).
 * @param gray Grayscale value (0-255)
 * @param a Alpha component (0-255), default: 255
 */
void ofSetBackgroundColor(uint8_t gray, uint8_t a = 255);

/**
 * Set the background color from hex.
 * @param hex Color as hexadecimal (0xRRGGBB or 0xAARRGGBB)
 * @param a Alpha component (0-255), default: 255
 */
void ofSetBackgroundHexColor(uint32_t hex, uint8_t a = 255);

// ============================================================================
// Background Clearing
// ============================================================================

/**
 * Clear the screen with current background color.
 */
void ofBackground();

/**
 * Clear the screen with specified color (0-255 range).
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @param a Alpha component (0-255), default: 255
 */
void ofBackground(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

/**
 * Clear the screen with specified color (0-255 range, grayscale).
 * @param gray Grayscale value (0-255)
 * @param a Alpha component (0-255), default: 255
 */
void ofBackground(uint8_t gray, uint8_t a = 255);

/**
 * Clear the screen with specified color from hex.
 * @param hex Color as hexadecimal (0xRRGGBB or 0xAARRGGBB)
 * @param a Alpha component (0-255), default: 255
 */
void ofBackgroundHex(uint32_t hex, uint8_t a = 255);

/**
 * Clear color, depth, and/or stencil buffers.
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @param a Alpha component (0-255)
 */
void ofClear(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

/**
 * Clear with current background color.
 */
void ofClear();

/**
 * Clear the depth buffer only.
 */
void ofClearDepth();

/**
 * Clear the alpha channel.
 */
void ofClearAlpha();

// ============================================================================
// Fill & Stroke Mode
// ============================================================================

/**
 * Enable fill mode (shapes are filled).
 * This is the default mode.
 */
void ofFill();

/**
 * Disable fill mode (shapes are outlined only).
 */
void ofNoFill();

/**
 * Check if fill mode is enabled.
 * @return true if fill mode is enabled, false otherwise
 */
bool ofGetFill();

// ============================================================================
// Line & Shape Settings
// ============================================================================

/**
 * Set the line width for stroke drawing.
 * @param width Line width in pixels (default: 1.0)
 */
void ofSetLineWidth(float width);

/**
 * Get the current line width.
 * @return Current line width in pixels
 */
float ofGetLineWidth();

/**
 * Set the circle resolution (number of segments).
 * Higher values = smoother circles, but slower rendering.
 * @param resolution Number of segments (default: 32)
 */
void ofSetCircleResolution(uint32_t resolution);

/**
 * Get the current circle resolution.
 * @return Number of segments used for circles
 */
uint32_t ofGetCircleResolution();

/**
 * Set the curve resolution (number of segments per curve).
 * @param resolution Number of segments (default: 20)
 */
void ofSetCurveResolution(uint32_t resolution);

/**
 * Get the current curve resolution.
 * @return Number of segments used for curves
 */
uint32_t ofGetCurveResolution();

/**
 * Set the sphere resolution (lat/lon segments).
 * @param resolution Number of segments (default: 20)
 */
void ofSetSphereResolution(uint32_t resolution);

/**
 * Get the current sphere resolution.
 * @return Number of segments used for spheres
 */
uint32_t ofGetSphereResolution();

// ============================================================================
// Drawing State Query
// ============================================================================

/**
 * Get the current draw color as RGBA (0-255 range).
 * @param r Output: Red component
 * @param g Output: Green component
 * @param b Output: Blue component
 * @param a Output: Alpha component
 */
void ofGetColor(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a);

/**
 * Get the current background color as RGBA (0-255 range).
 * @param r Output: Red component
 * @param g Output: Green component
 * @param b Output: Blue component
 * @param a Output: Alpha component
 */
void ofGetBackgroundColor(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a);

// ============================================================================
// Matrix Stack (Forward Declaration)
// ============================================================================

// Forward declare matrix type to avoid including heavy headers
namespace oflike { class ofMatrix4x4; }

/**
 * Push the current transformation matrix onto the stack.
 * Use this to save the current transformation state before making changes.
 */
void ofPushMatrix();

/**
 * Pop the transformation matrix from the stack.
 * Restores the previously saved transformation state.
 */
void ofPopMatrix();

/**
 * Apply a translation to the current matrix.
 * @param x Translation in X axis
 * @param y Translation in Y axis
 * @param z Translation in Z axis (default: 0)
 */
void ofTranslate(float x, float y, float z = 0.0f);

/**
 * Apply a rotation around an arbitrary axis.
 * @param angle Rotation angle in degrees
 * @param x X component of rotation axis
 * @param y Y component of rotation axis
 * @param z Z component of rotation axis
 */
void ofRotate(float angle, float x, float y, float z);

/**
 * Apply a rotation around the Z axis (2D rotation).
 * @param angle Rotation angle in degrees
 */
void ofRotate(float angle);

/**
 * Apply a rotation around the X axis.
 * @param angle Rotation angle in degrees
 */
void ofRotateX(float angle);

/**
 * Apply a rotation around the Y axis.
 * @param angle Rotation angle in degrees
 */
void ofRotateY(float angle);

/**
 * Apply a rotation around the Z axis.
 * @param angle Rotation angle in degrees
 */
void ofRotateZ(float angle);

/**
 * Apply a scale transformation.
 * @param x Scale factor in X axis
 * @param y Scale factor in Y axis
 * @param z Scale factor in Z axis (default: 1.0)
 */
void ofScale(float x, float y, float z = 1.0f);

/**
 * Apply a uniform scale transformation.
 * @param scale Scale factor applied to all axes
 */
void ofScale(float scale);

/**
 * Reset the current matrix to identity.
 */
void ofLoadIdentityMatrix();

/**
 * Load a custom matrix as the current transformation matrix.
 * @param m The matrix to load
 */
void ofLoadMatrix(const oflike::ofMatrix4x4& m);

/**
 * Multiply the current matrix by a custom matrix.
 * @param m The matrix to multiply with
 */
void ofMultMatrix(const oflike::ofMatrix4x4& m);

/**
 * Get the current transformation matrix.
 * @return The current matrix
 */
oflike::ofMatrix4x4 ofGetCurrentMatrix();

/**
 * Get the current depth of the matrix stack.
 * @return Stack depth
 */
int ofGetMatrixStackDepth();

// ============================================================================
// Basic Shape Drawing
// ============================================================================

/**
 * Draw a line between two points.
 * @param x1 Starting X coordinate
 * @param y1 Starting Y coordinate
 * @param x2 Ending X coordinate
 * @param y2 Ending Y coordinate
 */
void ofDrawLine(float x1, float y1, float x2, float y2);

/**
 * Draw a line between two points in 3D space.
 * @param x1 Starting X coordinate
 * @param y1 Starting Y coordinate
 * @param z1 Starting Z coordinate
 * @param x2 Ending X coordinate
 * @param y2 Ending Y coordinate
 * @param z2 Ending Z coordinate
 */
void ofDrawLine(float x1, float y1, float z1, float x2, float y2, float z2);

/**
 * Draw a rectangle.
 * @param x X coordinate of top-left corner
 * @param y Y coordinate of top-left corner
 * @param w Width of rectangle
 * @param h Height of rectangle
 */
void ofDrawRectangle(float x, float y, float w, float h);

/**
 * Draw a rectangle with rounded corners.
 * @param x X coordinate of top-left corner
 * @param y Y coordinate of top-left corner
 * @param w Width of rectangle
 * @param h Height of rectangle
 * @param r Corner radius
 */
void ofDrawRectRounded(float x, float y, float w, float h, float r);

/**
 * Draw a circle.
 * @param x X coordinate of center
 * @param y Y coordinate of center
 * @param radius Circle radius
 */
void ofDrawCircle(float x, float y, float radius);

/**
 * Draw a circle in 3D space.
 * @param x X coordinate of center
 * @param y Y coordinate of center
 * @param z Z coordinate of center
 * @param radius Circle radius
 */
void ofDrawCircle(float x, float y, float z, float radius);

/**
 * Draw an ellipse.
 * @param x X coordinate of center
 * @param y Y coordinate of center
 * @param width Ellipse width (diameter)
 * @param height Ellipse height (diameter)
 */
void ofDrawEllipse(float x, float y, float width, float height);

/**
 * Draw a triangle.
 * @param x1 First vertex X coordinate
 * @param y1 First vertex Y coordinate
 * @param x2 Second vertex X coordinate
 * @param y2 Second vertex Y coordinate
 * @param x3 Third vertex X coordinate
 * @param y3 Third vertex Y coordinate
 */
void ofDrawTriangle(float x1, float y1, float x2, float y2, float x3, float y3);

/**
 * Draw a triangle in 3D space.
 * @param x1 First vertex X coordinate
 * @param y1 First vertex Y coordinate
 * @param z1 First vertex Z coordinate
 * @param x2 Second vertex X coordinate
 * @param y2 Second vertex Y coordinate
 * @param z2 Second vertex Z coordinate
 * @param x3 Third vertex X coordinate
 * @param y3 Third vertex Y coordinate
 * @param z3 Third vertex Z coordinate
 */
void ofDrawTriangle(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3);

// ============================================================================
// Curve Drawing
// ============================================================================

/**
 * Draw a Catmull-Rom curve through four control points (2D).
 * The curve passes through p1 and p2, with p0 and p3 affecting curvature.
 * @param x0 First control point X
 * @param y0 First control point Y
 * @param x1 Second control point X (curve starts here)
 * @param y1 Second control point Y
 * @param x2 Third control point X (curve ends here)
 * @param y2 Third control point Y
 * @param x3 Fourth control point X
 * @param y3 Fourth control point Y
 */
void ofDrawCurve(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3);

/**
 * Draw a Catmull-Rom curve through four control points (3D).
 * @param x0 First control point X
 * @param y0 First control point Y
 * @param z0 First control point Z
 * @param x1 Second control point X (curve starts here)
 * @param y1 Second control point Y
 * @param z1 Second control point Z
 * @param x2 Third control point X (curve ends here)
 * @param y2 Third control point Y
 * @param z2 Third control point Z
 * @param x3 Fourth control point X
 * @param y3 Fourth control point Y
 * @param z3 Fourth control point Z
 */
void ofDrawCurve(float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3);

/**
 * Draw a cubic Bezier curve (2D).
 * @param x0 Start point X
 * @param y0 Start point Y
 * @param x1 First control point X
 * @param y1 First control point Y
 * @param x2 Second control point X
 * @param y2 Second control point Y
 * @param x3 End point X
 * @param y3 End point Y
 */
void ofDrawBezier(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3);

/**
 * Draw a cubic Bezier curve (3D).
 * @param x0 Start point X
 * @param y0 Start point Y
 * @param z0 Start point Z
 * @param x1 First control point X
 * @param y1 First control point Y
 * @param z1 First control point Z
 * @param x2 Second control point X
 * @param y2 Second control point Y
 * @param z2 Second control point Z
 * @param x3 End point X
 * @param y3 End point Y
 * @param z3 End point Z
 */
void ofDrawBezier(float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3);

// ============================================================================
// Shape API (Immediate Mode)
// ============================================================================

/**
 * Begin defining a custom shape using vertices.
 * Must be paired with ofEndShape().
 *
 * Usage:
 * @code
 * ofBeginShape();
 *   ofVertex(100, 100);
 *   ofVertex(200, 100);
 *   ofVertex(200, 200);
 * ofEndShape();
 * @endcode
 */
void ofBeginShape();

/**
 * End the current shape definition and draw it.
 * If close is true, the shape will be closed by connecting the last vertex to the first.
 * @param close If true, close the shape
 */
void ofEndShape(bool close = false);

/**
 * Add a vertex to the current shape (2D).
 * Must be called between ofBeginShape() and ofEndShape().
 * @param x X coordinate
 * @param y Y coordinate
 */
void ofVertex(float x, float y);

/**
 * Add a vertex to the current shape (3D).
 * Must be called between ofBeginShape() and ofEndShape().
 * @param x X coordinate
 * @param y Y coordinate
 * @param z Z coordinate
 */
void ofVertex(float x, float y, float z);

/**
 * Add a curve vertex to the current shape (2D).
 * Creates a Catmull-Rom curve through the vertices.
 * Must be called between ofBeginShape() and ofEndShape().
 * @param x X coordinate
 * @param y Y coordinate
 */
void ofCurveVertex(float x, float y);

/**
 * Add a curve vertex to the current shape (3D).
 * Creates a Catmull-Rom curve through the vertices.
 * Must be called between ofBeginShape() and ofEndShape().
 * @param x X coordinate
 * @param y Y coordinate
 * @param z Z coordinate
 */
void ofCurveVertex(float x, float y, float z);

/**
 * Add a Bezier control point to the current shape (2D).
 * Bezier vertices must be added in groups of 3:
 * - First two are control points
 * - Third is the end point
 * Must be called between ofBeginShape() and ofEndShape().
 * @param cx1 First control point X
 * @param cy1 First control point Y
 * @param cx2 Second control point X
 * @param cy2 Second control point Y
 * @param x End point X
 * @param y End point Y
 */
void ofBezierVertex(float cx1, float cy1, float cx2, float cy2, float x, float y);

/**
 * Add a Bezier control point to the current shape (3D).
 * Bezier vertices must be added in groups of 3:
 * - First two are control points
 * - Third is the end point
 * Must be called between ofBeginShape() and ofEndShape().
 * @param cx1 First control point X
 * @param cy1 First control point Y
 * @param cz1 First control point Z
 * @param cx2 Second control point X
 * @param cy2 Second control point Y
 * @param cz2 Second control point Z
 * @param x End point X
 * @param y End point Y
 * @param z End point Z
 */
void ofBezierVertex(float cx1, float cy1, float cz1, float cx2, float cy2, float cz2, float x, float y, float z);

/**
 * Start a new contour within the current shape.
 * This allows creating shapes with holes.
 * Must be called between ofBeginShape() and ofEndShape().
 *
 * Usage:
 * @code
 * ofBeginShape();
 *   // Outer contour
 *   ofVertex(0, 0);
 *   ofVertex(100, 0);
 *   ofVertex(100, 100);
 *   ofVertex(0, 100);
 *   ofNextContour();
 *   // Inner contour (hole)
 *   ofVertex(25, 25);
 *   ofVertex(75, 25);
 *   ofVertex(75, 75);
 *   ofVertex(25, 75);
 * ofEndShape(true);
 * @endcode
 */
void ofNextContour();

// ============================================================================
// Blend Mode
// ============================================================================

/**
 * Enable alpha blending with standard blend mode.
 * Equivalent to ofEnableBlendMode(OF_BLENDMODE_ALPHA).
 * This is the default mode for transparency.
 *
 * Blend equation: src.rgb * src.a + dst.rgb * (1 - src.a)
 */
void ofEnableAlphaBlending();

/**
 * Disable blending entirely.
 * New pixels will completely overwrite existing pixels.
 * Equivalent to ofEnableBlendMode(OF_BLENDMODE_DISABLED).
 */
void ofDisableAlphaBlending();

/**
 * Enable a specific blend mode.
 *
 * Available blend modes:
 * - OF_BLENDMODE_DISABLED (0): No blending
 * - OF_BLENDMODE_ALPHA (1): Standard alpha blending (default)
 * - OF_BLENDMODE_ADD (2): Additive blending (brightens)
 * - OF_BLENDMODE_SUBTRACT (3): Subtractive blending (darkens)
 * - OF_BLENDMODE_MULTIPLY (4): Multiply blending
 * - OF_BLENDMODE_SCREEN (5): Screen blending
 *
 * @param blendMode The blend mode to use (see OF_BLENDMODE_* constants)
 */
void ofEnableBlendMode(int blendMode);

// ============================================================================
// 3D Primitives
// ============================================================================

/**
 * Draw a 3D box at specified position.
 * @param x X position of box center
 * @param y Y position of box center
 * @param z Z position of box center
 * @param size Box size (width, height, depth all equal)
 */
void ofDrawBox(float x, float y, float z, float size);

/**
 * Draw a 3D box at specified position with custom dimensions.
 * @param x X position of box center
 * @param y Y position of box center
 * @param z Z position of box center
 * @param width Box width (X dimension)
 * @param height Box height (Y dimension)
 * @param depth Box depth (Z dimension)
 */
void ofDrawBox(float x, float y, float z, float width, float height, float depth);

/**
 * Draw a 3D box at origin (0,0,0).
 * @param size Box size (width, height, depth all equal)
 */
void ofDrawBox(float size);

/**
 * Draw a 3D sphere at specified position.
 * @param x X position of sphere center
 * @param y Y position of sphere center
 * @param z Z position of sphere center
 * @param radius Sphere radius
 */
void ofDrawSphere(float x, float y, float z, float radius);

/**
 * Draw a 3D sphere at origin (0,0,0).
 * @param radius Sphere radius
 */
void ofDrawSphere(float radius);

/**
 * Draw a 3D cone pointing up (+Y) at specified position.
 * @param x X position of cone base center
 * @param y Y position of cone base center
 * @param z Z position of cone base center
 * @param radius Radius of cone base
 * @param height Cone height along Y axis
 */
void ofDrawCone(float x, float y, float z, float radius, float height);

/**
 * Draw a 3D cone pointing up (+Y) at origin (0,0,0).
 * @param radius Radius of cone base
 * @param height Cone height along Y axis
 */
void ofDrawCone(float radius, float height);

/**
 * Draw a 3D cylinder along Y axis at specified position.
 * @param x X position of cylinder center
 * @param y Y position of cylinder center (bottom)
 * @param z Z position of cylinder center
 * @param radius Cylinder radius
 * @param height Cylinder height along Y axis
 */
void ofDrawCylinder(float x, float y, float z, float radius, float height);

/**
 * Draw a 3D cylinder along Y axis at origin (0,0,0).
 * @param radius Cylinder radius
 * @param height Cylinder height along Y axis
 */
void ofDrawCylinder(float radius, float height);

/**
 * Draw a 3D plane on XY plane at specified position.
 * @param x X position of plane center
 * @param y Y position of plane center
 * @param z Z position (depth)
 * @param width Plane width along X axis
 * @param height Plane height along Y axis
 */
void ofDrawPlane(float x, float y, float z, float width, float height);

/**
 * Draw a 3D plane on XY plane at origin (0,0,0).
 * @param width Plane width along X axis
 * @param height Plane height along Y axis
 */
void ofDrawPlane(float width, float height);

/**
 * Draw an icosphere (geodesic sphere) at specified position.
 * An icosphere is a sphere made of triangles with more uniform distribution
 * than a UV sphere.
 * @param x X position of icosphere center
 * @param y Y position of icosphere center
 * @param z Z position of icosphere center
 * @param radius Icosphere radius
 * @param subdivisions Number of subdivision iterations (default: 2)
 */
void ofDrawIcoSphere(float x, float y, float z, float radius, int subdivisions = 2);

/**
 * Draw an icosphere at origin (0,0,0).
 * @param radius Icosphere radius
 * @param subdivisions Number of subdivision iterations (default: 2)
 */
void ofDrawIcoSphere(float radius, int subdivisions = 2);

// ============================================================================
// 3D Helper Functions
// ============================================================================

/**
 * Draw 3D coordinate axes at origin.
 * Draws three colored lines representing X (red), Y (green), and Z (blue) axes.
 * @param size Length of each axis line (default: 1.0)
 */
void ofDrawAxis(float size = 1.0f);

/**
 * Draw a 3D grid on the XZ plane.
 * Grid extends from (-width/2, 0, -height/2) to (width/2, 0, height/2).
 * @param stepSize Spacing between grid lines
 * @param numberOfSteps Number of steps in each direction from center
 * @param labels If true, draw axis labels (default: false)
 * @param x If true, draw lines parallel to X axis (default: true)
 * @param y If true, draw horizontal center line (default: true)
 * @param z If true, draw lines parallel to Z axis (default: true)
 */
void ofDrawGrid(float stepSize = 1.0f, size_t numberOfSteps = 8, bool labels = false, bool x = true, bool y = true, bool z = true);

/**
 * Draw a 3D grid on the XZ plane (simplified version).
 * @param stepSize Spacing between grid lines
 * @param numberOfSteps Number of steps in each direction from center
 * @param labels If true, draw axis labels (default: false)
 */
void ofDrawGrid(float stepSize, size_t numberOfSteps, bool labels);

/**
 * Draw a 3D grid on the XZ plane at specified position.
 * @param x X position of grid center
 * @param y Y position of grid (height)
 * @param z Z position of grid center
 * @param stepSize Spacing between grid lines
 * @param numberOfSteps Number of steps in each direction from center
 * @param labels If true, draw axis labels (default: false)
 * @param xLines If true, draw lines parallel to X axis (default: true)
 * @param yLines If true, draw horizontal center line (default: true)
 * @param zLines If true, draw lines parallel to Z axis (default: true)
 */
void ofDrawGrid(float x, float y, float z, float stepSize, size_t numberOfSteps, bool labels = false, bool xLines = true, bool yLines = true, bool zLines = true);

/**
 * Draw a simple grid plane on XZ plane.
 * Convenience function for drawing a basic grid.
 * @param width Total width of grid (X dimension)
 * @param height Total depth of grid (Z dimension)
 * @param columns Number of columns (divisions along X)
 * @param rows Number of rows (divisions along Z)
 */
void ofDrawGridPlane(float width, float height, int columns, int rows);

/**
 * Draw a simple grid plane on XZ plane at origin.
 * Grid size is 1x1 with specified divisions.
 * @param columns Number of columns (divisions along X, default: 10)
 * @param rows Number of rows (divisions along Z, default: 10)
 */
void ofDrawGridPlane(int columns = 10, int rows = 10);

/**
 * Draw a 3D arrow from start to end position.
 * Arrow consists of a line with a cone at the end.
 * @param start Starting position of arrow
 * @param end Ending position of arrow (arrow head points here)
 * @param headSize Size of arrow head cone (default: 0.05)
 */
void ofDrawArrow(const oflike::ofVec3f& start, const oflike::ofVec3f& end, float headSize = 0.05f);

/**
 * Draw rotation axes showing current transformation orientation.
 * Draws three colored arrows (RGB for XYZ) at the specified position
 * showing the orientation of the current transformation matrix.
 * @param radius Length of each axis arrow
 * @param stripWidth Width of the axis lines (default: 0.01)
 */
void ofDrawRotationAxes(float radius, float stripWidth = 0.01f);

// ============================================================================
// Depth Testing & Culling
// ============================================================================

/**
 * Enable depth testing.
 * When enabled, fragments with greater depth values are discarded.
 * This is essential for correct 3D rendering when objects overlap.
 * Default state: disabled.
 */
void ofEnableDepthTest();

/**
 * Disable depth testing.
 * All fragments are drawn regardless of depth.
 * Useful for 2D rendering or special effects.
 */
void ofDisableDepthTest();

/**
 * Check if depth testing is enabled.
 * @return true if depth testing is enabled, false otherwise
 */
bool ofGetDepthTestEnabled();

/**
 * Enable or disable writing to the depth buffer.
 * When disabled, depth values are not updated even if depth testing is enabled.
 * Useful for rendering transparent objects or skyboxes.
 * Default state: enabled (when depth test is enabled).
 * @param enabled If true, depth values are written to the depth buffer
 */
void ofSetDepthWrite(bool enabled);

/**
 * Check if depth write is enabled.
 * @return true if depth write is enabled, false otherwise
 */
bool ofGetDepthWrite();

/**
 * Enable backface culling.
 * Triangles facing away from the camera are not rendered.
 * This improves performance and is commonly used for closed meshes.
 * Default state: disabled.
 */
void ofEnableCulling();

/**
 * Disable culling.
 * All triangles are rendered regardless of facing direction.
 * Useful for double-sided meshes or transparent objects.
 */
void ofDisableCulling();
