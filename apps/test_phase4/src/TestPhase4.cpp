#include "TestPhase4.h"
#include <cmath>
#include <string>

void TestPhase4::setup() {
    ofSetWindowTitle("test_phase4 - Phase 4 Feature Test (1-2: switch mode, P: primitive, W: wireframe)");
    ofSetCircleResolution(64);

    // Load font
    font.load("Helvetica", 18.0f);

    // Setup camera
    camera.setDistance(500);
    camera.setTarget(oflike::ofVec3f(0, 0, 0));

    // Setup light
    light.setPosition(200, 300, 200);
    light.setDiffuseColor(oflike::ofFloatColor(1.0f, 1.0f, 1.0f));
    light.setAmbientColor(oflike::ofFloatColor(0.2f, 0.2f, 0.2f));

    // Try to load a test sound (if available in data folder)
    soundLoaded = soundPlayer.load("data/test.wav");
    if (!soundLoaded) {
        soundLoaded = soundPlayer.load("data/test.mp3");
    }
    if (soundLoaded) {
        soundPlayer.setLoop(true);
    }

    // Setup 3D primitives
    box.set(100, 100, 100);
    sphere.setRadius(60);
    sphere.setResolution(32);
    cylinder.set(40, 120, 32, 1, true);
    cone.set(50, 100, 32, 1, true);
    plane.set(150, 100, 4, 4);
    icoSphere.setRadius(60);
    icoSphere.setResolution(2);
}

void TestPhase4::update() {
    rotationAngle += 0.5f;
    if (rotationAngle > 360.0f) rotationAngle -= 360.0f;

    // Rotate primitives
    box.setOrientation(rotationAngle * 0.5f, rotationAngle, rotationAngle * 0.3f);
    sphere.setOrientation(0, rotationAngle, 0);
    cylinder.setOrientation(rotationAngle * 0.3f, rotationAngle * 0.5f, 0);
    cone.setOrientation(0, rotationAngle, rotationAngle * 0.2f);
    plane.setOrientation(rotationAngle * 0.2f, rotationAngle * 0.3f, 0);
    icoSphere.setOrientation(rotationAngle * 0.4f, rotationAngle * 0.6f, 0);
}

void TestPhase4::draw() {
    ofBackground(30, 35, 45);

    switch (testMode) {
        case 0:
            drawSoundTest();
            break;
        case 1:
            draw3DPrimitiveTest();
            break;
    }

    // Draw test mode indicator
    ofSetColor(255);
    float y = static_cast<float>(ofGetHeight()) - 30;
    if (font.isLoaded()) {
        std::string modeNames[] = {"ofSoundPlayer", "of3dPrimitive"};
        std::string text = "Test " + std::to_string(testMode + 1) + "/" + std::to_string(NUM_TEST_MODES) + ": " + modeNames[testMode] + " (Press 1-2)";
        font.drawString(text, 20, y);
    }
}

void TestPhase4::drawSoundTest() {
    // Draw title
    ofSetColor(255, 255, 100);
    if (font.isLoaded()) {
        font.drawString("ofSoundPlayer Test - Audio Playback with AVFoundation", 20, 40);
    }

    if (soundLoaded) {
        // Draw sound info
        ofSetColor(200, 200, 200);
        float infoY = 100;
        if (font.isLoaded()) {
            font.drawString("Sound loaded successfully!", 20, infoY);
            infoY += 40;

            font.drawString("Playing: " + std::string(soundPlayer.isPlaying() ? "Yes" : "No"), 20, infoY);
            infoY += 30;

            font.drawString("Position: " + std::to_string(static_cast<int>(soundPlayer.getPosition() * 100)) + "%", 20, infoY);
            infoY += 30;

            font.drawString("Volume: " + std::to_string(static_cast<int>(soundPlayer.getVolume() * 100)) + "%", 20, infoY);
            infoY += 30;

            font.drawString("Speed: " + std::to_string(soundPlayer.getSpeed()) + "x", 20, infoY);
            infoY += 30;

            font.drawString("Looping: " + std::string(soundPlayer.isLooping() ? "Yes" : "No"), 20, infoY);
            infoY += 50;

            ofSetColor(150, 200, 150);
            font.drawString("Controls:", 20, infoY);
            infoY += 30;
            font.drawString("  Space: Play/Pause", 20, infoY);
            infoY += 25;
            font.drawString("  S: Stop", 20, infoY);
            infoY += 25;
            font.drawString("  Up/Down: Volume", 20, infoY);
            infoY += 25;
            font.drawString("  Left/Right: Seek", 20, infoY);
            infoY += 25;
            font.drawString("  +/-: Speed", 20, infoY);
            infoY += 25;
            font.drawString("  L: Toggle Loop", 20, infoY);
        }

        // Draw progress bar
        ofSetColor(100);
        ofDrawRectangle(20, 450, 400, 20);
        ofSetColor(100, 200, 100);
        float progress = soundPlayer.getPosition();
        ofDrawRectangle(20, 450, 400 * progress, 20);

        // Draw volume bar
        ofSetColor(100);
        ofDrawRectangle(20, 490, 200, 15);
        ofSetColor(100, 100, 200);
        ofDrawRectangle(20, 490, 200 * soundPlayer.getVolume(), 15);
        ofSetColor(200, 200, 200);
        if (font.isLoaded()) {
            font.drawString("Volume", 230, 503);
        }
    } else {
        // No sound loaded
        ofSetColor(200, 100, 100);
        if (font.isLoaded()) {
            font.drawString("No sound loaded.", 20, 100);
            font.drawString("", 20, 140);
            font.drawString("Place a sound file named 'test.wav' or 'test.mp3'", 20, 180);
            font.drawString("in the app's data folder.", 20, 210);
            font.drawString("", 20, 250);
            font.drawString("Supported formats: WAV, MP3, AAC, M4A, AIFF", 20, 290);
        }
    }
}

void TestPhase4::draw3DPrimitiveTest() {
    // Draw title
    ofSetColor(255, 255, 100);
    if (font.isLoaded()) {
        font.drawString("of3dPrimitive Test - 3D Geometry Classes", 20, 40);
    }

    // Draw current primitive name
    ofSetColor(200, 200, 200);
    if (font.isLoaded()) {
        font.drawString("Current: " + std::string(primitiveNames[currentPrimitive]) +
                       " (Press P to cycle, W for wireframe)", 20, 70);
        font.drawString("Wireframe: " + std::string(wireframe ? "ON" : "OFF"), 20, 100);
    }

    // Enable lighting
    ofEnableLighting();
    light.enable();

    // Start 3D camera
    camera.begin();

    // Draw grid
    ofSetColor(80, 80, 80);
    for (int i = -500; i <= 500; i += 100) {
        ofDrawLine(static_cast<float>(i), 0, -500, static_cast<float>(i), 0, 500);
        ofDrawLine(-500, 0, static_cast<float>(i), 500, 0, static_cast<float>(i));
    }

    // Draw current primitive
    ofSetColor(200, 150, 100);

    oflike::of3dPrimitive* prim = nullptr;
    switch (currentPrimitive) {
        case 0: prim = &box; break;
        case 1: prim = &sphere; break;
        case 2: prim = &cylinder; break;
        case 3: prim = &cone; break;
        case 4: prim = &plane; break;
        case 5: prim = &icoSphere; break;
    }

    if (prim) {
        if (wireframe) {
            prim->drawWireframe();
        } else {
            prim->draw();
        }
    }

    camera.end();

    // Disable lighting
    light.disable();
    ofDisableLighting();
}

void TestPhase4::keyPressed(int key) {
    // Number keys 1-2 to switch test modes
    if (key >= '1' && key <= '2') {
        testMode = key - '1';
    }

    // Sound controls (mode 0)
    if (testMode == 0 && soundLoaded) {
        if (key == ' ' || key == 49) {  // Space bar
            if (soundPlayer.isPlaying()) {
                soundPlayer.pause();
            } else {
                soundPlayer.play();
            }
        }

        if (key == 's' || key == 'S') {
            soundPlayer.stop();
        }

        if (key == 'l' || key == 'L') {
            soundPlayer.setLoop(!soundPlayer.isLooping());
        }

        // Volume up/down
        if (key == 126) {  // Up arrow
            soundPlayer.setVolume(std::min(1.0f, soundPlayer.getVolume() + 0.1f));
        }
        if (key == 125) {  // Down arrow
            soundPlayer.setVolume(std::max(0.0f, soundPlayer.getVolume() - 0.1f));
        }

        // Seek left/right
        if (key == 123) {  // Left arrow
            soundPlayer.setPosition(std::max(0.0f, soundPlayer.getPosition() - 0.05f));
        }
        if (key == 124) {  // Right arrow
            soundPlayer.setPosition(std::min(1.0f, soundPlayer.getPosition() + 0.05f));
        }

        // Speed +/-
        if (key == '+' || key == '=') {
            soundPlayer.setSpeed(std::min(2.0f, soundPlayer.getSpeed() + 0.1f));
        }
        if (key == '-') {
            soundPlayer.setSpeed(std::max(0.5f, soundPlayer.getSpeed() - 0.1f));
        }
    }

    // 3D primitive controls (mode 1)
    if (testMode == 1) {
        if (key == 'p' || key == 'P') {
            currentPrimitive = (currentPrimitive + 1) % NUM_PRIMITIVES;
        }
        if (key == 'w' || key == 'W') {
            wireframe = !wireframe;
        }
    }
}

void TestPhase4::keyReleased(int key) {
    (void)key;
}

void TestPhase4::mouseMoved(int x, int y) {
    (void)x; (void)y;
}

void TestPhase4::mouseDragged(int x, int y, int button) {
    (void)x; (void)y; (void)button;
}

void TestPhase4::mousePressed(int x, int y, int button) {
    (void)button;

    // Click on progress bar to seek sound
    if (testMode == 0 && soundLoaded) {
        if (y >= 450 && y <= 470 && x >= 20 && x <= 420) {
            float pct = static_cast<float>(x - 20) / 400.0f;
            soundPlayer.setPosition(pct);
        }
    }
}

void TestPhase4::mouseReleased(int x, int y, int button) {
    (void)x; (void)y; (void)button;
}

void TestPhase4::mouseScrolled(int x, int y, float scrollX, float scrollY) {
    (void)x; (void)y; (void)scrollX; (void)scrollY;
}

void TestPhase4::windowResized(int w, int h) {
    (void)w; (void)h;
}

extern "C" ofBaseApp* ofCreateApp() {
    return new TestPhase4();
}
