// 3D Primitives Example - ported from openFrameworks
// Demonstrates: 3D wireframe rendering, rotation, perspective projection

#include "../../src/oflike/ofMain.h"

class ThreeDApp : public AppBase {
public:
  float rotX = 0;
  float rotY = 0;
  float rotZ = 0;
  bool autoRotate = true;
  float fov = 500;

  void setup() override {
    ofBackground(20);
  }

  void update() override {
    if (autoRotate) {
      rotX += 0.005f;
      rotY += 0.008f;
      rotZ += 0.003f;
    }
  }

  void draw() override {
    float cx = ofGetWidth() / 2.0f;
    float cy = ofGetHeight() / 2.0f;

    // Draw grid on the floor
    ofDrawGrid3D(50, 5, fov);

    // Draw axis at center
    ofDrawAxis3D(100, fov);

    // Draw rotating box
    ofSetColor(255, 150, 100);
    ofDrawBox3D(-200, 0, 0, 80, 80, 80, rotX, rotY, rotZ, fov);

    // Draw rotating sphere
    ofSetColor(100, 200, 255);
    ofDrawSphere3D(0, -100, 50, 50, rotX * 0.5f, rotY * 0.5f, 8, fov);

    // Draw rotating cone
    ofSetColor(100, 255, 150);
    ofDrawCone3D(200, 0, 0, 40, 100, rotX, rotY, 12, fov);

    // Draw rotating cylinder
    ofSetColor(255, 200, 100);
    ofDrawCylinder3D(0, 100, -50, 30, 80, rotX * 0.7f, rotY * 0.7f, 12, fov);

    // Draw multiple boxes in a circle
    int numBoxes = 8;
    float radius = 250;
    for (int i = 0; i < numBoxes; i++) {
      float angle = i * TWO_PI / numBoxes + rotY;
      float bx = std::cos(angle) * radius;
      float bz = std::sin(angle) * radius;

      ofColor c = ofColor::fromHsb((i * 255 / numBoxes), 200, 255);
      ofSetColor(c.r, c.g, c.b);
      ofDrawBox3D(bx, 0, bz, 30, 30, 30, 0, -angle, 0, fov);
    }

    // Draw a tower of spheres
    ofSetColor(200, 100, 255);
    for (int i = 0; i < 5; i++) {
      float sy = -150 - i * 40;
      float sr = 20 - i * 3;
      ofDrawSphere3D(0, sy, 200, sr, 0, rotY, 6, fov);
    }

    // UI
    ofSetColor(255);
    ofDrawBitmapString("3D Primitives Example", 10, 20);
    ofDrawBitmapString("FPS: " + ofToString(ofGetFrameRate(), 1), 10, 35);
    ofDrawBitmapString("Auto-rotate: " + std::string(autoRotate ? "ON" : "OFF") + " (SPACE to toggle)", 10, 50);
    ofDrawBitmapString("FOV: " + ofToString(fov, 0) + " (UP/DOWN to change)", 10, 65);
    ofDrawBitmapString("Use mouse to rotate manually when auto-rotate is off", 10, 80);
  }

  void keyPressed(int key) override {
    if (key == ' ') {
      autoRotate = !autoRotate;
    }
    if (key == OF_KEY_UP) {
      fov += 50;
    }
    if (key == OF_KEY_DOWN) {
      fov = std::max(100.0f, fov - 50);
    }
    if (key == 'r' || key == 'R') {
      rotX = rotY = rotZ = 0;
    }
  }

  void mouseDragged(float x, float y, int button) override {
    if (!autoRotate) {
      rotY += (x - ofGetMouseX()) * 0.01f;
      rotX += (y - ofGetMouseY()) * 0.01f;
    }
  }
};

OF_MAIN(ThreeDApp);
