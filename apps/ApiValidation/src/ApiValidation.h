#pragma once

#include <oflike/ofApp.h>
#include <oflike/graphics/ofTrueTypeFont.h>
#include <oflike/image/ofImage.h>

#include <string>
#include <vector>

class ApiValidation: public ofBaseApp {
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

private:
    void drawScene();
    void drawOverlay();
    void drawSceneOverview();
    void drawScenePrimitives();
    void drawSceneTransforms();
    void drawSceneImageText();
    void buildFallbackImage();
    void updateSceneLabel();

    std::vector<std::string> sceneNames_;
    std::string sceneLabel_;
    int currentScene_ = 0;
    float angle_ = 0.0f;
    uint64_t frameCount_ = 0;

    ofTrueTypeFont overlayFont_;
    bool overlayFontLoaded_ = false;
    ofTrueTypeFont contentFont_;
    bool contentFontLoaded_ = false;
    oflike::ofImage testImage_;
    bool testImageLoaded_ = false;
};
