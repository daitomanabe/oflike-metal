#pragma once

#include <oflike/ofApp.h>
#include <oflike/graphics/ofCoreText.h>
#include <oflike/3d/ofEasyCam.h>
#include <oflike/lighting/ofLight.h>
#include <oflike/lighting/ofMaterial.h>

class Test16: public ofBaseApp {
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

    ofLight pointLight;
    ofLight directionalLight;
    ofLight spotLight;

    ofMaterial material;

    int lightMode = 0;  // 0: point, 1: directional, 2: spot, 3: all
};
