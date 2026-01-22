#pragma once

// AppBase: openFrameworks-style lifecycle and input callbacks.
// User code should derive from this class in C++.

class AppBase {
public:
  virtual ~AppBase() = default;

  // Lifecycle
  virtual void setup() {}
  virtual void update() {}
  virtual void draw() {}

  // Keyboard
  virtual void keyPressed(int key) {}
  virtual void keyReleased(int key) {}

  // Mouse (coordinates in points)
  virtual void mouseMoved(float x, float y) {}
  virtual void mouseDragged(float x, float y, int button) {}
  virtual void mousePressed(float x, float y, int button) {}
  virtual void mouseReleased(float x, float y, int button) {}

  // Window
  virtual void windowResized(int width, int height) {}
};
