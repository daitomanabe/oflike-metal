#pragma once
#include <oflike/ofApp.h>
#include <oflike/graphics/ofTrueTypeFont.h>
#include <oflike/image/ofImage.h>

class ValidationApp : public ofBaseApp {
public:
    void setup() override;
    void update() override;
    void draw() override;

private:
    ofImage testImage;
    ofTrueTypeFont testFont;
    float angle = 0.0f;
};
