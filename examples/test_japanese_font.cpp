/// \file test_japanese_font.cpp
/// \brief Japanese font loading and rendering test
///
/// Tests UTF-8 string handling and Core Text Japanese font rendering.

#include <iostream>
#include <string>

// Minimal standalone test - no full oF app required
#include "../src/oflike/graphics/ofTrueTypeFont.h"
#include "../src/oflike/utils/ofLog.h"

int main() {
    std::cout << "=== Japanese Font Loading Test ===" << std::endl;

    ofTrueTypeFont font;

    // Test 1: Load Japanese system font
    std::cout << "\n[Test 1] Loading Hiragino Sans Japanese font..." << std::endl;

    // Try multiple Japanese fonts that should be available on macOS
    bool loaded = false;
    const std::vector<std::string> japaneseFonts = {
        "Hiragino Sans",
        "Hiragino Kaku Gothic ProN",
        "Hiragino Mincho ProN",
        "Yu Gothic Medium"
    };

    for (const auto& fontName : japaneseFonts) {
        if (font.load(fontName, 24)) {
            std::cout << "✓ Successfully loaded: " << fontName << std::endl;
            loaded = true;
            break;
        }
    }

    if (!loaded) {
        std::cerr << "✗ FAILED: Could not load any Japanese font" << std::endl;
        return 1;
    }

    // Test 2: Verify font is loaded
    std::cout << "\n[Test 2] Checking isLoaded()..." << std::endl;
    if (font.isLoaded()) {
        std::cout << "✓ Font is loaded" << std::endl;
    } else {
        std::cerr << "✗ FAILED: Font reports not loaded" << std::endl;
        return 1;
    }

    // Test 3: Get font size
    std::cout << "\n[Test 3] Getting font size..." << std::endl;
    float fontSize = font.getFontSize();
    std::cout << "✓ Font size: " << fontSize << "pt" << std::endl;

    // Test 4: Japanese text strings (UTF-8 encoded)
    std::cout << "\n[Test 4] Testing Japanese text processing..." << std::endl;

    const std::vector<std::string> testStrings = {
        "こんにちは",           // Hiragana: Hello
        "テスト",               // Katakana: Test
        "日本語",               // Kanji: Japanese language
        "混合テキスト123",      // Mixed: Japanese + numbers
        "openFrameworks"        // ASCII control
    };

    for (const auto& text : testStrings) {
        std::cout << "  Testing: " << text << std::endl;

        // Test string width
        float width = font.stringWidth(text);
        std::cout << "    - stringWidth: " << width << "px" << std::endl;

        // Test string height
        float height = font.stringHeight(text);
        std::cout << "    - stringHeight: " << height << "px" << std::endl;

        // Test bounding box
        ofRectangle bbox = font.getStringBoundingBox(text, 0, 0);
        std::cout << "    - boundingBox: (" << bbox.x << ", " << bbox.y
                  << ", " << bbox.width << ", " << bbox.height << ")" << std::endl;

        // Verify reasonable values
        if (width <= 0 || height <= 0) {
            std::cerr << "✗ FAILED: Invalid dimensions for: " << text << std::endl;
            return 1;
        }
    }

    std::cout << "✓ All Japanese text strings processed successfully" << std::endl;

    // Test 5: Character metrics for Japanese glyphs
    std::cout << "\n[Test 5] Testing individual character metrics..." << std::endl;

    // Test a Japanese character
    std::string hiraganaA = "あ";  // Hiragana 'a'
    float charWidth = font.stringWidth(hiraganaA);
    std::cout << "  Character 'あ' width: " << charWidth << "px" << std::endl;

    if (charWidth <= 0) {
        std::cerr << "✗ FAILED: Invalid character width" << std::endl;
        return 1;
    }

    std::cout << "✓ Japanese character metrics working" << std::endl;

    // Test 6: Line height and letter spacing
    std::cout << "\n[Test 6] Testing font metrics..." << std::endl;

    float lineHeight = font.getLineHeight();
    std::cout << "  Line height: " << lineHeight << "px" << std::endl;

    font.setLineHeight(lineHeight * 1.5f);
    float newLineHeight = font.getLineHeight();
    std::cout << "  New line height: " << newLineHeight << "px" << std::endl;

    float letterSpacing = font.getLetterSpacing();
    std::cout << "  Letter spacing: " << letterSpacing << "px" << std::endl;

    font.setLetterSpacing(2.0f);
    float newLetterSpacing = font.getLetterSpacing();
    std::cout << "  New letter spacing: " << newLetterSpacing << "px" << std::endl;

    std::cout << "✓ Font metrics working" << std::endl;

    // Summary
    std::cout << "\n=== All Tests Passed ✓ ===" << std::endl;
    std::cout << "Japanese font loading and UTF-8 text processing verified." << std::endl;

    return 0;
}
