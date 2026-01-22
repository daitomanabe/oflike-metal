#import <Cocoa/Cocoa.h>
#include "../../src/oflike/app/ofWindow.h"

static NSWindow* getMainWindow() {
  return [[NSApplication sharedApplication] mainWindow];
}

void ofSetWindowTitle(const std::string& title) {
  NSWindow* window = getMainWindow();
  if (window) {
    [window setTitle:[NSString stringWithUTF8String:title.c_str()]];
  }
}

void ofSetFullscreen(bool fullscreen) {
  NSWindow* window = getMainWindow();
  if (!window) return;

  bool isFullscreen = ([window styleMask] & NSWindowStyleMaskFullScreen) != 0;

  if (fullscreen != isFullscreen) {
    [window toggleFullScreen:nil];
  }
}

void ofToggleFullscreen() {
  NSWindow* window = getMainWindow();
  if (window) {
    [window toggleFullScreen:nil];
  }
}

void ofHideCursor() {
  [NSCursor hide];
}

void ofShowCursor() {
  [NSCursor unhide];
}

static float targetFrameRate = 60.0f;

void ofSetFrameRate(float rate) {
  targetFrameRate = rate;
  // Frame rate is controlled by MTKView's preferredFramesPerSecond
  // This would need integration with the view setup
}
