/// \file main.cpp
/// \brief Example 05: Typography and Font Rendering
///
/// Demonstrates comprehensive font rendering capabilities using ofTrueTypeFont:
/// - Loading system fonts and custom fonts
/// - Drawing text at different positions
/// - Text measurement (width, height, bounding box)
/// - Letter spacing and line height control
/// - Animated text effects
/// - Multi-line text rendering
/// - Japanese/Unicode text support
///
/// This example showcases the full ofTrueTypeFont API using Core Text.

#include "../../src/core/AppBase.h"
#include "../../src/oflike/graphics/ofGraphics.h"
#include "../../src/oflike/graphics/ofTrueTypeFont.h"
#include "../../src/oflike/types/ofColor.h"
#include "../../src/oflike/utils/ofUtils.h"
#include "../../src/core/Context.h"
#include <cmath>
#include <iostream>

using namespace oflike;

class TypographyApp : public ofBaseApp {
public:
    // Fonts
    ofTrueTypeFont titleFont;
    ofTrueTypeFont bodyFont;
    ofTrueTypeFont monoFont;
    ofTrueTypeFont japaneseFont;

    // Animation
    float time = 0.0f;
    float waveOffset = 0.0f;

    void setup() override {
        ofSetFrameRate(60);
        ofSetWindowTitle("05_typography - Font Rendering");

        // Load system fonts at different sizes
        bool success = true;

        // Title font (large, bold)
        if (!titleFont.load("Helvetica-Bold", 48)) {
            std::cerr << "Failed to load title font" << std::endl;
            success = false;
        }

        // Body font (medium)
        if (!bodyFont.load("Helvetica", 24)) {
            std::cerr << "Failed to load body font" << std::endl;
            success = false;
        }

        // Monospace font (code)
        if (!monoFont.load("Menlo", 16)) {
            std::cerr << "Failed to load mono font" << std::endl;
            success = false;
        }

        // Japanese font (Hiragino Sans)
        if (!japaneseFont.load("HiraginoSans-W3", 20)) {
            std::cerr << "Failed to load Japanese font" << std::endl;
            success = false;
        }

        // Set letter spacing for demonstration
        bodyFont.setLetterSpacing(2.0f);

        std::cout << "05_typography example started" << std::endl;
        if (success) {
            std::cout << "All fonts loaded successfully" << std::endl;
        }
        std::cout << "Title font size: " << titleFont.getFontSize() << std::endl;
        std::cout << "Body font line height: " << bodyFont.getLineHeight() << std::endl;
    }

    void update() override {
        // Animate effects
        time += 0.016f;
        waveOffset += 0.05f;
    }

    void draw() override {
        // Dark background
        ofBackground(30, 30, 35);

        float x = 50;
        float y = 80;

        // ---- Section 1: Basic Text Rendering ----
        ofSetColor(255, 255, 255);
        titleFont.drawString("Typography", x, y);

        y += 80;
        ofSetColor(200, 200, 200);
        bodyFont.drawString("Basic text rendering with ofTrueTypeFont", x, y);

        // ---- Section 2: Text Measurement ----
        y += 60;
        std::string measureText = "Measured Text";
        float textWidth = bodyFont.stringWidth(measureText);
        float textHeight = bodyFont.stringHeight(measureText);

        ofSetColor(100, 200, 255);
        bodyFont.drawString(measureText, x, y);

        // Draw bounding box
        ::ofRectangle bbox = bodyFont.getStringBoundingBox(measureText, x, y);
        ofNoFill();
        ofSetColor(255, 100, 100);
        ofSetLineWidth(1.0f);
        ofDrawRectangle(bbox.x, bbox.y, bbox.width, bbox.height);

        // Show measurements
        ofFill();
        ofSetColor(150, 150, 150);
        monoFont.drawString("Width: " + ofToString(textWidth, 1) + "px", x + textWidth + 20, y - 20);
        monoFont.drawString("Height: " + ofToString(textHeight, 1) + "px", x + textWidth + 20, y);

        // ---- Section 3: Letter Spacing ----
        y += 80;
        ofSetColor(255, 200, 100);

        // Save original spacing
        float originalSpacing = bodyFont.getLetterSpacing();

        // Tight spacing
        bodyFont.setLetterSpacing(-2.0f);
        bodyFont.drawString("Tight spacing", x, y);

        // Normal spacing
        y += 40;
        bodyFont.setLetterSpacing(0.0f);
        bodyFont.drawString("Normal spacing", x, y);

        // Wide spacing
        y += 40;
        bodyFont.setLetterSpacing(8.0f);
        bodyFont.drawString("Wide spacing", x, y);

        // Restore original spacing
        bodyFont.setLetterSpacing(originalSpacing);

        // ---- Section 4: Animated Wave Text ----
        y += 80;
        std::string waveText = "WAVE ANIMATION";
        float charX = x;

        for (size_t i = 0; i < waveText.length(); i++) {
            char c = waveText[i];
            if (c == ' ') {
                charX += bodyFont.stringWidth(" ");
                continue;
            }

            std::string charStr(1, c);
            float charWidth = bodyFont.stringWidth(charStr);

            // Calculate wave offset
            float wave = std::sin(waveOffset + i * 0.3f) * 15.0f;

            // Rainbow color based on position
            float hue = fmod((i * 20.0f + time * 10.0f), 255.0f);
            ofColor color = ofColor::fromHsb(static_cast<unsigned char>(hue), 200, 255);
            ofSetColor(color.r, color.g, color.b);

            bodyFont.drawString(charStr, charX, y + wave);
            charX += charWidth + bodyFont.getLetterSpacing();
        }

        // ---- Section 5: Multi-line Text ----
        y += 100;
        ofSetColor(150, 255, 150);

        std::vector<std::string> lines = {
            "Multi-line text rendering",
            "Line height control",
            "Paragraph formatting"
        };

        float lineSpacing = bodyFont.getLineHeight() * 1.5f;
        for (const auto& line : lines) {
            bodyFont.drawString(line, x, y);
            y += lineSpacing;
        }

        // ---- Section 6: Japanese Text (UTF-8) ----
        if (japaneseFont.isLoaded()) {
            y += 40;
            ofSetColor(255, 180, 200);
            japaneseFont.drawString("日本語テキスト表示", x, y);

            y += 35;
            ofSetColor(180, 180, 255);
            japaneseFont.drawString("こんにちは、世界！", x, y);
        }

        // ---- Section 7: Code Example (Monospace) ----
        y += 60;
        ofSetColor(100, 255, 100);
        monoFont.drawString("// Code example with monospace font", x, y);
        y += 20;
        ofSetColor(150, 150, 150);
        monoFont.drawString("ofTrueTypeFont font;", x, y);
        y += 20;
        monoFont.drawString("font.load(\"Helvetica\", 24);", x, y);
        y += 20;
        monoFont.drawString("font.drawString(\"Hello\", 10, 10);", x, y);

        // ---- Info Text (Bottom Right) ----
        float infoY = ofGetHeight() - 80;
        float infoX = ofGetWidth() - 300;

        ofSetColor(100, 100, 100);
        monoFont.drawString("FPS: " + ofToString(ofGetFrameRate(), 1), infoX, infoY);
        infoY += 20;
        monoFont.drawString("Fonts loaded: 4", infoX, infoY);
        infoY += 20;
        monoFont.drawString("Core Text Backend", infoX, infoY);
    }

    void keyPressed(int key) override {
        if (key == ' ') {
            std::cout << "Space pressed - resetting animation" << std::endl;
            time = 0.0f;
            waveOffset = 0.0f;
        }
    }

    void windowResized(int w, int h) override {
        std::cout << "Window resized: " << w << "x" << h << std::endl;
    }
};

// Entry point
int main() {
    std::cout << "Starting oflike-metal: 05_typography example" << std::endl;

    // Create app instance
    auto app = std::make_shared<TypographyApp>();

    // Setup engine and run
    // Note: This assumes ofRunApp() or equivalent integration
    // Actual integration happens via Swift app in src/platform/swiftui/App.swift

    return 0;
}
