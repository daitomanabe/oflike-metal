#include "Test16.h"
#include <oflike/graphics/ofGraphics.h>
#include <oflike/math/ofMath.h>
#include <cmath>
#include <cstdio>

void Test16::setup() {
    ofSetWindowTitle("test16 - Lighting API Test (1-4: modes)");
    font.load("Helvetica", 16.0f);
    cam.setDistance(400);

    // Setup point light
    pointLight.setPointLight();
    pointLight.setPosition(100, 100, 100);
    pointLight.setDiffuseColor(255, 200, 200);
    pointLight.setSpecularColor(255, 255, 255);
    pointLight.setAttenuation(1.0f, 0.01f, 0.0f);

    // Setup directional light
    directionalLight.setDirectional();
    directionalLight.setDirection(0, -1, -1);
    directionalLight.setDiffuseColor(200, 200, 255);

    // Setup spot light
    spotLight.setSpotlight();
    spotLight.setPosition(0, 150, 100);
    spotLight.setDirection(0, -1, 0);
    spotLight.setSpotlightCutOff(30);
    spotLight.setSpotConcentration(20);
    spotLight.setDiffuseColor(200, 255, 200);

    // Setup material
    material.setAmbientColor(50, 50, 50);
    material.setDiffuseColor(200, 200, 200);
    material.setSpecularColor(255, 255, 255);
    material.setShininess(64);

    printf("=== Lighting Module Tests ===\n");
    printf("Point light position: (%.0f, %.0f, %.0f)\n",
           pointLight.getPosition().x, pointLight.getPosition().y, pointLight.getPosition().z);
    printf("Directional light direction: (%.1f, %.1f, %.1f)\n",
           directionalLight.getDirection().x, directionalLight.getDirection().y, directionalLight.getDirection().z);
    printf("Spot light cutoff: %.1f degrees\n", spotLight.getSpotlightCutOff());
    printf("Material shininess: %.1f\n", material.getShininess());
    printf("=============================\n");
}

void Test16::update() {
    cam.update();

    float time = ofGetElapsedTimef();

    // Animate point light position
    pointLight.setPosition(
        std::cos(time) * 150,
        100,
        std::sin(time) * 150
    );

    // Animate spot light direction
    float spotAngle = time * 0.5f;
    spotLight.setDirection(
        std::sin(spotAngle) * 0.5f,
        -1,
        std::cos(spotAngle) * 0.5f
    );
}

void Test16::draw() {
    ofBackground(20, 20, 30);

    // Draw title
    ofSetColor(255);
    if (font.isLoaded()) {
        font.drawString("Lighting & Material Test", 20, 30);
    }

    std::string modeStr;
    switch (lightMode) {
        case 0: modeStr = "Mode 1: Point Light (orbiting)"; break;
        case 1: modeStr = "Mode 2: Directional Light"; break;
        case 2: modeStr = "Mode 3: Spot Light"; break;
        case 3: modeStr = "Mode 4: All Lights"; break;
    }

    ofSetColor(200, 200, 100);
    if (font.isLoaded()) {
        font.drawString(modeStr, 20, 55);
    }

    // Begin 3D scene
    cam.begin();
    ofEnableDepthTest();

    // Draw floor grid
    ofSetColor(60, 60, 80);
    for (int i = -5; i <= 5; i++) {
        ofDrawLine(i * 30, -50, -150, i * 30, -50, 150);
        ofDrawLine(-150, -50, i * 30, 150, -50, i * 30);
    }

    // Enable appropriate lights based on mode
    // Note: Actual lighting in shaders would require integration
    // For now, we visualize the light properties

    // Draw light indicators
    oflike::ofVec3f pointPos = pointLight.getPosition();
    oflike::ofVec3f dirLightDir = directionalLight.getDirection();
    oflike::ofVec3f spotPos = spotLight.getPosition();
    oflike::ofVec3f spotDir = spotLight.getDirection();

    if (lightMode == 0 || lightMode == 3) {
        // Point light indicator
        ofSetColor(255, 200, 200);
        ofDrawSphere(pointPos.x, pointPos.y, pointPos.z, 8);

        // Draw rays from point light
        ofSetColor(255, 150, 150, 100);
        for (int i = 0; i < 8; i++) {
            float angle = i * TWO_PI / 8;
            ofDrawLine(pointPos.x, pointPos.y, pointPos.z,
                       pointPos.x + std::cos(angle) * 30,
                       pointPos.y,
                       pointPos.z + std::sin(angle) * 30);
        }
    }

    if (lightMode == 1 || lightMode == 3) {
        // Directional light indicator (arrows from above)
        ofSetColor(200, 200, 255);
        for (int x = -2; x <= 2; x++) {
            for (int z = -2; z <= 2; z++) {
                float startX = x * 40;
                float startZ = z * 40;
                float startY = 150;
                ofDrawLine(startX, startY, startZ,
                           startX + dirLightDir.x * 50,
                           startY + dirLightDir.y * 50,
                           startZ + dirLightDir.z * 50);
            }
        }
    }

    if (lightMode == 2 || lightMode == 3) {
        // Spot light indicator
        ofSetColor(200, 255, 200);
        ofDrawSphere(spotPos.x, spotPos.y, spotPos.z, 8);

        // Draw cone
        float coneLength = 100;
        float coneRadius = std::tan(spotLight.getSpotlightCutOff() * DEG_TO_RAD) * coneLength;

        oflike::ofVec3f endPos(
            spotPos.x + spotDir.x * coneLength,
            spotPos.y + spotDir.y * coneLength,
            spotPos.z + spotDir.z * coneLength
        );

        ofSetColor(150, 255, 150, 80);
        ofDrawLine(spotPos.x, spotPos.y, spotPos.z, endPos.x, endPos.y, endPos.z);

        // Draw cone outline
        for (int i = 0; i < 8; i++) {
            float angle = i * TWO_PI / 8;
            oflike::ofVec3f perpDir = spotDir.getPerpendicular();
            oflike::ofVec3f right = spotDir.cross(perpDir).getNormalized();

            float edgeX = endPos.x + (std::cos(angle) * perpDir.x + std::sin(angle) * right.x) * coneRadius;
            float edgeY = endPos.y + (std::cos(angle) * perpDir.y + std::sin(angle) * right.y) * coneRadius;
            float edgeZ = endPos.z + (std::cos(angle) * perpDir.z + std::sin(angle) * right.z) * coneRadius;

            ofDrawLine(spotPos.x, spotPos.y, spotPos.z, edgeX, edgeY, edgeZ);
        }
    }

    // Draw objects that would be lit
    // Since we don't have actual shader-based lighting integrated,
    // we simulate the appearance

    // Central sphere
    ofFill();
    if (lightMode == 0 || lightMode == 3) {
        // Simulate point light effect
        oflike::ofVec3f toLight = pointPos - oflike::ofVec3f(0, 0, 0);
        float dist = toLight.length();
        float attenuation = 1.0f / (1.0f + 0.01f * dist);
        int brightness = static_cast<int>(200 * attenuation);
        ofSetColor(brightness, brightness * 0.9f, brightness * 0.9f);
    } else if (lightMode == 1) {
        ofSetColor(150, 150, 180);
    } else if (lightMode == 2) {
        ofSetColor(150, 180, 150);
    } else {
        ofSetColor(180, 180, 180);
    }
    ofDrawSphere(0, 0, 0, 40);

    // Draw smaller spheres around
    for (int i = 0; i < 6; i++) {
        float angle = i * TWO_PI / 6;
        float x = std::cos(angle) * 80;
        float z = std::sin(angle) * 80;

        if (lightMode == 0 || lightMode == 3) {
            oflike::ofVec3f objPos(x, 0, z);
            oflike::ofVec3f toLight = pointPos - objPos;
            float dist = toLight.length();
            float attenuation = 1.0f / (1.0f + 0.01f * dist);
            int brightness = static_cast<int>(180 * attenuation);
            ofSetColor(brightness, brightness * 0.9f, brightness * 0.9f);
        } else {
            ofSetColor(120, 120, 140);
        }
        ofDrawSphere(x, 0, z, 20);
    }

    ofDisableDepthTest();
    cam.end();

    // Draw light info
    ofSetColor(180);
    if (font.isLoaded()) {
        char buf[256];

        int y = 100;
        font.drawString("Light Properties:", 20, y); y += 25;

        if (lightMode == 0 || lightMode == 3) {
            snprintf(buf, sizeof(buf), "Point: pos=(%.0f, %.0f, %.0f)",
                     pointPos.x, pointPos.y, pointPos.z);
            font.drawString(buf, 30, y); y += 20;
            auto diffuse = pointLight.getDiffuseColor();
            snprintf(buf, sizeof(buf), "  diffuse=(%.0f, %.0f, %.0f)",
                     diffuse.r * 255, diffuse.g * 255, diffuse.b * 255);
            font.drawString(buf, 30, y); y += 25;
        }

        if (lightMode == 1 || lightMode == 3) {
            snprintf(buf, sizeof(buf), "Directional: dir=(%.1f, %.1f, %.1f)",
                     dirLightDir.x, dirLightDir.y, dirLightDir.z);
            font.drawString(buf, 30, y); y += 25;
        }

        if (lightMode == 2 || lightMode == 3) {
            snprintf(buf, sizeof(buf), "Spot: cutoff=%.0f, concentration=%.0f",
                     spotLight.getSpotlightCutOff(), spotLight.getSpotConcentration());
            font.drawString(buf, 30, y); y += 25;
        }

        // Material info
        y += 10;
        font.drawString("Material Properties:", 20, y); y += 25;
        snprintf(buf, sizeof(buf), "Shininess: %.0f", material.getShininess());
        font.drawString(buf, 30, y); y += 20;

        auto ambient = material.getAmbientColor();
        snprintf(buf, sizeof(buf), "Ambient: (%.0f, %.0f, %.0f)",
                 ambient.r * 255, ambient.g * 255, ambient.b * 255);
        font.drawString(buf, 30, y); y += 20;

        auto diffuse = material.getDiffuseColor();
        snprintf(buf, sizeof(buf), "Diffuse: (%.0f, %.0f, %.0f)",
                 diffuse.r * 255, diffuse.g * 255, diffuse.b * 255);
        font.drawString(buf, 30, y);
    }

    // Key hints
    ofSetColor(150);
    if (font.isLoaded()) {
        font.drawString("Keys: 1-4 = switch light mode | Note: Visual simulation only (no shader lighting)", 20, ofGetHeight() - 10);
    }
}

void Test16::keyPressed(int key) {
    printf("Key pressed: %d\n", key);
    if (key == 18) lightMode = 0;      // 1
    else if (key == 19) lightMode = 1; // 2
    else if (key == 20) lightMode = 2; // 3
    else if (key == 21) lightMode = 3; // 4
}

void Test16::keyReleased(int key) { (void)key; }
void Test16::mouseMoved(int x, int y) { (void)x; (void)y; }
void Test16::mouseDragged(int x, int y, int button) { cam.onMouseDragged(x, y, button); }
void Test16::mousePressed(int x, int y, int button) { cam.onMousePressed(x, y, button); }
void Test16::mouseReleased(int x, int y, int button) { cam.onMouseReleased(x, y, button); }
void Test16::mouseScrolled(int x, int y, float scrollX, float scrollY) {
    (void)x; (void)y; (void)scrollX;
    cam.onMouseScrolled(0, scrollY);
}
void Test16::windowResized(int w, int h) { (void)w; (void)h; }

extern "C" ofBaseApp* ofCreateApp() {
    return new Test16();
}
