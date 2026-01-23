/// \file main.cpp
/// \brief Example 02: Shape Drawing
///
/// Demonstrates comprehensive shape drawing capabilities:
/// - Multiple primitive shapes (circle, rectangle, triangle, ellipse, line)
/// - Fill vs stroke modes (ofFill/ofNoFill)
/// - Line width control (ofSetLineWidth)
/// - Polygon drawing with ofPath
/// - Interactive color and mode switching
///
/// This example showcases the variety of 2D drawing primitives
/// available in oflike-metal.

#include "../../src/core/AppBase.h"
#include "../../src/oflike/graphics/ofGraphics.h"
#include "../../src/oflike/graphics/ofPath.h"
#include "../../src/oflike/types/ofColor.h"
#include "../../src/oflike/utils/ofUtils.h"
#include "../../src/core/Context.h"
#include <cmath>

using namespace oflike;

class ShapeDrawingApp : public ofBaseApp {
public:
    bool fillEnabled = true;
    float lineWidth = 2.0f;
    float hue = 0.0f;
    float rotation = 0.0f;
    int shapeIndex = 0;

    void setup() override {
        ofSetFrameRate(60);
    }

    void update() override {
        // Slowly cycle through colors
        hue += 0.5f;
        if (hue > 255.0f) hue = 0.0f;

        // Rotate shapes
        rotation += 0.5f;
        if (rotation > 360.0f) rotation -= 360.0f;
    }

    void draw() override {
        ofBackground(30);

        // Draw instructions
        ofSetColor(255);
        // Note: drawString not yet implemented, would draw here

        // Draw shapes in a grid layout
        float gridX = ofGetWidth() / 6.0f;
        float gridY = ofGetHeight() / 4.0f;

        // Set fill/stroke mode
        if (fillEnabled) {
            ofFill();
        } else {
            ofNoFill();
        }
        ofSetLineWidth(lineWidth);

        // Row 1: Basic shapes
        // Circle
        ofColor c1 = ofColor::fromHsb(hue, 200, 255);
        ofSetColor(c1.r, c1.g, c1.b);
        ofDrawCircle(gridX * 1, gridY * 1, 60);

        // Rectangle
        ofColor c2 = ofColor::fromHsb(fmod(hue + 40, 255), 200, 255);
        ofSetColor(c2.r, c2.g, c2.b);
        ofDrawRectangle(gridX * 2 - 60, gridY * 1 - 40, 120, 80);

        // Triangle
        ofColor c3 = ofColor::fromHsb(fmod(hue + 80, 255), 200, 255);
        ofSetColor(c3.r, c3.g, c3.b);
        ofDrawTriangle(
            gridX * 3, gridY * 1 - 60,
            gridX * 3 - 50, gridY * 1 + 40,
            gridX * 3 + 50, gridY * 1 + 40
        );

        // Ellipse
        ofColor c4 = ofColor::fromHsb(fmod(hue + 120, 255), 200, 255);
        ofSetColor(c4.r, c4.g, c4.b);
        ofDrawEllipse(gridX * 4, gridY * 1, 80, 50);

        // Rounded Rectangle
        ofColor c5 = ofColor::fromHsb(fmod(hue + 160, 255), 200, 255);
        ofSetColor(c5.r, c5.g, c5.b);
        ofDrawRectRounded(gridX * 5 - 60, gridY * 1 - 40, 120, 80, 20);

        // Row 2: Lines and polylines
        ofColor c6 = ofColor::fromHsb(fmod(hue + 200, 255), 200, 255);
        ofSetColor(c6.r, c6.g, c6.b);

        // Single line
        ofDrawLine(gridX * 1 - 40, gridY * 2 - 30, gridX * 1 + 40, gridY * 2 + 30);
        ofDrawLine(gridX * 1 - 40, gridY * 2 + 30, gridX * 1 + 40, gridY * 2 - 30);

        // Multi-segment polyline
        ofColor c7 = ofColor::fromHsb(fmod(hue + 240, 255), 200, 255);
        ofSetColor(c7.r, c7.g, c7.b);
        // Note: ofDrawPolyline would be used here
        // Drawing as connected lines for now
        float px = gridX * 2;
        float py = gridY * 2;
        for (int i = 0; i < 6; i++) {
            float x1 = px + cos(rotation * 0.017453f + i * 1.047f) * 50;
            float y1 = py + sin(rotation * 0.017453f + i * 1.047f) * 50;
            float x2 = px + cos(rotation * 0.017453f + (i+1) * 1.047f) * 50;
            float y2 = py + sin(rotation * 0.017453f + (i+1) * 1.047f) * 50;
            ofDrawLine(x1, y1, x2, y2);
        }

        // Row 3: ofPath demonstration
        ofColor c8 = ofColor::fromHsb(fmod(hue + 20, 255), 200, 255);
        ofSetColor(c8.r, c8.g, c8.b);

        // Star shape using ofPath
        ofPath starPath;
        float cx = gridX * 3;
        float cy = gridY * 2;
        float outerRadius = 60;
        float innerRadius = 25;

        starPath.moveTo(cx + outerRadius, cy);
        for (int i = 0; i < 10; i++) {
            float angle = i * 36.0f * 0.017453f; // degrees to radians
            float radius = (i % 2 == 0) ? outerRadius : innerRadius;
            float x = cx + cos(angle) * radius;
            float y = cy + sin(angle) * radius;
            starPath.lineTo(x, y);
        }
        starPath.close();

        if (fillEnabled) {
            starPath.setFilled(true);
            starPath.setStrokeWidth(0);
        } else {
            starPath.setFilled(false);
            starPath.setStrokeWidth(lineWidth);
        }
        ofColor c9 = ofColor::fromHsb(fmod(hue + 60, 255), 200, 255);
        starPath.setColor(c9.r, c9.g, c9.b);
        starPath.draw();

        // Heart shape using ofPath
        ofPath heartPath;
        cx = gridX * 4;
        cy = gridY * 2;
        float size = 50;

        heartPath.moveTo(cx, cy + size * 0.3f, 0);

        // Left curve
        heartPath.bezierTo(
            cx - size * 0.5f, cy - size * 0.3f, 0,
            cx - size, cy - size * 0.1f, 0,
            cx - size * 0.5f, cy + size * 0.5f, 0
        );

        // Bottom point
        heartPath.lineTo(cx, cy + size, 0);

        // Right curve
        heartPath.lineTo(cx + size * 0.5f, cy + size * 0.5f, 0);
        heartPath.bezierTo(
            cx + size, cy - size * 0.1f, 0,
            cx + size * 0.5f, cy - size * 0.3f, 0,
            cx, cy + size * 0.3f, 0
        );

        heartPath.close();

        if (fillEnabled) {
            heartPath.setFilled(true);
            heartPath.setStrokeWidth(0);
        } else {
            heartPath.setFilled(false);
            heartPath.setStrokeWidth(lineWidth);
        }
        ofColor c10 = ofColor::fromHsb(fmod(hue + 100, 255), 200, 255);
        heartPath.setColor(c10.r, c10.g, c10.b);
        heartPath.draw();

        // Hexagon using ofPath
        ofPath hexPath;
        cx = gridX * 5;
        cy = gridY * 2;
        float hexRadius = 60;

        for (int i = 0; i < 6; i++) {
            float angle = i * 60.0f * 0.017453f;
            float x = cx + cos(angle) * hexRadius;
            float y = cy + sin(angle) * hexRadius;
            if (i == 0) {
                hexPath.moveTo(x, y, 0);
            } else {
                hexPath.lineTo(x, y, 0);
            }
        }
        hexPath.close();

        if (fillEnabled) {
            hexPath.setFilled(true);
            hexPath.setStrokeWidth(0);
        } else {
            hexPath.setFilled(false);
            hexPath.setStrokeWidth(lineWidth);
        }
        ofColor c11 = ofColor::fromHsb(fmod(hue + 140, 255), 200, 255);
        hexPath.setColor(c11.r, c11.g, c11.b);
        hexPath.draw();

        // Row 3: More complex shapes
        // Bezier curve demonstration
        ofSetColor(255, 255, 255, 100);
        ofDrawRectangle(gridX * 1 - 70, gridY * 3 - 50, 140, 100);

        ofColor c12 = ofColor::fromHsb(fmod(hue + 180, 255), 200, 255);
        ofSetColor(c12.r, c12.g, c12.b);
        ofPath bezierPath;
        bezierPath.moveTo(gridX * 1 - 60, gridY * 3, 0);
        bezierPath.bezierTo(
            gridX * 1 - 20, gridY * 3 - 40, 0,
            gridX * 1 + 20, gridY * 3 + 40, 0,
            gridX * 1 + 60, gridY * 3, 0
        );
        bezierPath.setFilled(false);
        bezierPath.setStrokeWidth(lineWidth);
        ofColor c13 = ofColor::fromHsb(fmod(hue + 180, 255), 200, 255);
        bezierPath.setColor(c13.r, c13.g, c13.b);
        bezierPath.draw();

        // Arc demonstration
        ofColor c14 = ofColor::fromHsb(fmod(hue + 220, 255), 200, 255);
        ofSetColor(c14.r, c14.g, c14.b);
        ofPath arcPath;
        arcPath.arc(gridX * 2, gridY * 3, 60, 45, 315);
        arcPath.setFilled(false);
        arcPath.setStrokeWidth(lineWidth);
        ofColor c15 = ofColor::fromHsb(fmod(hue + 220, 255), 200, 255);
        arcPath.setColor(c15.r, c15.g, c15.b);
        arcPath.draw();

        // FPS counter
        ofSetColor(255);
        // Note: drawString not yet implemented
        // Would show FPS and instructions here
    }

    void keyPressed(int key) override {
        if (key == ' ') {
            // Toggle fill mode
            fillEnabled = !fillEnabled;
        } else if (key == '+' || key == '=') {
            // Increase line width
            lineWidth += 1.0f;
            if (lineWidth > 10.0f) lineWidth = 10.0f;
        } else if (key == '-' || key == '_') {
            // Decrease line width
            lineWidth -= 1.0f;
            if (lineWidth < 1.0f) lineWidth = 1.0f;
        }
    }

    void windowResized(int w, int h) override {
        // Shapes will automatically reposition based on window size
    }
};

// To integrate this into oflike-metal:
// 1. Replace TestApp in src/platform/bridge/SwiftBridge.mm with ShapeDrawingApp
// 2. Ensure SwiftBridge.mm includes this file
// 3. Build and run the Xcode project

int main() {
    ShapeDrawingApp app;
    app.setup();
    return 0;
}
