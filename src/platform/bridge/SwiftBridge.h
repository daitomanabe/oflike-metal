// SwiftBridge.h - Objective-C++ bridge for Swift to C++ communication
// This header is imported by Swift, so it must be pure Objective-C compatible

#import <Foundation/Foundation.h>

/// Bridge class that connects Swift UI layer to C++ engine
/// This is the Objective-C++ interface exposed to Swift
@interface OFLBridge : NSObject

/// Initialize the bridge
- (instancetype)init;

/// Initialize the global context with Metal device (Phase 2.1)
/// Must be called before setup()
/// @param device The MTLDevice instance
- (void)initializeContextWithDevice:(id)device;

/// Setup the C++ engine and user app
- (void)setup;

/// Update the C++ engine and user app (called every frame before draw)
- (void)update;

/// Draw the C++ engine and user app (called every frame)
- (void)draw;

/// Cleanup and shutdown
- (void)exit;

/// Window resize event
- (void)windowResizedWidth:(float)width height:(float)height;

/// Mouse events
- (void)mouseMovedX:(float)x y:(float)y;
- (void)mouseDraggedX:(float)x y:(float)y button:(int)button;
- (void)mousePressedX:(float)x y:(float)y button:(int)button;
- (void)mouseReleasedX:(float)x y:(float)y button:(int)button;
- (void)mouseScrolledX:(float)x y:(float)y scrollX:(float)scrollX scrollY:(float)scrollY;
- (void)mouseEnteredX:(float)x y:(float)y;
- (void)mouseExitedX:(float)x y:(float)y;

/// Keyboard events
- (void)keyPressed:(int)key;
- (void)keyReleased:(int)key;

@end
