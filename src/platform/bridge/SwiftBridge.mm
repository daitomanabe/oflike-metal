#import "SwiftBridge.h"
#include <memory>
#include <iostream>
#include "../../core/TestApp.h"

@interface OFLBridge() {
    // C++ test app instance for Phase 1.4 verification
    std::unique_ptr<TestApp> testApp_;

    bool isSetup_;
}
@end

@implementation OFLBridge

- (instancetype)init {
    self = [super init];
    if (self) {
        isSetup_ = false;
        std::cout << "[OFLBridge] Initialized" << std::endl;
    }
    return self;
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

        // Phase 1.4: Test C++ integration
        testApp_ = std::make_unique<TestApp>();
        testApp_->setup();

        isSetup_ = true;
        std::cout << "[OFLBridge] Setup complete - Swift → C++ calls verified!" << std::endl;
    }
}

- (void)update {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        // Phase 1.4: Test C++ update
        if (testApp_) {
            testApp_->update();
        }
    }
}

- (void)draw {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        // Phase 1.4: Test C++ draw
        if (testApp_) {
            testApp_->draw();
        }
    }
}

- (void)exit {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        std::cout << "[OFLBridge] Exit called" << std::endl;

        // Phase 1.4: Cleanup test app
        if (testApp_) {
            testApp_->exit();
            testApp_.reset();
        }

        isSetup_ = false;
    }
}

// MARK: - Window Events

- (void)windowResizedWidth:(float)width height:(float)height {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        // Phase 1.4: Test C++ callback
        if (testApp_) {
            testApp_->windowResized((int)width, (int)height);
        }
    }
}

// MARK: - Mouse Events

- (void)mouseMovedX:(float)x y:(float)y {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        // TODO Phase 13: Forward to C++ event system
        // engine_->mouseMoved((int)x, (int)y);
        // app_->mouseMoved((int)x, (int)y);
    }
}

- (void)mouseDraggedX:(float)x y:(float)y button:(int)button {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        // TODO Phase 13: Forward to C++ event system
        // engine_->mouseDragged((int)x, (int)y, button);
        // app_->mouseDragged((int)x, (int)y, button);
    }
}

- (void)mousePressedX:(float)x y:(float)y button:(int)button {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        // TODO Phase 13: Forward to C++ event system
        // engine_->mousePressed((int)x, (int)y, button);
        // app_->mousePressed((int)x, (int)y, button);
    }
}

- (void)mouseReleasedX:(float)x y:(float)y button:(int)button {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        // TODO Phase 13: Forward to C++ event system
        // engine_->mouseReleased((int)x, (int)y, button);
        // app_->mouseReleased((int)x, (int)y, button);
    }
}

- (void)mouseScrolledX:(float)x y:(float)y {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        // TODO Phase 13: Forward to C++ event system
        // engine_->mouseScrolled(x, y);
        // app_->mouseScrolled(x, y);
    }
}

// MARK: - Keyboard Events

- (void)keyPressed:(int)key {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        // TODO Phase 13: Forward to C++ event system
        // engine_->keyPressed(key);
        // app_->keyPressed(key);
    }
}

- (void)keyReleased:(int)key {
    @autoreleasepool {
        if (!isSetup_) {
            return;
        }

        // TODO Phase 13: Forward to C++ event system
        // engine_->keyReleased(key);
        // app_->keyReleased(key);
    }
}

@end
