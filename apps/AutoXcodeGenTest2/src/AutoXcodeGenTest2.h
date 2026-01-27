#pragma once

#include <core/AppBase.h>
#include <oflike/graphics/ofGraphics.h>
#include <oflike/types/ofColor.h>
#include <oflike/utils/ofUtils.h>

class AutoXcodeGenTest2: public ofBaseApp {
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
    void windowResized(int w, int h);
};
