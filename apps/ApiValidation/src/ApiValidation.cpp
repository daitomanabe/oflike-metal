#include "ApiValidation.h"

#include <oflike/image/ofPixels.h>

#include <cmath>

using oflike::ofColor;
using oflike::ofPixels;

void ApiValidation::setup() {
    ofSetWindowTitle("ApiValidation");

    sceneNames_ = {
        "Overview",
        "2D Primitives",
        "Transforms",
        "Image + Text"
    };
    updateSceneLabel();

    overlayFontLoaded_ = overlayFont_.load("Geneva", 14);
    if (!overlayFontLoaded_) {
        overlayFontLoaded_ = overlayFont_.load("Helvetica", 14);
    }

    contentFontLoaded_ = contentFont_.load("Geneva", 20);
    if (!contentFontLoaded_) {
        contentFontLoaded_ = contentFont_.load("Helvetica", 20);
    }

    testImageLoaded_ = testImage_.load("test.jpg");
    if (!testImageLoaded_) {
        testImageLoaded_ = testImage_.load("test.png");
    }
    if (!testImageLoaded_) {
        buildFallbackImage();
    }
}

void ApiValidation::update() {
    angle_ += 0.02f;
    frameCount_ += 1;
}

void ApiValidation::draw() {
    ofClear(30, 30, 30);
    drawScene();
    drawOverlay();
}

void ApiValidation::keyPressed(int key) {
    if (key >= '1' && key <= '9') {
        int idx = key - '1';
        if (idx >= 0 && idx < static_cast<int>(sceneNames_.size())) {
            currentScene_ = idx;
            updateSceneLabel();
        }
        return;
    }

    if (key == 'n') {
        currentScene_ = (currentScene_ + 1) % static_cast<int>(sceneNames_.size());
        updateSceneLabel();
        return;
    }

    if (key == 'p') {
        currentScene_ -= 1;
        if (currentScene_ < 0) {
            currentScene_ = static_cast<int>(sceneNames_.size()) - 1;
        }
        updateSceneLabel();
    }
}

void ApiValidation::keyReleased(int key) {
    (void)key;
}

void ApiValidation::mouseMoved(int x, int y) {
    (void)x;
    (void)y;
}

void ApiValidation::mouseDragged(int x, int y, int button) {
    (void)x;
    (void)y;
    (void)button;
}

void ApiValidation::mousePressed(int x, int y, int button) {
    (void)x;
    (void)y;
    (void)button;
}

void ApiValidation::mouseReleased(int x, int y, int button) {
    (void)x;
    (void)y;
    (void)button;
}

void ApiValidation::windowResized(int w, int h) {
    (void)w;
    (void)h;
}

void ApiValidation::drawScene() {
    switch (currentScene_) {
        case 0:
            drawSceneOverview();
            break;
        case 1:
            drawScenePrimitives();
            break;
        case 2:
            drawSceneTransforms();
            break;
        case 3:
            drawSceneImageText();
            break;
        default:
            drawSceneOverview();
            break;
    }
}

void ApiValidation::drawSceneOverview() {
    const float centerX = ofGetWidth() * 0.5f;
    const float centerY = ofGetHeight() * 0.5f;

    ofSetColor(80, 160, 255);
    ofDrawCircle(centerX, centerY, 120);

    const float orbitX = centerX + std::cos(angle_) * 180.0f;
    const float orbitY = centerY + std::sin(angle_) * 180.0f;
    ofSetColor(255, 180, 80);
    ofDrawCircle(orbitX, orbitY, 30);
}

void ApiValidation::drawScenePrimitives() {
    ofSetColor(255, 100, 100);
    ofDrawRectangle(80, 120, 220, 120);

    ofSetColor(100, 255, 140);
    ofDrawCircle(420, 180, 60);

    ofSetColor(100, 160, 255);
    ofDrawLine(80, 320, 420, 320);

    ofSetColor(240);
    ofDrawTriangle(520, 120, 640, 240, 520, 240);
}

void ApiValidation::drawSceneTransforms() {
    const float centerX = ofGetWidth() * 0.5f;
    const float centerY = ofGetHeight() * 0.5f;

    ofPushMatrix();
    ofTranslate(centerX, centerY);
    ofRotate(angle_ * 60.0f);
    ofSetColor(200, 120, 255);
    ofDrawRectangle(-100, -40, 200, 80);
    ofPopMatrix();

    ofPushMatrix();
    ofTranslate(centerX, centerY + 160);
    const float scale = 0.75f + 0.25f * std::sin(angle_ * 1.5f);
    ofScale(scale, scale);
    ofSetColor(120, 220, 200);
    ofDrawCircle(0, 0, 50);
    ofPopMatrix();
}

void ApiValidation::drawSceneImageText() {
    if (testImageLoaded_) {
        ofSetColor(255);
        testImage_.draw(80, 120, 220, 220);
    } else {
        ofSetColor(100);
        ofDrawRectangle(80, 120, 220, 220);
    }

    if (contentFontLoaded_) {
        ofSetColor(255);
        contentFont_.drawString("API Validation", 360, 180);
        contentFont_.drawString("Press 1-4 to switch", 360, 220);
    } else {
        ofSetColor(220);
        ofDrawRectangle(360, 150, 260, 12);
        ofDrawRectangle(360, 190, 260, 12);
    }
}

void ApiValidation::drawOverlay() {
    const float padding = 10.0f;
    float lineHeight = overlayFontLoaded_ ? overlayFont_.getLineHeight() : 16.0f;
    if (lineHeight <= 0.0f) {
        lineHeight = 16.0f;
    }

    std::vector<std::string> lines;
    lines.push_back("ApiValidation");
    lines.push_back(sceneLabel_);
    lines.push_back("FPS: " + ofToString(ofGetFrameRate(), 1));
    lines.push_back("Frame: " + ofToString(frameCount_));
    lines.push_back("Size: " + ofToString(ofGetWidth()) + " x " + ofToString(ofGetHeight()));
    lines.push_back("Keys: 1-4, n/p");

    float boxWidth = 200.0f;
    if (overlayFontLoaded_) {
        boxWidth = 0.0f;
        for (const auto& line : lines) {
            const float width = overlayFont_.stringWidth(line);
            if (width > boxWidth) {
                boxWidth = width;
            }
        }
        boxWidth += padding * 2.0f;
    }
    const float boxHeight = padding * 2.0f + lineHeight * static_cast<float>(lines.size());

    ofSetColor(0, 0, 0, 160);
    ofDrawRectangle(10, 10, boxWidth, boxHeight);

    ofSetColor(255);
    float x = 10.0f + padding;
    float y = 10.0f + padding + lineHeight;

    for (const auto& line : lines) {
        if (overlayFontLoaded_) {
            overlayFont_.drawString(line, x, y);
        } else {
            ofDrawRectangle(x, y - lineHeight + 4.0f, 120.0f, 4.0f);
        }
        y += lineHeight;
    }
}

void ApiValidation::buildFallbackImage() {
    const int width = 128;
    const int height = 128;

    ofPixels pixels;
    pixels.allocate(width, height, oflike::OF_IMAGE_COLOR);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const bool checker = ((x / 16) + (y / 16)) % 2 == 0;
            const uint8_t r = static_cast<uint8_t>((x * 255) / (width - 1));
            const uint8_t g = static_cast<uint8_t>((y * 255) / (height - 1));
            const uint8_t b = static_cast<uint8_t>(checker ? 220 : 90);
            pixels.setColor(x, y, ofColor(r, g, b));
        }
    }

    testImage_.setFromPixels(pixels);
    testImageLoaded_ = true;
}

void ApiValidation::updateSceneLabel() {
    const int total = static_cast<int>(sceneNames_.size());
    if (currentScene_ >= 0 && currentScene_ < total) {
        sceneLabel_ = "Scene " + ofToString(currentScene_ + 1) + "/" + ofToString(total) + ": " + sceneNames_[currentScene_];
    } else {
        sceneLabel_ = "Scene " + ofToString(currentScene_ + 1);
    }
}

extern "C" ofBaseApp* ofCreateApp() {
    return new ApiValidation();
}
