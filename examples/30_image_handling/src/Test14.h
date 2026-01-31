#pragma once

#include <oflike/ofApp.h>
#include <oflike/image/ofImage.h>
#include <oflike/graphics/ofCoreText.h>

class Test14: public ofBaseApp {
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
    oflike::ofImage testImage;
    oflike::ofImage generatedImage;
    ofCoreText font;
    bool imageLoaded = false;
    int testMode = 0;  // 0: generated, 1: loaded, 2: manipulated
};
