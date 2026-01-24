#include "ofMain.h"
#include "ofxXmlSettings.h"

class XMLSettingsApp : public ofBaseApp {
public:
    ofxXmlSettings xml;

    // Settings values
    int windowWidth = 1920;
    int windowHeight = 1080;
    float volume = 0.75f;
    std::string username = "user";
    bool fullscreen = false;

    // Display state
    std::string statusMessage;
    bool settingsLoaded = false;

    void setup() override {
        ofSetWindowTitle("16 - XML Settings Example");
        ofBackground(40);

        // Try to load existing settings
        if (xml.load("settings.xml")) {
            loadSettings();
            statusMessage = "Settings loaded from settings.xml";
            settingsLoaded = true;
        } else {
            // Create default settings
            createDefaultSettings();
            statusMessage = "Created default settings.xml";
        }
    }

    void loadSettings() {
        // Navigate into config tag
        if (xml.pushTag("config")) {

            // Window settings
            if (xml.pushTag("window")) {
                windowWidth = xml.getValue("width", 1920);
                windowHeight = xml.getValue("height", 1080);
                fullscreen = xml.getValue("fullscreen", false);
                xml.popTag(); // window
            }

            // Audio settings
            if (xml.pushTag("audio")) {
                volume = xml.getValue("volume", 0.75f);
                xml.popTag(); // audio
            }

            // User settings
            if (xml.pushTag("user")) {
                username = xml.getValue("name", std::string("user"));
                xml.popTag(); // user
            }

            xml.popTag(); // config
        }
    }

    void createDefaultSettings() {
        xml.clear();

        // Create config structure
        xml.addTag("config");
        xml.pushTag("config");

        // Window settings
        xml.addTag("window");
        xml.pushTag("window");
        xml.setValue("width", windowWidth);
        xml.setValue("height", windowHeight);
        xml.setValue("fullscreen", fullscreen);
        xml.popTag(); // window

        // Audio settings
        xml.addTag("audio");
        xml.pushTag("audio");
        xml.setValue("volume", volume);
        xml.popTag(); // audio

        // User settings
        xml.addTag("user");
        xml.pushTag("user");
        xml.setValue("name", username);
        xml.popTag(); // user

        xml.popTag(); // config

        // Save to file
        xml.save("settings.xml");
    }

    void saveSettings() {
        // Update values in XML
        xml.pushTag("config");

        xml.pushTag("window");
        xml.setValue("width", windowWidth);
        xml.setValue("height", windowHeight);
        xml.setValue("fullscreen", fullscreen);
        xml.popTag(); // window

        xml.pushTag("audio");
        xml.setValue("volume", volume);
        xml.popTag(); // audio

        xml.pushTag("user");
        xml.setValue("name", username);
        xml.popTag(); // user

        xml.popTag(); // config

        // Save to file
        if (xml.save("settings.xml")) {
            statusMessage = "Settings saved successfully";
        } else {
            statusMessage = "Failed to save settings";
        }
    }

    void update() override {
        // Example: modify settings over time
        static float time = 0;
        time += ofGetLastFrameTime();

        // Oscillate volume
        volume = 0.5f + 0.5f * sin(time);
    }

    void draw() override {
        ofSetColor(255);

        int y = 50;
        ofDrawBitmapString("ofxXmlSettings Example", 20, y);
        y += 40;

        ofDrawBitmapString("Status: " + statusMessage, 20, y);
        y += 40;

        ofDrawBitmapString("Current Settings:", 20, y);
        y += 30;

        ofDrawBitmapString("  Window: " + ofToString(windowWidth) + " x " + ofToString(windowHeight), 20, y);
        y += 25;

        ofDrawBitmapString("  Fullscreen: " + std::string(fullscreen ? "true" : "false"), 20, y);
        y += 25;

        ofDrawBitmapString("  Volume: " + ofToString(volume, 2), 20, y);
        y += 25;

        ofDrawBitmapString("  Username: " + username, 20, y);
        y += 50;

        ofDrawBitmapString("Controls:", 20, y);
        y += 30;

        ofDrawBitmapString("  's' - Save current settings to XML", 20, y);
        y += 25;

        ofDrawBitmapString("  'l' - Reload settings from XML", 20, y);
        y += 25;

        ofDrawBitmapString("  'f' - Toggle fullscreen setting", 20, y);
        y += 25;

        ofDrawBitmapString("  'UP/DOWN' - Adjust window height", 20, y);
        y += 25;

        ofDrawBitmapString("  'LEFT/RIGHT' - Adjust window width", 20, y);
        y += 50;

        ofDrawBitmapString("XML Structure:", 20, y);
        y += 30;

        ofSetColor(150, 200, 150);
        ofDrawBitmapString("<config>", 20, y);
        y += 20;
        ofDrawBitmapString("  <window>", 20, y);
        y += 20;
        ofDrawBitmapString("    <width>" + ofToString(windowWidth) + "</width>", 20, y);
        y += 20;
        ofDrawBitmapString("    <height>" + ofToString(windowHeight) + "</height>", 20, y);
        y += 20;
        ofDrawBitmapString("    <fullscreen>" + std::string(fullscreen ? "true" : "false") + "</fullscreen>", 20, y);
        y += 20;
        ofDrawBitmapString("  </window>", 20, y);
        y += 20;
        ofDrawBitmapString("  <audio>", 20, y);
        y += 20;
        ofDrawBitmapString("    <volume>" + ofToString(volume, 2) + "</volume>", 20, y);
        y += 20;
        ofDrawBitmapString("  </audio>", 20, y);
        y += 20;
        ofDrawBitmapString("  <user>", 20, y);
        y += 20;
        ofDrawBitmapString("    <name>" + username + "</name>", 20, y);
        y += 20;
        ofDrawBitmapString("  </user>", 20, y);
        y += 20;
        ofDrawBitmapString("</config>", 20, y);
    }

    void keyPressed(int key) override {
        if (key == 's') {
            saveSettings();
        }
        else if (key == 'l') {
            if (xml.load("settings.xml")) {
                loadSettings();
                statusMessage = "Settings reloaded from XML";
            } else {
                statusMessage = "Failed to load XML";
            }
        }
        else if (key == 'f') {
            fullscreen = !fullscreen;
            statusMessage = "Fullscreen toggled (press 's' to save)";
        }
        else if (key == OF_KEY_UP) {
            windowHeight += 100;
            statusMessage = "Height increased (press 's' to save)";
        }
        else if (key == OF_KEY_DOWN) {
            windowHeight = std::max(480, windowHeight - 100);
            statusMessage = "Height decreased (press 's' to save)";
        }
        else if (key == OF_KEY_RIGHT) {
            windowWidth += 100;
            statusMessage = "Width increased (press 's' to save)";
        }
        else if (key == OF_KEY_LEFT) {
            windowWidth = std::max(640, windowWidth - 100);
            statusMessage = "Width decreased (press 's' to save)";
        }
    }
};

int main() {
    ofSetupOpenGL(800, 600, OF_WINDOW);
    ofRunApp(new XMLSettingsApp());
}
