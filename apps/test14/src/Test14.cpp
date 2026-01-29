#include "Test14.h"
#include <oflike/graphics/ofGraphics.h>
#include <cmath>

void Test14::setup() {
    ofSetWindowTitle("test14 - Image API Test (1-3: modes, R: reload)");

    // Load system font
    font.load("Helvetica", 18.0f);

    // Create a generated image using ofPixels
    oflike::ofPixels pixels;
    pixels.allocate(256, 256, oflike::OF_IMAGE_COLOR_ALPHA);

    // Generate a gradient pattern
    for (size_t y = 0; y < 256; y++) {
        for (size_t x = 0; x < 256; x++) {
            float u = x / 255.0f;
            float v = y / 255.0f;

            // Create a colorful pattern
            uint8_t r = static_cast<uint8_t>(u * 255);
            uint8_t g = static_cast<uint8_t>(v * 255);
            uint8_t b = static_cast<uint8_t>((1.0f - u) * 255);
            uint8_t a = 255;

            // Add circular pattern
            float cx = (u - 0.5f) * 2.0f;
            float cy = (v - 0.5f) * 2.0f;
            float dist = std::sqrt(cx * cx + cy * cy);
            if (dist < 0.8f) {
                float wave = (std::sin(dist * 20.0f) + 1.0f) * 0.5f;
                r = static_cast<uint8_t>(r * wave + 128 * (1.0f - wave));
                g = static_cast<uint8_t>(g * wave + 64 * (1.0f - wave));
            }

            size_t idx = pixels.getPixelIndex(x, y);
            pixels[idx] = r;
            pixels[idx + 1] = g;
            pixels[idx + 2] = b;
            pixels[idx + 3] = a;
        }
    }

    generatedImage.setFromPixels(pixels);
    printf("Generated image: %dx%d\n", (int)generatedImage.getWidth(), (int)generatedImage.getHeight());

    // Try to load an image from data folder
    // First check if there's a test image
    if (testImage.load("data/test.png")) {
        imageLoaded = true;
        printf("Loaded test.png: %dx%d\n", (int)testImage.getWidth(), (int)testImage.getHeight());
    } else {
        printf("No test.png found in data folder\n");
    }
}

void Test14::update() {
}

void Test14::draw() {
    ofBackground(40, 40, 50);

    float time = ofGetElapsedTimef();

    // Draw title
    ofSetColor(255);
    if (font.isLoaded()) {
        font.drawString("ofImage / ofPixels / ofTexture Test", 20, 40);
    }

    // Mode info
    std::string modeStr;
    switch (testMode) {
        case 0: modeStr = "Mode 1: Generated Image (ofPixels -> ofImage)"; break;
        case 1: modeStr = "Mode 2: Loaded Image (file -> ofImage)"; break;
        case 2: modeStr = "Mode 3: Image Manipulation"; break;
    }

    ofSetColor(200, 200, 100);
    if (font.isLoaded()) {
        font.drawString(modeStr, 20, 70);
    }

    float startY = 100;

    if (testMode == 0) {
        // === Mode 0: Show generated image ===
        ofSetColor(255);
        if (generatedImage.isAllocated()) {
            // Original
            generatedImage.draw(50, startY);
            ofSetColor(180);
            if (font.isLoaded()) font.drawString("Original 256x256", 50, startY + 280);

            // Scaled up
            ofSetColor(255);
            generatedImage.draw(350, startY, 128, 128);
            ofSetColor(180);
            if (font.isLoaded()) font.drawString("Scaled 128x128", 350, startY + 150);

            // Scaled down
            ofSetColor(255);
            generatedImage.draw(520, startY, 64, 64);
            ofSetColor(180);
            if (font.isLoaded()) font.drawString("Scaled 64x64", 520, startY + 90);

            // Tinted
            ofSetColor(255, 128, 128);
            generatedImage.draw(50, startY + 300, 200, 200);
            ofSetColor(180);
            if (font.isLoaded()) font.drawString("Red Tint", 50, startY + 520);

            ofSetColor(128, 255, 128);
            generatedImage.draw(280, startY + 300, 200, 200);
            ofSetColor(180);
            if (font.isLoaded()) font.drawString("Green Tint", 280, startY + 520);

            ofSetColor(128, 128, 255);
            generatedImage.draw(510, startY + 300, 200, 200);
            ofSetColor(180);
            if (font.isLoaded()) font.drawString("Blue Tint", 510, startY + 520);
        }

    } else if (testMode == 1) {
        // === Mode 1: Show loaded image ===
        if (imageLoaded && testImage.isAllocated()) {
            ofSetColor(255);
            testImage.draw(50, startY);
            ofSetColor(180);
            if (font.isLoaded()) {
                char info[128];
                snprintf(info, sizeof(info), "Loaded: %dx%d", (int)testImage.getWidth(), (int)testImage.getHeight());
                font.drawString(info, 50, startY + testImage.getHeight() + 30);
            }

            // Draw at different sizes
            ofSetColor(255);
            testImage.draw(400, startY, 200, 200);
            testImage.draw(620, startY, 100, 100);
        } else {
            ofSetColor(255, 100, 100);
            if (font.isLoaded()) {
                font.drawString("No image loaded. Place test.png in data folder.", 50, startY + 50);
            }
        }

    } else if (testMode == 2) {
        // === Mode 2: Image manipulation demo ===
        ofSetColor(255);
        if (font.isLoaded()) {
            font.drawString("ofPixels manipulation demo:", 50, startY);
        }

        // Create animated pixels
        static oflike::ofImage animatedImage;
        static oflike::ofPixels animPixels;

        if (!animPixels.isAllocated()) {
            animPixels.allocate(128, 128, oflike::OF_IMAGE_COLOR);
        }

        // Update pixels with animation
        for (size_t y = 0; y < 128; y++) {
            for (size_t x = 0; x < 128; x++) {
                float u = x / 127.0f;
                float v = y / 127.0f;

                // Animated waves
                float wave1 = std::sin(u * 10.0f + time * 2.0f);
                float wave2 = std::cos(v * 8.0f + time * 1.5f);
                float combined = (wave1 + wave2 + 2.0f) * 0.25f;

                uint8_t r = static_cast<uint8_t>(combined * 255);
                uint8_t g = static_cast<uint8_t>((1.0f - combined) * 200);
                uint8_t b = static_cast<uint8_t>(std::sin(time + u * v * 10.0f) * 127 + 128);

                size_t idx = animPixels.getPixelIndex(x, y);
                animPixels[idx] = r;
                animPixels[idx + 1] = g;
                animPixels[idx + 2] = b;
            }
        }

        animatedImage.setFromPixels(animPixels);
        animatedImage.draw(50, startY + 30, 256, 256);

        ofSetColor(180);
        if (font.isLoaded()) {
            font.drawString("Real-time pixel animation", 50, startY + 310);
        }

        // Show pixel color access
        ofSetColor(255);
        if (font.isLoaded()) {
            font.drawString("getColor() demo:", 350, startY + 30);
        }

        // Sample colors from the animated image
        for (int i = 0; i < 8; i++) {
            size_t sampleX = (i * 16) % 128;
            size_t sampleY = (i * 16) / 128 * 16;
            oflike::ofColor c = animPixels.getColor(sampleX, sampleY);

            ofFill();
            ofSetColor(c.r, c.g, c.b);
            ofDrawRectangle(350 + i * 35, startY + 60, 30, 30);

            ofNoFill();
            ofSetColor(255);
            ofDrawRectangle(350 + i * 35, startY + 60, 30, 30);
        }
        ofFill();
    }

    // Draw key hints
    ofSetColor(150);
    if (font.isLoaded()) {
        font.drawString("Keys: 1-3 = switch mode, R = reload image", 20, ofGetHeight() - 20);
    }
}

void Test14::keyPressed(int key) {
    printf("Key pressed: %d\n", key);

    // Number keys 1-3 (keycodes 18, 19, 20 on macOS)
    if (key == 18) testMode = 0;  // 1
    else if (key == 19) testMode = 1;  // 2
    else if (key == 20) testMode = 2;  // 3
    else if (key == 15) {  // R key
        if (testImage.load("data/test.png")) {
            imageLoaded = true;
            printf("Reloaded test.png\n");
        }
    }
}

void Test14::keyReleased(int key) { (void)key; }
void Test14::mouseMoved(int x, int y) { (void)x; (void)y; }
void Test14::mouseDragged(int x, int y, int button) { (void)x; (void)y; (void)button; }
void Test14::mousePressed(int x, int y, int button) { (void)x; (void)y; (void)button; }
void Test14::mouseReleased(int x, int y, int button) { (void)x; (void)y; (void)button; }
void Test14::mouseScrolled(int x, int y, float scrollX, float scrollY) { (void)x; (void)y; (void)scrollX; (void)scrollY; }
void Test14::windowResized(int w, int h) { (void)w; (void)h; }

extern "C" ofBaseApp* ofCreateApp() {
    return new Test14();
}
