/// Example 25: Sharp Realtime Camera
/// Demonstrates ofxSharp with live camera input - Real-time 3D Gaussian Splatting
///
/// This example shows how to:
/// - Capture frames from the built-in camera
/// - Generate 3D Gaussian clouds from live camera frames
/// - Update the cloud representation in real-time
/// - Handle continuous inference pipeline
/// - Optimize for real-time performance

#include "core/AppBase.h"
#include "oflike/graphics/ofGraphics.h"
#include "oflike/3d/ofCamera.h"
#include "oflike/3d/ofEasyCam.h"
#include "oflike/image/ofImage.h"
#include "oflike/types/ofColor.h"
#include "oflike/utils/ofUtils.h"
#include "addons/apple_native/ofxSharp/ofxSharp.h"
#include <sstream>
#include <iomanip>
#include <AVFoundation/AVFoundation.h>
#include <CoreMedia/CoreMedia.h>
#include <CoreVideo/CoreVideo.h>

using namespace oflike;

// AVFoundation camera capture delegate
@interface CameraCaptureDelegate : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>
@property (nonatomic, copy) void (^frameCallback)(CVPixelBufferRef);
@end

@implementation CameraCaptureDelegate
- (void)captureOutput:(AVCaptureOutput *)output
didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
       fromConnection:(AVCaptureConnection *)connection {
    CVPixelBufferRef pixelBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    if (pixelBuffer && self.frameCallback) {
        self.frameCallback(pixelBuffer);
    }
}
@end

class ofApp : public ofBaseApp {
public:
    // Sharp components
    ofxSharp sharp;
    ofEasyCam camera;

    // Camera capture
    AVCaptureSession* captureSession;
    CameraCaptureDelegate* captureDelegate;
    dispatch_queue_t videoQueue;

    // Frame buffers
    ofImage currentFrame;
    ofImage displayFrame;
    bool hasNewFrame = false;
    bool displayCamera = true;

    // Generation state
    bool autoGenerate = false;
    bool isGenerating = false;
    bool cloudReady = false;
    float generationCooldown = 0.0f;
    float cooldownDuration = 2.0f; // Minimum seconds between generations

    // Stats
    double lastInferenceTime = 0.0;
    size_t gaussianCount = 0;
    int frameCount = 0;
    int capturedFrames = 0;
    float lastGenerationTime = 0.0f;

    // UI
    std::string statusMessage = "Press SPACE to start camera";
    bool cameraActive = false;

    // Settings
    float splatScale = 1.0f;
    float opacityScale = 1.0f;
    int shDegree = 3;

    void setup() override {
        ofSetFrameRate(60);
        ofBackground(20);

        // Setup Sharp
        if (!sharp.setup()) {
            statusMessage = "ERROR: Failed to initialize ofxSharp";
            ofLogError("sharp_realtime_camera") << statusMessage;
            return;
        }

        // Configure 3D camera
        camera.setDistance(5.0f);
        camera.setTarget(ofVec3f(0, 0, 0));
        camera.enableMouseInput();
        camera.setAutoDistance(false);

        // Allocate frame buffers
        currentFrame.allocate(640, 480, OF_IMAGE_COLOR);
        displayFrame.allocate(640, 480, OF_IMAGE_COLOR);

        ofLogNotice("sharp_realtime_camera") << "ofxSharp initialized successfully";
        ofLogNotice("sharp_realtime_camera") << "Neural Engine: " << (sharp.isUsingNeuralEngine() ? "YES" : "NO");
        ofLogNotice("sharp_realtime_camera") << "Press SPACE to start camera";
    }

    void update() override {
        frameCount++;

        // Update generation state
        isGenerating = sharp.isGenerating();
        cloudReady = sharp.hasCloud();

        if (cloudReady) {
            gaussianCount = sharp.getGaussianCount();
        }

        // Update cooldown timer
        if (generationCooldown > 0.0f) {
            generationCooldown -= 1.0f / 60.0f;
        }

        // Auto-generate from camera if enabled
        if (autoGenerate && cameraActive && hasNewFrame && !isGenerating && generationCooldown <= 0.0f) {
            generateFromCurrentFrame();
            hasNewFrame = false;
        }

        // Update display frame
        if (hasNewFrame && displayCamera) {
            displayFrame = currentFrame;
            hasNewFrame = false;
        }
    }

    void draw() override {
        ofBackground(20);

        // Split screen: camera preview (left) and 3D view (right)
        int w = ofGetWidth();
        int h = ofGetHeight();
        int previewWidth = w / 3;

        // Draw camera preview on left
        if (displayCamera && cameraActive) {
            ofSetColor(255);

            // Calculate aspect-correct dimensions
            float previewAspect = displayFrame.getWidth() / (float)displayFrame.getHeight();
            float targetAspect = previewWidth / (float)h;

            int drawW = previewWidth;
            int drawH = h;
            int drawX = 0;
            int drawY = 0;

            if (previewAspect > targetAspect) {
                drawH = previewWidth / previewAspect;
                drawY = (h - drawH) / 2;
            } else {
                drawW = h * previewAspect;
                drawX = (previewWidth - drawW) / 2;
            }

            displayFrame.draw(drawX, drawY, drawW, drawH);

            // Draw border
            ofNoFill();
            ofSetColor(100);
            ofDrawRectangle(0, 0, previewWidth, h);
            ofFill();
        }

        // Draw 3D view on right
        ofPushMatrix();
        ofTranslate(previewWidth, 0);

        // Set viewport for 3D rendering
        int renderWidth = w - previewWidth;
        int renderHeight = h;

        if (cloudReady) {
            ofEnableDepthTest();

            // Setup camera with offset viewport
            // Note: ofCamera doesn't support viewport directly,
            // so we use translate instead

            camera.begin();

            // Draw coordinate axes
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
        } else {
            // No cloud yet - draw placeholder
            ofSetColor(100);
            ofDrawBitmapString("3D View\n\nPress SPACE to start camera\nPress G to generate 3D",
                             renderWidth / 2 - 100, renderHeight / 2);
        }

        ofPopMatrix();

        // Draw UI overlay
        drawUI();
    }

    void drawUI() {
        ofSetColor(255);
        int y = 20;
        int lineHeight = 18;
        int x = ofGetWidth() / 3 + 20; // Start after preview

        // Title
        ofDrawBitmapString("ofxSharp - Real-time Camera", x, y);
        y += lineHeight * 2;

        // Status
        ofSetColor(isGenerating ? ofColor(255, 200, 0) : ofColor(255));
        ofDrawBitmapString("Status: " + statusMessage, x, y);
        y += lineHeight;

        // Camera info
        ofSetColor(255);
        ofDrawBitmapString("Camera: " + std::string(cameraActive ? "ACTIVE" : "INACTIVE"), x, y);
        y += lineHeight;

        if (cameraActive) {
            std::ostringstream fps;
            fps << "Captured Frames: " << capturedFrames;
            ofDrawBitmapString(fps.str(), x, y);
            y += lineHeight;
        }

        // Generation stats
        if (cloudReady) {
            y += lineHeight;
            ofSetColor(200, 255, 200);
            ofDrawBitmapString("3D Cloud Ready:", x, y);
            y += lineHeight;

            std::ostringstream stats;
            stats << "  Gaussians: " << gaussianCount;
            ofDrawBitmapString(stats.str(), x, y);
            y += lineHeight;

            stats.str("");
            stats << "  Inference Time: " << (int)lastInferenceTime << " ms";
            ofDrawBitmapString(stats.str(), x, y);
            y += lineHeight;

            stats.str("");
            stats << "  Neural Engine: " << (sharp.isUsingNeuralEngine() ? "YES" : "NO");
            ofDrawBitmapString(stats.str(), x, y);
            y += lineHeight;

            // Render stats
            const auto& renderStats = sharp.getRenderStats();
            stats.str("");
            stats << "  Visible Splats: " << renderStats.visibleSplatCount;
            ofDrawBitmapString(stats.str(), x, y);
            y += lineHeight;

            stats.str("");
            stats << "  Render Time: " << std::fixed << std::setprecision(2)
                  << renderStats.gpuTimeMs << " ms";
            ofDrawBitmapString(stats.str(), x, y);
            y += lineHeight;
        }

        // Auto-generate status
        y += lineHeight;
        ofSetColor(autoGenerate ? ofColor(0, 255, 0) : ofColor(180));
        ofDrawBitmapString("Auto-generate: " + std::string(autoGenerate ? "ON" : "OFF"), x, y);
        y += lineHeight;

        if (generationCooldown > 0.0f) {
            ofSetColor(255, 200, 0);
            std::ostringstream cooldown;
            cooldown << "Cooldown: " << std::fixed << std::setprecision(1) << generationCooldown << "s";
            ofDrawBitmapString(cooldown.str(), x, y);
            y += lineHeight;
        }

        // Settings
        y += lineHeight;
        ofSetColor(200, 200, 255);
        ofDrawBitmapString("Settings:", x, y);
        y += lineHeight;

        std::ostringstream settings;
        settings << "  Splat Scale: " << std::fixed << std::setprecision(1) << splatScale;
        ofDrawBitmapString(settings.str(), x, y);
        y += lineHeight;

        settings.str("");
        settings << "  Opacity: " << std::fixed << std::setprecision(1) << opacityScale;
        ofDrawBitmapString(settings.str(), x, y);
        y += lineHeight;

        settings.str("");
        settings << "  SH Degree: " << shDegree;
        ofDrawBitmapString(settings.str(), x, y);
        y += lineHeight;

        settings.str("");
        settings << "  Cooldown: " << std::fixed << std::setprecision(1) << cooldownDuration << "s";
        ofDrawBitmapString(settings.str(), x, y);
        y += lineHeight;

        // Controls
        y += lineHeight;
        ofSetColor(180);
        ofDrawBitmapString("Controls:", x, y);
        y += lineHeight;
        ofDrawBitmapString("  SPACE - Start/Stop camera", x, y);
        y += lineHeight;
        ofDrawBitmapString("  G     - Generate 3D from current frame", x, y);
        y += lineHeight;
        ofDrawBitmapString("  A     - Toggle auto-generate", x, y);
        y += lineHeight;
        ofDrawBitmapString("  V     - Toggle camera preview", x, y);
        y += lineHeight;
        ofDrawBitmapString("  S     - Save cloud to PLY", x, y);
        y += lineHeight;
        ofDrawBitmapString("  +/-   - Adjust splat scale", x, y);
        y += lineHeight;
        ofDrawBitmapString("  [/]   - Adjust opacity", x, y);
        y += lineHeight;
        ofDrawBitmapString("  1-4   - Set SH degree (0-3)", x, y);
        y += lineHeight;
        ofDrawBitmapString("  </,   - Decrease cooldown", x, y);
        y += lineHeight;
        ofDrawBitmapString("  >/.   - Increase cooldown", x, y);
        y += lineHeight;
        ofDrawBitmapString("  R     - Reset transform", x, y);
        y += lineHeight;
        ofDrawBitmapString("  Mouse - Rotate 3D camera", x, y);

        // FPS
        ofSetColor(255, 200);
        ofDrawBitmapString("FPS: " + ofToString((int)ofGetFrameRate()), ofGetWidth() - 100, 20);
    }

    void keyPressed(int key) override {
        if (key == ' ') {
            if (cameraActive) {
                stopCamera();
            } else {
                startCamera();
            }
        }
        else if (key == 'g' || key == 'G') {
            if (cameraActive) {
                generateFromCurrentFrame();
            } else {
                statusMessage = "Start camera first (SPACE)";
            }
        }
        else if (key == 'a' || key == 'A') {
            autoGenerate = !autoGenerate;
            statusMessage = "Auto-generate: " + std::string(autoGenerate ? "ON" : "OFF");
        }
        else if (key == 'v' || key == 'V') {
            displayCamera = !displayCamera;
            statusMessage = "Camera preview: " + std::string(displayCamera ? "ON" : "OFF");
        }
        else if (key == 's' || key == 'S') {
            if (cloudReady) {
                std::string filepath = "camera_cloud_" + ofGetTimestampString() + ".ply";
                if (sharp.saveCloud(filepath)) {
                    statusMessage = "Saved cloud to: " + filepath;
                    ofLogNotice("sharp_realtime_camera") << statusMessage;
                } else {
                    statusMessage = "Failed to save cloud";
                    ofLogError("sharp_realtime_camera") << statusMessage;
                }
            }
        }
        else if (key == '+' || key == '=') {
            splatScale = ofClamp(splatScale + 0.1f, 0.1f, 5.0f);
            sharp.setSplatScale(splatScale);
            statusMessage = "Splat scale: " + ofToString(splatScale, 1);
        }
        else if (key == '-' || key == '_') {
            splatScale = ofClamp(splatScale - 0.1f, 0.1f, 5.0f);
            sharp.setSplatScale(splatScale);
            statusMessage = "Splat scale: " + ofToString(splatScale, 1);
        }
        else if (key == '[') {
            opacityScale = ofClamp(opacityScale - 0.1f, 0.1f, 1.0f);
            sharp.setOpacityScale(opacityScale);
            statusMessage = "Opacity: " + ofToString(opacityScale, 1);
        }
        else if (key == ']') {
            opacityScale = ofClamp(opacityScale + 0.1f, 0.1f, 1.0f);
            sharp.setOpacityScale(opacityScale);
            statusMessage = "Opacity: " + ofToString(opacityScale, 1);
        }
        else if (key >= '1' && key <= '4') {
            shDegree = key - '1';
            sharp.setMaxSHDegree(shDegree);
            statusMessage = "SH degree: " + ofToString(shDegree);
        }
        else if (key == '<' || key == ',') {
            cooldownDuration = std::max(0.5f, cooldownDuration - 0.5f);
            statusMessage = "Cooldown: " + ofToString(cooldownDuration, 1) + "s";
        }
        else if (key == '>' || key == '.') {
            cooldownDuration = std::min(10.0f, cooldownDuration + 0.5f);
            statusMessage = "Cooldown: " + ofToString(cooldownDuration, 1) + "s";
        }
        else if (key == 'r' || key == 'R') {
            sharp.resetTransform();
            statusMessage = "Transform reset";
        }
    }

    void startCamera() {
        @autoreleasepool {
            // Request camera permission first
            if (@available(macOS 10.14, *)) {
                AVAuthorizationStatus authStatus = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];
                if (authStatus == AVAuthorizationStatusRestricted || authStatus == AVAuthorizationStatusDenied) {
                    statusMessage = "ERROR: Camera permission denied";
                    ofLogError("sharp_realtime_camera") << statusMessage;
                    return;
                }
            }

            // Create capture session
            captureSession = [[AVCaptureSession alloc] init];
            [captureSession setSessionPreset:AVCaptureSessionPreset640x480];

            // Get default video device
            AVCaptureDevice* device = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
            if (!device) {
                statusMessage = "ERROR: No camera found";
                ofLogError("sharp_realtime_camera") << statusMessage;
                return;
            }

            // Create input
            NSError* error = nil;
            AVCaptureDeviceInput* input = [AVCaptureDeviceInput deviceInputWithDevice:device error:&error];
            if (error || ![captureSession canAddInput:input]) {
                statusMessage = "ERROR: Cannot add camera input";
                ofLogError("sharp_realtime_camera") << statusMessage;
                return;
            }
            [captureSession addInput:input];

            // Create output
            AVCaptureVideoDataOutput* output = [[AVCaptureVideoDataOutput alloc] init];
            output.videoSettings = @{
                (NSString*)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA)
            };

            // Create delegate and queue
            captureDelegate = [[CameraCaptureDelegate alloc] init];
            videoQueue = dispatch_queue_create("com.oflike.sharp.videoQueue", DISPATCH_QUEUE_SERIAL);
            [output setSampleBufferDelegate:captureDelegate queue:videoQueue];

            // Set frame callback
            __weak ofApp* weakSelf = this;
            captureDelegate.frameCallback = ^(CVPixelBufferRef pixelBuffer) {
                if (weakSelf) {
                    weakSelf->processCameraFrame(pixelBuffer);
                }
            };

            if (![captureSession canAddOutput:output]) {
                statusMessage = "ERROR: Cannot add video output";
                ofLogError("sharp_realtime_camera") << statusMessage;
                return;
            }
            [captureSession addOutput:output];

            // Start session
            [captureSession startRunning];

            cameraActive = true;
            capturedFrames = 0;
            statusMessage = "Camera started - Press G to generate 3D";
            ofLogNotice("sharp_realtime_camera") << statusMessage;
        }
    }

    void stopCamera() {
        @autoreleasepool {
            if (captureSession) {
                [captureSession stopRunning];
                captureSession = nil;
            }

            if (videoQueue) {
                videoQueue = nil;
            }

            captureDelegate = nil;
            cameraActive = false;
            statusMessage = "Camera stopped";
            ofLogNotice("sharp_realtime_camera") << statusMessage;
        }
    }

    void processCameraFrame(CVPixelBufferRef pixelBuffer) {
        @autoreleasepool {
            CVPixelBufferLockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);

            size_t width = CVPixelBufferGetWidth(pixelBuffer);
            size_t height = CVPixelBufferGetHeight(pixelBuffer);
            void* baseAddress = CVPixelBufferGetBaseAddress(pixelBuffer);
            size_t bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer);

            // Convert BGRA to RGB
            if (currentFrame.getWidth() != width || currentFrame.getHeight() != height) {
                currentFrame.allocate(width, height, OF_IMAGE_COLOR);
            }

            ofPixels& pixels = currentFrame.getPixels();
            unsigned char* src = (unsigned char*)baseAddress;

            for (size_t y = 0; y < height; y++) {
                unsigned char* row = src + y * bytesPerRow;
                for (size_t x = 0; x < width; x++) {
                    size_t srcIdx = x * 4; // BGRA
                    size_t dstIdx = (y * width + x) * 3; // RGB

                    pixels[dstIdx + 0] = row[srcIdx + 2]; // R
                    pixels[dstIdx + 1] = row[srcIdx + 1]; // G
                    pixels[dstIdx + 2] = row[srcIdx + 0]; // B
                }
            }

            CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);

            hasNewFrame = true;
            capturedFrames++;
        }
    }

    void generateFromCurrentFrame() {
        if (isGenerating) {
            statusMessage = "Generation already in progress...";
            return;
        }

        if (!currentFrame.isAllocated() || currentFrame.getWidth() == 0) {
            statusMessage = "No camera frame available";
            return;
        }

        statusMessage = "Generating 3D from camera frame...";
        ofLogNotice("sharp_realtime_camera") << statusMessage;

        lastGenerationTime = ofGetElapsedTimef();

        // Generate asynchronously
        sharp.generateFromImageAsync(currentFrame, [this](bool success) {
            if (success) {
                lastInferenceTime = sharp.getLastInferenceTime();
                float totalTime = ofGetElapsedTimef() - lastGenerationTime;
                statusMessage = "Generation complete! (" + ofToString((int)lastInferenceTime) + " ms)";
                ofLogNotice("sharp_realtime_camera") << statusMessage;
                ofLogNotice("sharp_realtime_camera") << "Total time: " << (int)(totalTime * 1000) << " ms";
                ofLogNotice("sharp_realtime_camera") << "Gaussian count: " << sharp.getGaussianCount();

                // Reset cooldown
                generationCooldown = cooldownDuration;
            } else {
                statusMessage = "Generation failed: " + sharp.getLastError();
                ofLogError("sharp_realtime_camera") << statusMessage;
            }
        });
    }

    void exit() override {
        stopCamera();
    }
};

// Entry point
#define OF_APP ofApp
#include "oflike/app/ofMain.h"
