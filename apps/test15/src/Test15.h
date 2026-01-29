#pragma once

#include <oflike/ofApp.h>
#include <oflike/graphics/ofCoreText.h>
#include <oflike/3d/ofEasyCam.h>

class Test15: public ofBaseApp {
public:
    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseScrolled(int x, int y, float scrollX, float scrollY);
    void windowResized(int w, int h);

private:
    ofCoreText font;
    oflike::ofEasyCam cam;
    int testMode = 0;  // 0: vectors, 1: matrix, 2: quaternion, 3: math functions
};
