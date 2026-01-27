#pragma once

#include <string>
#include <vector>
#include <cstdint>

/// \brief Represents an OSC message with an address pattern and arguments.
///
/// ofxOscMessage allows you to create and manipulate OSC (Open Sound Control) messages.
/// Messages consist of an address pattern (like "/synth/freq") and a list of arguments
/// of various types (int, float, string, blob, etc.).
///
/// Example usage:
/// \code
/// ofxOscMessage msg;
/// msg.setAddress("/synth/note");
/// msg.addIntArg(60);        // MIDI note number
/// msg.addFloatArg(0.8);     // velocity
/// msg.addStringArg("piano"); // instrument name
/// \endcode
class ofxOscMessage {
public:
    /// \brief Argument types supported by OSC
    enum ArgType {
        OFX_OSC_TYPE_INT32,
        OFX_OSC_TYPE_FLOAT,
        OFX_OSC_TYPE_STRING,
        OFX_OSC_TYPE_BLOB,
        OFX_OSC_TYPE_INT64,
        OFX_OSC_TYPE_DOUBLE,
        OFX_OSC_TYPE_TRUE,
        OFX_OSC_TYPE_FALSE,
        OFX_OSC_TYPE_NONE,       // Nil
        OFX_OSC_TYPE_TRIGGER,    // Impulse/bang
        OFX_OSC_TYPE_TIMETAG,
        OFX_OSC_TYPE_CHAR,
        OFX_OSC_TYPE_RGBA_COLOR,
        OFX_OSC_TYPE_MIDI_MESSAGE
    };

    /// \brief Represents a single argument in an OSC message
    struct Argument {
        ArgType type;

        union {
            int32_t asInt32;
            int64_t asInt64;
            float asFloat;
            double asDouble;
            char asChar;
            bool asBool;
            uint64_t asTimeTag;
            uint32_t asRgbaColor;
            uint32_t asMidiMessage;
        };

        std::string asString;
        std::vector<uint8_t> asBlob;

        Argument() : type(OFX_OSC_TYPE_NONE), asInt32(0) {}
    };

    ofxOscMessage();
    ~ofxOscMessage();

    // Copy constructor and assignment
    ofxOscMessage(const ofxOscMessage& other);
    ofxOscMessage& operator=(const ofxOscMessage& other);

    // Address pattern
    /// \brief Set the OSC address pattern (e.g. "/synth/freq")
    void setAddress(const std::string& address);

    /// \brief Get the OSC address pattern
    std::string getAddress() const;

    // Add arguments
    /// \brief Add a 32-bit integer argument
    void addIntArg(int32_t value);

    /// \brief Add a 64-bit integer argument
    void addInt64Arg(int64_t value);

    /// \brief Add a float argument
    void addFloatArg(float value);

    /// \brief Add a double argument
    void addDoubleArg(double value);

    /// \brief Add a string argument
    void addStringArg(const std::string& value);

    /// \brief Add a blob (binary data) argument
    void addBlobArg(const std::vector<uint8_t>& blob);
    void addBlobArg(const void* data, size_t size);

    /// \brief Add a boolean argument (sent as True or False type tag)
    void addBoolArg(bool value);

    /// \brief Add a character argument
    void addCharArg(char value);

    /// \brief Add a None/Nil argument
    void addNoneArg();

    /// \brief Add a trigger/impulse/bang argument
    void addTriggerArg();

    /// \brief Add an RGBA color argument (32-bit: 0xRRGGBBAA)
    void addRgbaColorArg(uint32_t rgba);

    /// \brief Add a MIDI message argument (32-bit: port, status, data1, data2)
    void addMidiMessageArg(uint32_t midiMessage);
    void addMidiMessageArg(uint8_t port, uint8_t status, uint8_t data1, uint8_t data2);

    /// \brief Add a time tag argument (NTP timestamp)
    void addTimeTagArg(uint64_t timeTag);

    // Query arguments
    /// \brief Get the number of arguments
    size_t getNumArgs() const;

    /// \brief Get the type of the argument at index
    ArgType getArgType(size_t index) const;

    /// \brief Get argument type as a string (e.g. "int32", "float", "string")
    std::string getArgTypeName(size_t index) const;

    // Get arguments (with bounds checking)
    /// \brief Get integer argument at index
    int32_t getArgAsInt32(size_t index) const;

    /// \brief Get 64-bit integer argument at index
    int64_t getArgAsInt64(size_t index) const;

    /// \brief Get float argument at index
    float getArgAsFloat(size_t index) const;

    /// \brief Get double argument at index
    double getArgAsDouble(size_t index) const;

    /// \brief Get string argument at index
    std::string getArgAsString(size_t index) const;

    /// \brief Get blob argument at index
    std::vector<uint8_t> getArgAsBlob(size_t index) const;

    /// \brief Get boolean argument at index
    bool getArgAsBool(size_t index) const;

    /// \brief Get character argument at index
    char getArgAsChar(size_t index) const;

    /// \brief Get RGBA color argument at index
    uint32_t getArgAsRgbaColor(size_t index) const;

    /// \brief Get MIDI message argument at index
    uint32_t getArgAsMidiMessage(size_t index) const;

    /// \brief Get time tag argument at index
    uint64_t getArgAsTimeTag(size_t index) const;

    // Clear
    /// \brief Clear all arguments and reset the address
    void clear();

    // Internal: Get raw argument for serialization
    const Argument& getArgument(size_t index) const;
    const std::vector<Argument>& getArguments() const { return arguments_; }

private:
    std::string address_;
    std::vector<Argument> arguments_;

    void checkArgumentIndex(size_t index) const;
};
