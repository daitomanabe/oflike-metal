// 04_image - Image Loading and Manipulation Example
// Demonstrates ofImage loading, drawing, pixel manipulation, and basic operations

#include "../../src/core/AppBase.h"
#include "../../src/oflike/graphics/ofGraphics.h"
#include "../../src/oflike/image/ofImage.h"
#include "../../src/oflike/utils/ofUtils.h"
#include "../../src/core/Context.h"
#include <cmath>
#include <iostream>

using namespace oflike;

class ImageManipulationApp : public ofBaseApp {
public:
    void setup() override {
        ofSetFrameRate(60);
        ofSetWindowTitle("04_image - Image Loading & Manipulation");

        // Create a procedural test image
        createTestImage();

        // Test image operations
        createTintedImage();
        createGrayscaleImage();
        createCroppedImage();
        createResizedImage();

        mode = 0;
        std::cout << "04_image example started" << std::endl;
        std::cout << "Press SPACE to cycle through different image operations" << std::endl;
        std::cout << "Press 'S' to save the current image" << std::endl;
    }

    void update() override {
        // Animate tint color
        time += 0.016f;
    }

    void draw() override {
        ofBackground(40, 40, 45);

        // Draw title
        ofSetColor(255, 255, 255);
        std::string title;
        float x = 50;
        float y = 100;

        switch (mode) {
            case 0:
                title = "Original Image (Procedural)";
                if (originalImage.isAllocated()) {
                    originalImage.draw(x, y);
                }
                break;

            case 1:
                title = "Tinted Image (setColor)";
                if (tintedImage.isAllocated()) {
                    tintedImage.draw(x, y);
                }
                break;

            case 2:
                title = "Grayscale Conversion";
                if (grayscaleImage.isAllocated()) {
                    grayscaleImage.draw(x, y);
                }
                break;

            case 3:
                title = "Cropped Image";
                if (croppedImage.isAllocated()) {
                    croppedImage.draw(x, y);
                }
                break;

            case 4:
                title = "Resized Image (256x256)";
                if (resizedImage.isAllocated()) {
                    resizedImage.draw(x, y);
                }
                break;

            case 5:
                title = "Image with Effects";
                drawImageWithEffects(x, y);
                break;
        }

        // Draw UI text
        ofSetColor(255, 255, 255);
        drawText(title, 50, 50);
        drawText("SPACE: Next mode | S: Save image", 50, ofGetHeight() - 30);

        // Draw mode indicator
        std::string modeText = "Mode " + std::to_string(mode + 1) + " / 6";
        drawText(modeText, ofGetWidth() - 150, 50);

        // Draw FPS
        drawText("FPS: " + std::to_string(static_cast<int>(ofGetFrameRate())),
                ofGetWidth() - 150, ofGetHeight() - 30);
    }

    void keyPressed(int key) override {
        if (key == ' ') {
            mode = (mode + 1) % 6;
            std::cout << "Switched to mode " << (mode + 1) << std::endl;
        } else if (key == 's' || key == 'S') {
            saveCurrentImage();
        }
    }

    void windowResized(int w, int h) override {
        std::cout << "Window resized: " << w << "x" << h << std::endl;
    }

private:
    ofImage originalImage;
    ofImage tintedImage;
    ofImage grayscaleImage;
    ofImage croppedImage;
    ofImage resizedImage;

    int mode;
    float time;

    void createTestImage() {
        // Create a 512x512 procedural test image
        const int width = 512;
        const int height = 512;

        ofPixels pixels;
        pixels.allocate(width, height, OF_IMAGE_COLOR);

        // Generate gradient with circular pattern
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                float centerX = width / 2.0f;
                float centerY = height / 2.0f;
                float dist = std::sqrt((x - centerX) * (x - centerX) +
                                      (y - centerY) * (y - centerY));
                float maxDist = std::sqrt(centerX * centerX + centerY * centerY);

                // Create colorful gradient
                unsigned char r = static_cast<unsigned char>(255 * (1.0f - dist / maxDist));
                unsigned char g = static_cast<unsigned char>(255 * (x / static_cast<float>(width)));
                unsigned char b = static_cast<unsigned char>(255 * (y / static_cast<float>(height)));

                pixels.setColor(x, y, ofColor(r, g, b));
            }
        }

        originalImage.setFromPixels(pixels);
        std::cout << "Created procedural test image: " << width << "x" << height << std::endl;
    }

    void createTintedImage() {
        // Create a tinted version by manipulating pixels
        ofPixels pixels = originalImage.getPixels();

        for (size_t i = 0; i < pixels.size(); i += 3) {
            // Apply magenta tint
            pixels[i + 0] = static_cast<unsigned char>(pixels[i + 0] * 1.2f); // R
            pixels[i + 1] = static_cast<unsigned char>(pixels[i + 1] * 0.7f); // G
            pixels[i + 2] = static_cast<unsigned char>(pixels[i + 2] * 1.1f); // B
        }

        tintedImage.setFromPixels(pixels);
        std::cout << "Created tinted image" << std::endl;
    }

    void createGrayscaleImage() {
        // Convert to grayscale
        ofPixels pixels = originalImage.getPixels();
        ofPixels grayPixels;
        grayPixels.allocate(pixels.getWidth(), pixels.getHeight(), OF_IMAGE_COLOR);

        for (int y = 0; y < pixels.getHeight(); y++) {
            for (int x = 0; x < pixels.getWidth(); x++) {
                ofColor color = pixels.getColor(x, y);
                unsigned char gray = static_cast<unsigned char>(
                    0.299f * color.r + 0.587f * color.g + 0.114f * color.b
                );
                grayPixels.setColor(x, y, ofColor(gray, gray, gray));
            }
        }

        grayscaleImage.setFromPixels(grayPixels);
        std::cout << "Created grayscale image" << std::endl;
    }

    void createCroppedImage() {
        // Crop center region
        ofPixels pixels = originalImage.getPixels();
        int cropWidth = pixels.getWidth() / 2;
        int cropHeight = pixels.getHeight() / 2;
        int cropX = pixels.getWidth() / 4;
        int cropY = pixels.getHeight() / 4;

        ofPixels croppedPixels;
        croppedPixels.allocate(cropWidth, cropHeight, OF_IMAGE_COLOR);

        for (int y = 0; y < cropHeight; y++) {
            for (int x = 0; x < cropWidth; x++) {
                ofColor color = pixels.getColor(cropX + x, cropY + y);
                croppedPixels.setColor(x, y, color);
            }
        }

        croppedImage.setFromPixels(croppedPixels);
        std::cout << "Created cropped image: " << cropWidth << "x" << cropHeight << std::endl;
    }

    void createResizedImage() {
        // Resize to 256x256
        resizedImage = originalImage;
        resizedImage.resize(256, 256);
        std::cout << "Created resized image: 256x256" << std::endl;
    }

    void drawImageWithEffects(float x, float y) {
        if (!originalImage.isAllocated()) return;

        // Draw with various transformations
        ofPushMatrix();
        ofTranslate(x + 256, y + 256); // Center

        // Rotate
        ofRotateZ(std::sin(time) * 15.0f);

        // Scale
        float scale = 1.0f + std::sin(time * 2.0f) * 0.2f;
        ofScale(scale, scale, 1.0f);

        // Draw with tint
        ofSetColor(255, 200 + std::sin(time * 3.0f) * 55, 200);
        originalImage.draw(-256, -256);

        ofPopMatrix();
    }

    void saveCurrentImage() {
        std::string filename;
        ofImage* imgToSave = nullptr;

        switch (mode) {
            case 0: filename = "original.png"; imgToSave = &originalImage; break;
            case 1: filename = "tinted.png"; imgToSave = &tintedImage; break;
            case 2: filename = "grayscale.png"; imgToSave = &grayscaleImage; break;
            case 3: filename = "cropped.png"; imgToSave = &croppedImage; break;
            case 4: filename = "resized.png"; imgToSave = &resizedImage; break;
            case 5: filename = "original.png"; imgToSave = &originalImage; break;
        }

        if (imgToSave && imgToSave->isAllocated()) {
            imgToSave->save(filename);
            std::cout << "Saved: " << filename << std::endl;
        }
    }

    void drawText(const std::string& text, float x, float y) {
        // Simple text rendering using rectangles (placeholder until ofTrueTypeFont is integrated)
        // In a real implementation, this would use ofDrawBitmapString or ofTrueTypeFont
        ofSetColor(255, 255, 255);
        // For now, just draw a line to indicate text position
        ofDrawLine(x, y, x + text.length() * 6, y);
    }
};

int main() {
    auto app = std::make_shared<ImageManipulationApp>();
    return ofAppRunner::run(app, 1024, 768, "04_image");
}
