/// \file main.cpp
/// \brief Example 17: SVG Loader
///
/// Demonstrates ofxSvg addon:
/// - Load SVG file
/// - Access individual paths
/// - Draw SVG content
/// - Display SVG dimensions
/// - Mouse interaction for scale/position
///
/// This example shows how to load and render SVG files
/// using the ofxSvg addon powered by nanosvg.

#include "../../src/core/AppBase.h"
#include "../../src/oflike/graphics/ofGraphics.h"
#include "../../src/oflike/types/ofColor.h"
#include "../../src/oflike/utils/ofUtils.h"
#include "../../src/oflike/utils/ofLog.h"
#include "../../src/core/Context.h"
#include "../../src/addons/core/ofxSvg/ofxSvg.h"

class SvgLoaderApp : public ofBaseApp {
public:
    ofxSvg svg;
    float scale = 1.0f;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    bool dragging = false;
    float lastMouseX = 0.0f;
    float lastMouseY = 0.0f;

    void setup() override {
        // Set frame rate
        ofSetFrameRate(60);

        // Set background color
        ofSetBackgroundColor(240);

        // Create a simple SVG programmatically since we don't have an SVG file yet
        std::string svgContent = R"(
<svg width="400" height="400" xmlns="http://www.w3.org/2000/svg">
  <!-- Red circle -->
  <circle cx="100" cy="100" r="50" fill="#ff0000" stroke="#000000" stroke-width="2"/>

  <!-- Blue rectangle -->
  <rect x="200" y="50" width="100" height="100" fill="#0000ff" stroke="#000000" stroke-width="2"/>

  <!-- Green triangle (path) -->
  <path d="M 100 250 L 150 350 L 50 350 Z" fill="#00ff00" stroke="#000000" stroke-width="2"/>

  <!-- Orange star (complex path) -->
  <path d="M 300 250 L 315 290 L 360 295 L 325 325 L 335 370 L 300 345 L 265 370 L 275 325 L 240 295 L 285 290 Z"
        fill="#ff8800" stroke="#000000" stroke-width="1"/>

  <!-- Purple bezier curve -->
  <path d="M 50 200 Q 100 150 150 200" fill="none" stroke="#ff00ff" stroke-width="3"/>
</svg>
)";

        // Load SVG from string
        if (svg.loadFromString(svgContent)) {
            oflike::ofLogNotice("SvgLoaderApp") << "SVG loaded successfully!";
            oflike::ofLogNotice("SvgLoaderApp") << "Size: " << svg.getWidth() << " x " << svg.getHeight();
            oflike::ofLogNotice("SvgLoaderApp") << "Number of paths: " << svg.getNumPaths();

            // Center the SVG
            offsetX = (ofGetWidth() - svg.getWidth()) * 0.5f;
            offsetY = (ofGetHeight() - svg.getHeight()) * 0.5f;
        } else {
            oflike::ofLogError("SvgLoaderApp") << "Failed to load SVG";
        }

        // You can also load from a file:
        // svg.load("path/to/your/file.svg");
    }

    void update() override {
        // Animation could go here
    }

    void draw() override {
        // Clear background
        ofClear(240, 240, 240);

        // Apply transformations
        ofPushMatrix();
        ofTranslate(offsetX, offsetY);
        ofScale(scale, scale);

        // Draw SVG
        if (svg.isLoaded()) {
            svg.draw();
        }

        ofPopMatrix();

        // Draw origin marker (red dot)
        ofSetColor(255, 0, 0);
        ofDrawCircle(offsetX, offsetY, 5);

        // Draw UI overlay
        ofSetColor(50, 50, 50, 200);
        ofDrawRectangle(10, 10, 300, 140);

        // Note: ofDrawBitmapString not yet implemented
        // For now, see console logs for SVG info
        // Controls are: Mouse wheel = zoom, Mouse drag = pan, R = reset
    }

    void keyPressed(int key) override {
        if (key == 'r' || key == 'R') {
            // Reset view
            scale = 1.0f;
            offsetX = (ofGetWidth() - svg.getWidth()) * 0.5f;
            offsetY = (ofGetHeight() - svg.getHeight()) * 0.5f;
            oflike::ofLogNotice("SvgLoaderApp") << "View reset";
        }
    }

    void mouseDragged(int x, int y, int button) override {
        if (dragging) {
            float dx = x - lastMouseX;
            float dy = y - lastMouseY;
            offsetX += dx;
            offsetY += dy;
            lastMouseX = x;
            lastMouseY = y;
        }
    }

    void mousePressed(int x, int y, int button) override {
        dragging = true;
        lastMouseX = x;
        lastMouseY = y;
    }

    void mouseReleased(int x, int y, int button) override {
        dragging = false;
    }

    void mouseScrolled(int x, int y, float scrollX, float scrollY) override {
        (void)x; (void)y; (void)scrollX; // Unused parameters

        // Zoom in/out
        float zoomSpeed = 0.1f;
        if (scrollY > 0) {
            scale *= (1.0f + zoomSpeed);
        } else if (scrollY < 0) {
            scale *= (1.0f - zoomSpeed);
        }

        // Clamp scale
        if (scale < 0.1f) scale = 0.1f;
        if (scale > 10.0f) scale = 10.0f;
    }
};

int main() {
    auto app = std::make_shared<SvgLoaderApp>();
    Context::getInstance().run(app, 1024, 768, "SVG Loader Example");
    return 0;
}
