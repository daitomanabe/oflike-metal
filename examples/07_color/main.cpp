// Ported from openFrameworks colorExample
// Demonstrates HSB color mode and color manipulation

#include "../../src/oflike/ofMain.h"

class ColorExampleApp : public AppBase {
public:
  void setup() override {
    ofBackground(0, 0, 0);
    ofEnableSmoothing();
    ofEnableAlphaBlending();
    ofSetRectMode(OF_RECTMODE_CENTER);

    // initialise member variables to the centre of the screen
    mouseXPercent = 0.5f;
    mouseYPercent = 0.5f;
  }

  void update() override {
  }

  void draw() override {
    // here we demonstrate setting colors using HSB (Hue/Saturation/Brightness) rather than the
    // more well-known RGB (Red/Green/Blue).

    // HSB allows colors to be specified in a way that is perhaps more natural to the understanding
    // of color that we have through language, using numerical values to describe 'hue',
    // 'saturation' and 'brightness'.

    // 'hue' refers to the 'color' in the rainbow sense, moving from red through yellow through
    //   green through blue through purple through red, looping around again.
    // 'saturation' refers to the intensity of the color. high saturation means intense color,
    //   low saturation means washed out or black and white.
    // 'brightness' refers to how light or dark the color is. high brightness means a bright color,
    //   low brightness is dark. if the brightness is 0 the resulting color will be black, regardless
    //   of the values for hue or saturation.

    // we want to draw a grid of 5 pixel x 5 pixel rectangles with a fixed hue, varying in
    // saturation and brightness over X and Y

    // we use one hue (value from 0..255) for the whole grid. it changes over time.
    float hue = fmodf(ofGetElapsedTimef() * 10, 255);

    int step = 5;
    int w = ofGetWidth();
    int h = ofGetHeight();

    // step through horizontally
    for (int i = 0; i < w; i += step) {
      // step through vertically
      for (int j = 0; j < h; j += step) {
        // set HSB using our hue value that changes over time, saturation from the X position (i),
        // and brightness from the Y position (j). we also invert the Y value since it looks
        // nicer if the dark/black colors are along the bottom.
        ofColor c;
        // the range of each of the arguments here is 0..255 so we map i and j to that range.
        c.setHsb(hue, ofMap(i, 0, w, 0, 255), ofMap(j, h, 0, 0, 255));

        // assign the color and draw a rectangle
        ofSetColor(c);
        ofDrawRectangle(i, j, step - 1, step - 1);
      }
    }

    // now we will draw a larger rectangle taking the color under the mouse
    int mouseX = ofGetMouseX();
    int mouseY = ofGetMouseY();

    // calculate the color under the mouse, using the same calculations as when drawing the grid
    ofColor color = ofColor::fromHsb(hue,
                                     ofMap(mouseX, 0, w, 0, 255),
                                     ofMap(mouseY, h, 0, 0, 255));
    ofSetColor(color);
    ofFill();
    ofDrawRectangle(mouseX, mouseY, 100, 100);

    // now draw a white border around the rectangle
    ofNoFill();
    ofSetHexColor(0xFFFFFF);
    ofDrawRectangle(mouseX, mouseY, 100, 100);
    ofFill();

    // finally we draw text over the rectangle giving the resulting HSB and RGB values
    // under the mouse
    ofSetHexColor(0xFFFFFF);
    ofDrawBitmapString("HSB: " + ofToString(int(hue)) +
                       " " + ofToString(int(color.getSaturation())) +
                       " " + ofToString(int(color.getBrightness())),
                       10, h - 30);
    ofDrawBitmapString("RGB: " + ofToString(int(color.r)) +
                       " " + ofToString(int(color.g)) +
                       " " + ofToString(int(color.b)),
                       200, h - 30);

    // Info display
    ofDrawBitmapString("FPS: " + ofToString(ofGetFrameRate(), 1), 10, 20);
  }

  void mouseMoved(float x, float y) override {
    // update mouse x and y percent when the mouse moves
    mouseXPercent = x / ofGetWidth();
    mouseYPercent = y / ofGetHeight();
  }

private:
  float mouseXPercent;
  float mouseYPercent;
};

OF_MAIN(ColorExampleApp);
