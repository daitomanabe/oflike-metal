# Event System API

## Overview

Event system for mouse, keyboard, and window events. All events are delivered to ofBaseApp callbacks.

---

## Mouse Events

Implemented in ofBaseApp subclass:

```cpp
class MyApp : public ofBaseApp {
    void mouseMoved(int x, int y) override {
        // Mouse moved without button pressed
    }

    void mouseDragged(int x, int y, int button) override {
        // Mouse moved with button pressed
        // button: 0=left, 1=right, 2=middle
    }

    void mousePressed(int x, int y, int button) override {
        // Mouse button pressed
    }

    void mouseReleased(int x, int y, int button) override {
        // Mouse button released
    }

    void mouseScrolled(int x, int y, float scrollX, float scrollY) override {
        // Mouse wheel scrolled
    }

    void mouseEntered(int x, int y) override {
        // Mouse entered window
    }

    void mouseExited(int x, int y) override {
        // Mouse exited window
    }
};
```

---

## Keyboard Events

```cpp
class MyApp : public ofBaseApp {
    void keyPressed(int key) override {
        if (key == 'f') {
            ctx().requestFullscreen(!ctx().isFullscreen());
        }
        if (key == OF_KEY_ESC) {
            // Escape key
        }
    }

    void keyReleased(int key) override {
        // Key released
    }
};
```

### Key Constants

```cpp
OF_KEY_ESC
OF_KEY_RETURN
OF_KEY_TAB
OF_KEY_DELETE
OF_KEY_BACKSPACE
OF_KEY_UP
OF_KEY_DOWN
OF_KEY_LEFT
OF_KEY_RIGHT
OF_KEY_F1 ... OF_KEY_F12
```

---

## Keyboard State Queries

```cpp
// Check if key is currently pressed
if (ctx().getKeyPressed(' ')) {
    // Space bar is down
}

if (ctx().getKeyPressed(OF_KEY_SHIFT)) {
    // Shift is down
}
```

---

## Window Events

```cpp
class MyApp : public ofBaseApp {
    void windowResized(int w, int h) override {
        // Window size changed
    }

    void dragEvent(int x, int y) override {
        // Files dragged onto window
    }
};
```

---

## Example: Interactive Drawing

```cpp
class MyApp : public ofBaseApp {
    vector<ofVec2f> points;
    bool drawing = false;

    void mousePressed(int x, int y, int button) override {
        if (button == 0) {  // Left button
            drawing = true;
            points.clear();
            points.push_back(ofVec2f(x, y));
        }
    }

    void mouseDragged(int x, int y, int button) override {
        if (drawing) {
            points.push_back(ofVec2f(x, y));
        }
    }

    void mouseReleased(int x, int y, int button) override {
        drawing = false;
    }

    void keyPressed(int key) override {
        if (key == 'c') {
            points.clear();  // Clear on 'c'
        }
    }

    void draw() override {
        ofBackground(0);
        ofSetColor(255);
        ofSetLineWidth(3);

        if (points.size() > 1) {
            for (size_t i = 0; i < points.size() - 1; i++) {
                ofDrawLine(points[i].x, points[i].y,
                           points[i+1].x, points[i+1].y);
            }
        }
    }
};
```

---

## See Also

- [00_foundation.md](00_foundation.md) - ofBaseApp lifecycle
- [11_utils.md](11_utils.md) - Utility functions
