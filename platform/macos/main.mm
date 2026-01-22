#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/QuartzCore.h>

#include "../../src/oflike/ofContext.hpp"
#include "../../src/core/Engine.hpp"
#include "../../src/oflike/ofAppRunner.h"

// Custom MTKView subclass that handles input events
@interface InputMTKView : MTKView
@property (nonatomic, assign) oflike::Engine* engine;
@end

@implementation InputMTKView

- (BOOL)acceptsFirstResponder {
  return YES;
}

- (void)mouseDown:(NSEvent*)event {
  if (!_engine || !_engine->app()) return;
  NSPoint loc = [self convertPoint:event.locationInWindow fromView:nil];
  // Flip Y to match oF-style coordinate system (origin top-left)
  float x = (float)loc.x;
  float y = (float)(self.bounds.size.height - loc.y);
  _engine->app()->mousePressed(x, y, (int)event.buttonNumber);
}

- (void)mouseUp:(NSEvent*)event {
  if (!_engine || !_engine->app()) return;
  NSPoint loc = [self convertPoint:event.locationInWindow fromView:nil];
  float x = (float)loc.x;
  float y = (float)(self.bounds.size.height - loc.y);
  _engine->app()->mouseReleased(x, y, (int)event.buttonNumber);
}

- (void)mouseDragged:(NSEvent*)event {
  if (!_engine || !_engine->app()) return;
  NSPoint loc = [self convertPoint:event.locationInWindow fromView:nil];
  float x = (float)loc.x;
  float y = (float)(self.bounds.size.height - loc.y);
  _engine->app()->mouseDragged(x, y, (int)event.buttonNumber);
}

- (void)mouseMoved:(NSEvent*)event {
  if (!_engine || !_engine->app()) return;
  NSPoint loc = [self convertPoint:event.locationInWindow fromView:nil];
  float x = (float)loc.x;
  float y = (float)(self.bounds.size.height - loc.y);
  _engine->app()->mouseMoved(x, y);
}

- (void)rightMouseDown:(NSEvent*)event {
  if (!_engine || !_engine->app()) return;
  NSPoint loc = [self convertPoint:event.locationInWindow fromView:nil];
  float x = (float)loc.x;
  float y = (float)(self.bounds.size.height - loc.y);
  _engine->app()->mousePressed(x, y, (int)event.buttonNumber);
}

- (void)rightMouseUp:(NSEvent*)event {
  if (!_engine || !_engine->app()) return;
  NSPoint loc = [self convertPoint:event.locationInWindow fromView:nil];
  float x = (float)loc.x;
  float y = (float)(self.bounds.size.height - loc.y);
  _engine->app()->mouseReleased(x, y, (int)event.buttonNumber);
}

- (void)rightMouseDragged:(NSEvent*)event {
  if (!_engine || !_engine->app()) return;
  NSPoint loc = [self convertPoint:event.locationInWindow fromView:nil];
  float x = (float)loc.x;
  float y = (float)(self.bounds.size.height - loc.y);
  _engine->app()->mouseDragged(x, y, (int)event.buttonNumber);
}

- (void)keyDown:(NSEvent*)event {
  // ESC key (keyCode 53) terminates the app
  if (event.keyCode == 53) {
    [NSApp terminate:nil];
    return;
  }

  if (!_engine || !_engine->app()) return;
  // Use keyCode for now; could map to ASCII or oF key constants later
  _engine->app()->keyPressed((int)event.keyCode);
}

- (void)keyUp:(NSEvent*)event {
  if (!_engine || !_engine->app()) return;
  _engine->app()->keyReleased((int)event.keyCode);
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
    engine.setApp(oflike::createApp());
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
