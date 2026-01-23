#include "ofUtils.h"
#include "../../core/Context.h"
#include "../../core/EventDispatcher.h"

// MARK: - Time Functions

float ofGetElapsedTimef() {
    return static_cast<float>(ctx().getElapsedTime());
}

unsigned long long ofGetElapsedTimeMillis() {
    return ctx().getElapsedTimeMillis();
}

unsigned long long ofGetFrameNum() {
    return ctx().getFrameNum();
}

float ofGetFrameRate() {
    return ctx().getFrameRate();
}

void ofSetFrameRate(float targetRate) {
    ctx().setFrameRate(targetRate);
}

// MARK: - Keyboard State Functions

bool ofGetKeyPressed(int key) {
    return ctx().getKeyPressed(key);
}

// MARK: - Mouse State Functions

int ofGetMouseX() {
    return EventDispatcher::instance().getMouseX();
}

int ofGetMouseY() {
    return EventDispatcher::instance().getMouseY();
}

int ofGetPreviousMouseX() {
    return EventDispatcher::instance().getPreviousMouseX();
}

int ofGetPreviousMouseY() {
    return EventDispatcher::instance().getPreviousMouseY();
}

bool ofGetMousePressed(int button) {
    return EventDispatcher::instance().getMousePressed(button);
}

// MARK: - Window Functions

int ofGetWidth() {
    return ctx().getWidth();
}

int ofGetHeight() {
    return ctx().getHeight();
}

int ofGetWindowWidth() {
    return ctx().getWindowWidth();
}

int ofGetWindowHeight() {
    return ctx().getWindowHeight();
}

void ofSetWindowShape(int width, int height) {
    ctx().requestWindowResize(width, height);
}

void ofSetWindowPosition(int x, int y) {
    ctx().requestWindowPosition(x, y);
}

void ofSetWindowTitle(const std::string& title) {
    ctx().requestWindowTitle(title);
}

void ofSetFullscreen(bool fullscreen) {
    ctx().requestFullscreen(fullscreen);
}
