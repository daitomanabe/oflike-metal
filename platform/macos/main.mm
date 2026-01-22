#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/QuartzCore.h>

#include "../../src/core/Context.h"
#include "../../src/core/Engine.h"
#include "../../src/oflike/app/ofAppRunner.h"

// Custom MTKView subclass that handles input events
@interface InputMTKView : MTKView
@property (nonatomic, assign) oflike::Engine* engine;
@end

@implementation InputMTKView

- (BOOL)acceptsFirstResponder {
  return YES;
}

- (void)mouseDown:(NSEvent*)event {
  if (!_engine) return;
  NSPoint loc = [self convertPoint:event.locationInWindow fromView:nil];
  // Flip Y to match oF-style coordinate system (origin top-left)
  float x = (float)loc.x;
  float y = (float)(self.bounds.size.height - loc.y);
  _engine->setMousePosition((int)x, (int)y);
  _engine->setMousePressed((int)event.buttonNumber, true);
  if (_engine->app()) _engine->app()->mousePressed(x, y, (int)event.buttonNumber);
}

- (void)mouseUp:(NSEvent*)event {
  if (!_engine) return;
  NSPoint loc = [self convertPoint:event.locationInWindow fromView:nil];
  float x = (float)loc.x;
  float y = (float)(self.bounds.size.height - loc.y);
  _engine->setMousePosition((int)x, (int)y);
  _engine->setMousePressed((int)event.buttonNumber, false);
  if (_engine->app()) _engine->app()->mouseReleased(x, y, (int)event.buttonNumber);
}

- (void)mouseDragged:(NSEvent*)event {
  if (!_engine) return;
  NSPoint loc = [self convertPoint:event.locationInWindow fromView:nil];
  float x = (float)loc.x;
  float y = (float)(self.bounds.size.height - loc.y);
  _engine->setMousePosition((int)x, (int)y);
  if (_engine->app()) _engine->app()->mouseDragged(x, y, (int)event.buttonNumber);
}

- (void)mouseMoved:(NSEvent*)event {
  if (!_engine) return;
  NSPoint loc = [self convertPoint:event.locationInWindow fromView:nil];
  float x = (float)loc.x;
  float y = (float)(self.bounds.size.height - loc.y);
  _engine->setMousePosition((int)x, (int)y);
  if (_engine->app()) _engine->app()->mouseMoved(x, y);
}

- (void)rightMouseDown:(NSEvent*)event {
  if (!_engine) return;
  NSPoint loc = [self convertPoint:event.locationInWindow fromView:nil];
  float x = (float)loc.x;
  float y = (float)(self.bounds.size.height - loc.y);
  _engine->setMousePosition((int)x, (int)y);
  _engine->setMousePressed((int)event.buttonNumber, true);
  if (_engine->app()) _engine->app()->mousePressed(x, y, (int)event.buttonNumber);
}

- (void)rightMouseUp:(NSEvent*)event {
  if (!_engine) return;
  NSPoint loc = [self convertPoint:event.locationInWindow fromView:nil];
  float x = (float)loc.x;
  float y = (float)(self.bounds.size.height - loc.y);
  _engine->setMousePosition((int)x, (int)y);
  _engine->setMousePressed((int)event.buttonNumber, false);
  if (_engine->app()) _engine->app()->mouseReleased(x, y, (int)event.buttonNumber);
}

- (void)rightMouseDragged:(NSEvent*)event {
  if (!_engine) return;
  NSPoint loc = [self convertPoint:event.locationInWindow fromView:nil];
  float x = (float)loc.x;
  float y = (float)(self.bounds.size.height - loc.y);
  _engine->setMousePosition((int)x, (int)y);
  if (_engine->app()) _engine->app()->mouseDragged(x, y, (int)event.buttonNumber);
}

- (void)keyDown:(NSEvent*)event {
  // ESC key (keyCode 53) terminates the app
  if (event.keyCode == 53) {
    [NSApp terminate:nil];
    return;
  }

  if (!_engine) return;

  // Update modifier keys
  NSEventModifierFlags flags = event.modifierFlags;
  _engine->setModifierKeys(
    (flags & NSEventModifierFlagShift) != 0,
    (flags & NSEventModifierFlagControl) != 0,
    (flags & NSEventModifierFlagOption) != 0,
    (flags & NSEventModifierFlagCommand) != 0
  );

  // Track key state and get character
  int key = (int)event.keyCode;
  _engine->setKeyPressed(key, true);

  // Also try to get ASCII character
  NSString* chars = event.charactersIgnoringModifiers;
  if (chars.length > 0) {
    unichar c = [chars characterAtIndex:0];
    if (c < 256) {
      _engine->setKeyPressed((int)c, true);
      key = (int)c;
    }
  }

  if (_engine->app()) _engine->app()->keyPressed(key);
}

- (void)keyUp:(NSEvent*)event {
  if (!_engine) return;

  // Update modifier keys
  NSEventModifierFlags flags = event.modifierFlags;
  _engine->setModifierKeys(
    (flags & NSEventModifierFlagShift) != 0,
    (flags & NSEventModifierFlagControl) != 0,
    (flags & NSEventModifierFlagOption) != 0,
    (flags & NSEventModifierFlagCommand) != 0
  );

  int key = (int)event.keyCode;
  _engine->setKeyPressed(key, false);

  // Also release ASCII character
  NSString* chars = event.charactersIgnoringModifiers;
  if (chars.length > 0) {
    unichar c = [chars characterAtIndex:0];
    if (c < 256) {
      _engine->setKeyPressed((int)c, false);
      key = (int)c;
    }
  }

  if (_engine->app()) _engine->app()->keyReleased(key);
}

- (void)flagsChanged:(NSEvent*)event {
  if (!_engine) return;

  NSEventModifierFlags flags = event.modifierFlags;
  _engine->setModifierKeys(
    (flags & NSEventModifierFlagShift) != 0,
    (flags & NSEventModifierFlagControl) != 0,
    (flags & NSEventModifierFlagOption) != 0,
    (flags & NSEventModifierFlagCommand) != 0
  );
}

@end

@interface MetalViewDelegate : NSObject <MTKViewDelegate>
- (instancetype)initWithEngine:(oflike::Engine*)engine;
@end

@implementation MetalViewDelegate {
  oflike::Engine* _engine;
  CFTimeInterval _lastTime;
}

- (instancetype)initWithEngine:(oflike::Engine*)engine {
  if (self = [super init]) {
    _engine = engine;
    _lastTime = CACurrentMediaTime();
  }
  return self;
}

- (void)mtkView:(MTKView*)view drawableSizeWillChange:(CGSize)size {
  if (_engine) {
    _engine->setDrawableSizePixels((int)size.width, (int)size.height);
    // Update content scale for Retina displays
    CGFloat scale = view.window.backingScaleFactor;
    if (scale > 0) {
      _engine->setContentScale((float)scale);
    }
  }
}

- (void)drawInMTKView:(MTKView*)view {
  if (!_engine) return;

  CFTimeInterval now = CACurrentMediaTime();
  double dt = (double)(now - _lastTime);
  _lastTime = now;

  // Prefer drawableSize, not bounds.size (Retina-safe)
  CGSize ds = view.drawableSize;
  _engine->setDrawableSizePixels((int)ds.width, (int)ds.height);

  // Update content scale every frame (needed for proper Retina support)
  CGFloat scale = view.window.backingScaleFactor;
  if (scale > 0) {
    _engine->setContentScale((float)scale);
  }

  MTLRenderPassDescriptor* pass = view.currentRenderPassDescriptor;
  id<CAMetalDrawable> drawable = view.currentDrawable;
  if (!pass || !drawable) return;

  _engine->tick(dt, (__bridge void*)pass, (__bridge void*)drawable);
}

@end

int main(int argc, const char* argv[]) {
  @autoreleasepool {
    (void)argc; (void)argv;

    NSApplication* app = [NSApplication sharedApplication];
    [app setActivationPolicy:NSApplicationActivationPolicyRegular];

    NSRect frame = NSMakeRect(0, 0, 1024, 768);
    NSWindowStyleMask style = (NSWindowStyleMaskTitled |
                               NSWindowStyleMaskClosable |
                               NSWindowStyleMaskResizable |
                               NSWindowStyleMaskMiniaturizable);

    NSWindow* window = [[NSWindow alloc] initWithContentRect:frame
                                                   styleMask:style
                                                     backing:NSBackingStoreBuffered
                                                       defer:NO];
    [window setTitle:@"oflike_metal_skeleton"];

    // Use custom InputMTKView instead of plain MTKView
    InputMTKView* view = [[InputMTKView alloc] initWithFrame:frame];
    view.device = MTLCreateSystemDefaultDevice();
    view.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
    view.preferredFramesPerSecond = 60;
    view.paused = NO;
    view.enableSetNeedsDisplay = NO;

    // Engine lifetime must outlive delegate
    static oflike::Engine engine;
    oflike::setEngine(&engine);
    engine.setApp(::oflike::createApp());
    engine.attachToView((__bridge void*)view);

    // Set engine reference for input handling
    view.engine = &engine;

    MetalViewDelegate* delegate = [[MetalViewDelegate alloc] initWithEngine:&engine];
    view.delegate = delegate;

    [window setContentView:view];
    [window makeFirstResponder:view];
    [window makeKeyAndOrderFront:nil];
    [app activateIgnoringOtherApps:YES];

    [app run];
  }
  return 0;
}
