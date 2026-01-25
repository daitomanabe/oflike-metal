#import "SwiftBridge.h"
#include <memory>
#include <iostream>
#include "../../core/TestApp.h"
#include "../../core/Context.h"
#include "../../core/EventDispatcher.h"
#include "../../render/metal/MetalRenderer.h"  // Phase 16.2: For performance stats

@interface OFLBridge() {
    // Phase 2.1: User app instance (created via factory)
    std::unique_ptr<ofBaseApp> userApp_;

    // Phase 2.1: Factory function to create user app
    OfAppFactoryFunc appFactory_;

    bool isSetup_;
}
@end

@implementation OFLBridge

- (instancetype)init {
    self = [super init];
    if (self) {
        isSetup_ = false;
        appFactory_ = nullptr;
        std::cout << "[OFLBridge] Initialized" << std::endl;
    }
    return self;
}

- (void)setAppFactory:(OfAppFactoryFunc)factory {
    @autoreleasepool {
        appFactory_ = factory;
        std::cout << "[OFLBridge] App factory registered" << std::endl;
    }
}

- (void)initializeContextWithDevice:(id)device {
    @autoreleasepool {
        std::cout << "[OFLBridge] Initializing global context with Metal device" << std::endl;

        // Initialize global Context singleton with Metal device
        Context::instance().initialize((__bridge void*)device);

        if (Context::instance().isInitialized()) {
            std::cout << "[OFLBridge] Context initialization successful" << std::endl;
        } else {
            std::cerr << "[OFLBridge] Context initialization failed!" << std::endl;
        }
    }
}

- (void)dealloc {
    [self exit];
}

- (void)setup {
    @autoreleasepool {
        if (isSetup_) {
            return;
        }

        std::cout << "[OFLBridge] Setup called" << std::endl;

        // Phase 2.1: Create user app via factory
        if (appFactory_) {
            userApp_.reset(appFactory_());
            std::cout << "[OFLBridge] User app created via factory" << std::endl;
        } else {
            // Fallback: Create TestApp if no factory is registered
            // This maintains backward compatibility for Phase 1 verification
            std::cout << "[OFLBridge] WARNING: No app factory registered, using TestApp fallback" << std::endl;
            userApp_ = std::make_unique<TestApp>();
        }

        // Phase 13.4: Register app with EventDispatcher
        EventDispatcher::instance().setApp(userApp_.get());

        userApp_->setup();

        isSetup_ = true;
        std::cout << "[OFLBridge] Setup complete" << std::endl;
    }
}

- (void)update {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        // Phase 2.1: Increment frame counter in context
        Context::instance().incrementFrame();

        // Phase 2.1: Update user app
        if (userApp_) {
            userApp_->update();
        }
    }
}

- (void)draw {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        // Phase 2.1: Draw user app
        if (userApp_) {
            userApp_->draw();
        }
    }
}

- (void)exit {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        std::cout << "[OFLBridge] Exit called" << std::endl;

        // Phase 2.1: Cleanup user app
        if (userApp_) {
            userApp_->exit();
            userApp_.reset();
        }

        // Phase 13.4: Clear EventDispatcher app reference
        EventDispatcher::instance().setApp(nullptr);

        // Phase 2.1: Shutdown global context
        Context::instance().shutdown();

        isSetup_ = false;
    }
}

// MARK: - Window Events

- (void)windowResizedWidth:(float)width height:(float)height {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        // Phase 2.1: Update global context window size
        Context::instance().setWindowSize((int)width, (int)height);

        // Phase 13.4: Dispatch through EventDispatcher
        EventDispatcher::instance().dispatchWindowResized((int)width, (int)height);
    }
}

// MARK: - Mouse Events

- (void)mouseMovedX:(float)x y:(float)y {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        // Phase 13.4: Dispatch through EventDispatcher
        EventDispatcher::instance().dispatchMouseMoved((int)x, (int)y);
    }
}

- (void)mouseDraggedX:(float)x y:(float)y button:(int)button {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        // Phase 13.4: Dispatch through EventDispatcher
        EventDispatcher::instance().dispatchMouseDragged((int)x, (int)y, button);
    }
}

- (void)mousePressedX:(float)x y:(float)y button:(int)button {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        // Phase 13.4: Dispatch through EventDispatcher
        EventDispatcher::instance().dispatchMousePressed((int)x, (int)y, button);
    }
}

- (void)mouseReleasedX:(float)x y:(float)y button:(int)button {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        // Phase 13.4: Dispatch through EventDispatcher
        EventDispatcher::instance().dispatchMouseReleased((int)x, (int)y, button);
    }
}

- (void)mouseScrolledX:(float)x y:(float)y scrollX:(float)scrollX scrollY:(float)scrollY {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        // Phase 13.4: Dispatch through EventDispatcher
        EventDispatcher::instance().dispatchMouseScrolled((int)x, (int)y, scrollX, scrollY);
    }
}

- (void)mouseEnteredX:(float)x y:(float)y {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        // Phase 13.4: Dispatch through EventDispatcher
        EventDispatcher::instance().dispatchMouseEntered((int)x, (int)y);
    }
}

- (void)mouseExitedX:(float)x y:(float)y {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        // Phase 13.4: Dispatch through EventDispatcher
        EventDispatcher::instance().dispatchMouseExited((int)x, (int)y);
    }
}

// MARK: - Keyboard Events

- (void)keyPressed:(int)key {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        // Phase 13.2: Update keyboard state in Context
        Context::instance().setKeyState(key, true);

        // Phase 13.4: Dispatch through EventDispatcher
        EventDispatcher::instance().dispatchKeyPressed(key);
    }
}

- (void)keyReleased:(int)key {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        // Phase 13.2: Update keyboard state in Context
        Context::instance().setKeyState(key, false);

        // Phase 13.4: Dispatch through EventDispatcher
        EventDispatcher::instance().dispatchKeyReleased(key);
    }
}

// MARK: - Drag and Drop Events

- (void)dragEventX:(float)x y:(float)y {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        // Phase 13.4: Dispatch through EventDispatcher
        EventDispatcher::instance().dispatchDragEvent((int)x, (int)y);
    }
}

// MARK: - Window Control

- (void)setWindowResizeCallback:(void (*)(int width, int height))callback {
    @autoreleasepool {
        // Phase 14.1: Register callback in Context
        Context::instance().setWindowResizeCallback(callback);
    }
}

- (void)setWindowPositionCallback:(void (*)(int x, int y))callback {
    @autoreleasepool {
        // Phase 14.1: Register callback in Context
        Context::instance().setWindowPositionCallback(callback);
    }
}

- (void)setWindowTitleCallback:(void (*)(const char* title))callback {
    @autoreleasepool {
        // Phase 14.1: Register callback in Context
        Context::instance().setWindowTitleCallback(callback);
    }
}

- (void)setFullscreenCallback:(void (*)(bool fullscreen))callback {
    @autoreleasepool {
        // Phase 14.1: Register fullscreen callback in Context
        Context::instance().setFullscreenCallback(callback);
    }
}

- (void)setFullscreenState:(bool)fullscreen {
    @autoreleasepool {
        // Phase 14.1: Update fullscreen state in Context
        Context::instance().setFullscreenState(fullscreen);
    }
}

- (void)getPerformanceStats:(uint32_t*)outDrawCalls
                   vertices:(uint32_t*)outVertices
                    gpuTime:(double*)outGPUTime {
    @autoreleasepool {
        // Phase 16.2: Get performance statistics from renderer
        if (outDrawCalls && outVertices && outGPUTime) {
            auto* renderer = Context::instance().renderer();
            if (renderer) {
                renderer->getStatistics(*outDrawCalls, *outVertices);
                *outGPUTime = renderer->getLastGPUTime();
            } else {
                *outDrawCalls = 0;
                *outVertices = 0;
                *outGPUTime = 0.0;
            }
        }
    }
}

@end
