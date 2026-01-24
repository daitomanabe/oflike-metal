/// Example 24: Sharp Video Export
/// Demonstrates Sharp::VideoExporter - Export Gaussian Splatting animations to video
///
/// This example shows how to:
/// - Configure video export settings (codec, resolution, quality)
/// - Create animated camera paths
/// - Export scene animations to video files
/// - Monitor export progress
/// - Use different codecs (H.265, ProRes)

#include "core/AppBase.h"
#include "oflike/graphics/ofGraphics.h"
#include "oflike/3d/ofCamera.h"
#include "oflike/3d/ofEasyCam.h"
#include "oflike/types/ofColor.h"
#include "oflike/utils/ofUtils.h"
#include "addons/apple_native/ofxSharp/SharpVideoExporter.h"
#include "addons/apple_native/ofxSharp/SharpCameraPath.h"
#include "addons/apple_native/ofxSharp/SharpScene.h"
#include "addons/apple_native/ofxSharp/SharpGaussianCloud.h"
#include "addons/apple_native/ofxSharp/SharpGaussian.h"
#include <sstream>
#include <iomanip>

using namespace oflike;

class ofApp : public ofBaseApp {
public:
    // Sharp components
    Sharp::SharpScene scene;
    Sharp::CameraPath cameraPath;
    Sharp::VideoExporter exporter;
    ofEasyCam camera;

    // Scene objects
    Sharp::ObjectID centerObj = Sharp::kInvalidObjectID;
    Sharp::ObjectID orbitObj = Sharp::kInvalidObjectID;

    // Export state
    bool isExporting = false;
    bool sceneReady = false;
    float exportProgress = 0.0f;
    size_t exportedFrames = 0;
    size_t totalFrames = 0;
    std::string exportStatus = "Ready";
    std::string lastExportPath;

    // Export settings
    Sharp::VideoExportSettings settings;
    int selectedCodec = 1; // 0=H264, 1=H265, 2=ProRes422, 3=ProRes4444
    int selectedResolution = 1; // 0=720p, 1=1080p, 2=1440p, 3=4K
    int selectedQuality = 2; // 0=Low, 1=Medium, 2=High, 3=Lossless
    int exportFramerate = 30;
    float exportDuration = 5.0f; // seconds

    // UI state
    std::string statusMessage = "Press SPACE to create scene, then E to export";

    void setup() override {
        ofSetFrameRate(60);
        ofBackground(20);

        // Configure camera
        camera.setDistance(6.0f);
        camera.setTarget(ofVec3f(0, 0, 0));
        camera.enableMouseInput();
        camera.setAutoDistance(false);

        // Setup default export settings
        settings.codec = Sharp::VideoCodec::H265;
        settings.resolution = Sharp::VideoResolution::HD_1080p;
        settings.quality = Sharp::VideoQuality::High;
        settings.framerate = 30;
        settings.useHardwareAcceleration = true;

        ofLog() << "Sharp::VideoExporter demonstration initialized";
        ofLog() << "Hardware acceleration: " << (settings.useHardwareAcceleration ? "ON" : "OFF");
    }

    void update() override {
        if (isExporting) {
            // Export progress is updated via callback
            exportProgress = exporter.getProgress();
            exportedFrames = exporter.getEncodedFrameCount();
        }
    }

    void draw() override {
        ofBackground(20);

        // Draw 3D preview
        if (sceneReady) {
            ofEnableDepthTest();
            camera.begin();

            // Draw coordinate axes
            drawAxes();

            // Draw scene visualization
            drawSceneBounds();

            camera.end();
            ofDisableDepthTest();
        }

        // Draw UI
        drawUI();
    }

    void drawAxes() {
        float axisLength = 2.0f;
        ofSetColor(255, 0, 0);
        ofDrawLine(0, 0, 0, axisLength, 0, 0);
        ofSetColor(0, 255, 0);
        ofDrawLine(0, 0, 0, 0, axisLength, 0);
        ofSetColor(0, 0, 255);
        ofDrawLine(0, 0, 0, 0, 0, axisLength);
    }

    void drawSceneBounds() {
        auto ids = scene.getObjectIDs();
        for (auto id : ids) {
            const Sharp::SceneObject* obj = scene.getObject(id);
            if (!obj || !obj->visible) continue;

            ofVec3f pos(obj->position.x, obj->position.y, obj->position.z);
            float scale = obj->scale.x;

            ofPushMatrix();
            ofTranslate(pos);
            ofScale(scale, scale, scale);

            std::string name = scene.getName(id);
            if (name == "center") ofSetColor(255, 100, 100, 150);
            else if (name == "orbit") ofSetColor(100, 255, 100, 150);
            else ofSetColor(255, 150);

            ofNoFill();
            ofDrawBox(0, 0, 0, 0.5f);
            ofFill();

            ofPopMatrix();
        }
    }

    void drawUI() {
        ofSetColor(255);
        int y = 20;
        int lineHeight = 18;

        // Title
        ofDrawBitmapString("Sharp::VideoExporter - Animation Export", 20, y);
        y += lineHeight * 2;

        // Status
        ofDrawBitmapString("Status: " + statusMessage, 20, y);
        y += lineHeight * 2;

        if (sceneReady) {
            // Scene info
            std::ostringstream info;
            info << "Scene: " << scene.getObjectCount() << " objects, "
                 << scene.getTotalGaussianCount() << " gaussians";
            ofDrawBitmapString(info.str(), 20, y);
            y += lineHeight * 2;
        }

        // Export settings
        ofDrawBitmapString("Export Settings:", 20, y);
        y += lineHeight;

        std::string codecNames[] = {"H.264", "H.265", "ProRes 422", "ProRes 4444"};
        ofDrawBitmapString("  Codec: " + codecNames[selectedCodec] + " (1-4)", 20, y);
        y += lineHeight;

        std::string resNames[] = {"720p", "1080p", "1440p", "4K"};
        ofDrawBitmapString("  Resolution: " + resNames[selectedResolution] + " (5-8)", 20, y);
        y += lineHeight;

        std::string qualNames[] = {"Low", "Medium", "High", "Lossless"};
        ofDrawBitmapString("  Quality: " + qualNames[selectedQuality] + " (Q/W)", 20, y);
        y += lineHeight;

        std::ostringstream fps;
        fps << "  Framerate: " << exportFramerate << " fps (F/G)";
        ofDrawBitmapString(fps.str(), 20, y);
        y += lineHeight;

        std::ostringstream dur;
        dur << "  Duration: " << std::fixed << std::setprecision(1)
            << exportDuration << " sec (D/S)";
        ofDrawBitmapString(dur.str(), 20, y);
        y += lineHeight;

        totalFrames = (size_t)(exportDuration * exportFramerate);
        std::ostringstream frames;
        frames << "  Total Frames: " << totalFrames;
        ofDrawBitmapString(frames.str(), 20, y);
        y += lineHeight * 2;

        // Export progress
        if (isExporting) {
            ofSetColor(255, 255, 0);
            ofDrawBitmapString("EXPORTING...", 20, y);
            y += lineHeight;

            ofSetColor(255);
            std::ostringstream progress;
            progress << "Progress: " << (int)(exportProgress * 100) << "% ("
                     << exportedFrames << "/" << totalFrames << " frames)";
            ofDrawBitmapString(progress.str(), 20, y);
            y += lineHeight;

            ofDrawBitmapString("Status: " + exportStatus, 20, y);
            y += lineHeight;

            std::ostringstream speed;
            speed << "Speed: " << std::fixed << std::setprecision(1)
                  << exporter.getEncodingSpeed() << " fps";
            ofDrawBitmapString(speed.str(), 20, y);
            y += lineHeight;

            float timeRemaining = exporter.getEstimatedTimeRemaining();
            if (timeRemaining > 0) {
                std::ostringstream eta;
                eta << "ETA: " << (int)timeRemaining << " seconds";
                ofDrawBitmapString(eta.str(), 20, y);
                y += lineHeight;
            }

            // Progress bar
            y += lineHeight;
            ofSetColor(100);
            ofDrawRectangle(20, y, 400, 20);
            ofSetColor(0, 255, 0);
            ofDrawRectangle(20, y, 400 * exportProgress, 20);
            y += 30;
        } else if (!lastExportPath.empty()) {
            ofSetColor(0, 255, 0);
            ofDrawBitmapString("Last export: " + lastExportPath, 20, y);
            y += lineHeight * 2;
        }

        // Controls
        ofSetColor(180);
        ofDrawBitmapString("Controls:", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  SPACE - Create demo scene", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  E     - Export animation", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  X     - Cancel export", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  C     - Clear scene", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  1-4   - Select codec", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  5-8   - Select resolution", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  Q/W   - Adjust quality", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  F/G   - Adjust framerate", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  D/S   - Adjust duration", 20, y);

        // FPS
        ofSetColor(255);
        std::ostringstream fpsStr;
        fpsStr << "FPS: " << (int)ofGetFrameRate();
        ofDrawBitmapString(fpsStr.str(), ofGetWidth() - 100, 20);
    }

    void keyPressed(int key) override {
        if (isExporting) {
            if (key == 'x' || key == 'X') {
                cancelExport();
            }
            return; // Ignore other keys during export
        }

        if (key == ' ') {
            createScene();
        }
        else if (key == 'e' || key == 'E') {
            if (sceneReady) {
                startExport();
            } else {
                statusMessage = "Create scene first (SPACE)";
            }
        }
        else if (key == 'c' || key == 'C') {
            clearScene();
        }
        else if (key >= '1' && key <= '4') {
            selectedCodec = key - '1';
            updateExportSettings();
        }
        else if (key >= '5' && key <= '8') {
            selectedResolution = key - '5';
            updateExportSettings();
        }
        else if (key == 'q' || key == 'Q') {
            selectedQuality = std::max(0, selectedQuality - 1);
            updateExportSettings();
        }
        else if (key == 'w' || key == 'W') {
            selectedQuality = std::min(3, selectedQuality + 1);
            updateExportSettings();
        }
        else if (key == 'f' || key == 'F') {
            exportFramerate = std::max(15, exportFramerate - 15);
        }
        else if (key == 'g' || key == 'G') {
            exportFramerate = std::min(120, exportFramerate + 15);
        }
        else if (key == 'd' || key == 'D') {
            exportDuration = std::max(1.0f, exportDuration - 1.0f);
        }
        else if (key == 's' || key == 'S') {
            exportDuration = std::min(30.0f, exportDuration + 1.0f);
        }
    }

    void updateExportSettings() {
        // Update codec
        Sharp::VideoCodec codecs[] = {
            Sharp::VideoCodec::H264,
            Sharp::VideoCodec::H265,
            Sharp::VideoCodec::ProRes422,
            Sharp::VideoCodec::ProRes4444
        };
        settings.codec = codecs[selectedCodec];

        // Update resolution
        Sharp::VideoResolution resolutions[] = {
            Sharp::VideoResolution::HD_720p,
            Sharp::VideoResolution::HD_1080p,
            Sharp::VideoResolution::QHD_1440p,
            Sharp::VideoResolution::UHD_4K
        };
        settings.resolution = resolutions[selectedResolution];

        // Update quality
        Sharp::VideoQuality qualities[] = {
            Sharp::VideoQuality::Low,
            Sharp::VideoQuality::Medium,
            Sharp::VideoQuality::High,
            Sharp::VideoQuality::Lossless
        };
        settings.quality = qualities[selectedQuality];

        settings.framerate = exportFramerate;

        statusMessage = "Settings updated";
    }

    void createScene() {
        if (sceneReady) {
            clearScene();
        }

        statusMessage = "Creating demo scene...";
        ofLog() << statusMessage;

        // Create 2 Gaussian clouds
        Sharp::GaussianCloud cloud1 = createProceduralCloud(120, ofColor(255, 100, 100));
        Sharp::GaussianCloud cloud2 = createProceduralCloud(80, ofColor(100, 255, 100));

        // Add to scene
        centerObj = scene.addCloud(std::move(cloud1), "center");
        orbitObj = scene.addCloud(std::move(cloud2), "orbit");

        // Set positions
        scene.setPosition(centerObj, oflike::float3{0, 0, 0});
        scene.setPosition(orbitObj, oflike::float3{2, 0, 0});

        // Setup camera path - orbit around scene
        cameraPath.setOrbitPath(
            oflike::float3{0, 0, 0},  // center
            5.0f,                      // radius
            exportDuration,            // duration
            oflike::float3{0, 1, 0},  // Y-up
            0.0f                       // start angle
        );
        cameraPath.setPlaybackMode(Sharp::PlaybackMode::Loop);

        sceneReady = true;
        statusMessage = "Scene ready. Press E to export animation.";
        ofLog() << statusMessage;
    }

    Sharp::GaussianCloud createProceduralCloud(int count, const ofColor& baseColor) {
        Sharp::GaussianCloud cloud;
        std::vector<Sharp::Gaussian> gaussians;
        gaussians.reserve(count);

        for (int i = 0; i < count; i++) {
            Sharp::Gaussian g;

            // Position - random in unit sphere
            float theta = ofRandom(TWO_PI);
            float phi = ofRandom(PI);
            float r = ofRandom(0.5f);

            g.position.x = r * sin(phi) * cos(theta);
            g.position.y = r * sin(phi) * sin(theta);
            g.position.z = r * cos(phi);

            // Color
            g.color.x = ofClamp(baseColor.r / 255.0f + ofRandom(-0.1f, 0.1f), 0.0f, 1.0f);
            g.color.y = ofClamp(baseColor.g / 255.0f + ofRandom(-0.1f, 0.1f), 0.0f, 1.0f);
            g.color.z = ofClamp(baseColor.b / 255.0f + ofRandom(-0.1f, 0.1f), 0.0f, 1.0f);

            g.opacity = ofRandom(0.8f, 1.0f);
            g.scale = {ofRandom(0.08f, 0.12f), ofRandom(0.08f, 0.12f), ofRandom(0.08f, 0.12f)};
            g.rotation = simd_quaternion(0, 0, 0, 1);

            for (int j = 0; j < 48; j++) {
                g.sh_coefficients[j] = 0.0f;
            }

            gaussians.push_back(g);
        }

        if (!cloud.fromGaussians(gaussians.data(), gaussians.size())) {
            ofLog() << "ERROR: Failed to create Gaussian cloud";
        }

        return cloud;
    }

    void startExport() {
        if (isExporting) {
            statusMessage = "Already exporting!";
            return;
        }

        // Update settings
        updateExportSettings();

        // Setup exporter
        if (!exporter.setup(settings)) {
            statusMessage = "ERROR: Failed to setup exporter: " + exporter.getLastError();
            ofLog() << statusMessage;
            return;
        }

        // Generate output path
        std::string codecExt = Sharp::VideoExporter::getFileExtension(settings.codec);
        lastExportPath = "sharp_export_" + ofGetTimestampString() + codecExt;

        // Set progress callback
        exporter.setProgressCallback([this](float progress, size_t frameIdx, size_t total) {
            exportProgress = progress;
            exportedFrames = frameIdx;
            totalFrames = total;
        });

        // Set expected total frames
        totalFrames = (size_t)(exportDuration * exportFramerate);
        exporter.setExpectedTotalFrames(totalFrames);

        // Start export
        statusMessage = "Starting export: " + lastExportPath;
        ofLog() << statusMessage;

        // Use convenience method to export scene with camera path
        // This is done synchronously here for simplicity
        // In a real app, you'd do this on a background thread
        bool success = exporter.exportScene(scene, cameraPath, lastExportPath, settings);

        if (success) {
            statusMessage = "Export completed: " + lastExportPath;
            ofLog() << statusMessage;

            size_t fileSize = exporter.getEstimatedFileSize();
            float fileSizeMB = fileSize / 1024.0f / 1024.0f;
            ofLog() << "File size: " << fileSizeMB << " MB";
        } else {
            statusMessage = "Export failed: " + exporter.getLastError();
            ofLog() << "ERROR: " << statusMessage;
        }

        isExporting = false;
        exportProgress = 0.0f;
    }

    void cancelExport() {
        if (!isExporting) return;

        exporter.cancelExport();
        isExporting = false;
        exportProgress = 0.0f;
        statusMessage = "Export cancelled";
        ofLog() << statusMessage;
    }

    void clearScene() {
        scene.clear();
        centerObj = orbitObj = Sharp::kInvalidObjectID;
        sceneReady = false;
        statusMessage = "Scene cleared";
        ofLog() << statusMessage;
    }
};

// Entry point
#define OF_APP ofApp
#include "oflike/app/ofMain.h"
