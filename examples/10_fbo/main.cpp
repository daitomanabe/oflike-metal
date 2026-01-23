/*
 * 10_fbo - ofFbo Demonstration
 *
 * Demonstrates ofFbo (Framebuffer Object) API with:
 * - Offscreen rendering to FBO
 * - Multiple render passes
 * - Texture readback (ofFbo::readToPixels)
 * - Multisampling / MSAA
 * - Depth buffer usage
 * - Multiple color attachments
 * - FBO as texture source
 *
 * Controls:
 * - 1-6: Switch FBO modes
 * - SPACE: Toggle animation
 * - M: Toggle MSAA (multisampling)
 * - S: Save FBO contents to image
 * - UP/DOWN: Adjust FBO resolution
 *
 * Integration:
 * - Copy this file to your project
 * - Link against oflike-metal library
 * - Build and run
 */

#include "ofApp.h"
#include "ofGraphics.h"
#include "ofPath.h"
#include "ofTrueTypeFont.h"
#include "ofUtils.h"
#include "ofMath.h"
#include "ofTypes.h"
#include "ofFbo.h"
#include "ofCamera.h"
#include "of3dPrimitives.h"
#include "ofLight.h"
#include "ofMaterial.h"

using namespace oflike;

class FboApp : public ofBaseApp {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void keyPressed(int key) override;
    void windowResized(int w, int h) override;

private:
    void createFbos();
    void renderToFbo1();
    void renderToFbo2();
    void renderToMultiSampleFbo();
    void renderToMultiAttachmentFbo();
    void renderToDepthFbo();
    void drawFboContents();
    void drawUI();

    // FBOs for different use cases
    ofFbo fbo1_;                    // Basic offscreen rendering
    ofFbo fbo2_;                    // Second pass rendering
    ofFbo multiSampleFbo_;          // MSAA FBO
    ofFbo multiAttachmentFbo_;      // Multiple color attachments
    ofFbo depthFbo_;                // With depth buffer

    // Scene elements
    ofCamera camera_;
    ofLight light_;
    ofMaterial material_;

    // State
    int currentMode_ = 1;           // 1-6: Different FBO modes
    bool animating_ = true;
    bool useMsaa_ = true;
    int fboResolution_ = 512;
    float time_ = 0.0f;
    float rotation_ = 0.0f;

    // Font for UI
    ofTrueTypeFont font_;
};

void FboApp::setup() {
    ofSetFrameRate(60);
    ofSetWindowTitle("10_fbo - Framebuffer Objects");
    ofBackground(40);

    // Load font
    if (!font_.load("Helvetica", 14)) {
        ofLogNotice("FboApp") << "Font loading failed (OK for headless)";
    }

    // Create FBOs
    createFbos();

    // Setup camera
    camera_.setPosition(0, 0, 10);
    camera_.lookAt(ofVec3f(0, 0, 0));
    camera_.setNearClip(0.1f);
    camera_.setFarClip(100.0f);
    camera_.setFov(60.0f);
    camera_.setAspectRatio(1.0f);  // FBO is square

    // Setup light
    light_.setPointLight();
    light_.setPosition(5, 5, 5);
    light_.setDiffuseColor(ofColor(255, 255, 255));
    light_.setSpecularColor(ofColor(255, 255, 255));
    light_.setAmbientColor(ofColor(50, 50, 50));

    // Setup material
    material_.setDiffuseColor(ofColor(100, 150, 255));
    material_.setSpecularColor(ofColor(255, 255, 255));
    material_.setShininess(64.0f);

    ofLogNotice("FboApp") << "Setup complete - FBO resolution: " << fboResolution_ << "x" << fboResolution_;
    ofLogNotice("FboApp") << "Press 1-6 to switch FBO modes";
}

void FboApp::createFbos() {
    // Mode 1: Basic offscreen rendering
    fbo1_.allocate(fboResolution_, fboResolution_, GL_RGBA);
    fbo1_.clear();

    // Mode 2: Second pass rendering (same size)
    fbo2_.allocate(fboResolution_, fboResolution_, GL_RGBA);
    fbo2_.clear();

    // Mode 3: Multisample FBO (MSAA 4x)
    ofFboSettings msaaSettings;
    msaaSettings.width = fboResolution_;
    msaaSettings.height = fboResolution_;
    msaaSettings.internalformat = GL_RGBA;
    msaaSettings.numSamples = 4;
    msaaSettings.useDepth = true;
    multiSampleFbo_.allocateWithSettings(msaaSettings);
    multiSampleFbo_.clear();

    // Mode 4: Multiple color attachments
    ofFboSettings multiSettings;
    multiSettings.width = fboResolution_;
    multiSettings.height = fboResolution_;
    multiSettings.numColorbuffers = 2;
    multiSettings.useDepth = true;
    multiAttachmentFbo_.allocateWithSettings(multiSettings);
    multiAttachmentFbo_.clear();

    // Mode 5: Depth buffer FBO
    ofFboSettings depthSettings;
    depthSettings.width = fboResolution_;
    depthSettings.height = fboResolution_;
    depthSettings.internalformat = GL_RGBA;
    depthSettings.useDepth = true;
    depthSettings.depthStencilAsTexture = true;
    depthFbo_.allocateWithSettings(depthSettings);
    depthFbo_.clear();

    ofLogNotice("FboApp") << "FBOs created:";
    ofLogNotice("FboApp") << "  fbo1: " << fbo1_.getWidth() << "x" << fbo1_.getHeight();
    ofLogNotice("FboApp") << "  multiSampleFbo: " << multiSampleFbo_.getWidth() << "x" << multiSampleFbo_.getHeight() << " (4x MSAA)";
    ofLogNotice("FboApp") << "  multiAttachmentFbo: " << multiAttachmentFbo_.getWidth() << "x" << multiAttachmentFbo_.getHeight() << " (" << multiAttachmentFbo_.getNumTextures() << " attachments)";
    ofLogNotice("FboApp") << "  depthFbo: " << depthFbo_.getWidth() << "x" << depthFbo_.getHeight() << " (with depth)";
}

void FboApp::update() {
    if (animating_) {
        time_ += ofGetLastFrameTime();
        rotation_ += 30.0f * ofGetLastFrameTime();
        if (rotation_ > 360.0f) rotation_ -= 360.0f;
    }
}

void FboApp::draw() {
    // Render to appropriate FBO based on mode
    switch (currentMode_) {
        case 1:
            renderToFbo1();
            break;
        case 2:
            renderToFbo1();
            renderToFbo2();
            break;
        case 3:
            renderToMultiSampleFbo();
            break;
        case 4:
            renderToMultiAttachmentFbo();
            break;
        case 5:
            renderToDepthFbo();
            break;
        case 6:
            // Mode 6: Texture readback demo (render to fbo1, read back, modify, display)
            renderToFbo1();
            break;
    }

    // Draw to main screen
    ofBackground(40);
    drawFboContents();
    drawUI();
}

void FboApp::renderToFbo1() {
    fbo1_.begin();
    ofClear(20, 20, 30, 255);

    // Render 3D scene
    ofEnableDepthTest();
    ofEnableLighting();
    light_.enable();
    material_.begin();

    camera_.begin();

    // Central rotating sphere
    ofPushMatrix();
    ofRotateY(rotation_);
    ofRotateX(rotation_ * 0.5f);
    ofDrawSphere(0, 0, 0, 1.5f);
    ofPopMatrix();

    // Orbiting cubes
    for (int i = 0; i < 4; i++) {
        float angle = rotation_ + i * 90.0f;
        float x = cos(ofDegToRad(angle)) * 3.0f;
        float z = sin(ofDegToRad(angle)) * 3.0f;

        ofPushMatrix();
        ofTranslate(x, 0, z);
        ofRotateY(rotation_ * 2.0f);
        ofDrawBox(0, 0, 0, 0.8f);
        ofPopMatrix();
    }

    camera_.end();

    material_.end();
    light_.disable();
    ofDisableLighting();
    ofDisableDepthTest();

    fbo1_.end();
}

void FboApp::renderToFbo2() {
    // Second pass: apply post-processing effect using fbo1 as input
    fbo2_.begin();
    ofClear(0, 0, 0, 255);

    ofSetColor(255);

    // Draw fbo1 with offset copies (simple "glow" effect)
    ofEnableAlphaBlending();
    ofSetColor(255, 255, 255, 100);
    fbo1_.draw(2, 2);
    fbo1_.draw(-2, -2);
    fbo1_.draw(2, -2);
    fbo1_.draw(-2, 2);

    ofSetColor(255, 255, 255, 255);
    fbo1_.draw(0, 0);
    ofDisableAlphaBlending();

    fbo2_.end();
}

void FboApp::renderToMultiSampleFbo() {
    multiSampleFbo_.begin();
    ofClear(20, 20, 30, 255);

    ofEnableDepthTest();
    ofEnableLighting();
    light_.enable();
    material_.begin();

    camera_.begin();

    // Render complex shapes to show MSAA benefit
    for (int i = 0; i < 8; i++) {
        float angle = i * 45.0f + rotation_;
        float x = cos(ofDegToRad(angle)) * 2.5f;
        float z = sin(ofDegToRad(angle)) * 2.5f;

        ofPushMatrix();
        ofTranslate(x, 0, z);
        ofRotateY(angle);
        ofRotateX(rotation_);
        ofDrawCylinder(0, 0, 0, 0.3f, 1.5f);
        ofPopMatrix();
    }

    camera_.end();

    material_.end();
    light_.disable();
    ofDisableLighting();
    ofDisableDepthTest();

    multiSampleFbo_.end();
}

void FboApp::renderToMultiAttachmentFbo() {
    multiAttachmentFbo_.begin();

    // Render to first color attachment (default)
    ofClear(20, 20, 30, 255);

    ofEnableDepthTest();
    ofEnableLighting();
    light_.enable();
    material_.begin();

    camera_.begin();

    // Central sphere
    ofDrawSphere(0, 0, 0, 2.0f);

    // Orbiting objects
    for (int i = 0; i < 6; i++) {
        float angle = rotation_ + i * 60.0f;
        float x = cos(ofDegToRad(angle)) * 3.0f;
        float z = sin(ofDegToRad(angle)) * 3.0f;

        ofPushMatrix();
        ofTranslate(x, 0, z);
        ofRotateY(angle);
        ofDrawBox(0, 0, 0, 0.8f);
        ofPopMatrix();
    }

    camera_.end();

    material_.end();
    light_.disable();
    ofDisableLighting();
    ofDisableDepthTest();

    // Note: Multiple attachments would require custom shaders
    // to write to different render targets
    // This example shows the setup, actual multi-target rendering
    // would need fragment shader with multiple outputs

    multiAttachmentFbo_.end();
}

void FboApp::renderToDepthFbo() {
    depthFbo_.begin();
    ofClear(20, 20, 30, 255);

    ofEnableDepthTest();
    ofEnableLighting();
    light_.enable();
    material_.begin();

    camera_.begin();

    // Render objects at different depths
    for (int i = 0; i < 5; i++) {
        float z = -2.0f + i * 1.0f;
        float scale = 1.0f + i * 0.2f;

        ofPushMatrix();
        ofTranslate(0, 0, z);
        ofRotateY(rotation_ + i * 30.0f);
        ofDrawBox(0, 0, 0, scale);
        ofPopMatrix();
    }

    camera_.end();

    material_.end();
    light_.disable();
    ofDisableLighting();
    ofDisableDepthTest();

    depthFbo_.end();
}

void FboApp::drawFboContents() {
    ofSetColor(255);

    float windowW = ofGetWidth();
    float windowH = ofGetHeight();
    float fboAspect = 1.0f;  // Square FBOs
    float windowAspect = windowW / windowH;

    // Calculate display size to fit window while maintaining aspect ratio
    float displayW, displayH;
    if (windowAspect > fboAspect) {
        displayH = windowH * 0.7f;
        displayW = displayH * fboAspect;
    } else {
        displayW = windowW * 0.7f;
        displayH = displayW / fboAspect;
    }

    float x = (windowW - displayW) * 0.5f;
    float y = 50.0f;

    switch (currentMode_) {
        case 1:
            // Mode 1: Basic FBO
            fbo1_.draw(x, y, displayW, displayH);
            break;
        case 2:
            // Mode 2: Two-pass rendering (show both)
            fbo1_.draw(x - displayW * 0.55f, y, displayW * 0.5f, displayH * 0.5f);
            fbo2_.draw(x + displayW * 0.05f, y, displayW * 0.5f, displayH * 0.5f);
            break;
        case 3:
            // Mode 3: Multisample FBO
            multiSampleFbo_.draw(x, y, displayW, displayH);
            break;
        case 4:
            // Mode 4: Multiple attachments (show first attachment)
            multiAttachmentFbo_.draw(x, y, displayW, displayH);
            // Could show second attachment side-by-side if implemented
            break;
        case 5:
            // Mode 5: Depth FBO (show color and depth)
            depthFbo_.draw(x - displayW * 0.55f, y, displayW * 0.5f, displayH * 0.5f);
            // Draw depth texture (if available)
            if (depthFbo_.getDepthTexture().isAllocated()) {
                depthFbo_.getDepthTexture().draw(x + displayW * 0.05f, y, displayW * 0.5f, displayH * 0.5f);
            }
            break;
        case 6:
            // Mode 6: Texture readback demo
            {
                // Read pixels from FBO
                ofPixels pixels;
                fbo1_.readToPixels(pixels);

                // Modify pixels (invert colors)
                for (size_t i = 0; i < pixels.size(); i += 4) {
                    pixels[i + 0] = 255 - pixels[i + 0];  // R
                    pixels[i + 1] = 255 - pixels[i + 1];  // G
                    pixels[i + 2] = 255 - pixels[i + 2];  // B
                    // Keep alpha
                }

                // Draw original and modified side by side
                fbo1_.draw(x - displayW * 0.55f, y, displayW * 0.5f, displayH * 0.5f);

                // Draw modified pixels
                ofTexture modifiedTex;
                modifiedTex.allocate(pixels.getWidth(), pixels.getHeight(), GL_RGBA);
                modifiedTex.loadData(pixels);
                modifiedTex.draw(x + displayW * 0.05f, y, displayW * 0.5f, displayH * 0.5f);
            }
            break;
    }
}

void FboApp::drawUI() {
    if (!font_.isLoaded()) return;

    ofSetColor(255);

    float x = 20.0f;
    float y = ofGetHeight() - 160.0f;
    float lineHeight = 20.0f;

    // Mode descriptions
    std::string modeDesc[] = {
        "",
        "Mode 1: Basic Offscreen Rendering",
        "Mode 2: Two-Pass Rendering (FBO1 → FBO2)",
        "Mode 3: Multisample FBO (MSAA 4x)",
        "Mode 4: Multiple Color Attachments",
        "Mode 5: Depth Buffer Visualization",
        "Mode 6: Texture Readback & CPU Processing"
    };

    font_.drawString(modeDesc[currentMode_], x, y);
    y += lineHeight;

    font_.drawString("FBO Resolution: " + ofToString(fboResolution_) + "x" + ofToString(fboResolution_), x, y);
    y += lineHeight;

    font_.drawString("MSAA: " + std::string(useMsaa_ ? "ON (4x)" : "OFF"), x, y);
    y += lineHeight;

    font_.drawString("Animation: " + std::string(animating_ ? "ON" : "OFF"), x, y);
    y += lineHeight;

    font_.drawString("FPS: " + ofToString((int)ofGetFrameRate()), x, y);
    y += lineHeight;

    // Controls
    y += 10.0f;
    ofSetColor(150);
    font_.drawString("1-6: Switch modes | SPACE: Animation | M: MSAA | S: Save | UP/DOWN: Resolution", x, y);
}

void FboApp::keyPressed(int key) {
    switch (key) {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
            currentMode_ = key - '0';
            ofLogNotice("FboApp") << "Switched to mode " << currentMode_;
            break;

        case ' ':
            animating_ = !animating_;
            ofLogNotice("FboApp") << "Animation: " << (animating_ ? "ON" : "OFF");
            break;

        case 'm':
        case 'M':
            useMsaa_ = !useMsaa_;
            ofLogNotice("FboApp") << "MSAA: " << (useMsaa_ ? "ON" : "OFF");
            // Recreate FBOs with new setting
            createFbos();
            break;

        case 's':
        case 'S':
            {
                ofPixels pixels;
                fbo1_.readToPixels(pixels);
                ofImage img;
                img.setFromPixels(pixels);
                std::string filename = "fbo_capture_" + ofGetTimestampString() + ".png";
                img.save(filename);
                ofLogNotice("FboApp") << "Saved FBO contents to: " << filename;
            }
            break;

        case OF_KEY_UP:
            fboResolution_ = ofClamp(fboResolution_ + 128, 256, 2048);
            ofLogNotice("FboApp") << "FBO resolution: " << fboResolution_;
            createFbos();
            break;

        case OF_KEY_DOWN:
            fboResolution_ = ofClamp(fboResolution_ - 128, 256, 2048);
            ofLogNotice("FboApp") << "FBO resolution: " << fboResolution_;
            createFbos();
            break;
    }
}

void FboApp::windowResized(int w, int h) {
    ofLogNotice("FboApp") << "Window resized: " << w << "x" << h;
}

int main() {
    ofSetupOpenGL(1280, 720, OF_WINDOW);
    ofRunApp(new FboApp());
    return 0;
}
