//
// 08_lighting - oflike-metal Example
//
// Demonstrates comprehensive lighting system:
// - ofLight: point, directional, spotlight
// - ofMaterial: ambient, diffuse, specular, shininess
// - Multiple lights (up to 8)
// - Phong shading
// - Interactive controls
//
// Controls:
//   1-3:     Toggle lights (point, directional, spot)
//   Q/W:     Adjust light 1 intensity
//   A/S:     Adjust light 2 intensity
//   Z/X:     Adjust light 3 intensity
//   4-6:     Switch material preset
//   UP/DOWN: Adjust shininess
//   SPACE:   Toggle animation
//   L:       Toggle lighting system
//   R:       Reset to defaults
//

#include "ofMain.h"

using namespace oflike;

class LightingApp : public ofBaseApp {
public:
    void setup() override {
        ofSetFrameRate(60);
        ofSetWindowTitle("08_lighting - ofLight, ofMaterial, Multiple Lights, Phong");
        ofBackground(20, 20, 25);
        ofEnableDepthTest();

        // Setup camera
        camera.setPosition(0, 3, 8);
        camera.lookAt(ofVec3f(0, 0, 0));
        camera.setNearClip(0.1f);
        camera.setFarClip(100.0f);

        // Setup lights
        setupLights();

        // Setup materials
        setupMaterials();

        // Scene parameters
        animationTime = 0.0f;
        animationEnabled = true;
        lightingEnabled = true;

        // Print controls
        ofLogNotice("08_lighting") << "=== Controls ===";
        ofLogNotice("08_lighting") << "1-3: Toggle lights (point, directional, spot)";
        ofLogNotice("08_lighting") << "Q/W: Light 1 intensity";
        ofLogNotice("08_lighting") << "A/S: Light 2 intensity";
        ofLogNotice("08_lighting") << "Z/X: Light 3 intensity";
        ofLogNotice("08_lighting") << "4-6: Material presets";
        ofLogNotice("08_lighting") << "UP/DOWN: Shininess";
        ofLogNotice("08_lighting") << "SPACE: Toggle animation";
        ofLogNotice("08_lighting") << "L: Toggle lighting";
        ofLogNotice("08_lighting") << "R: Reset";
    }

    void setupLights() {
        // Light 1: Point light (Red-Orange) - orbiting
        light1.setPointLight();
        light1.setDiffuseColor(ofColor(255, 100, 50));  // Warm orange
        light1.setSpecularColor(ofColor(255, 200, 150));
        light1.setAmbientColor(ofColor(50, 20, 10));
        light1.setAttenuation(1.0f, 0.1f, 0.01f);  // constant, linear, quadratic
        light1.enable();
        light1Enabled = true;
        light1Intensity = 1.0f;

        // Light 2: Directional light (Blue) - top-down
        light2.setDirectional();
        light2.setDiffuseColor(ofColor(100, 150, 255));  // Cool blue
        light2.setSpecularColor(ofColor(200, 220, 255));
        light2.setAmbientColor(ofColor(10, 15, 30));
        light2.setPosition(0, 10, 0);
        light2.enable();
        light2Enabled = true;
        light2Intensity = 0.6f;

        // Light 3: Spotlight (Green) - from front
        light3.setSpotlight();
        light3.setDiffuseColor(ofColor(100, 255, 100));  // Green
        light3.setSpecularColor(ofColor(200, 255, 200));
        light3.setAmbientColor(ofColor(10, 30, 10));
        light3.setSpotlightCutOff(25.0f);  // 25 degree cone
        light3.setSpotConcentration(20.0f);  // Focus
        light3.setAttenuation(1.0f, 0.05f, 0.005f);
        light3.enable();
        light3Enabled = true;
        light3Intensity = 0.8f;
    }

    void setupMaterials() {
        // Material 1: Shiny metal (high specular, high shininess)
        materialMetal.setAmbientColor(ofColor(40, 40, 40));
        materialMetal.setDiffuseColor(ofColor(180, 180, 180));
        materialMetal.setSpecularColor(ofColor(255, 255, 255));
        materialMetal.setShininess(128.0f);

        // Material 2: Matte plastic (low specular, low shininess)
        materialPlastic.setAmbientColor(ofColor(50, 20, 20));
        materialPlastic.setDiffuseColor(ofColor(200, 80, 80));
        materialPlastic.setSpecularColor(ofColor(100, 100, 100));
        materialPlastic.setShininess(32.0f);

        // Material 3: Glossy paint (medium specular, medium shininess)
        materialPaint.setAmbientColor(ofColor(20, 40, 60));
        materialPaint.setDiffuseColor(ofColor(80, 160, 240));
        materialPaint.setSpecularColor(ofColor(200, 220, 255));
        materialPaint.setShininess(64.0f);

        // Set default
        currentMaterial = 0;  // Metal
        currentShininess = 128.0f;
    }

    void update() override {
        if (animationEnabled) {
            animationTime += ofGetLastFrameTime();
        }

        // Update light 1 position (orbiting point light)
        float orbitRadius = 5.0f;
        float orbitSpeed = 0.5f;
        float angle1 = animationTime * orbitSpeed;
        ofVec3f light1Pos(
            cos(angle1) * orbitRadius,
            2.0f + sin(animationTime * 0.3f) * 1.0f,  // Vertical bobbing
            sin(angle1) * orbitRadius
        );
        light1.setPosition(light1Pos);

        // Update light 2 direction (rotating directional)
        float angle2 = animationTime * 0.3f;
        ofVec3f light2Dir(
            sin(angle2) * 0.5f,
            -1.0f,  // Always pointing down
            cos(angle2) * 0.5f
        );
        light2.setPosition(light2Dir * 10.0f);

        // Update light 3 (spotlight from rotating position)
        float angle3 = animationTime * 0.7f + 3.14159f;  // Opposite to light1
        ofVec3f light3Pos(
            cos(angle3) * 4.0f,
            3.0f,
            sin(angle3) * 4.0f
        );
        light3.setPosition(light3Pos);
        // Spotlight always points to center
        ofVec3f spotDir = ofVec3f(0, 0, 0) - light3Pos;
        spotDir.normalize();
        light3.setPosition(light3Pos);  // Position set, direction handled by lookAt in ofLight

        // Update light intensities
        updateLightColors();
    }

    void updateLightColors() {
        // Apply intensity multipliers to light colors
        ofColor baseColor1(255, 100, 50);
        light1.setDiffuseColor(baseColor1 * light1Intensity);
        light1.setSpecularColor(ofColor(255, 200, 150) * light1Intensity);

        ofColor baseColor2(100, 150, 255);
        light2.setDiffuseColor(baseColor2 * light2Intensity);
        light2.setSpecularColor(ofColor(200, 220, 255) * light2Intensity);

        ofColor baseColor3(100, 255, 100);
        light3.setDiffuseColor(baseColor3 * light3Intensity);
        light3.setSpecularColor(ofColor(200, 255, 200) * light3Intensity);
    }

    void draw() override {
        // Enable/disable lighting system
        if (lightingEnabled) {
            ofEnableLighting();
        } else {
            ofDisableLighting();
        }

        // Begin camera
        camera.begin();

        // Draw scene with materials
        drawScene();

        // Draw light visualization (unlit)
        ofDisableLighting();
        drawLightIndicators();
        if (lightingEnabled) {
            ofEnableLighting();
        }

        camera.end();

        // Draw UI
        ofDisableLighting();
        drawUI();
    }

    void drawScene() {
        // Draw ground plane with material
        ofPushMatrix();
        ofTranslate(0, -1, 0);
        getCurrentMaterial().begin();
        ofDrawPlane(0, 0, 12, 12);
        getCurrentMaterial().end();
        ofPopMatrix();

        // Draw center sphere with material
        ofPushMatrix();
        ofTranslate(0, 0, 0);
        getCurrentMaterial().begin();
        ofDrawSphere(0, 0, 0, 1.0f);
        getCurrentMaterial().end();
        ofPopMatrix();

        // Draw 4 corner boxes with different materials
        float spacing = 3.0f;
        for (int i = 0; i < 4; i++) {
            float angle = i * 3.14159f * 0.5f;
            ofVec3f pos(cos(angle) * spacing, 0, sin(angle) * spacing);

            ofPushMatrix();
            ofTranslate(pos.x, pos.y, pos.z);
            ofRotateY(animationTime * 20.0f + i * 45.0f);

            // Cycle through materials
            if (i == 0) materialMetal.begin();
            else if (i == 1) materialPlastic.begin();
            else if (i == 2) materialPaint.begin();
            else getCurrentMaterial().begin();

            ofDrawBox(0, 0, 0, 1.2f);

            if (i == 0) materialMetal.end();
            else if (i == 1) materialPlastic.end();
            else if (i == 2) materialPaint.end();
            else getCurrentMaterial().end();

            ofPopMatrix();
        }

        // Draw cylinders showing light directions
        for (int i = 0; i < 8; i++) {
            float angle = i * 3.14159f * 0.25f;
            float radius = 5.5f;
            ofVec3f pos(cos(angle) * radius, 0, sin(angle) * radius);

            ofPushMatrix();
            ofTranslate(pos.x, pos.y, pos.z);
            getCurrentMaterial().begin();
            ofDrawCylinder(0, 0, 0, 0.3f, 2.0f);
            getCurrentMaterial().end();
            ofPopMatrix();
        }
    }

    void drawLightIndicators() {
        // Draw small spheres at light positions
        if (light1Enabled) {
            ofSetColor(255, 100, 50);
            ofVec3f pos1 = light1.getPosition();
            ofDrawSphere(pos1.x, pos1.y, pos1.z, 0.2f);
        }

        if (light2Enabled) {
            ofSetColor(100, 150, 255);
            ofVec3f pos2 = light2.getPosition();
            ofDrawSphere(pos2.x, pos2.y, pos2.z, 0.15f);
        }

        if (light3Enabled) {
            ofSetColor(100, 255, 100);
            ofVec3f pos3 = light3.getPosition();
            ofDrawSphere(pos3.x, pos3.y, pos3.z, 0.2f);

            // Draw spotlight cone visualization
            ofSetColor(100, 255, 100, 50);
            ofPushMatrix();
            ofTranslate(pos3.x, pos3.y, pos3.z);
            // Draw line to center
            ofSetLineWidth(2);
            ofSetColor(100, 255, 100);
            ofDrawLine(0, 0, 0, -pos3.x, -pos3.y, -pos3.z);
            ofPopMatrix();
        }

        // Draw coordinate axes at origin
        ofSetLineWidth(2);
        ofSetColor(255, 0, 0);
        ofDrawLine(0, 0, 0, 1, 0, 0);
        ofSetColor(0, 255, 0);
        ofDrawLine(0, 0, 0, 0, 1, 0);
        ofSetColor(0, 0, 255);
        ofDrawLine(0, 0, 0, 0, 0, 1);
    }

    void drawUI() {
        ofSetColor(255);

        int y = 30;
        int lineHeight = 20;

        // Title
        ofDrawBitmapString("08_lighting - Multiple Lights + Materials", 20, y);
        y += lineHeight * 2;

        // Light status
        ofDrawBitmapString("Lights:", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  1. Point Light (Red-Orange): " +
            std::string(light1Enabled ? "ON" : "OFF") +
            " | Intensity: " + ofToString(light1Intensity, 2), 20, y);
        y += lineHeight;
        ofDrawBitmapString("  2. Directional (Blue): " +
            std::string(light2Enabled ? "ON" : "OFF") +
            " | Intensity: " + ofToString(light2Intensity, 2), 20, y);
        y += lineHeight;
        ofDrawBitmapString("  3. Spotlight (Green): " +
            std::string(light3Enabled ? "ON" : "OFF") +
            " | Intensity: " + ofToString(light3Intensity, 2), 20, y);
        y += lineHeight * 2;

        // Material status
        std::string matName = currentMaterial == 0 ? "Metal (Shiny)" :
                             currentMaterial == 1 ? "Plastic (Matte)" :
                             "Paint (Glossy)";
        ofDrawBitmapString("Material: " + matName, 20, y);
        y += lineHeight;
        ofDrawBitmapString("Shininess: " + ofToString(currentShininess, 1), 20, y);
        y += lineHeight * 2;

        // Status
        ofDrawBitmapString("Lighting System: " + std::string(lightingEnabled ? "ON" : "OFF"), 20, y);
        y += lineHeight;
        ofDrawBitmapString("Animation: " + std::string(animationEnabled ? "ON" : "OFF"), 20, y);
        y += lineHeight;
        ofDrawBitmapString("FPS: " + ofToString(ofGetFrameRate(), 1), 20, y);

        // Controls reminder
        y = ofGetHeight() - 140;
        ofDrawBitmapString("Controls:", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  1-3: Toggle lights | Q/W A/S Z/X: Intensities", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  4-6: Material presets | UP/DOWN: Shininess", 20, y);
        y += lineHeight;
        ofDrawBitmapString("  SPACE: Animation | L: Lighting | R: Reset", 20, y);
    }

    ofMaterial& getCurrentMaterial() {
        if (currentMaterial == 0) return materialMetal;
        if (currentMaterial == 1) return materialPlastic;
        return materialPaint;
    }

    void keyPressed(int key) override {
        // Light toggles
        if (key == '1') {
            light1Enabled = !light1Enabled;
            if (light1Enabled) light1.enable();
            else light1.disable();
            ofLogNotice("08_lighting") << "Light 1 (Point): " << (light1Enabled ? "ON" : "OFF");
        }
        if (key == '2') {
            light2Enabled = !light2Enabled;
            if (light2Enabled) light2.enable();
            else light2.disable();
            ofLogNotice("08_lighting") << "Light 2 (Directional): " << (light2Enabled ? "ON" : "OFF");
        }
        if (key == '3') {
            light3Enabled = !light3Enabled;
            if (light3Enabled) light3.enable();
            else light3.disable();
            ofLogNotice("08_lighting") << "Light 3 (Spotlight): " << (light3Enabled ? "ON" : "OFF");
        }

        // Light intensities
        if (key == 'q' || key == 'Q') {
            light1Intensity = ofClamp(light1Intensity - 0.1f, 0.0f, 2.0f);
            ofLogNotice("08_lighting") << "Light 1 intensity: " << light1Intensity;
        }
        if (key == 'w' || key == 'W') {
            light1Intensity = ofClamp(light1Intensity + 0.1f, 0.0f, 2.0f);
            ofLogNotice("08_lighting") << "Light 1 intensity: " << light1Intensity;
        }
        if (key == 'a' || key == 'A') {
            light2Intensity = ofClamp(light2Intensity - 0.1f, 0.0f, 2.0f);
            ofLogNotice("08_lighting") << "Light 2 intensity: " << light2Intensity;
        }
        if (key == 's' || key == 'S') {
            light2Intensity = ofClamp(light2Intensity + 0.1f, 0.0f, 2.0f);
            ofLogNotice("08_lighting") << "Light 2 intensity: " << light2Intensity;
        }
        if (key == 'z' || key == 'Z') {
            light3Intensity = ofClamp(light3Intensity - 0.1f, 0.0f, 2.0f);
            ofLogNotice("08_lighting") << "Light 3 intensity: " << light3Intensity;
        }
        if (key == 'x' || key == 'X') {
            light3Intensity = ofClamp(light3Intensity + 0.1f, 0.0f, 2.0f);
            ofLogNotice("08_lighting") << "Light 3 intensity: " << light3Intensity;
        }

        // Material presets
        if (key == '4') {
            currentMaterial = 0;
            currentShininess = 128.0f;
            ofLogNotice("08_lighting") << "Material: Metal (Shiny)";
        }
        if (key == '5') {
            currentMaterial = 1;
            currentShininess = 32.0f;
            ofLogNotice("08_lighting") << "Material: Plastic (Matte)";
        }
        if (key == '6') {
            currentMaterial = 2;
            currentShininess = 64.0f;
            ofLogNotice("08_lighting") << "Material: Paint (Glossy)";
        }

        // Shininess adjustment
        if (key == OF_KEY_UP) {
            currentShininess = ofClamp(currentShininess + 8.0f, 1.0f, 256.0f);
            getCurrentMaterial().setShininess(currentShininess);
            ofLogNotice("08_lighting") << "Shininess: " << currentShininess;
        }
        if (key == OF_KEY_DOWN) {
            currentShininess = ofClamp(currentShininess - 8.0f, 1.0f, 256.0f);
            getCurrentMaterial().setShininess(currentShininess);
            ofLogNotice("08_lighting") << "Shininess: " << currentShininess;
        }

        // Toggle animation
        if (key == ' ') {
            animationEnabled = !animationEnabled;
            ofLogNotice("08_lighting") << "Animation: " << (animationEnabled ? "ON" : "OFF");
        }

        // Toggle lighting system
        if (key == 'l' || key == 'L') {
            lightingEnabled = !lightingEnabled;
            ofLogNotice("08_lighting") << "Lighting system: " << (lightingEnabled ? "ON" : "OFF");
        }

        // Reset
        if (key == 'r' || key == 'R') {
            setupLights();
            setupMaterials();
            animationTime = 0.0f;
            animationEnabled = true;
            lightingEnabled = true;
            ofLogNotice("08_lighting") << "Reset to defaults";
        }
    }

    void windowResized(int w, int h) override {
        camera.setAspectRatio(static_cast<float>(w) / static_cast<float>(h));
    }

private:
    // Camera
    ofCamera camera;

    // Lights
    ofLight light1;  // Point light (orbiting)
    ofLight light2;  // Directional light (top-down)
    ofLight light3;  // Spotlight (front)

    bool light1Enabled;
    bool light2Enabled;
    bool light3Enabled;

    float light1Intensity;
    float light2Intensity;
    float light3Intensity;

    // Materials
    ofMaterial materialMetal;
    ofMaterial materialPlastic;
    ofMaterial materialPaint;

    int currentMaterial;  // 0=metal, 1=plastic, 2=paint
    float currentShininess;

    // Animation
    float animationTime;
    bool animationEnabled;
    bool lightingEnabled;
};

int main() {
    ofSetupOpenGL(1280, 720, OF_WINDOW);
    ofRunApp(new LightingApp());
}
