# Utilities API

## Overview

Utility functions for strings, files, logging, and system information.

## Headers

```cpp
#include <oflike/utils/ofUtils.h>
#include <oflike/utils/ofLog.h>
#include <oflike/utils/ofFile.h>
```

---

## Time Functions

```cpp
// Timing
double t = ofGetElapsedTimef();          // Seconds since start
uint64_t ms = ofGetElapsedTimeMillis();  // Milliseconds
uint64_t frame = ofGetFrameNum();        // Current frame number
float fps = ofGetFrameRate();            // Current FPS

// Frame rate control
ofSetFrameRate(60);                      // Target 60 FPS
```

---

## String Utilities

```cpp
// Conversion
string s = ofToString(42);
string s = ofToString(3.14159, 2);       // 2 decimal places
int i = ofToInt("123");
float f = ofToFloat("3.14");

// String manipulation
string upper = ofToUpper("hello");       // "HELLO"
string lower = ofToLower("HELLO");       // "hello"
vector<string> parts = ofSplitString("a,b,c", ",");
```

---

## Random

```cpp
// Random numbers
float r = ofRandom(1.0f);                // 0.0 to 1.0
float r = ofRandom(-10, 10);             // -10 to 10
int i = ofRandomi(100);                  // 0 to 99

// Seed
ofSeedRandom();                          // Random seed
ofSeedRandom(12345);                     // Fixed seed
```

---

## Math Utilities

```cpp
// Mapping
float mapped = ofMap(value, 0, 100, 0, 1);  // Remap range
float clamped = ofClamp(value, 0, 1);       // Constrain
float lerped = ofLerp(a, b, 0.5f);          // Linear interpolation

// Trigonometry
float rad = ofDegToRad(180);             // 3.14159...
float deg = ofRadToDeg(PI);              // 180

// Distance
float d = ofDist(x1, y1, x2, y2);        // 2D distance
float d = ofDist(x1, y1, z1, x2, y2, z2); // 3D distance
```

---

## Noise

```cpp
// Perlin noise
float n = ofNoise(x);                    // 1D (0.0-1.0)
float n = ofNoise(x, y);                 // 2D
float n = ofNoise(x, y, z);              // 3D
float n = ofSignedNoise(x);              // -1.0 to 1.0
```

---

## Logging

```cpp
// Log messages
ofLogNotice() << "Info message";
ofLogWarning() << "Warning message";
ofLogError() << "Error message";
ofLogFatalError() << "Fatal error";

// Log with values
ofLogNotice() << "FPS: " << ofGetFrameRate();
```

---

## File System

```cpp
// File operations
ofFile file("data/config.txt");
bool exists = file.exists();
string contents = file.readToBuffer().getText();

// Directory operations
ofDirectory dir("data/images");
dir.listDir();
for (size_t i = 0; i < dir.size(); i++) {
    string path = dir.getPath(i);
}

// Path utilities
string abs = ofFilePath::getAbsolutePath("data/file.txt");
string ext = ofFilePath::getFileExt(path);
string name = ofFilePath::getFileName(path);
```

---

## System Info

```cpp
// Window
int w = ofGetWindowWidth();
int h = ofGetWindowHeight();

// Screen
int sw = ofGetScreenWidth();
int sh = ofGetScreenHeight();

// Cursor
ofHideCursor();
ofShowCursor();
```

---

## Example: Data Visualization

```cpp
class MyApp : public ofBaseApp {
    vector<float> data;

    void setup() override {
        ofSetFrameRate(60);
        ofSeedRandom();

        // Generate random data
        for (int i = 0; i < 100; i++) {
            data.push_back(ofRandom(100, 500));
        }
    }

    void update() override {
        // Animate data with noise
        float t = ofGetElapsedTimef();
        for (size_t i = 0; i < data.size(); i++) {
            float n = ofNoise(i * 0.1f, t * 0.5f);
            data[i] = ofMap(n, 0, 1, 100, 500);
        }
    }

    void draw() override {
        ofBackground(20);

        // Draw bar chart
        float barWidth = ofGetWindowWidth() / (float)data.size();
        for (size_t i = 0; i < data.size(); i++) {
            float x = i * barWidth;
            float h = data[i];

            // Color based on value
            float hue = ofMap(h, 100, 500, 0, 255);
            ofSetColor(hue, 200, 200);
            ofDrawRectangle(x, ofGetWindowHeight() - h, barWidth - 2, h);
        }

        // Draw FPS
        ofSetColor(255);
        ofDrawBitmapString("FPS: " + ofToString(ofGetFrameRate(), 1),
                           10, 20);
    }

    void keyPressed(int key) override {
        if (key == 'r') {
            // Regenerate data
            for (size_t i = 0; i < data.size(); i++) {
                data[i] = ofRandom(100, 500);
            }
        }
    }
};
```

---

## See Also

- [00_foundation.md](00_foundation.md) - Core classes
- [01_math.md](01_math.md) - Math utilities
