#import "EventDispatcher.h"
#import "AppBase.h"
#import <array>
#import <iostream>

// MARK: - Implementation Details (pImpl)

struct EventDispatcher::Impl {
    // Application instance
    ofBaseApp* app = nullptr;

    // Mouse state
    int mouseX = 0;
    int mouseY = 0;
    int prevMouseX = 0;
    int prevMouseY = 0;

    // Mouse button states (left, right, middle, etc.)
    std::array<bool, 8> mouseButtons = {false};

    /// Update mouse position and store previous position
    void updateMousePosition(int x, int y) {
        prevMouseX = mouseX;
        prevMouseY = mouseY;
        mouseX = x;
        mouseY = y;
    }
};

// MARK: - EventDispatcher Implementation

EventDispatcher::EventDispatcher() : impl_(std::make_unique<Impl>()) {
    std::cout << "[EventDispatcher] Initialized" << std::endl;
}

EventDispatcher::~EventDispatcher() {
    std::cout << "[EventDispatcher] Destroyed" << std::endl;
}

EventDispatcher& EventDispatcher::instance() {
    static EventDispatcher instance;
    return instance;
}

void EventDispatcher::setApp(ofBaseApp* app) {
    impl_->app = app;
    if (app) {
        std::cout << "[EventDispatcher] App instance set" << std::endl;
    } else {
        std::cout << "[EventDispatcher] App instance cleared" << std::endl;
    }
}

ofBaseApp* EventDispatcher::getApp() const {
    return impl_->app;
}

// MARK: - Mouse Events

void EventDispatcher::dispatchMouseMoved(int x, int y) {
    impl_->updateMousePosition(x, y);

    if (impl_->app) {
        impl_->app->mouseMoved(x, y);
    }
}

void EventDispatcher::dispatchMouseDragged(int x, int y, int button) {
    impl_->updateMousePosition(x, y);

    if (impl_->app) {
        impl_->app->mouseDragged(x, y, button);
    }
}

void EventDispatcher::dispatchMousePressed(int x, int y, int button) {
    impl_->updateMousePosition(x, y);

    // Update button state
    if (button >= 0 && button < static_cast<int>(impl_->mouseButtons.size())) {
        impl_->mouseButtons[button] = true;
    }

    if (impl_->app) {
        impl_->app->mousePressed(x, y, button);
    }
}

void EventDispatcher::dispatchMouseReleased(int x, int y, int button) {
    impl_->updateMousePosition(x, y);

    // Update button state
    if (button >= 0 && button < static_cast<int>(impl_->mouseButtons.size())) {
        impl_->mouseButtons[button] = false;
    }

    if (impl_->app) {
        impl_->app->mouseReleased(x, y, button);
    }
}

void EventDispatcher::dispatchMouseScrolled(int x, int y, float scrollX, float scrollY) {
    impl_->updateMousePosition(x, y);

    if (impl_->app) {
        impl_->app->mouseScrolled(x, y, scrollX, scrollY);
    }
}

void EventDispatcher::dispatchMouseEntered(int x, int y) {
    impl_->updateMousePosition(x, y);

    if (impl_->app) {
        impl_->app->mouseEntered(x, y);
    }
}

void EventDispatcher::dispatchMouseExited(int x, int y) {
    impl_->updateMousePosition(x, y);

    if (impl_->app) {
        impl_->app->mouseExited(x, y);
    }
}

// MARK: - Keyboard Events

void EventDispatcher::dispatchKeyPressed(int key) {
    if (impl_->app) {
        impl_->app->keyPressed(key);
    }
}

void EventDispatcher::dispatchKeyReleased(int key) {
    if (impl_->app) {
        impl_->app->keyReleased(key);
    }
}

// MARK: - Window Events

void EventDispatcher::dispatchWindowResized(int w, int h) {
    if (impl_->app) {
        impl_->app->windowResized(w, h);
    }
}

void EventDispatcher::dispatchDragEvent(int x, int y) {
    impl_->updateMousePosition(x, y);

    if (impl_->app) {
        impl_->app->dragEvent(x, y);
    }
}

// MARK: - Mouse State Queries

int EventDispatcher::getMouseX() const {
    return impl_->mouseX;
}

int EventDispatcher::getMouseY() const {
    return impl_->mouseY;
}

int EventDispatcher::getPreviousMouseX() const {
    return impl_->prevMouseX;
}

int EventDispatcher::getPreviousMouseY() const {
    return impl_->prevMouseY;
}

bool EventDispatcher::getMousePressed(int button) const {
    if (button >= 0 && button < static_cast<int>(impl_->mouseButtons.size())) {
        return impl_->mouseButtons[button];
    }
    return false;
}
