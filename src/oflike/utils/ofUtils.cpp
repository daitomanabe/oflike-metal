#include "ofUtils.h"
#include "../../core/Context.h"

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
