#pragma once

#include <oflike/ofApp.h>
#include <oflike/3d/ofEasyCam.h>
#include <oflike/graphics/ofCoreText.h>

class Test13: public ofBaseApp {
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
    oflike::ofEasyCam cam;
    ofCoreText font;
    bool show3D = false;  // Toggle between 2D and 3D view
};
