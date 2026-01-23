//
// 09_mesh - oflike-metal Example
//
// Demonstrates comprehensive mesh API:
// - ofMesh: vertices, normals, texcoords, colors, indices
// - Primitive modes: triangles, lines, points, triangle strip, triangle fan
// - Procedural mesh generation: sphere, box, plane, cone, cylinder, icosphere
// - Mesh manipulation: adding vertices, updating data, normals
// - Indexed vs non-indexed meshes
// - Lighting integration with materials
//
// Controls:
//   1-7:     Switch mesh modes (see console for mode names)
//   SPACE:   Toggle animation
//   W:       Toggle wireframe
//   N:       Toggle normals
//   L:       Toggle lighting
//   R:       Reset camera and animation
//   UP/DOWN: Adjust mesh resolution (where applicable)
//

#include "ofMain.h"
#include <cmath>

using namespace oflike;

class MeshApp : public ofBaseApp {
public:
    void setup() override {
        ofSetFrameRate(60);
        ofSetWindowTitle("09_mesh - ofMesh API, Primitives, Procedural Generation");
        ofBackground(30, 30, 35);
        ofEnableDepthTest();

        // Setup camera
        camera.setPosition(0, 4, 10);
        camera.lookAt(ofVec3f(0, 0, 0));
        camera.setNearClip(0.1f);
        camera.setFarClip(100.0f);

        // Setup lighting
        light.setPointLight();
        light.setPosition(5, 5, 5);
        light.setDiffuseColor(ofColor(255, 255, 255));
        light.setSpecularColor(ofColor(255, 255, 255));
        light.setAmbientColor(ofColor(50, 50, 50));
        light.setAttenuation(1.0f, 0.001f, 0.0001f);

        // Setup materials
        material.setDiffuseColor(ofColor(100, 150, 255));
        material.setSpecularColor(ofColor(255, 255, 255));
        material.setAmbientColor(ofColor(50, 75, 128));
        material.setShininess(64);

        // State
        currentMeshMode = 0;
        animationTime = 0.0f;
        animationEnabled = true;
        wireframeMode = false;
        showNormals = false;
        lightingEnabled = true;
        meshResolution = 20;

        // Build all meshes
        buildMeshes();

        // Console info
        ofLogNotice("09_mesh") << "Mesh demonstration started";
        ofLogNotice("Controls") << "1-7: Switch mesh modes, SPACE: Toggle animation";
        ofLogNotice("Controls") << "W: Wireframe, N: Show normals, L: Lighting, UP/DOWN: Resolution";
        printCurrentMode();
    }

    void update() override {
        if (animationEnabled) {
            animationTime += ofGetLastFrameTime();
        }

        // Update camera rotation for automatic rotation
        float orbitAngle = animationTime * 0.3f;
        camera.setPosition(
            10.0f * std::cos(orbitAngle),
            4.0f + 2.0f * std::sin(animationTime * 0.5f),
            10.0f * std::sin(orbitAngle)
        );
        camera.lookAt(ofVec3f(0, 0, 0));

        // Update light position
        float lightAngle = animationTime * 0.8f;
        light.setPosition(
            8.0f * std::cos(lightAngle),
            6.0f,
            8.0f * std::sin(lightAngle)
        );
    }

    void draw() override {
        // Enable lighting if requested
        if (lightingEnabled) {
            ofEnableLighting();
            light.enable();
            material.begin();
        }

        // Draw current mesh
        camera.begin();

        drawCurrentMesh();

        camera.end();

        if (lightingEnabled) {
            material.end();
            light.disable();
            ofDisableLighting();
        }

        // Draw UI overlay
        drawUI();
    }

    void buildMeshes() {
        // Mode 0: Custom triangle mesh with per-vertex colors
        buildCustomTriangleMesh();

        // Mode 1: Procedural sphere (UV sphere)
        meshes[1] = ofMesh::sphere(2.0f, meshResolution);

        // Mode 2: Procedural box
        meshes[2] = ofMesh::box(3.0f, 3.0f, 3.0f);

        // Mode 3: Procedural plane with subdivisions
        meshes[3] = ofMesh::plane(4.0f, 4.0f, meshResolution / 4, meshResolution / 4);

        // Mode 4: Mesh with different primitive modes (line strip)
        buildLineStripMesh();

        // Mode 5: Mesh with points primitive mode
        buildPointsMesh();

        // Mode 6: Procedural icosphere (geodesic sphere)
        meshes[6] = ofMesh::icosphere(2.0f, 2);

        ofLogNotice("buildMeshes") << "Built 7 mesh modes";
    }

    void buildCustomTriangleMesh() {
        meshes[0].clear();
        meshes[0].setMode(OF_PRIMITIVE_TRIANGLES);

        // Create a pyramid with 5 vertices and per-vertex colors
        // Apex
        meshes[0].addVertex(ofVec3f(0, 2, 0));
        meshes[0].addColor(ofColor(255, 0, 0));  // Red
        meshes[0].addNormal(ofVec3f(0, 1, 0));

        // Base vertices (square)
        meshes[0].addVertex(ofVec3f(-1.5f, -1, -1.5f));
        meshes[0].addColor(ofColor(0, 255, 0));  // Green
        meshes[0].addNormal(ofVec3f(0, -1, 0));

        meshes[0].addVertex(ofVec3f(1.5f, -1, -1.5f));
        meshes[0].addColor(ofColor(0, 0, 255));  // Blue
        meshes[0].addNormal(ofVec3f(0, -1, 0));

        meshes[0].addVertex(ofVec3f(1.5f, -1, 1.5f));
        meshes[0].addColor(ofColor(255, 255, 0));  // Yellow
        meshes[0].addNormal(ofVec3f(0, -1, 0));

        meshes[0].addVertex(ofVec3f(-1.5f, -1, 1.5f));
        meshes[0].addColor(ofColor(255, 0, 255));  // Magenta
        meshes[0].addNormal(ofVec3f(0, -1, 0));

        // Indexed triangles for 4 side faces
        // Front face
        meshes[0].addTriangle(0, 1, 2);
        // Right face
        meshes[0].addTriangle(0, 2, 3);
        // Back face
        meshes[0].addTriangle(0, 3, 4);
        // Left face
        meshes[0].addTriangle(0, 4, 1);

        // Base triangles
        meshes[0].addTriangle(1, 3, 2);
        meshes[0].addTriangle(1, 4, 3);

        ofLogNotice("buildCustomTriangleMesh") << "Pyramid: " << meshes[0].getNumVertices()
                                                << " vertices, " << meshes[0].getNumIndices()
                                                << " indices";
    }

    void buildLineStripMesh() {
        meshes[4].clear();
        meshes[4].setMode(OF_PRIMITIVE_LINE_STRIP);

        // Create a spiral line strip
        int segments = 100;
        float radius = 2.0f;
        float height = 4.0f;

        for (int i = 0; i < segments; i++) {
            float t = (float)i / (float)(segments - 1);
            float angle = t * 6.28318f * 4.0f;  // 4 revolutions
            float x = radius * std::cos(angle) * (1.0f - t * 0.5f);
            float y = -height * 0.5f + height * t;
            float z = radius * std::sin(angle) * (1.0f - t * 0.5f);

            meshes[4].addVertex(ofVec3f(x, y, z));

            // Color gradient from cyan to magenta
            ofColor color = ofColor::fromHsb((int)(t * 180.0f + 120.0f) % 256, 255, 255);
            meshes[4].addColor(color);
        }

        ofLogNotice("buildLineStripMesh") << "Spiral: " << meshes[4].getNumVertices() << " vertices";
    }

    void buildPointsMesh() {
        meshes[5].clear();
        meshes[5].setMode(OF_PRIMITIVE_POINTS);

        // Create a point cloud in a sphere distribution
        int numPoints = 1000;
        for (int i = 0; i < numPoints; i++) {
            // Random spherical coordinates
            float theta = ((float)rand() / RAND_MAX) * 6.28318f;
            float phi = ((float)rand() / RAND_MAX) * 3.14159f;
            float r = 2.0f * std::pow((float)rand() / RAND_MAX, 0.33f);  // Cubic distribution

            float x = r * std::sin(phi) * std::cos(theta);
            float y = r * std::sin(phi) * std::sin(theta);
            float z = r * std::cos(phi);

            meshes[5].addVertex(ofVec3f(x, y, z));

            // Color based on position
            ofColor color = ofColor::fromHsb(
                (int)((phi / 3.14159f) * 255.0f),
                255,
                (int)((r / 2.0f) * 255.0f)
            );
            meshes[5].addColor(color);
        }

        ofLogNotice("buildPointsMesh") << "Point cloud: " << meshes[5].getNumVertices() << " points";
    }

    void drawCurrentMesh() {
        if (currentMeshMode < 0 || currentMeshMode >= 7) return;

        ofPushMatrix();

        // Rotate mesh for better viewing
        ofRotateY(animationTime * 30.0f);
        ofRotateX(std::sin(animationTime * 0.5f) * 20.0f);

        // Draw mesh
        if (wireframeMode) {
            meshes[currentMeshMode].drawWireframe();
        } else {
            meshes[currentMeshMode].draw();
        }

        // Draw normals if requested
        if (showNormals) {
            drawMeshNormals(meshes[currentMeshMode]);
        }

        ofPopMatrix();
    }

    void drawMeshNormals(const ofMesh& mesh) {
        if (!mesh.hasNormals()) return;

        ofDisableLighting();
        ofSetColor(255, 255, 0);
        ofSetLineWidth(1.0f);

        const auto& vertices = mesh.getVertices();
        const auto& normals = mesh.getNormals();

        for (size_t i = 0; i < vertices.size() && i < normals.size(); i++) {
            ofVec3f start = vertices[i];
            ofVec3f end = start + normals[i] * 0.3f;
            ofDrawLine(start.x, start.y, start.z, end.x, end.y, end.z);
        }

        if (lightingEnabled) {
            ofEnableLighting();
        }
    }

    void drawUI() {
        ofDisableLighting();
        ofSetColor(255, 255, 255);

        int y = 30;
        ofDrawBitmapString("09_mesh - ofMesh API Demonstration", 20, y);
        y += 25;

        ofDrawBitmapString("Mode: " + getModeDescription(), 20, y);
        y += 20;

        const ofMesh& mesh = meshes[currentMeshMode];
        ofDrawBitmapString("Vertices: " + ofToString((int)mesh.getNumVertices()), 20, y);
        y += 20;
        ofDrawBitmapString("Indices: " + ofToString((int)mesh.getNumIndices()) +
                          (mesh.hasIndices() ? " (indexed)" : " (non-indexed)"), 20, y);
        y += 20;
        ofDrawBitmapString("Primitive: " + getPrimitiveModeString(mesh.getMode()), 20, y);
        y += 20;
        ofDrawBitmapString("Normals: " + std::string(mesh.hasNormals() ? "Yes" : "No"), 20, y);
        y += 20;
        ofDrawBitmapString("Colors: " + std::string(mesh.hasColors() ? "Yes" : "No"), 20, y);
        y += 30;

        ofDrawBitmapString("Animation: " + std::string(animationEnabled ? "ON" : "OFF"), 20, y);
        y += 20;
        ofDrawBitmapString("Wireframe: " + std::string(wireframeMode ? "ON" : "OFF"), 20, y);
        y += 20;
        ofDrawBitmapString("Show Normals: " + std::string(showNormals ? "ON" : "OFF"), 20, y);
        y += 20;
        ofDrawBitmapString("Lighting: " + std::string(lightingEnabled ? "ON" : "OFF"), 20, y);
        y += 20;
        ofDrawBitmapString("Resolution: " + ofToString(meshResolution), 20, y);
        y += 30;

        ofDrawBitmapString("FPS: " + ofToString((int)ofGetFrameRate()), 20, y);
    }

    std::string getModeDescription() const {
        switch (currentMeshMode) {
            case 0: return "Custom Triangle Mesh (Pyramid with per-vertex colors)";
            case 1: return "Procedural Sphere (UV Sphere)";
            case 2: return "Procedural Box";
            case 3: return "Procedural Plane (Subdivided)";
            case 4: return "Line Strip (Spiral)";
            case 5: return "Points (Point Cloud)";
            case 6: return "Procedural Icosphere (Geodesic)";
            default: return "Unknown";
        }
    }

    std::string getPrimitiveModeString(ofPrimitiveMode mode) const {
        switch (mode) {
            case OF_PRIMITIVE_TRIANGLES: return "TRIANGLES";
            case OF_PRIMITIVE_TRIANGLE_STRIP: return "TRIANGLE_STRIP";
            case OF_PRIMITIVE_TRIANGLE_FAN: return "TRIANGLE_FAN";
            case OF_PRIMITIVE_LINES: return "LINES";
            case OF_PRIMITIVE_LINE_STRIP: return "LINE_STRIP";
            case OF_PRIMITIVE_LINE_LOOP: return "LINE_LOOP";
            case OF_PRIMITIVE_POINTS: return "POINTS";
            default: return "UNKNOWN";
        }
    }

    void printCurrentMode() {
        ofLogNotice("Mode " + ofToString(currentMeshMode + 1)) << getModeDescription();
        ofLogNotice() << "Vertices: " << meshes[currentMeshMode].getNumVertices()
                      << ", Indices: " << meshes[currentMeshMode].getNumIndices()
                      << ", Primitive: " << getPrimitiveModeString(meshes[currentMeshMode].getMode());
    }

    void keyPressed(int key) override {
        switch (key) {
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                currentMeshMode = key - '1';
                printCurrentMode();
                break;

            case ' ':
                animationEnabled = !animationEnabled;
                ofLogNotice("keyPressed") << "Animation: " << (animationEnabled ? "ON" : "OFF");
                break;

            case 'w':
            case 'W':
                wireframeMode = !wireframeMode;
                ofLogNotice("keyPressed") << "Wireframe: " << (wireframeMode ? "ON" : "OFF");
                break;

            case 'n':
            case 'N':
                showNormals = !showNormals;
                ofLogNotice("keyPressed") << "Show Normals: " << (showNormals ? "ON" : "OFF");
                break;

            case 'l':
            case 'L':
                lightingEnabled = !lightingEnabled;
                ofLogNotice("keyPressed") << "Lighting: " << (lightingEnabled ? "ON" : "OFF");
                break;

            case 'r':
            case 'R':
                animationTime = 0.0f;
                camera.setPosition(0, 4, 10);
                camera.lookAt(ofVec3f(0, 0, 0));
                ofLogNotice("keyPressed") << "Reset camera and animation";
                break;

            case OF_KEY_UP:
                meshResolution += 4;
                if (meshResolution > 60) meshResolution = 60;
                buildMeshes();
                ofLogNotice("keyPressed") << "Resolution increased to " << meshResolution;
                break;

            case OF_KEY_DOWN:
                meshResolution -= 4;
                if (meshResolution < 8) meshResolution = 8;
                buildMeshes();
                ofLogNotice("keyPressed") << "Resolution decreased to " << meshResolution;
                break;
        }
    }

    void windowResized(int w, int h) override {
        camera.setAspectRatio((float)w / (float)h);
    }

private:
    // Camera and lighting
    ofCamera camera;
    ofLight light;
    ofMaterial material;

    // Meshes (7 different modes)
    ofMesh meshes[7];

    // State
    int currentMeshMode;
    float animationTime;
    bool animationEnabled;
    bool wireframeMode;
    bool showNormals;
    bool lightingEnabled;
    int meshResolution;
};

// ========================================================================
// Main entry point
// ========================================================================

int main() {
    ofSetupOpenGL(1280, 720, OF_WINDOW);
    ofRunApp(new MeshApp());
    return 0;
}
