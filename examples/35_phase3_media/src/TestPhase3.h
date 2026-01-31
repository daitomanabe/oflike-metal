#pragma once

// Test app for Phase 3 features: ofVideoPlayer, ofVideoGrabber, ofImageFilter

#include <oflike/ofApp.h>
#include <oflike/video/ofVideoPlayer.h>
#include <oflike/video/ofVideoGrabber.h>
#include <oflike/image/ofImage.h>
#include <oflike/graphics/ofCoreText.h>

class TestPhase3 : public ofBaseApp {
public:
    void setup() override;
    void update() override;
    void draw() override;

    void keyPressed(int key) override;
    void keyReleased(int key) override;
    void mouseMoved(int x, int y) override;
    void mouseDragged(int x, int y, int button) override;
    void mousePressed(int x, int y, int button) override;
    void mouseReleased(int x, int y, int button) override;
    void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
    void windowResized(int w, int h) override;

private:
    void drawVideoPlayerTest();
    void drawVideoGrabberTest();
    void drawImageFilterTest();

    void createTestPattern();
    void applyCurrentFilter();

    // UI
    ofCoreText font;

    // Test mode
    int testMode = 0;
    static constexpr int NUM_TEST_MODES = 3;

    // Video player
    oflike::ofVideoPlayer videoPlayer;
    bool videoLoaded = false;
    bool videoPaused = false;

    // Video grabber (camera)
    oflike::ofVideoGrabber videoGrabber;
    bool cameraInitialized = false;

    // Image filter test
    oflike::ofImage testImage;
    oflike::ofImage filteredImage;
    int currentFilter = 0;
    static constexpr int NUM_FILTERS = 10;
    const char* filterNames[NUM_FILTERS] = {
        "Original",
        "Blur",
        "Sharpen",
        "Contrast+",
        "Brightness+",
        "Grayscale",
        "Invert",
        "Sobel Edge",
        "Threshold",
        "Noise"
    };
};
