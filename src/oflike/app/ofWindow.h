#pragma once

#include <string>
#include "../../core/Context.h"

// Window control functions (implementations in platform/macos/)
void ofSetWindowTitle(const std::string& title);
void ofSetFullscreen(bool fullscreen);
void ofToggleFullscreen();
void ofHideCursor();
void ofShowCursor();
void ofSetFrameRate(float targetRate);

// Keyboard state
inline bool ofGetKeyPressed(int key) {
  return oflike::engine().getKeyPressed(key);
}

inline bool ofGetShiftPressed() {
  return oflike::engine().isShiftPressed();
}

inline bool ofGetCtrlPressed() {
  return oflike::engine().isCtrlPressed();
}

inline bool ofGetAltPressed() {
  return oflike::engine().isAltPressed();
}

inline bool ofGetCmdPressed() {
  return oflike::engine().isCmdPressed();
}
