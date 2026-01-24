#pragma once

// ofxGui - SwiftUI-based GUI for oflike-metal
// Architecture: SwiftUI for GUI rendering, C++ for parameter management
// Compatible with: macOS 13.0+

#include <string>
#include <functional>
#include <memory>
#include "../../../oflike/types/ofColor.h"

// Use oflike types
using oflike::ofColor;

// Forward declarations
class GuiFloatParameter;
class GuiIntParameter;
class GuiBoolParameter;
class GuiColorParameter;
class GuiButtonParameter;
class GuiStringParameter;

// MARK: - ofxGuiGroup

/// GUI parameter group
/// Groups related parameters together in the GUI
class ofxGuiGroup {
public:
    ofxGuiGroup();
    ofxGuiGroup(const std::string& name);
    ~ofxGuiGroup();

    /// Set group name
    void setName(const std::string& name);

    /// Get group name
    std::string getName() const;

    // Parameter creation methods
    void addSlider(const std::string& name, float& value, float min, float max);
    void addSlider(const std::string& name, int& value, int min, int max);
    void addToggle(const std::string& name, bool& value);
    void addButton(const std::string& name, std::function<void()> callback);
    void addColor(const std::string& name, ofColor& color);
    void addLabel(const std::string& name, std::string& value);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// MARK: - ofxPanel

/// Main GUI panel
/// Container for GUI groups and parameters
/// Renders using SwiftUI with .ultraThinMaterial background
class ofxPanel {
public:
    ofxPanel();
    ~ofxPanel();

    /// Setup panel with name
    void setup(const std::string& name = "GUI Panel");

    /// Add a parameter group
    void add(ofxGuiGroup& group);

    // Direct parameter addition (creates default group)
    void addSlider(const std::string& name, float& value, float min, float max);
    void addSlider(const std::string& name, int& value, int min, int max);
    void addToggle(const std::string& name, bool& value);
    void addButton(const std::string& name, std::function<void()> callback);
    void addColor(const std::string& name, ofColor& color);
    void addLabel(const std::string& name, std::string& value);

    /// Get panel name
    std::string getName() const;

    /// Clear all parameters
    void clear();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// MARK: - Convenience Types

/// Float slider parameter wrapper
class ofxFloatSlider {
public:
    ofxFloatSlider();
    ofxFloatSlider(const std::string& name, float value, float min, float max, const std::string& group = "General");
    ~ofxFloatSlider();

    void setup(const std::string& name, float value, float min, float max, const std::string& group = "General");

    float getValue() const;
    void setValue(float value);

    operator float() const;
    ofxFloatSlider& operator=(float value);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/// Int slider parameter wrapper
class ofxIntSlider {
public:
    ofxIntSlider();
    ofxIntSlider(const std::string& name, int value, int min, int max, const std::string& group = "General");
    ~ofxIntSlider();

    void setup(const std::string& name, int value, int min, int max, const std::string& group = "General");

    int getValue() const;
    void setValue(int value);

    operator int() const;
    ofxIntSlider& operator=(int value);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/// Boolean toggle parameter wrapper
class ofxToggle {
public:
    ofxToggle();
    ofxToggle(const std::string& name, bool value, const std::string& group = "General");
    ~ofxToggle();

    void setup(const std::string& name, bool value, const std::string& group = "General");

    bool getValue() const;
    void setValue(bool value);

    operator bool() const;
    ofxToggle& operator=(bool value);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/// Button parameter wrapper
class ofxButton {
public:
    ofxButton();
    ofxButton(const std::string& name, std::function<void()> callback, const std::string& group = "General");
    ~ofxButton();

    void setup(const std::string& name, std::function<void()> callback, const std::string& group = "General");

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/// Color picker parameter wrapper
class ofxColorPicker {
public:
    ofxColorPicker();
    ofxColorPicker(const std::string& name, const ofColor& color, const std::string& group = "General");
    ~ofxColorPicker();

    void setup(const std::string& name, const ofColor& color, const std::string& group = "General");

    ofColor getColor() const;
    void setColor(const ofColor& color);

    operator ofColor() const;
    ofxColorPicker& operator=(const ofColor& color);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/// Label (text field) parameter wrapper
class ofxLabel {
public:
    ofxLabel();
    ofxLabel(const std::string& name, const std::string& value, const std::string& group = "General");
    ~ofxLabel();

    void setup(const std::string& name, const std::string& value, const std::string& group = "General");

    std::string getValue() const;
    void setValue(const std::string& value);

    operator std::string() const;
    ofxLabel& operator=(const std::string& value);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
