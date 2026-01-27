#pragma once

#include <string>
#include <vector>
#include <memory>

/// \brief XML Settings reader/writer using pugixml
///
/// ofxXmlSettings provides a simple interface for reading and writing XML configuration files.
/// It uses a tag-based navigation system where you can push/pop into nested tags and
/// read/write values at each level.
///
/// Example usage:
/// \code
/// ofxXmlSettings xml;
/// xml.load("settings.xml");
///
/// xml.pushTag("config");
/// xml.pushTag("display");
/// int width = xml.getValue("width", 1920);
/// int height = xml.getValue("height", 1080);
/// xml.popTag(); // display
/// xml.popTag(); // config
/// \endcode
class ofxXmlSettings {
public:
    ofxXmlSettings();
    ~ofxXmlSettings();

    // Copy constructor and assignment
    ofxXmlSettings(const ofxXmlSettings& other);
    ofxXmlSettings& operator=(const ofxXmlSettings& other);

    // File I/O
    /// \brief Load XML from file
    /// \param path Path to XML file
    /// \return true if successful
    bool load(const std::string& path);

    /// \brief Save XML to file
    /// \param path Path to save XML file
    /// \return true if successful
    bool save(const std::string& path);

    /// \brief Load XML from string
    /// \param xmlStr XML content as string
    /// \return true if successful
    bool loadFromString(const std::string& xmlStr);

    /// \brief Get XML content as string
    /// \return XML content
    std::string toString() const;

    /// \brief Clear all data
    void clear();

    // Navigation
    /// \brief Push into a tag (move down the XML tree)
    /// \param name Tag name to push into
    /// \param which Index if multiple tags with same name (default 0)
    /// \return true if tag exists
    bool pushTag(const std::string& name, int which = 0);

    /// \brief Pop out of current tag (move up the XML tree)
    /// \return true if successful
    bool popTag();

    /// \brief Get current tag level depth
    /// \return Current level (0 = root)
    int getLevel() const;

    /// \brief Check if tag exists at current level
    /// \param name Tag name to check
    /// \param which Index if multiple tags with same name (default 0)
    /// \return true if tag exists
    bool tagExists(const std::string& name, int which = 0) const;

    /// \brief Get number of tags with given name at current level
    /// \param name Tag name to count
    /// \return Number of tags
    int getNumTags(const std::string& name) const;

    /// \brief Get current tag name
    /// \return Tag name at current level
    std::string getCurrentTag() const;

    // Value getters
    /// \brief Get integer value
    /// \param name Tag name
    /// \param defaultValue Default value if tag doesn't exist
    /// \param which Index if multiple tags with same name (default 0)
    /// \return Integer value
    int getValue(const std::string& name, int defaultValue, int which = 0) const;

    /// \brief Get float value
    /// \param name Tag name
    /// \param defaultValue Default value if tag doesn't exist
    /// \param which Index if multiple tags with same name (default 0)
    /// \return Float value
    float getValue(const std::string& name, float defaultValue, int which = 0) const;

    /// \brief Get double value
    /// \param name Tag name
    /// \param defaultValue Default value if tag doesn't exist
    /// \param which Index if multiple tags with same name (default 0)
    /// \return Double value
    double getValue(const std::string& name, double defaultValue, int which = 0) const;

    /// \brief Get string value
    /// \param name Tag name
    /// \param defaultValue Default value if tag doesn't exist
    /// \param which Index if multiple tags with same name (default 0)
    /// \return String value
    std::string getValue(const std::string& name, const std::string& defaultValue, int which = 0) const;

    /// \brief Get boolean value
    /// \param name Tag name
    /// \param defaultValue Default value if tag doesn't exist
    /// \param which Index if multiple tags with same name (default 0)
    /// \return Boolean value
    bool getValue(const std::string& name, bool defaultValue, int which = 0) const;

    // Value setters
    /// \brief Set integer value
    /// \param name Tag name
    /// \param value Value to set
    /// \param which Index if multiple tags with same name (default 0)
    /// \return true if successful
    bool setValue(const std::string& name, int value, int which = 0);

    /// \brief Set float value
    /// \param name Tag name
    /// \param value Value to set
    /// \param which Index if multiple tags with same name (default 0)
    /// \return true if successful
    bool setValue(const std::string& name, float value, int which = 0);

    /// \brief Set double value
    /// \param name Tag name
    /// \param value Value to set
    /// \param which Index if multiple tags with same name (default 0)
    /// \return true if successful
    bool setValue(const std::string& name, double value, int which = 0);

    /// \brief Set string value
    /// \param name Tag name
    /// \param value Value to set
    /// \param which Index if multiple tags with same name (default 0)
    /// \return true if successful
    bool setValue(const std::string& name, const std::string& value, int which = 0);

    /// \brief Set boolean value
    /// \param name Tag name
    /// \param value Value to set
    /// \param which Index if multiple tags with same name (default 0)
    /// \return true if successful
    bool setValue(const std::string& name, bool value, int which = 0);

    // Tag manipulation
    /// \brief Add a new tag at current level
    /// \param name Tag name to add
    /// \return Index of newly added tag
    int addTag(const std::string& name);

    /// \brief Remove a tag at current level
    /// \param name Tag name to remove
    /// \param which Index if multiple tags with same name (default 0)
    /// \return true if successful
    bool removeTag(const std::string& name, int which = 0);

    // Attributes
    /// \brief Get attribute value
    /// \param tag Tag name
    /// \param attribute Attribute name
    /// \param defaultValue Default value if attribute doesn't exist
    /// \param which Index if multiple tags with same name (default 0)
    /// \return Attribute value as string
    std::string getAttribute(const std::string& tag, const std::string& attribute,
                            const std::string& defaultValue, int which = 0) const;

    /// \brief Set attribute value
    /// \param tag Tag name
    /// \param attribute Attribute name
    /// \param value Attribute value
    /// \param which Index if multiple tags with same name (default 0)
    /// \return true if successful
    bool setAttribute(const std::string& tag, const std::string& attribute,
                     const std::string& value, int which = 0);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
