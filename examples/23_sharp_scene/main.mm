/// Example 23: Sharp Scene
/// Demonstrates Sharp::SharpScene - Multi-object Gaussian Splatting scene management
///
/// NOTE: This is a demonstration of the scene management API.
/// Actual rendering requires integration with the Sharp rendering pipeline.
/// This example shows scene creation, object manipulation, and serialization.

#include "core/AppBase.h"
#include "oflike/graphics/ofGraphics.h"
#include "oflike/3d/ofCamera.h"
#include "oflike/3d/ofEasyCam.h"
#include "oflike/types/ofColor.h"
#include "oflike/utils/ofUtils.h"
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
    ofEasyCam camera;

    // Scene object IDs
    Sharp::ObjectID centerObj = Sharp::kInvalidObjectID;
    Sharp::ObjectID leftObj = Sharp::kInvalidObjectID;
    Sharp::ObjectID rightObj = Sharp::kInvalidObjectID;
    Sharp::ObjectID topObj = Sharp::kInvalidObjectID;

    // Animation
    float animationTime = 0.0f;
    bool animate = true;

    // UI state
    std::string statusMessage = "Press SPACE to create scene";
    bool sceneReady = false;
    int selectedObject = -1; // -1 = none, 0-3 = object index

    void setup() override {
        ofSetFrameRate(60);
        ofBackground(20);

        // Configure camera
        camera.setDistance(8.0f);
        camera.setTarget(ofVec3f(0, 0, 0));
        camera.enableMouseInput();
        camera.setAutoDistance(false);

        ofLog() << "Sharp::SharpScene API demonstration initialized";
        ofLog() << "This example shows scene management without actual rendering";
    }

    void update() override {
        if (animate && sceneReady) {
            float dt = 1.0f / 60.0f; // Fixed timestep
            animationTime += dt;
            updateAnimations();
        }
    }

    void draw() override {
        ofBackground(20);

        // Draw 3D visualization
        if (sceneReady) {
            ofEnableDepthTest();
            camera.begin();

            // Draw coordinate axes
            drawAxes();

            // Draw bounding boxes for scene objects
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
        // Draw bounding boxes for each object in the scene
        auto ids = scene.getObjectIDs();
        for (auto id : ids) {
            const Sharp::SceneObject* obj = scene.getObject(id);
            if (!obj || !obj->visible) continue;

            // Convert position
            ofVec3f pos(obj->position.x, obj->position.y, obj->position.z);
            float scale = obj->scale.x;

            ofPushMatrix();
            ofTranslate(pos);
            ofScale(scale, scale, scale);

            // Color based on object name
            std::string name = scene.getName(id);
            if (name == "center") ofSetColor(255, 100, 100, 150);
            else if (name == "left") ofSetColor(100, 255, 100, 150);
            else if (name == "right") ofSetColor(100, 100, 255, 150);
            else if (name == "top") ofSetColor(255, 255, 100, 150);
            else ofSetColor(255, 150);

            // Draw wireframe box
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
        ofDrawBitmapString("Sharp::SharpScene - Multi-Object Management", 20, y);
        y += lineHeight * 2;

        // Status
        ofDrawBitmapString("Status: " + statusMessage, 20, y);
        y += lineHeight * 2;

        if (sceneReady) {
            // Scene stats
            std::ostringstream stats;
            stats << "Scene Statistics:";
            ofDrawBitmapString(stats.str(), 20, y);
            y += lineHeight;

            stats.str("");
            stats << "  Objects: " << scene.getObjectCount();
            ofDrawBitmapString(stats.str(), 20, y);
            y += lineHeight;

            stats.str("");
            stats << "  Total Gaussians: " << scene.getTotalGaussianCount();
            ofDrawBitmapString(stats.str(), 20, y);
            y += lineHeight;

            stats.str("");
            stats << "  Memory: " << std::fixed << std::setprecision(2)
                  << (scene.getTotalMemoryUsage() / 1024.0f / 1024.0f) << " MB";
            ofDrawBitmapString(stats.str(), 20, y);
            y += lineHeight * 2;

            // Object info
            ofDrawBitmapString("Objects:", 20, y);
            y += lineHeight;

            drawObjectInfo(centerObj, "Center", 0, 20, y, lineHeight);
            drawObjectInfo(leftObj, "Left", 1, 20, y, lineHeight);
            drawObjectInfo(rightObj, "Right", 2, 20, y, lineHeight);
            drawObjectInfo(topObj, "Top", 3, 20, y, lineHeight);
        }

        // Controls
        y += lineHeight;
        ofSetColor(180);
        ofDrawBitmapString("Controls:", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  SPACE - Create scene", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  A     - Toggle animation", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  1-4   - Select object", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  H     - Hide/show object", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  C     - Clear scene", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  S     - Save scene", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  L     - Load scene", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  UP/DN - Scale object", 20, y);

        // Selected object
        if (selectedObject >= 0 && sceneReady) {
            y += lineHeight * 2;
            ofSetColor(255, 255, 0);
            std::string objNames[] = {"Center", "Left", "Right", "Top"};
            ofDrawBitmapString("Selected: " + objNames[selectedObject], 20, y);
        }

        // FPS
        ofSetColor(255);
        std::ostringstream fps;
        fps << "FPS: " << (int)ofGetFrameRate();
        ofDrawBitmapString(fps.str(), ofGetWidth() - 100, 20);
    }

    void drawObjectInfo(Sharp::ObjectID id, const std::string& name, int index,
                        int x, int& y, int lineHeight) {
        if (id == Sharp::kInvalidObjectID) return;

        const Sharp::SceneObject* obj = scene.getObject(id);
        if (!obj) return;

        // Highlight if selected
        if (selectedObject == index) {
            ofSetColor(255, 255, 0);
        } else {
            ofSetColor(200);
        }

        std::ostringstream info;
        info << "  " << name << ": "
             << "pos(" << std::fixed << std::setprecision(1)
             << obj->position.x << "," << obj->position.y << "," << obj->position.z << ") "
             << "scale(" << obj->scale.x << ") "
             << (obj->visible ? "VIS" : "HID");

        ofDrawBitmapString(info.str(), x, y);
        y += lineHeight;
    }

    void keyPressed(int key) override {
        if (key == ' ') {
            createScene();
        }
        else if (key == 'a' || key == 'A') {
            animate = !animate;
            statusMessage = "Animation: " + std::string(animate ? "ON" : "OFF");
        }
        else if (key >= '1' && key <= '4') {
            if (sceneReady) {
                selectedObject = key - '1';
                std::string objNames[] = {"Center", "Left", "Right", "Top"};
                statusMessage = "Selected: " + objNames[selectedObject];
            }
        }
        else if (key == 'h' || key == 'H') {
            if (sceneReady && selectedObject >= 0) {
                Sharp::ObjectID ids[] = {centerObj, leftObj, rightObj, topObj};
                Sharp::ObjectID id = ids[selectedObject];
                bool visible = scene.isVisible(id);
                scene.setVisible(id, !visible);
                statusMessage = scene.getName(id) + " " + (!visible ? "shown" : "hidden");
            }
        }
        else if (key == 'c' || key == 'C') {
            clearScene();
        }
        else if (key == 's' || key == 'S') {
            saveScene();
        }
        else if (key == 'l' || key == 'L') {
            loadScene();
        }
        else if (key == OF_KEY_UP) {
            if (sceneReady && selectedObject >= 0) {
                Sharp::ObjectID ids[] = {centerObj, leftObj, rightObj, topObj};
                Sharp::ObjectID id = ids[selectedObject];
                oflike::float3 scale = scene.getScale(id);
                scale *= 1.1f;
                scene.setScale(id, scale);
                statusMessage = "Scaled up: " + scene.getName(id);
            }
        }
        else if (key == OF_KEY_DOWN) {
            if (sceneReady && selectedObject >= 0) {
                Sharp::ObjectID ids[] = {centerObj, leftObj, rightObj, topObj};
                Sharp::ObjectID id = ids[selectedObject];
                oflike::float3 scale = scene.getScale(id);
                scale *= 0.9f;
                scene.setScale(id, scale);
                statusMessage = "Scaled down: " + scene.getName(id);
            }
        }
    }

    void createScene() {
        if (sceneReady) {
            clearScene();
        }

        statusMessage = "Creating scene with 4 Gaussian clouds...";
        ofLog() << statusMessage;

        // Create 4 different procedural Gaussian clouds
        Sharp::GaussianCloud cloud1 = createProceduralCloud(100, ofColor(255, 100, 100));
        Sharp::GaussianCloud cloud2 = createProceduralCloud(80, ofColor(100, 255, 100));
        Sharp::GaussianCloud cloud3 = createProceduralCloud(80, ofColor(100, 100, 255));
        Sharp::GaussianCloud cloud4 = createProceduralCloud(60, ofColor(255, 255, 100));

        // Add clouds to scene with names
        centerObj = scene.addCloud(std::move(cloud1), "center");
        leftObj = scene.addCloud(std::move(cloud2), "left");
        rightObj = scene.addCloud(std::move(cloud3), "right");
        topObj = scene.addCloud(std::move(cloud4), "top");

        // Set initial positions
        scene.setPosition(centerObj, oflike::float3{0, 0, 0});
        scene.setPosition(leftObj, oflike::float3{-2, 0, 0});
        scene.setPosition(rightObj, oflike::float3{2, 0, 0});
        scene.setPosition(topObj, oflike::float3{0, 2, 0});

        // Set initial scales
        scene.setScale(centerObj, 1.0f);
        scene.setScale(leftObj, 0.8f);
        scene.setScale(rightObj, 0.8f);
        scene.setScale(topObj, 0.6f);

        sceneReady = true;
        animationTime = 0.0f;

        std::ostringstream msg;
        msg << "Scene created: " << scene.getObjectCount() << " objects, "
            << scene.getTotalGaussianCount() << " gaussians";
        statusMessage = msg.str();
        ofLog() << statusMessage;
    }

    Sharp::GaussianCloud createProceduralCloud(int count, const ofColor& baseColor) {
        Sharp::GaussianCloud cloud;

        std::vector<Sharp::Gaussian> gaussians;
        gaussians.reserve(count);

        // Create random Gaussians in a spherical distribution
        for (int i = 0; i < count; i++) {
            Sharp::Gaussian g;

            // Position - random in unit sphere
            float theta = ofRandom(TWO_PI);
            float phi = ofRandom(PI);
            float r = ofRandom(0.5f);

            g.position.x = r * sin(phi) * cos(theta);
            g.position.y = r * sin(phi) * sin(theta);
            g.position.z = r * cos(phi);

            // Color - variations of base color
            g.color.x = ofClamp(baseColor.r / 255.0f + ofRandom(-0.1f, 0.1f), 0.0f, 1.0f);
            g.color.y = ofClamp(baseColor.g / 255.0f + ofRandom(-0.1f, 0.1f), 0.0f, 1.0f);
            g.color.z = ofClamp(baseColor.b / 255.0f + ofRandom(-0.1f, 0.1f), 0.0f, 1.0f);

            // Opacity
            g.opacity = ofRandom(0.8f, 1.0f);

            // Scale - small random values
            g.scale = {ofRandom(0.08f, 0.12f), ofRandom(0.08f, 0.12f), ofRandom(0.08f, 0.12f)};

            // Rotation - identity quaternion
            g.rotation = simd_quaternion(0, 0, 0, 1);

            // SH coefficients - zero (flat shading)
            for (int j = 0; j < 48; j++) {
                g.sh_coefficients[j] = 0.0f;
            }

            gaussians.push_back(g);
        }

        // Create cloud from Gaussians
        if (!cloud.fromGaussians(gaussians.data(), gaussians.size())) {
            ofLog() << "ERROR: Failed to create Gaussian cloud";
        }

        return cloud;
    }

    void updateAnimations() {
        // Rotate center object
        if (centerObj != Sharp::kInvalidObjectID) {
            float angle = animationTime * 0.5f;
            scene.setRotation(centerObj, angle, oflike::float3{0, 1, 0});
        }

        // Orbit left object
        if (leftObj != Sharp::kInvalidObjectID) {
            float angle = animationTime * 0.8f;
            scene.setRotation(leftObj, angle, oflike::float3{1, 0, 0});

            float orbitAngle = animationTime * 0.3f;
            float radius = 2.0f;
            scene.setPosition(leftObj, oflike::float3{
                cos(orbitAngle) * radius,
                0,
                sin(orbitAngle) * radius
            });
        }

        // Orbit right object (opposite direction)
        if (rightObj != Sharp::kInvalidObjectID) {
            float angle = animationTime * 0.6f;
            scene.setRotation(rightObj, angle, oflike::float3{0, 0, 1});

            float orbitAngle = -animationTime * 0.3f;
            float radius = 2.0f;
            scene.setPosition(rightObj, oflike::float3{
                cos(orbitAngle) * radius,
                0,
                sin(orbitAngle) * radius
            });
        }

        // Bounce top object
        if (topObj != Sharp::kInvalidObjectID) {
            float angle = animationTime * 1.0f;
            oflike::float3 axis = {1, 1, 0};
            axis = simd_normalize(axis);
            scene.setRotation(topObj, angle, axis);

            float y = 2.0f + sin(animationTime * 2.0f) * 0.5f;
            scene.setPosition(topObj, oflike::float3{0, y, 0});
        }
    }

    void clearScene() {
        scene.clear();
        centerObj = leftObj = rightObj = topObj = Sharp::kInvalidObjectID;
        sceneReady = false;
        selectedObject = -1;
        statusMessage = "Scene cleared";
        ofLog() << statusMessage;
    }

    void saveScene() {
        if (!sceneReady) {
            statusMessage = "No scene to save";
            return;
        }

        std::string filepath = "sharp_scene_" + ofGetTimestampString() + ".sharp";
        if (scene.save(filepath)) {
            statusMessage = "Saved: " + filepath;
            ofLog() << statusMessage;
        } else {
            statusMessage = "Failed to save scene";
            ofLog() << "ERROR: " << statusMessage;
        }
    }

    void loadScene() {
        statusMessage = "Load from file not fully implemented (needs file dialog)";
        ofLog() << statusMessage;

        // Example code for loading:
        // if (scene.load("my_scene.sharp")) {
        //     auto ids = scene.getObjectIDs();
        //     if (ids.size() >= 4) {
        //         centerObj = ids[0];
        //         leftObj = ids[1];
        //         rightObj = ids[2];
        //         topObj = ids[3];
        //         sceneReady = true;
        //         statusMessage = "Scene loaded";
        //     }
        // }
    }
};

// Entry point
#define OF_APP ofApp
#include "oflike/app/ofMain.h"
