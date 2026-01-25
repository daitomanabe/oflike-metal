// SwiftBridge.h - Objective-C++ bridge for Swift to C++ communication
// This header is imported by Swift, so it must be pure Objective-C compatible

#import <Foundation/Foundation.h>

// Forward declaration of app factory function pointer type
// This allows users to register their app creation function
#ifdef __cplusplus
class ofBaseApp;
typedef ofBaseApp* (*OfAppFactoryFunc)();
#endif

/// Bridge class that connects Swift UI layer to C++ engine
/// This is the Objective-C++ interface exposed to Swift
@interface OFLBridge : NSObject

/// Initialize the bridge
- (instancetype)init;

/// Initialize the global context with Metal device (Phase 2.1)
/// Must be called before setup()
/// @param device The MTLDevice instance
- (void)initializeContextWithDevice:(id)device;

#ifdef __cplusplus
/// Set the app factory function (Phase 2.1)
/// This function will be called to create the user's app instance
/// @param factory Function pointer that creates and returns an ofBaseApp instance
- (void)setAppFactory:(OfAppFactoryFunc)factory;
#endif

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

/// Drag and drop event
/// @param x Drop position x in pixels
/// @param y Drop position y in pixels
- (void)dragEventX:(float)x y:(float)y;

/// Set window resize callback
/// @param callback C function pointer for window resize requests
- (void)setWindowResizeCallback:(void (*)(int width, int height))callback;

/// Set window position callback
/// @param callback C function pointer for window position change requests
- (void)setWindowPositionCallback:(void (*)(int x, int y))callback;

/// Set window title callback
/// @param callback C function pointer for window title change requests
- (void)setWindowTitleCallback:(void (*)(const char* title))callback;

/// Set fullscreen callback
/// @param callback C function pointer for fullscreen change requests
- (void)setFullscreenCallback:(void (*)(bool fullscreen))callback;

/// Update fullscreen state (called when fullscreen mode changes)
/// @param fullscreen true if fullscreen mode is active, false otherwise
- (void)setFullscreenState:(bool)fullscreen;

/// Get performance statistics for current frame
/// @param outDrawCalls Pointer to receive draw call count
/// @param outVertices Pointer to receive vertex count
/// @param outGPUTime Pointer to receive GPU time in milliseconds
- (void)getPerformanceStats:(uint32_t*)outDrawCalls
                   vertices:(uint32_t*)outVertices
                    gpuTime:(double*)outGPUTime;

@end
