/// \file main.cpp
/// \brief Example 03: Color Operations
///
/// Demonstrates comprehensive color manipulation capabilities:
/// - Creating colors with different methods (RGB, fromHex, fromHsb)
/// - HSB color space manipulation (hue, saturation, brightness)
/// - Color interpolation (lerp)
/// - Color operations (invert, clamp, arithmetic)
/// - Predefined color constants
/// - Color cycling and animation
///
/// This example showcases the full ofColor API for creative color control.

#include "../../src/core/AppBase.h"
#include "../../src/oflike/graphics/ofGraphics.h"
#include "../../src/oflike/types/ofColor.h"
#include "../../src/oflike/utils/ofUtils.h"
#include "../../src/core/Context.h"
#include <cmath>

using namespace oflike;

class ColorOperationsApp : public ofBaseApp {
public:
    // Animation parameters
    float hue = 0.0f;
    float time = 0.0f;
    float lerpAmount = 0.0f;
    bool lerpDirection = true;

    // Demo colors
    ofColor color1;
    ofColor color2;
    ofColor lerpedColor;

    void setup() override {
        ofSetFrameRate(60);

        // Initialize demo colors
        color1 = ofColor::fromHex(0xFF6B35);  // Orange
        color2 = ofColor::fromHex(0x004E89);  // Blue
    }

    void update() override {
        // Animate hue for HSB demonstration
        hue += 0.5f;
        if (hue >= 255.0f) hue = 0.0f;

        // Animate time for various effects
        time += 0.01f;

        // Animate lerp amount back and forth
        if (lerpDirection) {
            lerpAmount += 0.005f;
            if (lerpAmount >= 1.0f) {
                lerpAmount = 1.0f;
                lerpDirection = false;
            }
        } else {
            lerpAmount -= 0.005f;
            if (lerpAmount <= 0.0f) {
                lerpAmount = 0.0f;
                lerpDirection = true;
            }
        }

        // Calculate lerped color
        lerpedColor = color1.lerp(color2, lerpAmount);
    }

    void draw() override {
        ofBackground(20);

        float w = ofGetWidth();
        float h = ofGetHeight();
        float boxSize = 80.0f;
        float padding = 20.0f;
        float startY = 40.0f;

        // Title (placeholder - no text rendering yet)
        ofSetColor(255);
        ofDrawCircle(w/2, 20, 5);  // Title marker

        // ========================================
        // Row 1: Predefined Colors
        // ========================================
        float row1Y = startY;
        drawColorBox(padding, row1Y, boxSize, ofColor::white, "white");
        drawColorBox(padding + (boxSize + padding) * 1, row1Y, boxSize, ofColor::gray, "gray");
        drawColorBox(padding + (boxSize + padding) * 2, row1Y, boxSize, ofColor::black, "black");
        drawColorBox(padding + (boxSize + padding) * 3, row1Y, boxSize, ofColor::red, "red");
        drawColorBox(padding + (boxSize + padding) * 4, row1Y, boxSize, ofColor::green, "green");
        drawColorBox(padding + (boxSize + padding) * 5, row1Y, boxSize, ofColor::blue, "blue");
        drawColorBox(padding + (boxSize + padding) * 6, row1Y, boxSize, ofColor::cyan, "cyan");
        drawColorBox(padding + (boxSize + padding) * 7, row1Y, boxSize, ofColor::magenta, "magenta");
        drawColorBox(padding + (boxSize + padding) * 8, row1Y, boxSize, ofColor::yellow, "yellow");

        // ========================================
        // Row 2: HSB Color Cycling (Hue variation)
        // ========================================
        float row2Y = row1Y + boxSize + padding * 2;
        for (int i = 0; i < 9; i++) {
            float h = hue + i * 28.0f;  // Spread hues across spectrum
            if (h >= 255.0f) h -= 255.0f;
            ofColor hsbColor = ofColor::fromHsb(h, 255, 255);
            drawColorBox(padding + (boxSize + padding) * i, row2Y, boxSize, hsbColor, "HSB");
        }

        // ========================================
        // Row 3: Saturation Variation (same hue)
        // ========================================
        float row3Y = row2Y + boxSize + padding * 2;
        for (int i = 0; i < 9; i++) {
            float saturation = i * 32.0f;  // 0 to 255
            ofColor satColor = ofColor::fromHsb(hue, saturation, 255);
            drawColorBox(padding + (boxSize + padding) * i, row3Y, boxSize, satColor, "Sat");
        }

        // ========================================
        // Row 4: Brightness Variation (same hue, full saturation)
        // ========================================
        float row4Y = row3Y + boxSize + padding * 2;
        for (int i = 0; i < 9; i++) {
            float brightness = 32.0f + i * 28.0f;  // 32 to 255
            ofColor brightColor = ofColor::fromHsb(hue, 255, brightness);
            drawColorBox(padding + (boxSize + padding) * i, row4Y, boxSize, brightColor, "Brt");
        }

        // ========================================
        // Row 5: Hex Colors
        // ========================================
        float row5Y = row4Y + boxSize + padding * 2;
        drawColorBox(padding + (boxSize + padding) * 0, row5Y, boxSize, ofColor::fromHex(0xFF6B35), "Hex1");
        drawColorBox(padding + (boxSize + padding) * 1, row5Y, boxSize, ofColor::fromHex(0xF7931E), "Hex2");
        drawColorBox(padding + (boxSize + padding) * 2, row5Y, boxSize, ofColor::fromHex(0xFDC830), "Hex3");
        drawColorBox(padding + (boxSize + padding) * 3, row5Y, boxSize, ofColor::fromHex(0xC5D86D), "Hex4");
        drawColorBox(padding + (boxSize + padding) * 4, row5Y, boxSize, ofColor::fromHex(0x83D475), "Hex5");
        drawColorBox(padding + (boxSize + padding) * 5, row5Y, boxSize, ofColor::fromHex(0x00B4D8), "Hex6");
        drawColorBox(padding + (boxSize + padding) * 6, row5Y, boxSize, ofColor::fromHex(0x0077B6), "Hex7");
        drawColorBox(padding + (boxSize + padding) * 7, row5Y, boxSize, ofColor::fromHex(0x004E89), "Hex8");
        drawColorBox(padding + (boxSize + padding) * 8, row5Y, boxSize, ofColor::fromHex(0x6A4C93), "Hex9");

        // ========================================
        // Row 6: Color Lerp (Interpolation)
        // ========================================
        float row6Y = row5Y + boxSize + padding * 2;

        // Show source colors and lerp result
        drawColorBox(padding, row6Y, boxSize, color1, "Src1");
        drawColorBox(padding + (boxSize + padding) * 1, row6Y, boxSize, color2, "Src2");

        // Draw lerped color with animated interpolation
        drawColorBox(padding + (boxSize + padding) * 2, row6Y, boxSize, lerpedColor, "Lerp");

        // Show lerp progression as gradient
        for (int i = 0; i < 6; i++) {
            float t = i / 5.0f;
            ofColor gradColor = color1.lerp(color2, t);
            drawColorBox(padding + (boxSize + padding) * (3 + i), row6Y, boxSize, gradColor, "");
        }

        // ========================================
        // Row 7: Color Operations
        // ========================================
        float row7Y = row6Y + boxSize + padding * 2;

        // Base color for operations
        ofColor baseColor = ofColor::fromHsb(hue, 200, 230);
        drawColorBox(padding, row7Y, boxSize, baseColor, "Base");

        // Inverted color
        ofColor inverted = baseColor.getInverted();
        drawColorBox(padding + (boxSize + padding) * 1, row7Y, boxSize, inverted, "Inv");

        // Brightness operations
        ofColor darker = baseColor * 0.5f;
        drawColorBox(padding + (boxSize + padding) * 2, row7Y, boxSize, darker, "*0.5");

        ofColor brighter = baseColor * 1.5f;
        ofColor brighterClamped = brighter.getClamped();
        drawColorBox(padding + (boxSize + padding) * 3, row7Y, boxSize, brighterClamped, "*1.5");

        // Color addition
        ofColor addColor = ofColor::fromHsb(hue + 128, 150, 180);
        drawColorBox(padding + (boxSize + padding) * 4, row7Y, boxSize, addColor, "Add1");

        ofColor sumColor = (baseColor + addColor).getClamped();
        drawColorBox(padding + (boxSize + padding) * 5, row7Y, boxSize, sumColor, "Sum");

        // Color multiplication
        ofColor mulColor1 = ofColor::fromHsb(hue, 255, 255);
        ofColor mulColor2 = ofColor::fromHsb(hue + 128, 255, 255);
        drawColorBox(padding + (boxSize + padding) * 6, row7Y, boxSize, mulColor1, "Mul1");
        drawColorBox(padding + (boxSize + padding) * 7, row7Y, boxSize, mulColor2, "Mul2");

        ofColor multiplied = mulColor1 * mulColor2;
        drawColorBox(padding + (boxSize + padding) * 8, row7Y, boxSize, multiplied, "Prod");

        // ========================================
        // Bottom: Animated Color Wave
        // ========================================
        float waveY = row7Y + boxSize + padding * 2;
        int waveCount = static_cast<int>(w / 4);

        ofFill();
        for (int i = 0; i < waveCount; i++) {
            float x = i * 4.0f;
            float colorHue = fmod(hue + i * 2.0f + time * 50.0f, 255.0f);
            float brightness = 128.0f + 127.0f * sin(i * 0.1f + time * 2.0f);

            ofColor waveColor = ofColor::fromHsb(colorHue, 255, brightness);
            ofSetColor(waveColor.r, waveColor.g, waveColor.b);

            float barHeight = 40.0f + 30.0f * sin(i * 0.15f + time * 3.0f);
            ofDrawRectangle(x, waveY, 4, barHeight);
        }

        // FPS indicator
        ofSetColor(255);
        float fps = ofGetFrameRate();
        drawFPSIndicator(w - 80, h - 30, fps);
    }

    void keyPressed(int key) override {
        if (key == ' ') {
            // Reset animation
            hue = 0.0f;
            time = 0.0f;
            lerpAmount = 0.0f;
            lerpDirection = true;
        }
    }

    void windowResized(int w, int h) override {
        // Layout adjusts automatically based on ofGetWidth/Height
    }

private:
    /// \brief Draw a color box with optional label marker
    void drawColorBox(float x, float y, float size, const ofColor& color, const char* label) {
        // Draw filled box with color
        ofFill();
        ofSetColor(color.r, color.g, color.b);
        ofDrawRectangle(x, y, size, size);

        // Draw white border
        ofNoFill();
        ofSetColor(255);
        ofSetLineWidth(1.0f);
        ofDrawRectangle(x, y, size, size);

        // Draw label marker (small circle at top-left)
        if (label && label[0] != '\0') {
            ofFill();
            ofSetColor(255);
            ofDrawCircle(x + 5, y + 5, 2);
        }
    }

    /// \brief Draw simple FPS indicator using shapes
    void drawFPSIndicator(float x, float y, float fps) {
        // Draw FPS as a bar graph (60fps = full bar)
        float barWidth = 60.0f;
        float barHeight = 10.0f;
        float fillAmount = (fps / 60.0f) * barWidth;

        // Background
        ofNoFill();
        ofSetColor(100);
        ofSetLineWidth(1.0f);
        ofDrawRectangle(x, y, barWidth, barHeight);

        // Fill based on FPS
        ofFill();
        if (fps >= 55.0f) {
            ofSetColor(0, 255, 0);  // Green: good FPS
        } else if (fps >= 30.0f) {
            ofSetColor(255, 255, 0);  // Yellow: moderate FPS
        } else {
            ofSetColor(255, 0, 0);  // Red: low FPS
        }
        ofDrawRectangle(x, y, fillAmount, barHeight);
    }
};

// ============================================================================
// Integration Instructions
// ============================================================================
// To run this example:
// 1. Build the oflike-metal library
// 2. In src/platform/bridge/SwiftBridge.mm, replace TestApp with:
//    #include "../../../examples/03_color/main.cpp"
//    return new ColorOperationsApp();
// 3. Rebuild and run
//
// This demonstrates the comprehensive ofColor API:
// - Predefined colors (red, green, blue, etc.)
// - HSB color space (hue cycling, saturation, brightness)
// - Hex color creation
// - Color interpolation (lerp)
// - Color operations (invert, multiply, add, clamp)
// - Animated color effects
//
// Key Concepts:
// - ofColor stores RGBA as 0-255 uint8_t values
// - fromHsb() creates colors from Hue/Saturation/Brightness
// - fromHex() creates colors from hex values (0xRRGGBB)
// - lerp() interpolates between two colors
// - Color arithmetic: +, -, *, / operators
// - getClamped() clamps values to valid range
// - getInverted() creates complementary colors
//
// openFrameworks API Compatibility: Level A
// ============================================================================
