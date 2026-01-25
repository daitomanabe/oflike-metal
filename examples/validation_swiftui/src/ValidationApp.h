#pragma once
#include <core/AppBase.h>
#include <oflike/ofMain.h>

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
