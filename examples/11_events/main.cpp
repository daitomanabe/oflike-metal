/*
 * 11_events - Event Handling Demonstration
 *
 * Demonstrates ofBaseApp event handling with:
 * - Mouse events (moved, dragged, pressed, released, scrolled)
 * - Keyboard events (keyPressed, keyReleased)
 * - Window events (windowResized, dragEvent)
 * - Event state queries (ofGetMouseX, ofGetMousePressed, ofGetKeyPressed)
 * - Visual feedback for all events
 * - Event logging
 * - Mouse trail visualization
 * - Keyboard input display
 *
 * Controls:
 * - Mouse: Move, drag, click, scroll
 * - Keyboard: Type any keys
 * - C: Clear event log
 * - T: Toggle mouse trail
 * - L: Toggle event logging
 * - R: Reset demo
 * - Drag files onto window to test dragEvent
 *
 * Integration:
 * - Copy this file to your project
 * - Link against oflike-metal library
 * - Build and run
 */

#include "ofApp.h"
#include "ofGraphics.h"
#include "ofPath.h"
#include "ofTrueTypeFont.h"
#include "ofUtils.h"
#include "ofMath.h"
#include "ofTypes.h"

using namespace oflike;

struct MouseTrailPoint {
    ofVec2f position;
    float time;
    ofColor color;
};

struct EventLogEntry {
    std::string message;
    float time;
    ofColor color;
};

class EventsApp : public ofBaseApp {
public:
    void setup() override;
    void update() override;
    void draw() override;

    // Mouse events
    void mouseMoved(int x, int y) override;
    void mouseDragged(int x, int y, int button) override;
    void mousePressed(int x, int y, int button) override;
    void mouseReleased(int x, int y, int button) override;
    void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
    void mouseEntered(int x, int y) override;
    void mouseExited(int x, int y) override;

    // Keyboard events
    void keyPressed(int key) override;
    void keyReleased(int key) override;

    // Window events
    void windowResized(int w, int h) override;
    void dragEvent(ofDragInfo dragInfo) override;

private:
    void drawMouseInfo();
    void drawKeyboardInfo();
    void drawEventLog();
    void drawMouseTrail();
    void drawClickEffect();
    void drawScrollIndicator();
    void addLogEntry(const std::string& message, const ofColor& color);
    void clearOldTrailPoints();
    void clearOldLogEntries();

    // Visual state
    std::vector<MouseTrailPoint> mouseTrail_;
    std::vector<EventLogEntry> eventLog_;

    // Mouse state
    ofVec2f mousePos_;
    ofVec2f prevMousePos_;
    bool mouseDown_[3] = {false, false, false};  // Left, Middle, Right
    float mouseDownTime_[3] = {0.0f, 0.0f, 0.0f};
    ofVec2f mouseDownPos_[3];
    float scrollAmount_ = 0.0f;
    float scrollDecay_ = 0.0f;

    // Keyboard state
    std::vector<int> pressedKeys_;
    std::string lastKeyPressed_;
    float keyPressTime_ = 0.0f;

    // Settings
    bool showTrail_ = true;
    bool logEvents_ = true;
    float currentTime_ = 0.0f;

    // Font
    ofTrueTypeFont font_;
    ofTrueTypeFont largeFont_;

    // Colors
    ofColor bgColor_;
    ofColor leftButtonColor_;
    ofColor middleButtonColor_;
    ofColor rightButtonColor_;
};

void EventsApp::setup() {
    ofSetFrameRate(60);
    ofSetWindowTitle("11_events - Event Handling");

    bgColor_ = ofColor(30, 30, 35);
    leftButtonColor_ = ofColor(100, 150, 255);
    middleButtonColor_ = ofColor(255, 150, 100);
    rightButtonColor_ = ofColor(150, 255, 100);

    ofBackground(bgColor_);

    // Load fonts
    if (!font_.load("Helvetica", 12)) {
        ofLogWarning("EventsApp") << "Font loading failed (OK for headless)";
    }
    if (!largeFont_.load("Helvetica", 24)) {
        ofLogWarning("EventsApp") << "Large font loading failed (OK for headless)";
    }

    // Initialize
    mousePos_ = ofVec2f(ofGetWidth() / 2, ofGetHeight() / 2);
    prevMousePos_ = mousePos_;

    addLogEntry("Event Demo Started", ofColor(100, 255, 100));
    addLogEntry("Move mouse, click, type, scroll...", ofColor(200, 200, 200));

    ofLogNotice("EventsApp") << "Setup complete";
}

void EventsApp::update() {
    currentTime_ += ofGetLastFrameTime();

    // Decay scroll indicator
    scrollDecay_ *= 0.9f;
    if (fabs(scrollDecay_) < 0.01f) scrollDecay_ = 0.0f;

    // Update mouse position from system
    mousePos_.x = ofGetMouseX();
    mousePos_.y = ofGetMouseY();

    // Add trail point if mouse moved and trail is enabled
    if (showTrail_ && mousePos_ != prevMousePos_) {
        MouseTrailPoint point;
        point.position = mousePos_;
        point.time = currentTime_;

        // Color based on button state
        if (mouseDown_[0]) {
            point.color = leftButtonColor_;
        } else if (mouseDown_[1]) {
            point.color = middleButtonColor_;
        } else if (mouseDown_[2]) {
            point.color = rightButtonColor_;
        } else {
            point.color = ofColor(150, 150, 150);
        }

        mouseTrail_.push_back(point);
    }

    prevMousePos_ = mousePos_;

    // Clean up old trail points
    clearOldTrailPoints();
    clearOldLogEntries();
}

void EventsApp::draw() {
    ofBackground(bgColor_);

    // Draw mouse trail
    if (showTrail_) {
        drawMouseTrail();
    }

    // Draw click effects
    drawClickEffect();

    // Draw scroll indicator
    drawScrollIndicator();

    // Draw info panels
    drawMouseInfo();
    drawKeyboardInfo();
    drawEventLog();

    // Draw crosshair at mouse position
    ofSetColor(255, 200);
    ofSetLineWidth(1);
    ofDrawLine(mousePos_.x - 10, mousePos_.y, mousePos_.x + 10, mousePos_.y);
    ofDrawLine(mousePos_.x, mousePos_.y - 10, mousePos_.x, mousePos_.y + 10);
    ofNoFill();
    ofDrawCircle(mousePos_.x, mousePos_.y, 15);
    ofFill();
}

void EventsApp::drawMouseInfo() {
    if (!font_.isLoaded()) return;

    float x = 20;
    float y = 30;
    float lineHeight = 18;

    ofSetColor(255, 240);
    font_.drawString("MOUSE INFO", x, y);
    y += lineHeight * 1.5f;

    ofSetColor(200);
    font_.drawString("Position: " + ofToString((int)mousePos_.x) + ", " + ofToString((int)mousePos_.y), x, y);
    y += lineHeight;

    font_.drawString("Previous: " + ofToString((int)prevMousePos_.x) + ", " + ofToString((int)prevMousePos_.y), x, y);
    y += lineHeight;

    // Button states
    y += lineHeight * 0.5f;

    for (int i = 0; i < 3; i++) {
        std::string buttonName = (i == 0) ? "Left" : (i == 1) ? "Middle" : "Right";
        ofColor buttonColor = (i == 0) ? leftButtonColor_ : (i == 1) ? middleButtonColor_ : rightButtonColor_;

        if (mouseDown_[i]) {
            ofSetColor(buttonColor);
            font_.drawString(buttonName + ": DOWN", x, y);
        } else {
            ofSetColor(100);
            font_.drawString(buttonName + ": up", x, y);
        }
        y += lineHeight;
    }

    // Mouse state queries
    y += lineHeight * 0.5f;
    ofSetColor(150);
    font_.drawString("ofGetMousePressed(): " + std::string(ofGetMousePressed() ? "true" : "false"), x, y);
    y += lineHeight;

    // Scroll
    if (fabs(scrollDecay_) > 0.01f) {
        ofSetColor(255, 200, 100);
        font_.drawString("Scroll: " + ofToString(scrollDecay_, 2), x, y);
    }
}

void EventsApp::drawKeyboardInfo() {
    if (!font_.isLoaded()) return;

    float x = 20;
    float y = 230;
    float lineHeight = 18;

    ofSetColor(255, 240);
    font_.drawString("KEYBOARD INFO", x, y);
    y += lineHeight * 1.5f;

    ofSetColor(200);

    if (!lastKeyPressed_.empty() && (currentTime_ - keyPressTime_) < 1.0f) {
        ofSetColor(100, 255, 100);
        font_.drawString("Last Key: " + lastKeyPressed_, x, y);
    } else {
        ofSetColor(100);
        font_.drawString("Last Key: (none)", x, y);
    }
    y += lineHeight;

    ofSetColor(150);
    font_.drawString("Pressed Keys: " + ofToString(pressedKeys_.size()), x, y);
    y += lineHeight;

    // Show currently pressed keys
    if (!pressedKeys_.empty()) {
        std::string keysStr = "Currently: ";
        for (size_t i = 0; i < pressedKeys_.size() && i < 10; i++) {
            int key = pressedKeys_[i];
            if (key >= 32 && key < 127) {
                keysStr += (char)key;
            } else {
                keysStr += "[" + ofToString(key) + "]";
            }
            if (i < pressedKeys_.size() - 1) keysStr += ", ";
        }
        if (pressedKeys_.size() > 10) keysStr += "...";

        ofSetColor(200);
        font_.drawString(keysStr, x, y);
    }
    y += lineHeight;

    // Key state query example
    y += lineHeight * 0.5f;
    ofSetColor(150);
    font_.drawString("ofGetKeyPressed('a'): " + std::string(ofGetKeyPressed('a') ? "true" : "false"), x, y);
}

void EventsApp::drawEventLog() {
    if (!font_.isLoaded() || eventLog_.empty()) return;

    float x = ofGetWidth() - 350;
    float y = 30;
    float lineHeight = 18;

    ofSetColor(255, 240);
    font_.drawString("EVENT LOG", x, y);
    y += lineHeight * 1.5f;

    // Draw recent log entries
    size_t maxEntries = 25;
    size_t startIdx = (eventLog_.size() > maxEntries) ? (eventLog_.size() - maxEntries) : 0;

    for (size_t i = startIdx; i < eventLog_.size(); i++) {
        const EventLogEntry& entry = eventLog_[i];
        float age = currentTime_ - entry.time;
        float alpha = ofClamp(1.0f - age / 10.0f, 0.0f, 1.0f) * 255.0f;

        ofColor color = entry.color;
        color.a = alpha;
        ofSetColor(color);

        font_.drawString(entry.message, x, y);
        y += lineHeight;
    }

    // Controls at bottom
    y = ofGetHeight() - 100;
    ofSetColor(150);
    font_.drawString("CONTROLS", x, y);
    y += lineHeight * 1.5f;

    ofSetColor(100);
    font_.drawString("C: Clear log", x, y);
    y += lineHeight;
    font_.drawString("T: Toggle trail (" + std::string(showTrail_ ? "ON" : "OFF") + ")", x, y);
    y += lineHeight;
    font_.drawString("L: Toggle logging (" + std::string(logEvents_ ? "ON" : "OFF") + ")", x, y);
    y += lineHeight;
    font_.drawString("R: Reset demo", x, y);
}

void EventsApp::drawMouseTrail() {
    if (mouseTrail_.empty()) return;

    ofSetLineWidth(2);

    for (size_t i = 1; i < mouseTrail_.size(); i++) {
        const MouseTrailPoint& p1 = mouseTrail_[i - 1];
        const MouseTrailPoint& p2 = mouseTrail_[i];

        float age = currentTime_ - p2.time;
        float alpha = ofClamp(1.0f - age / 2.0f, 0.0f, 1.0f) * 200.0f;

        ofColor color = p2.color;
        color.a = alpha;
        ofSetColor(color);

        ofDrawLine(p1.position.x, p1.position.y, p2.position.x, p2.position.y);
    }

    ofSetLineWidth(1);
}

void EventsApp::drawClickEffect() {
    for (int i = 0; i < 3; i++) {
        if (mouseDown_[i]) {
            float elapsed = currentTime_ - mouseDownTime_[i];
            if (elapsed < 0.5f) {
                ofColor color = (i == 0) ? leftButtonColor_ : (i == 1) ? middleButtonColor_ : rightButtonColor_;
                float alpha = (1.0f - elapsed / 0.5f) * 150.0f;
                color.a = alpha;

                ofSetColor(color);
                ofNoFill();
                ofSetLineWidth(3);
                float radius = 20.0f + elapsed * 60.0f;
                ofDrawCircle(mouseDownPos_[i].x, mouseDownPos_[i].y, radius);
                ofFill();
                ofSetLineWidth(1);
            }
        }
    }
}

void EventsApp::drawScrollIndicator() {
    if (fabs(scrollDecay_) < 0.01f) return;

    float centerX = ofGetWidth() / 2;
    float centerY = ofGetHeight() / 2;

    ofSetColor(255, 200, 100, 150);
    ofNoFill();
    ofSetLineWidth(3);

    if (scrollDecay_ > 0) {
        // Scroll up arrow
        ofDrawTriangle(centerX, centerY - 50 - scrollDecay_ * 10,
                      centerX - 20, centerY - 30 - scrollDecay_ * 10,
                      centerX + 20, centerY - 30 - scrollDecay_ * 10);
    } else {
        // Scroll down arrow
        ofDrawTriangle(centerX, centerY + 50 - scrollDecay_ * 10,
                      centerX - 20, centerY + 30 - scrollDecay_ * 10,
                      centerX + 20, centerY + 30 - scrollDecay_ * 10);
    }

    ofFill();
    ofSetLineWidth(1);
}

void EventsApp::addLogEntry(const std::string& message, const ofColor& color) {
    if (!logEvents_) return;

    EventLogEntry entry;
    entry.message = message;
    entry.time = currentTime_;
    entry.color = color;

    eventLog_.push_back(entry);

    // Limit log size
    if (eventLog_.size() > 100) {
        eventLog_.erase(eventLog_.begin());
    }
}

void EventsApp::clearOldTrailPoints() {
    while (!mouseTrail_.empty() && (currentTime_ - mouseTrail_.front().time) > 2.0f) {
        mouseTrail_.erase(mouseTrail_.begin());
    }
}

void EventsApp::clearOldLogEntries() {
    while (!eventLog_.empty() && (currentTime_ - eventLog_.front().time) > 30.0f) {
        eventLog_.erase(eventLog_.begin());
    }
}

// Mouse event handlers

void EventsApp::mouseMoved(int x, int y) {
    addLogEntry("mouseMoved(" + ofToString(x) + ", " + ofToString(y) + ")", ofColor(150, 150, 150));
}

void EventsApp::mouseDragged(int x, int y, int button) {
    ofColor color = (button == 0) ? leftButtonColor_ : (button == 1) ? middleButtonColor_ : rightButtonColor_;
    std::string buttonName = (button == 0) ? "Left" : (button == 1) ? "Middle" : "Right";
    addLogEntry("mouseDragged(" + ofToString(x) + ", " + ofToString(y) + ", " + buttonName + ")", color);
}

void EventsApp::mousePressed(int x, int y, int button) {
    if (button >= 0 && button < 3) {
        mouseDown_[button] = true;
        mouseDownTime_[button] = currentTime_;
        mouseDownPos_[button] = ofVec2f(x, y);
    }

    ofColor color = (button == 0) ? leftButtonColor_ : (button == 1) ? middleButtonColor_ : rightButtonColor_;
    std::string buttonName = (button == 0) ? "Left" : (button == 1) ? "Middle" : "Right";
    addLogEntry("mousePressed(" + ofToString(x) + ", " + ofToString(y) + ", " + buttonName + ")", color);

    ofLogNotice("EventsApp") << "Mouse pressed: button=" << button << " at (" << x << ", " << y << ")";
}

void EventsApp::mouseReleased(int x, int y, int button) {
    if (button >= 0 && button < 3) {
        mouseDown_[button] = false;
    }

    ofColor color = (button == 0) ? leftButtonColor_ : (button == 1) ? middleButtonColor_ : rightButtonColor_;
    std::string buttonName = (button == 0) ? "Left" : (button == 1) ? "Middle" : "Right";
    addLogEntry("mouseReleased(" + ofToString(x) + ", " + ofToString(y) + ", " + buttonName + ")", color);

    ofLogNotice("EventsApp") << "Mouse released: button=" << button << " at (" << x << ", " << y << ")";
}

void EventsApp::mouseScrolled(int x, int y, float scrollX, float scrollY) {
    scrollAmount_ += scrollY;
    scrollDecay_ = scrollY * 5.0f;

    addLogEntry("mouseScrolled(" + ofToString(scrollX, 2) + ", " + ofToString(scrollY, 2) + ")", ofColor(255, 200, 100));

    ofLogNotice("EventsApp") << "Mouse scrolled: (" << scrollX << ", " << scrollY << ") at (" << x << ", " << y << ")";
}

void EventsApp::mouseEntered(int x, int y) {
    addLogEntry("mouseEntered(" + ofToString(x) + ", " + ofToString(y) + ")", ofColor(100, 255, 100));
    ofLogNotice("EventsApp") << "Mouse entered window at (" << x << ", " << y << ")";
}

void EventsApp::mouseExited(int x, int y) {
    addLogEntry("mouseExited(" + ofToString(x) + ", " + ofToString(y) + ")", ofColor(255, 100, 100));
    ofLogNotice("EventsApp") << "Mouse exited window at (" << x << ", " << y << ")";
}

// Keyboard event handlers

void EventsApp::keyPressed(int key) {
    // Handle special controls
    switch (key) {
        case 'c':
        case 'C':
            eventLog_.clear();
            addLogEntry("Event log cleared", ofColor(255, 255, 100));
            return;

        case 't':
        case 'T':
            showTrail_ = !showTrail_;
            addLogEntry("Mouse trail: " + std::string(showTrail_ ? "ON" : "OFF"), ofColor(255, 255, 100));
            return;

        case 'l':
        case 'L':
            logEvents_ = !logEvents_;
            addLogEntry("Event logging: " + std::string(logEvents_ ? "ON" : "OFF"), ofColor(255, 255, 100));
            return;

        case 'r':
        case 'R':
            mouseTrail_.clear();
            eventLog_.clear();
            scrollAmount_ = 0;
            scrollDecay_ = 0;
            pressedKeys_.clear();
            addLogEntry("Demo reset", ofColor(255, 255, 100));
            return;
    }

    // Add to pressed keys list
    pressedKeys_.push_back(key);

    // Format key name
    std::string keyName;
    if (key >= 32 && key < 127) {
        keyName = "'" + std::string(1, (char)key) + "'";
    } else if (key == OF_KEY_RETURN) {
        keyName = "RETURN";
    } else if (key == OF_KEY_ESC) {
        keyName = "ESC";
    } else if (key == OF_KEY_TAB) {
        keyName = "TAB";
    } else if (key == OF_KEY_BACKSPACE) {
        keyName = "BACKSPACE";
    } else if (key == OF_KEY_DEL) {
        keyName = "DELETE";
    } else if (key == OF_KEY_LEFT) {
        keyName = "LEFT";
    } else if (key == OF_KEY_RIGHT) {
        keyName = "RIGHT";
    } else if (key == OF_KEY_UP) {
        keyName = "UP";
    } else if (key == OF_KEY_DOWN) {
        keyName = "DOWN";
    } else {
        keyName = "KEY[" + ofToString(key) + "]";
    }

    lastKeyPressed_ = keyName;
    keyPressTime_ = currentTime_;

    addLogEntry("keyPressed(" + keyName + ")", ofColor(100, 200, 255));

    ofLogNotice("EventsApp") << "Key pressed: " << key << " (" << keyName << ")";
}

void EventsApp::keyReleased(int key) {
    // Remove from pressed keys list
    auto it = std::find(pressedKeys_.begin(), pressedKeys_.end(), key);
    if (it != pressedKeys_.end()) {
        pressedKeys_.erase(it);
    }

    // Format key name
    std::string keyName;
    if (key >= 32 && key < 127) {
        keyName = "'" + std::string(1, (char)key) + "'";
    } else {
        keyName = "KEY[" + ofToString(key) + "]";
    }

    addLogEntry("keyReleased(" + keyName + ")", ofColor(150, 180, 255));

    ofLogNotice("EventsApp") << "Key released: " << key;
}

// Window event handlers

void EventsApp::windowResized(int w, int h) {
    addLogEntry("windowResized(" + ofToString(w) + ", " + ofToString(h) + ")", ofColor(255, 150, 255));
    ofLogNotice("EventsApp") << "Window resized: " << w << "x" << h;
}

void EventsApp::dragEvent(ofDragInfo dragInfo) {
    std::string message = "dragEvent: " + ofToString(dragInfo.files.size()) + " file(s)";
    addLogEntry(message, ofColor(255, 200, 100));

    ofLogNotice("EventsApp") << "Drag event: " << dragInfo.files.size() << " files at ("
                             << dragInfo.position.x << ", " << dragInfo.position.y << ")";

    for (size_t i = 0; i < dragInfo.files.size(); i++) {
        ofLogNotice("EventsApp") << "  [" << i << "] " << dragInfo.files[i];
    }
}

int main() {
    ofSetupOpenGL(1280, 720, OF_WINDOW);
    ofRunApp(new EventsApp());
    return 0;
}
