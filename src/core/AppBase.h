#pragma once

// Key constants for openFrameworks compatibility
constexpr int OF_KEY_RETURN    = 13;
constexpr int OF_KEY_ESC       = 27;
constexpr int OF_KEY_TAB       = 9;
constexpr int OF_KEY_BACKSPACE = 127;
constexpr int OF_KEY_DEL       = 127;
constexpr int OF_KEY_SHIFT     = 0x1001;
constexpr int OF_KEY_CONTROL   = 0x1002;
constexpr int OF_KEY_ALT       = 0x1003;
constexpr int OF_KEY_SUPER     = 0x1004;
constexpr int OF_KEY_LEFT      = 0x2001;
constexpr int OF_KEY_UP        = 0x2002;
constexpr int OF_KEY_RIGHT     = 0x2003;
constexpr int OF_KEY_DOWN      = 0x2004;

// Base class for user applications.
// Users inherit from this and override setup(), update(), draw(), and event handlers.
class AppBase {
public:
  virtual ~AppBase() = default;

  virtual void setup() {}
  virtual void update() {}
  virtual void draw() {}

  // Mouse events
  virtual void mousePressed(float x, float y, int button) {}
  virtual void mouseReleased(float x, float y, int button) {}
  virtual void mouseMoved(float x, float y) {}
  virtual void mouseDragged(float x, float y, int button) {}
  virtual void mouseScrolled(float x, float y, float scrollX, float scrollY) {}

  // Keyboard events
  virtual void keyPressed(int key) {}
  virtual void keyReleased(int key) {}

  // Window events
  virtual void windowResized(int w, int h) {}
};
