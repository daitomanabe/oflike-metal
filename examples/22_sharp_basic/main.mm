/// Example 22: Sharp Basic
/// Demonstrates ofxSharp - 3D Gaussian Splatting from a single image

#include "core/AppBase.h"
#include "oflike/graphics/ofGraphics.h"
#include "oflike/3d/ofCamera.h"
#include "oflike/3d/ofEasyCam.h"
#include "oflike/image/ofImage.h"
#include "oflike/types/ofColor.h"
#include "oflike/utils/ofUtils.h"
#include "addons/apple_native/ofxSharp/ofxSharp.h"
#include <sstream>

using namespace oflike;

class ofApp : public ofBaseApp {
public:
    // Sharp components
    ofxSharp sharp;
    ofEasyCam camera;
    ofImage sourceImage;

    // UI state
    bool isGenerating = false;
    bool cloudReady = false;
    std::string statusMessage = "Press SPACE to load image and generate 3D";

    // Rendering settings
    float splatScale = 1.0f;
    float opacityScale = 1.0f;
    bool depthSort = true;
    bool antiAliasing = true;
    int shDegree = 3;

    // Info display
    double lastInferenceTime = 0.0;
    size_t gaussianCount = 0;

    void setup() override {
        ofSetFrameRate(60);
        ofBackground(20);

        // Setup Sharp
        if (!sharp.setup()) {
            statusMessage = "ERROR: Failed to initialize ofxSharp";
            ofLogError("sharp_basic") << statusMessage;
            return;
        }

        // Configure camera
        camera.setDistance(5.0f);
        camera.setTarget(ofVec3f(0, 0, 0));
        camera.enableMouseInput();
        camera.setAutoDistance(false);

        ofLogNotice("sharp_basic") << "ofxSharp initialized successfully";
        ofLogNotice("sharp_basic") << "Neural Engine: " << (sharp.isUsingNeuralEngine() ? "YES" : "NO");
    }

    void update() override {
        isGenerating = sharp.isGenerating();
        cloudReady = sharp.hasCloud();

        if (cloudReady) {
            gaussianCount = sharp.getGaussianCount();
        }
    }

    void draw() override {
        ofBackground(20);

        // Draw 3D scene
        if (cloudReady) {
            ofEnableDepthTest();

            camera.begin();

            // Draw coordinate axes for reference
            ofSetColor(255, 0, 0);
            ofDrawLine(0, 0, 0, 1, 0, 0);
            ofSetColor(0, 255, 0);
            ofDrawLine(0, 0, 0, 0, 1, 0);
            ofSetColor(0, 0, 255);
            ofDrawLine(0, 0, 0, 0, 0, 1);

            // Draw Gaussian cloud
            sharp.drawCloud(camera);

            camera.end();

            ofDisableDepthTest();
        }

        // Draw UI
        drawUI();
    }

    void drawUI() {
        ofSetColor(255);
        int y = 20;
        int lineHeight = 20;

        // Title
        ofDrawBitmapString("ofxSharp - 3D Gaussian Splatting", 20, y);
        y += lineHeight * 2;

        // Status
        ofSetColor(isGenerating ? ofColor(255, 200, 0) : ofColor(255));
        ofDrawBitmapString("Status: " + statusMessage, 20, y);
        y += lineHeight;

        if (cloudReady) {
            ofSetColor(255);

            // Stats
            std::ostringstream stats;
            stats << "Gaussians: " << gaussianCount;
            ofDrawBitmapString(stats.str(), 20, y);
            y += lineHeight;

            stats.str("");
            stats << "Inference Time: " << (int)lastInferenceTime << " ms";
            ofDrawBitmapString(stats.str(), 20, y);
            y += lineHeight;

            stats.str("");
            stats << "Neural Engine: " << (sharp.isUsingNeuralEngine() ? "YES" : "NO");
            ofDrawBitmapString(stats.str(), 20, y);
            y += lineHeight;

            // Render stats
            const auto& renderStats = sharp.getRenderStats();
            stats.str("");
            stats << "Visible Splats: " << renderStats.visibleSplatCount;
            ofDrawBitmapString(stats.str(), 20, y);
            y += lineHeight;

            stats.str("");
            stats << "Render Time: " << renderStats.gpuTimeMs << " ms";
            ofDrawBitmapString(stats.str(), 20, y);
            y += lineHeight;

            y += lineHeight;

            // Camera info
            ofVec3f pos = camera.getPosition();
            stats.str("");
            stats << "Camera: (" << (int)pos.x << ", " << (int)pos.y << ", " << (int)pos.z << ")";
            ofDrawBitmapString(stats.str(), 20, y);
            y += lineHeight;

            stats.str("");
            stats << "Distance: " << camera.getDistance();
            ofDrawBitmapString(stats.str(), 20, y);
            y += lineHeight;
        }

        // Controls
        y += lineHeight;
        ofSetColor(180);
        ofDrawBitmapString("Controls:", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  SPACE  - Load image and generate 3D", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  S      - Save cloud to PLY", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  L      - Load cloud from PLY", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  +/-    - Adjust splat scale", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  [/]    - Adjust opacity", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  D      - Toggle depth sort", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  A      - Toggle anti-aliasing", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  1-4    - Set SH degree (0-3)", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  R      - Reset transform", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  Mouse  - Rotate camera", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  Wheel  - Zoom camera", 20, y);

        // Current settings
        if (cloudReady) {
            y += lineHeight;
            ofSetColor(200, 200, 255);
            ofDrawBitmapString("Settings:", 20, y);
            y += lineHeight;

            std::ostringstream settings;
            settings << "  Splat Scale: " << splatScale;
            ofDrawBitmapString(settings.str(), 20, y);
            y += lineHeight;

            settings.str("");
            settings << "  Opacity: " << opacityScale;
            ofDrawBitmapString(settings.str(), 20, y);
            y += lineHeight;

            settings.str("");
            settings << "  Depth Sort: " << (depthSort ? "ON" : "OFF");
            ofDrawBitmapString(settings.str(), 20, y);
            y += lineHeight;

            settings.str("");
            settings << "  Anti-aliasing: " << (antiAliasing ? "ON" : "OFF");
            ofDrawBitmapString(settings.str(), 20, y);
            y += lineHeight;

            settings.str("");
            settings << "  SH Degree: " << shDegree;
            ofDrawBitmapString(settings.str(), 20, y);
        }

        // FPS
        ofSetColor(255, 200);
        ofDrawBitmapString("FPS: " + ofToString((int)ofGetFrameRate()), ofGetWidth() - 100, 20);
    }

    void keyPressed(int key) override {
        if (key == ' ') {
            // Load and generate
            loadAndGenerate();
        }
        else if (key == 's' || key == 'S') {
            // Save cloud
            if (cloudReady) {
                std::string filepath = "gaussian_cloud_" + ofGetTimestampString() + ".ply";
                if (sharp.saveCloud(filepath)) {
                    statusMessage = "Saved cloud to: " + filepath;
                    ofLogNotice("sharp_basic") << statusMessage;
                } else {
                    statusMessage = "Failed to save cloud";
                    ofLogError("sharp_basic") << statusMessage;
                }
            }
        }
        else if (key == 'l' || key == 'L') {
            // Load cloud (you'd need to provide a path)
            statusMessage = "Load PLY not implemented (add file dialog)";
        }
        else if (key == '+' || key == '=') {
            // Increase splat scale
            splatScale = ofClamp(splatScale + 0.1f, 0.1f, 5.0f);
            sharp.setSplatScale(splatScale);
            statusMessage = "Splat scale: " + ofToString(splatScale, 1);
        }
        else if (key == '-' || key == '_') {
            // Decrease splat scale
            splatScale = ofClamp(splatScale - 0.1f, 0.1f, 5.0f);
            sharp.setSplatScale(splatScale);
            statusMessage = "Splat scale: " + ofToString(splatScale, 1);
        }
        else if (key == '[') {
            // Decrease opacity
            opacityScale = ofClamp(opacityScale - 0.1f, 0.1f, 1.0f);
            sharp.setOpacityScale(opacityScale);
            statusMessage = "Opacity: " + ofToString(opacityScale, 1);
        }
        else if (key == ']') {
            // Increase opacity
            opacityScale = ofClamp(opacityScale + 0.1f, 0.1f, 1.0f);
            sharp.setOpacityScale(opacityScale);
            statusMessage = "Opacity: " + ofToString(opacityScale, 1);
        }
        else if (key == 'd' || key == 'D') {
            // Toggle depth sort
            depthSort = !depthSort;
            sharp.setDepthSortEnabled(depthSort);
            statusMessage = "Depth sort: " + std::string(depthSort ? "ON" : "OFF");
        }
        else if (key == 'a' || key == 'A') {
            // Toggle anti-aliasing
            antiAliasing = !antiAliasing;
            sharp.setAntiAliasingEnabled(antiAliasing);
            statusMessage = "Anti-aliasing: " + std::string(antiAliasing ? "ON" : "OFF");
        }
        else if (key >= '1' && key <= '4') {
            // Set SH degree
            shDegree = key - '1';
            sharp.setMaxSHDegree(shDegree);
            statusMessage = "SH degree: " + ofToString(shDegree);
        }
        else if (key == 'r' || key == 'R') {
            // Reset transform
            sharp.resetTransform();
            statusMessage = "Transform reset";
        }
    }

    void loadAndGenerate() {
        if (isGenerating) {
            statusMessage = "Generation already in progress...";
            return;
        }

        // Try to load a test image (you should provide your own image path)
        // For this example, we'll use a placeholder approach
        // In a real app, you'd use a file dialog or specific image path

        // Create a simple test image programmatically if no file is available
        if (!sourceImage.load("test_image.jpg")) {
            // Create a gradient test image
            ofLogNotice("sharp_basic") << "No test_image.jpg found, creating test image";
            createTestImage();
        }

        statusMessage = "Generating 3D from image... (Neural Engine)";
        ofLogNotice("sharp_basic") << statusMessage;

        // Generate asynchronously
        sharp.generateFromImageAsync(sourceImage, [this](bool success) {
            if (success) {
                lastInferenceTime = sharp.getLastInferenceTime();
                statusMessage = "Generation complete! (" + ofToString((int)lastInferenceTime) + " ms)";
                ofLogNotice("sharp_basic") << statusMessage;
                ofLogNotice("sharp_basic") << "Gaussian count: " << sharp.getGaussianCount();
            } else {
                statusMessage = "Generation failed: " + sharp.getLastError();
                ofLogError("sharp_basic") << statusMessage;
            }
        });
    }

    void createTestImage() {
        // Create a simple gradient test image
        int w = 512, h = 512;
        sourceImage.allocate(w, h, OF_IMAGE_COLOR);

        ofPixels& pixels = sourceImage.getPixels();
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                float r = (float)x / w * 255;
                float g = (float)y / h * 255;
                float b = 128;
                pixels.setColor(x, y, ofColor(r, g, b));
            }
        }

        sourceImage.update();
        ofLogNotice("sharp_basic") << "Created test gradient image";
    }
};

// Entry point
#define OF_APP ofApp
#include "oflike/app/ofMain.h"
