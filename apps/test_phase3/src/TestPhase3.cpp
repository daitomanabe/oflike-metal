#include "TestPhase3.h"
#include <oflike/image/ofImageFilter.h>
#include <string>
#include <cmath>

void TestPhase3::setup() {
    ofSetWindowTitle("test_phase3 - Phase 3 Feature Test (1-3: switch, Space: play/pause)");
    ofSetCircleResolution(64);

    // Load font
    font.load("Helvetica", 18.0f);

    // Try to load a test video (if available in data folder)
    // Users should provide their own video file at data/test.mp4
    videoLoaded = videoPlayer.load("data/test.mp4");
    if (videoLoaded) {
        videoPlayer.setLoopState(oflike::OF_LOOP_NORMAL);
    }

    // Setup video grabber (camera)
    // Request camera permission first
    if (!oflike::ofVideoGrabber::isAuthorized()) {
        oflike::ofVideoGrabber::requestPermission();
    }

    // List available cameras
    auto devices = oflike::ofVideoGrabber::listDevices();
    if (!devices.empty()) {
        videoGrabber.setDeviceID(0);
        cameraInitialized = videoGrabber.setup(640, 480);
    }

    // Create a test image for filter demonstration
    createTestPattern();
}

void TestPhase3::createTestPattern() {
    // Create a colorful test pattern
    oflike::ofPixels pixels;
    const int size = 256;
    pixels.allocate(size, size, 4);

    unsigned char* data = pixels.getData();
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            int idx = (y * size + x) * 4;

            // Gradient background
            data[idx + 0] = static_cast<unsigned char>(x);     // R: horizontal gradient
            data[idx + 1] = static_cast<unsigned char>(y);     // G: vertical gradient
            data[idx + 2] = static_cast<unsigned char>(128);   // B: constant

            // Add some shapes
            int cx = size / 2;
            int cy = size / 2;
            int dx = x - cx;
            int dy = y - cy;
            int dist = static_cast<int>(std::sqrt(dx * dx + dy * dy));

            // Concentric circles
            if (dist % 30 < 5) {
                data[idx + 0] = 255;
                data[idx + 1] = 255;
                data[idx + 2] = 255;
            }

            // Checkerboard in corner
            if (x < 64 && y < 64) {
                int checkX = x / 8;
                int checkY = y / 8;
                if ((checkX + checkY) % 2 == 0) {
                    data[idx + 0] = 0;
                    data[idx + 1] = 0;
                    data[idx + 2] = 0;
                }
            }

            data[idx + 3] = 255;  // A: opaque
        }
    }

    testImage.setFromPixels(pixels);
    filteredImage.setFromPixels(pixels);
}

void TestPhase3::applyCurrentFilter() {
    // Copy original to filtered
    filteredImage.setFromPixels(testImage.getPixels());

    oflike::ofPixels& pixels = filteredImage.getPixels();

    switch (currentFilter) {
        case 0:  // Original
            break;
        case 1:  // Blur
            oflike::ofImageFilter::blur(pixels, 5.0f);
            break;
        case 2:  // Sharpen
            oflike::ofImageFilter::sharpen(pixels, 2.0f, 1.0f);
            break;
        case 3:  // Contrast+
            oflike::ofImageFilter::contrast(pixels, 1.5f);
            break;
        case 4:  // Brightness+
            oflike::ofImageFilter::brightness(pixels, 50.0f);
            break;
        case 5:  // Grayscale
            oflike::ofImageFilter::grayscale(pixels);
            break;
        case 6:  // Invert
            oflike::ofImageFilter::invert(pixels);
            break;
        case 7:  // Sobel Edge
            oflike::ofImageFilter::sobel(pixels);
            break;
        case 8:  // Threshold
            oflike::ofImageFilter::threshold(pixels, 128);
            break;
        case 9:  // Noise
            oflike::ofImageFilter::addNoise(pixels, 0.3f);
            break;
    }

    filteredImage.update();
}

void TestPhase3::update() {
    // Update video player
    if (videoLoaded) {
        videoPlayer.update();
    }

    // Update video grabber
    if (cameraInitialized) {
        videoGrabber.update();
    }
}

void TestPhase3::draw() {
    ofBackground(30, 35, 45);

    switch (testMode) {
        case 0:
            drawVideoPlayerTest();
            break;
        case 1:
            drawVideoGrabberTest();
            break;
        case 2:
            drawImageFilterTest();
            break;
    }

    // Draw test mode indicator
    ofSetColor(255);
    float y = ofGetHeight() - 30;
    if (font.isLoaded()) {
        std::string modeNames[] = {"ofVideoPlayer", "ofVideoGrabber", "ofImageFilter"};
        std::string text = "Test " + std::to_string(testMode + 1) + "/" + std::to_string(NUM_TEST_MODES) + ": " + modeNames[testMode] + " (Press 1-3)";
        font.drawString(text, 20, y);
    }
}

void TestPhase3::drawVideoPlayerTest() {
    // Draw title
    ofSetColor(255, 255, 100);
    if (font.isLoaded()) {
        font.drawString("ofVideoPlayer Test - Video Playback with AVFoundation", 20, 40);
    }

    if (videoLoaded) {
        // Draw video
        ofSetColor(255);
        float videoW = videoPlayer.getWidth();
        float videoH = videoPlayer.getHeight();

        // Scale to fit
        float scale = std::min(700.0f / videoW, 500.0f / videoH);
        float drawW = videoW * scale;
        float drawH = videoH * scale;

        videoPlayer.draw(50, 80, drawW, drawH);

        // Draw info
        ofSetColor(200, 200, 200);
        float infoY = 620;
        if (font.isLoaded()) {
            font.drawString("Size: " + std::to_string((int)videoW) + "x" + std::to_string((int)videoH), 20, infoY);
            infoY += 25;
            font.drawString("Duration: " + std::to_string(videoPlayer.getDuration()) + "s", 20, infoY);
            infoY += 25;
            font.drawString("Time: " + std::to_string(videoPlayer.getCurrentTime()) + "s", 20, infoY);
            infoY += 25;
            font.drawString("Frame: " + std::to_string(videoPlayer.getCurrentFrame()) + "/" + std::to_string(videoPlayer.getTotalNumFrames()), 20, infoY);
            infoY += 25;
            font.drawString("Playing: " + std::string(videoPlayer.isPlaying() ? "Yes" : "No"), 20, infoY);
            infoY += 25;
            font.drawString("Space: Play/Pause, Left/Right: Seek", 20, infoY);
        }

        // Draw progress bar
        ofSetColor(100);
        ofDrawRectangle(50, 590, 700, 10);
        ofSetColor(100, 200, 100);
        float progress = videoPlayer.getPosition();
        ofDrawRectangle(50, 590, 700 * progress, 10);
    } else {
        // No video loaded
        ofSetColor(200, 100, 100);
        if (font.isLoaded()) {
            font.drawString("No video loaded.", 20, 100);
            font.drawString("Place a video file named 'test.mp4' in the app resources,", 20, 140);
            font.drawString("or modify TestPhase3.cpp to load your video file.", 20, 170);
            font.drawString("", 20, 210);
            font.drawString("Supported formats: MP4, MOV, M4V, AVI", 20, 250);
        }
    }
}

void TestPhase3::drawVideoGrabberTest() {
    // Draw title
    ofSetColor(255, 255, 100);
    if (font.isLoaded()) {
        font.drawString("ofVideoGrabber Test - Camera Input with AVCaptureSession", 20, 40);
    }

    if (cameraInitialized) {
        // Draw camera feed
        ofSetColor(255);
        float camW = videoGrabber.getWidth();
        float camH = videoGrabber.getHeight();

        // Scale to fit
        float scale = std::min(700.0f / camW, 500.0f / camH);
        float drawW = camW * scale;
        float drawH = camH * scale;

        videoGrabber.draw(50, 80, drawW, drawH);

        // Draw info
        ofSetColor(200, 200, 200);
        float infoY = 620;
        if (font.isLoaded()) {
            font.drawString("Camera size: " + std::to_string((int)camW) + "x" + std::to_string((int)camH), 20, infoY);
            infoY += 25;
            font.drawString("Frame new: " + std::string(videoGrabber.isFrameNew() ? "Yes" : "No"), 20, infoY);
            infoY += 25;

            if (oflike::ofVideoGrabber::isAuthorized()) {
                font.drawString("Camera authorized: Yes", 20, infoY);
            } else {
                ofSetColor(255, 100, 100);
                font.drawString("Camera authorized: No (check System Preferences)", 20, infoY);
            }
        }
    } else {
        // Camera not initialized
        ofSetColor(200, 100, 100);
        if (font.isLoaded()) {
            font.drawString("Camera not initialized.", 20, 100);
            font.drawString("", 20, 140);

            if (!oflike::ofVideoGrabber::isAuthorized()) {
                font.drawString("Camera access not authorized.", 20, 180);
                font.drawString("Please grant camera permission in System Preferences.", 20, 220);
            } else {
                font.drawString("No camera device found.", 20, 180);
            }
        }
    }
}

void TestPhase3::drawImageFilterTest() {
    // Draw title
    ofSetColor(255, 255, 100);
    if (font.isLoaded()) {
        font.drawString("ofImageFilter Test - Image Processing with Accelerate", 20, 40);
    }

    // Draw original and filtered images side by side
    ofSetColor(255);

    float imgW = static_cast<float>(testImage.getWidth());
    float imgH = static_cast<float>(testImage.getHeight());

    // Draw original
    testImage.draw(50, 100, imgW * 1.5f, imgH * 1.5f);
    ofSetColor(200, 200, 200);
    if (font.isLoaded()) {
        font.drawString("Original", 50, 520);
    }

    // Draw filtered
    ofSetColor(255);
    filteredImage.draw(450, 100, imgW * 1.5f, imgH * 1.5f);
    ofSetColor(200, 200, 200);
    if (font.isLoaded()) {
        font.drawString("Filter: " + std::string(filterNames[currentFilter]), 450, 520);
    }

    // Draw instructions
    ofSetColor(200, 200, 200);
    float infoY = 580;
    if (font.isLoaded()) {
        font.drawString("Press F to cycle through filters", 20, infoY);
        infoY += 25;
        font.drawString("Available filters:", 20, infoY);
        infoY += 25;

        for (int i = 0; i < NUM_FILTERS; ++i) {
            if (i == currentFilter) {
                ofSetColor(100, 255, 100);
            } else {
                ofSetColor(150, 150, 150);
            }
            std::string filterText = std::to_string(i) + ": " + filterNames[i];
            font.drawString(filterText, 20 + (i / 5) * 200, infoY + (i % 5) * 20);
        }
    }
}

void TestPhase3::keyPressed(int key) {
    // Number keys 1-3 to switch test modes
    if (key >= '1' && key <= '3') {
        testMode = key - '1';
    }
    // macOS number key codes
    if (key >= 18 && key <= 20) {
        testMode = key - 18;
    }

    // Space to play/pause video
    if (key == ' ' || key == 49) {  // Space bar
        if (testMode == 0 && videoLoaded) {
            if (videoPlayer.isPlaying()) {
                videoPlayer.pause();
            } else {
                videoPlayer.play();
            }
        }
    }

    // Left/Right arrows to seek video
    if (testMode == 0 && videoLoaded) {
        if (key == 123) {  // Left arrow
            videoPlayer.seekBackward(5.0f);
        } else if (key == 124) {  // Right arrow
            videoPlayer.seekForward(5.0f);
        }
    }

    // F key to cycle filters (keycode 3 on macOS, or 'f'/'F')
    if (key == 'f' || key == 'F' || key == 3) {
        if (testMode == 2) {
            currentFilter = (currentFilter + 1) % NUM_FILTERS;
            applyCurrentFilter();
        }
    }

    // Number keys 0-9 for direct filter selection
    if (testMode == 2) {
        if (key >= '0' && key <= '9') {
            int filterIndex = key - '0';
            if (filterIndex < NUM_FILTERS) {
                currentFilter = filterIndex;
                applyCurrentFilter();
            }
        }
    }
}

void TestPhase3::keyReleased(int key) {
    (void)key;
}

void TestPhase3::mouseMoved(int x, int y) {
    (void)x; (void)y;
}

void TestPhase3::mouseDragged(int x, int y, int button) {
    (void)x; (void)y; (void)button;
}

void TestPhase3::mousePressed(int x, int y, int button) {
    (void)x; (void)y; (void)button;

    // Click on progress bar to seek video
    if (testMode == 0 && videoLoaded) {
        if (y >= 590 && y <= 600 && x >= 50 && x <= 750) {
            float pct = static_cast<float>(x - 50) / 700.0f;
            videoPlayer.setPosition(pct);
        }
    }
}

void TestPhase3::mouseReleased(int x, int y, int button) {
    (void)x; (void)y; (void)button;
}

void TestPhase3::mouseScrolled(int x, int y, float scrollX, float scrollY) {
    (void)x; (void)y; (void)scrollX; (void)scrollY;
}

void TestPhase3::windowResized(int w, int h) {
    (void)w; (void)h;
}

extern "C" ofBaseApp* ofCreateApp() {
    return new TestPhase3();
}
