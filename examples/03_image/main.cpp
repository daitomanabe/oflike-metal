// Image Example - ported from openFrameworks
// Demonstrates: ofImage, ofPixels, procedural textures, image manipulation

#include "../../src/oflike/ofMain.h"

class ImageApp : public AppBase {
public:
  ofImage proceduralImg;
  ofImage gradientImg;
  ofImage noiseImg;

  int imgSize = 256;
  float noiseScale = 0.02f;
  float noiseOffset = 0;

  void setup() override {
    ofBackground(40);

    // Create procedural checkerboard image
    createCheckerboard();

    // Create gradient image
    createGradient();

    // Create noise image
    createNoiseImage();
  }

  void createCheckerboard() {
    proceduralImg.allocate(imgSize, imgSize, 4);
    oflike::ofPixels& pixels = proceduralImg.getPixels();

    int tileSize = 32;
    for (int y = 0; y < imgSize; y++) {
      for (int x = 0; x < imgSize; x++) {
        bool isWhite = ((x / tileSize) + (y / tileSize)) % 2 == 0;
        unsigned char c = isWhite ? 255 : 50;
        pixels.setColor(x, y, c, c, c, 255);
      }
    }
    proceduralImg.update();
  }

  void createGradient() {
    gradientImg.allocate(imgSize, imgSize, 4);
    oflike::ofPixels& pixels = gradientImg.getPixels();

    for (int y = 0; y < imgSize; y++) {
      for (int x = 0; x < imgSize; x++) {
        float hue = (float)x / imgSize * 255;
        float sat = (float)y / imgSize * 255;
        ofColor c = ofColor::fromHsb(hue, sat, 255);
        pixels.setColor(x, y, c.r, c.g, c.b, 255);
      }
    }
    gradientImg.update();
  }

  void createNoiseImage() {
    noiseImg.allocate(imgSize, imgSize, 4);
    updateNoiseImage();
  }

  void updateNoiseImage() {
    oflike::ofPixels& pixels = noiseImg.getPixels();

    for (int y = 0; y < imgSize; y++) {
      for (int x = 0; x < imgSize; x++) {
        float n = ofNoise(x * noiseScale + noiseOffset, y * noiseScale + noiseOffset);
        unsigned char c = (unsigned char)(n * 255);
        pixels.setColor(x, y, c, c, c, 255);
      }
    }
    noiseImg.update();
  }

  void update() override {
    // Animate noise
    noiseOffset += 0.01f;
    updateNoiseImage();
  }

  void draw() override {
    float spacing = 20;
    float imgDisplaySize = 200;

    // Draw checkerboard
    ofSetColor(255);
    proceduralImg.draw(spacing, 80, imgDisplaySize, imgDisplaySize);
    ofDrawBitmapString("Checkerboard", spacing, 70);

    // Draw gradient
    gradientImg.draw(spacing + imgDisplaySize + spacing, 80, imgDisplaySize, imgDisplaySize);
    ofDrawBitmapString("HSB Gradient", spacing + imgDisplaySize + spacing, 70);

    // Draw noise
    noiseImg.draw(spacing + (imgDisplaySize + spacing) * 2, 80, imgDisplaySize, imgDisplaySize);
    ofDrawBitmapString("Animated Noise", spacing + (imgDisplaySize + spacing) * 2, 70);

    // Draw image at mouse position with tint
    ofSetColor(ofColor::fromHsb(fmod(ofGetElapsedTimef() * 50, 255), 200, 255));
    proceduralImg.draw(ofGetMouseX() - 50, ofGetMouseY() - 50, 100, 100);

    // Draw stretched/squashed versions
    ofSetColor(255);
    float y2 = 320;
    ofDrawBitmapString("Stretched:", spacing, y2);
    proceduralImg.draw(spacing, y2 + 10, 300, 100);

    ofDrawBitmapString("Squashed:", spacing, y2 + 130);
    proceduralImg.draw(spacing, y2 + 140, 100, 200);

    // UI
    ofSetColor(255);
    ofDrawBitmapString("Image Example", 10, 20);
    ofDrawBitmapString("FPS: " + ofToString(ofGetFrameRate(), 1), 10, 35);
    ofDrawBitmapString("Noise Scale: " + ofToString(noiseScale, 3) + " (UP/DOWN)", 10, 50);

    // Instructions
    ofDrawBitmapString("Move mouse to see tinted image follow cursor", 10, ofGetHeight() - 20);
  }

  void keyPressed(int key) override {
    if (key == OF_KEY_UP) {
      noiseScale += 0.005f;
    }
    if (key == OF_KEY_DOWN) {
      noiseScale = std::max(0.001f, noiseScale - 0.005f);
    }
    if (key == 'r' || key == 'R') {
      // Regenerate images
      createCheckerboard();
      createGradient();
    }
  }
};

OF_MAIN(ImageApp);
