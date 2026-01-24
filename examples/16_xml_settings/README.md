# 16 - XML Settings Example

This example demonstrates how to use `ofxXmlSettings` to read and write configuration settings in XML format.

## Features Demonstrated

### Core Functionality
- **Load XML**: Read settings from an XML file
- **Save XML**: Write settings to an XML file
- **Tag Navigation**: Navigate XML tree structure with `pushTag()` / `popTag()`
- **Value Types**: Read/write int, float, string, and bool values
- **Default Values**: Provide fallback values when tags don't exist

### XML Operations
- **Create Structure**: Build XML hierarchy programmatically
- **Read Values**: Extract typed values from XML nodes
- **Update Values**: Modify existing XML values
- **File I/O**: Load from and save to disk

## XML Structure

The example creates and manages this XML structure:

```xml
<config>
  <window>
    <width>1920</width>
    <height>1080</height>
    <fullscreen>false</fullscreen>
  </window>
  <audio>
    <volume>0.75</volume>
  </audio>
  <user>
    <name>user</name>
  </user>
</config>
```

## Controls

- **'s'** - Save current settings to `settings.xml`
- **'l'** - Reload settings from `settings.xml`
- **'f'** - Toggle fullscreen setting
- **UP/DOWN** - Adjust window height
- **LEFT/RIGHT** - Adjust window width

## Usage Pattern

### Reading Settings

```cpp
ofxXmlSettings xml;
xml.load("settings.xml");

// Navigate into nested tags
xml.pushTag("config");
xml.pushTag("window");

// Read values with defaults
int width = xml.getValue("width", 1920);
int height = xml.getValue("height", 1080);
bool fullscreen = xml.getValue("fullscreen", false);

// Navigate back up
xml.popTag(); // window
xml.popTag(); // config
```

### Writing Settings

```cpp
ofxXmlSettings xml;

// Create structure
xml.addTag("config");
xml.pushTag("config");

xml.addTag("window");
xml.pushTag("window");
xml.setValue("width", 1920);
xml.setValue("height", 1080);
xml.popTag(); // window

xml.popTag(); // config

// Save to file
xml.save("settings.xml");
```

## Implementation Details

- **pugixml Backend**: Uses the fast pugixml library for XML parsing
- **Stack-Based Navigation**: Maintains a stack of XML nodes for traversal
- **Type Safety**: Overloaded getValue/setValue methods for different types
- **Error Handling**: Returns bool success/failure, provides default values

## Use Cases

This pattern is useful for:

- Application configuration files
- User preferences
- Scene/project settings
- Data serialization
- Import/export of structured data

## API Reference

### File I/O
- `bool load(const std::string& path)` - Load from file
- `bool save(const std::string& path)` - Save to file
- `void clear()` - Clear all data

### Navigation
- `bool pushTag(const std::string& name, int which = 0)` - Navigate into tag
- `bool popTag()` - Navigate out of tag
- `int getLevel()` - Get current depth
- `bool tagExists(const std::string& name, int which = 0)` - Check if tag exists
- `int getNumTags(const std::string& name)` - Count tags with name

### Values
- `T getValue(const std::string& name, T defaultValue, int which = 0)` - Read value
- `bool setValue(const std::string& name, T value, int which = 0)` - Write value

Supported types: `int`, `float`, `double`, `string`, `bool`

### Tags
- `int addTag(const std::string& name)` - Add new tag
- `bool removeTag(const std::string& name, int which = 0)` - Remove tag

### Attributes
- `string getAttribute(const string& tag, const string& attr, const string& defaultValue, int which = 0)` - Read attribute
- `bool setAttribute(const string& tag, const string& attr, const string& value, int which = 0)` - Write attribute

## Notes

- The `which` parameter handles multiple tags with the same name (0-indexed)
- Settings are automatically created if they don't exist during `setValue()`
- The example updates volume over time to show real-time value changes
- Press 's' after making changes to persist them to disk
