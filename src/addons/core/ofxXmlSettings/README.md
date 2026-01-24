# ofxXmlSettings

XML configuration file reader/writer addon for oflike-metal.

## Dependencies

- **pugixml** (MIT License) - Fast and lightweight XML parser

## Features

- Load and save XML files
- Tag-based navigation (push/pop)
- Multiple data types: int, float, double, string, bool
- XML attributes support
- Multiple tags with same name support

## Usage

```cpp
#include "ofxXmlSettings.h"

// Create and save XML
ofxXmlSettings xml;
xml.addTag("config");
xml.pushTag("config");

xml.addTag("window");
xml.pushTag("window");
xml.setValue("width", 1920);
xml.setValue("height", 1080);
xml.setValue("fullscreen", false);
xml.popTag(); // window

xml.addTag("display");
xml.pushTag("display");
xml.setValue("fps", 60);
xml.setValue("vsync", true);
xml.popTag(); // display

xml.popTag(); // config

xml.save("settings.xml");

// Load and read XML
ofxXmlSettings xml2;
xml2.load("settings.xml");

xml2.pushTag("config");
xml2.pushTag("window");

int width = xml2.getValue("width", 800);
int height = xml2.getValue("height", 600);
bool fullscreen = xml2.getValue("fullscreen", false);

xml2.popTag(); // window
xml2.popTag(); // config
```

## API Reference

### File I/O

- `bool load(const std::string& path)` - Load XML from file
- `bool save(const std::string& path)` - Save XML to file
- `bool loadFromString(const std::string& xmlStr)` - Load from string
- `std::string toString()` - Get XML as string
- `void clear()` - Clear all data

### Navigation

- `bool pushTag(const std::string& name, int which = 0)` - Move into tag
- `bool popTag()` - Move out of tag
- `int getLevel()` - Get current depth
- `bool tagExists(const std::string& name, int which = 0)` - Check if tag exists
- `int getNumTags(const std::string& name)` - Count tags with name
- `std::string getCurrentTag()` - Get current tag name

### Values

- `T getValue(const std::string& name, T defaultValue, int which = 0)` - Get value (T = int, float, double, string, bool)
- `bool setValue(const std::string& name, T value, int which = 0)` - Set value

### Tag Manipulation

- `int addTag(const std::string& name)` - Add new tag
- `bool removeTag(const std::string& name, int which = 0)` - Remove tag

### Attributes

- `std::string getAttribute(const std::string& tag, const std::string& attribute, const std::string& defaultValue, int which = 0)` - Get attribute
- `bool setAttribute(const std::string& tag, const std::string& attribute, const std::string& value, int which = 0)` - Set attribute

## Example XML

```xml
<?xml version="1.0"?>
<config>
  <window>
    <width>1920</width>
    <height>1080</height>
    <fullscreen>false</fullscreen>
  </window>
  <display>
    <fps>60</fps>
    <vsync>true</vsync>
  </display>
</config>
```

## openFrameworks Compatibility

ofxXmlSettings provides the same API as openFrameworks' ofxXmlSettings, ensuring easy migration.
