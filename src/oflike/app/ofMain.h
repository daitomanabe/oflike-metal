/// \file ofMain.h
/// \brief Main entry point for oflike-metal applications
///
/// Include this header in your main.mm file AFTER defining your App class.
/// It provides the main() function and all necessary Metal/Cocoa boilerplate.
///
/// Usage:
///   // main.mm
///   #include "ofApp.h"  // Your app class (must inherit from ofBaseApp)
///   #define OF_APP ofApp // Define which class to instantiate
///   #include "ofMain.h"  // This provides main()
///
/// Or use the simpler macro:
///   #include "ofApp.h"
///   OF_RUN_APP(ofApp)

#pragma once

#ifndef OF_APP
#error "OF_APP must be defined before including ofMain.h. Example: #define OF_APP MyApp"
#endif

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include "core/AppBase.h"
#include "core/Context.h"
#include "render/metal/MetalRenderer.h"
#include "render/DrawList.h"
#include "oflike/graphics/ofGraphics.h"
#include "oflike/types/ofColor.h"
#include "oflike/utils/ofUtils.h"

// ============================================================================
// Internal: Application State
// ============================================================================

namespace oflike_internal {
    static std::unique_ptr<ofBaseApp> gApp;
    static std::unique_ptr<render::metal::MetalRenderer> gRenderer;
    static bool gInitialized = false;
    static NSString* gWindowTitle = @"oflike-metal";
    static int gWindowWidth = 1024;
    static int gWindowHeight = 768;
}

// ============================================================================
// Configuration Functions (call before OF_RUN_APP or in ofApp constructor)
// ============================================================================

inline void ofSetWindowTitle(const std::string& title) {
    oflike_internal::gWindowTitle = [NSString stringWithUTF8String:title.c_str()];
}

inline void ofSetWindowShape(int width, int height) {
    oflike_internal::gWindowWidth = width;
    oflike_internal::gWindowHeight = height;
}

// ============================================================================
// Internal: Metal View
// ============================================================================

@interface OFMetalView : MTKView <MTKViewDelegate>
@end

@implementation OFMetalView

- (instancetype)initWithFrame:(CGRect)frameRect device:(id<MTLDevice>)device {
    self = [super initWithFrame:frameRect device:device];
    if (self) {
        self.delegate = self;
        self.preferredFramesPerSecond = 60;
        self.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
        self.depthStencilPixelFormat = MTLPixelFormatDepth32Float;
        self.clearColor = MTLClearColorMake(0.1, 0.1, 0.1, 1.0);

        Context::instance().initialize((__bridge void*)device);
        Context::instance().setWindowSize(frameRect.size.width, frameRect.size.height);
    }
    return self;
}

- (void)initializeIfNeeded {
    if (oflike_internal::gInitialized) return;

    id<MTLDevice> device = self.device;
    if (!device) return;

    oflike_internal::gRenderer = std::make_unique<render::metal::MetalRenderer>(
        (__bridge void*)device,
        (__bridge void*)self
    );
    oflike_internal::gRenderer->initialize();

    oflike_internal::gApp = std::make_unique<OF_APP>();
    oflike_internal::gApp->setup();

    oflike_internal::gInitialized = true;
}

- (void)drawInMTKView:(MTKView *)view {
    [self initializeIfNeeded];
    if (!oflike_internal::gApp || !oflike_internal::gRenderer) return;
    if (!oflike_internal::gRenderer->isInitialized()) return;

    Context::instance().incrementFrame();
    oflike_internal::gApp->update();

    if (!oflike_internal::gRenderer->beginFrame()) {
        return;
    }

    oflike_internal::gApp->draw();

    render::DrawList& drawList = Context::instance().getDrawList();
    if (drawList.getCommandCount() > 0) {
        oflike_internal::gRenderer->executeDrawList(drawList);
        drawList.reset();
    }

    oflike_internal::gRenderer->endFrame();
}

- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {
    Context::instance().setWindowSize(size.width, size.height);
    if (oflike_internal::gApp) {
        oflike_internal::gApp->windowResized(size.width, size.height);
    }
    if (oflike_internal::gRenderer) {
        oflike_internal::gRenderer->setViewport(0, 0, size.width, size.height);
    }
}

- (void)keyDown:(NSEvent *)event {
    if (oflike_internal::gApp) {
        oflike_internal::gApp->keyPressed([event keyCode]);
    }
}

- (void)keyUp:(NSEvent *)event {
    if (oflike_internal::gApp) {
        oflike_internal::gApp->keyReleased([event keyCode]);
    }
}

- (void)mouseDown:(NSEvent *)event {
    NSPoint loc = [self convertPoint:[event locationInWindow] fromView:nil];
    if (oflike_internal::gApp) {
        oflike_internal::gApp->mousePressed(loc.x, self.bounds.size.height - loc.y, 0);
    }
}

- (void)mouseUp:(NSEvent *)event {
    NSPoint loc = [self convertPoint:[event locationInWindow] fromView:nil];
    if (oflike_internal::gApp) {
        oflike_internal::gApp->mouseReleased(loc.x, self.bounds.size.height - loc.y, 0);
    }
}

- (void)mouseDragged:(NSEvent *)event {
    NSPoint loc = [self convertPoint:[event locationInWindow] fromView:nil];
    if (oflike_internal::gApp) {
        oflike_internal::gApp->mouseDragged(loc.x, self.bounds.size.height - loc.y, 0);
    }
}

- (void)mouseMoved:(NSEvent *)event {
    NSPoint loc = [self convertPoint:[event locationInWindow] fromView:nil];
    if (oflike_internal::gApp) {
        oflike_internal::gApp->mouseMoved(loc.x, self.bounds.size.height - loc.y);
    }
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

@end

// ============================================================================
// Internal: App Delegate
// ============================================================================

@interface OFAppDelegate : NSObject <NSApplicationDelegate>
@property (strong) NSWindow* window;
@property (strong) OFMetalView* metalView;
@end

@implementation OFAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device) {
        NSLog(@"Metal is not supported");
        [NSApp terminate:nil];
        return;
    }

    int w = oflike_internal::gWindowWidth;
    int h = oflike_internal::gWindowHeight;
    NSRect frame = NSMakeRect(0, 0, w, h);
    NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                       NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable;

    self.window = [[NSWindow alloc] initWithContentRect:frame
                                              styleMask:style
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];
    [self.window setTitle:oflike_internal::gWindowTitle];
    [self.window center];

    self.metalView = [[OFMetalView alloc] initWithFrame:frame device:device];
    [self.window setContentView:self.metalView];
    [self.window makeFirstResponder:self.metalView];
    [self.window makeKeyAndOrderFront:nil];

    [NSApp activateIgnoringOtherApps:YES];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

- (void)applicationWillTerminate:(NSNotification *)notification {
    if (oflike_internal::gApp) {
        oflike_internal::gApp->exit();
    }
    oflike_internal::gApp.reset();
    if (oflike_internal::gRenderer) {
        oflike_internal::gRenderer->shutdown();
        oflike_internal::gRenderer.reset();
    }
    Context::instance().shutdown();
}

@end

// ============================================================================
// Main Entry Point
// ============================================================================

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        NSApplication* app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];

        // Create menu bar
        NSMenu* menuBar = [[NSMenu alloc] init];
        NSMenuItem* appMenuItem = [[NSMenuItem alloc] init];
        [menuBar addItem:appMenuItem];
        NSMenu* appMenu = [[NSMenu alloc] init];
        [appMenu addItemWithTitle:@"Quit" action:@selector(terminate:) keyEquivalent:@"q"];
        [appMenuItem setSubmenu:appMenu];
        [app setMainMenu:menuBar];

        OFAppDelegate* delegate = [[OFAppDelegate alloc] init];
        [app setDelegate:delegate];
        [app run];
    }
    return 0;
}

// ============================================================================
// Convenience Macro
// ============================================================================

#define OF_RUN_APP(AppClass) \
    int main(int argc, const char * argv[]) { \
        @autoreleasepool { \
            NSApplication* app = [NSApplication sharedApplication]; \
            [app setActivationPolicy:NSApplicationActivationPolicyRegular]; \
            NSMenu* menuBar = [[NSMenu alloc] init]; \
            NSMenuItem* appMenuItem = [[NSMenuItem alloc] init]; \
            [menuBar addItem:appMenuItem]; \
            NSMenu* appMenu = [[NSMenu alloc] init]; \
            [appMenu addItemWithTitle:@"Quit" action:@selector(terminate:) keyEquivalent:@"q"]; \
            [appMenuItem setSubmenu:appMenu]; \
            [app setMainMenu:menuBar]; \
            OFAppDelegate* delegate = [[OFAppDelegate alloc] init]; \
            [app setDelegate:delegate]; \
            [app run]; \
        } \
        return 0; \
    }
