// Typography Example - ported from openFrameworks
// Demonstrates: Text rendering, animated text, color effects

#include "../../src/oflike/ofMain.h"
#include <vector>
#include <string>

class TypographyApp : public AppBase {
public:
  std::string sampleText = "Hello Metal!";
  std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  std::string numbers = "0123456789";
  std::vector<float> letterOffsets;

  void setup() override {
    ofBackground(30);

    // Initialize letter animation offsets
    for (size_t i = 0; i < alphabet.size(); i++) {
      letterOffsets.push_back(ofRandom(0.0f, TWO_PI));
    }
  }

  void update() override {
  }

  void draw() override {
    float t = ofGetElapsedTimef();

    // Section 1: Basic text
    ofSetColor(255);
    ofDrawBitmapString("Typography Example - Bitmap Font Demo", 20, 30);
    ofDrawBitmapString("FPS: " + ofToString(ofGetFrameRate(), 1), 20, 50);

    // Section 2: Rainbow text
    float y = 100;
    ofDrawBitmapString("Rainbow Text:", 20, y);
    y += 20;

    std::string rainbowText = "The quick brown fox jumps over the lazy dog";
    float x = 20;
    for (size_t i = 0; i < rainbowText.size(); i++) {
      ofColor c = ofColor::fromHsb(fmod((t * 50 + i * 10), 255), 200, 255);
      ofSetColor(c.r, c.g, c.b);
      ofDrawBitmapString(std::string(1, rainbowText[i]), x, y);
      x += 8; // Character width
    }

    // Section 3: Wavy text
    y = 160;
    ofSetColor(255);
    ofDrawBitmapString("Wavy Text:", 20, y);
    y += 20;

    x = 20;
    for (size_t i = 0; i < sampleText.size(); i++) {
      float waveY = std::sin(t * 3 + i * 0.5f) * 10;
      ofSetColor(200, 220, 255);
      ofDrawBitmapString(std::string(1, sampleText[i]), x, y + waveY);
      x += 10;
    }

    // Section 4: Animated alphabet
    y = 220;
    ofSetColor(255);
    ofDrawBitmapString("Bouncing Alphabet:", 20, y);
    y += 30;

    x = 20;
    for (size_t i = 0; i < alphabet.size(); i++) {
      float bounce = std::abs(std::sin(t * 2 + letterOffsets[i])) * 20;
      ofColor c = ofColor::fromHsb((i * 255.0f / alphabet.size()), 180, 255);
      ofSetColor(c.r, c.g, c.b);
      ofDrawBitmapString(std::string(1, alphabet[i]), x, y - bounce);
      x += 14;
    }

    // Section 5: Rotating text effect (simulated with position)
    y = 300;
    ofSetColor(255);
    ofDrawBitmapString("Circular Text:", 20, y);

    float centerX = 200;
    float centerY = 400;
    float radius = 80;
    std::string circleText = "CIRCULAR TEXT ";

    for (size_t i = 0; i < circleText.size(); i++) {
      float angle = t + i * (TWO_PI / circleText.size());
      float cx = centerX + std::cos(angle) * radius;
      float cy = centerY + std::sin(angle) * radius;
      ofColor c = ofColor::fromHsb(fmod((i * 20), 255), 200, 255);
      ofSetColor(c.r, c.g, c.b);
      ofDrawBitmapString(std::string(1, circleText[i]), cx, cy);
    }

    // Draw center circle
    ofSetColor(100);
    ofNoFill();
    ofDrawCircle(centerX, centerY, radius);

    // Section 6: Typewriter effect
    y = 320;
    ofSetColor(255);
    ofDrawBitmapString("Typewriter Effect:", 400, y);

    std::string typewriterText = "This text appears letter by letter...";
    int visibleChars = (int)(fmod(t * 8, typewriterText.size() + 10));
    visibleChars = std::min(visibleChars, (int)typewriterText.size());

    ofSetColor(100, 255, 100);
    ofDrawBitmapString(typewriterText.substr(0, visibleChars), 400, y + 20);

    // Blinking cursor
    if (visibleChars < (int)typewriterText.size() && fmod(t * 2, 1.0f) > 0.5f) {
      ofDrawBitmapString("_", 400 + visibleChars * 8, y + 20);
    }

    // Section 7: Scale simulation with multiple draws
    y = 400;
    ofSetColor(255);
    ofDrawBitmapString("Size Variations (simulated):", 400, y);

    // Small (normal)
    ofSetColor(200, 200, 255);
    ofDrawBitmapString("Small", 400, y + 25);

    // Medium (draw twice offset)
    ofSetColor(200, 255, 200);
    ofDrawBitmapString("Medium", 400, y + 45);
    ofSetColor(180, 235, 180);
    ofDrawBitmapString("Medium", 401, y + 45);

    // Large (draw multiple times)
    ofSetColor(255, 200, 200);
    for (int ox = 0; ox < 2; ox++) {
      for (int oy = 0; oy < 2; oy++) {
        ofDrawBitmapString("Large", 400 + ox, y + 70 + oy);
      }
    }

    // Section 8: Text grid
    y = 520;
    ofSetColor(255);
    ofDrawBitmapString("Number Grid:", 400, y);

    for (int row = 0; row < 3; row++) {
      for (int col = 0; col < 10; col++) {
        float hue = fmod((t * 30 + row * 30 + col * 10), 255);
        ofColor c = ofColor::fromHsb(hue, 150, 255);
        ofSetColor(c.r, c.g, c.b);
        ofDrawBitmapString(std::string(1, numbers[col]), 400 + col * 15, y + 20 + row * 15);
      }
    }

    // Instructions
    ofSetColor(150);
    ofDrawBitmapString("Press 'r' to reset animation | SPACE to pause", 20, ofGetHeight() - 20);
  }

  void keyPressed(int key) override {
    if (key == 'r' || key == 'R') {
      // Reset letter offsets
      for (size_t i = 0; i < letterOffsets.size(); i++) {
        letterOffsets[i] = ofRandom(0.0f, TWO_PI);
      }
    }
  }
};

OF_MAIN(TypographyApp);
