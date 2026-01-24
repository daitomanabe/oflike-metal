#include "ofxOscMessage.h"
#include <stdexcept>
#include <sstream>

ofxOscMessage::ofxOscMessage() {
}

ofxOscMessage::~ofxOscMessage() {
}

ofxOscMessage::ofxOscMessage(const ofxOscMessage& other)
    : address_(other.address_)
    , arguments_(other.arguments_) {
}

ofxOscMessage& ofxOscMessage::operator=(const ofxOscMessage& other) {
    if (this != &other) {
        address_ = other.address_;
        arguments_ = other.arguments_;
    }
    return *this;
}

void ofxOscMessage::setAddress(const std::string& address) {
    address_ = address;
}

std::string ofxOscMessage::getAddress() const {
    return address_;
}

void ofxOscMessage::addIntArg(int32_t value) {
    Argument arg;
    arg.type = OFX_OSC_TYPE_INT32;
    arg.asInt32 = value;
    arguments_.push_back(arg);
}

void ofxOscMessage::addInt64Arg(int64_t value) {
    Argument arg;
    arg.type = OFX_OSC_TYPE_INT64;
    arg.asInt64 = value;
    arguments_.push_back(arg);
}

void ofxOscMessage::addFloatArg(float value) {
    Argument arg;
    arg.type = OFX_OSC_TYPE_FLOAT;
    arg.asFloat = value;
    arguments_.push_back(arg);
}

void ofxOscMessage::addDoubleArg(double value) {
    Argument arg;
    arg.type = OFX_OSC_TYPE_DOUBLE;
    arg.asDouble = value;
    arguments_.push_back(arg);
}

void ofxOscMessage::addStringArg(const std::string& value) {
    Argument arg;
    arg.type = OFX_OSC_TYPE_STRING;
    arg.asString = value;
    arguments_.push_back(arg);
}

void ofxOscMessage::addBlobArg(const std::vector<uint8_t>& blob) {
    Argument arg;
    arg.type = OFX_OSC_TYPE_BLOB;
    arg.asBlob = blob;
    arguments_.push_back(arg);
}

void ofxOscMessage::addBlobArg(const void* data, size_t size) {
    Argument arg;
    arg.type = OFX_OSC_TYPE_BLOB;
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    arg.asBlob.assign(bytes, bytes + size);
    arguments_.push_back(arg);
}

void ofxOscMessage::addBoolArg(bool value) {
    Argument arg;
    arg.type = value ? OFX_OSC_TYPE_TRUE : OFX_OSC_TYPE_FALSE;
    arg.asBool = value;
    arguments_.push_back(arg);
}

void ofxOscMessage::addCharArg(char value) {
    Argument arg;
    arg.type = OFX_OSC_TYPE_CHAR;
    arg.asChar = value;
    arguments_.push_back(arg);
}

void ofxOscMessage::addNoneArg() {
    Argument arg;
    arg.type = OFX_OSC_TYPE_NONE;
    arguments_.push_back(arg);
}

void ofxOscMessage::addTriggerArg() {
    Argument arg;
    arg.type = OFX_OSC_TYPE_TRIGGER;
    arguments_.push_back(arg);
}

void ofxOscMessage::addRgbaColorArg(uint32_t rgba) {
    Argument arg;
    arg.type = OFX_OSC_TYPE_RGBA_COLOR;
    arg.asRgbaColor = rgba;
    arguments_.push_back(arg);
}

void ofxOscMessage::addMidiMessageArg(uint32_t midiMessage) {
    Argument arg;
    arg.type = OFX_OSC_TYPE_MIDI_MESSAGE;
    arg.asMidiMessage = midiMessage;
    arguments_.push_back(arg);
}

void ofxOscMessage::addMidiMessageArg(uint8_t port, uint8_t status, uint8_t data1, uint8_t data2) {
    uint32_t midiMessage = (port << 24) | (status << 16) | (data1 << 8) | data2;
    addMidiMessageArg(midiMessage);
}

void ofxOscMessage::addTimeTagArg(uint64_t timeTag) {
    Argument arg;
    arg.type = OFX_OSC_TYPE_TIMETAG;
    arg.asTimeTag = timeTag;
    arguments_.push_back(arg);
}

size_t ofxOscMessage::getNumArgs() const {
    return arguments_.size();
}

ofxOscMessage::ArgType ofxOscMessage::getArgType(size_t index) const {
    checkArgumentIndex(index);
    return arguments_[index].type;
}

std::string ofxOscMessage::getArgTypeName(size_t index) const {
    checkArgumentIndex(index);

    switch (arguments_[index].type) {
        case OFX_OSC_TYPE_INT32: return "int32";
        case OFX_OSC_TYPE_INT64: return "int64";
        case OFX_OSC_TYPE_FLOAT: return "float";
        case OFX_OSC_TYPE_DOUBLE: return "double";
        case OFX_OSC_TYPE_STRING: return "string";
        case OFX_OSC_TYPE_BLOB: return "blob";
        case OFX_OSC_TYPE_TRUE: return "true";
        case OFX_OSC_TYPE_FALSE: return "false";
        case OFX_OSC_TYPE_NONE: return "none";
        case OFX_OSC_TYPE_TRIGGER: return "trigger";
        case OFX_OSC_TYPE_TIMETAG: return "timetag";
        case OFX_OSC_TYPE_CHAR: return "char";
        case OFX_OSC_TYPE_RGBA_COLOR: return "rgba";
        case OFX_OSC_TYPE_MIDI_MESSAGE: return "midi";
        default: return "unknown";
    }
}

int32_t ofxOscMessage::getArgAsInt32(size_t index) const {
    checkArgumentIndex(index);
    const Argument& arg = arguments_[index];

    switch (arg.type) {
        case OFX_OSC_TYPE_INT32:
            return arg.asInt32;
        case OFX_OSC_TYPE_INT64:
            return static_cast<int32_t>(arg.asInt64);
        case OFX_OSC_TYPE_FLOAT:
            return static_cast<int32_t>(arg.asFloat);
        case OFX_OSC_TYPE_DOUBLE:
            return static_cast<int32_t>(arg.asDouble);
        default:
            throw std::runtime_error("Cannot convert argument to int32");
    }
}

int64_t ofxOscMessage::getArgAsInt64(size_t index) const {
    checkArgumentIndex(index);
    const Argument& arg = arguments_[index];

    switch (arg.type) {
        case OFX_OSC_TYPE_INT32:
            return static_cast<int64_t>(arg.asInt32);
        case OFX_OSC_TYPE_INT64:
            return arg.asInt64;
        case OFX_OSC_TYPE_FLOAT:
            return static_cast<int64_t>(arg.asFloat);
        case OFX_OSC_TYPE_DOUBLE:
            return static_cast<int64_t>(arg.asDouble);
        default:
            throw std::runtime_error("Cannot convert argument to int64");
    }
}

float ofxOscMessage::getArgAsFloat(size_t index) const {
    checkArgumentIndex(index);
    const Argument& arg = arguments_[index];

    switch (arg.type) {
        case OFX_OSC_TYPE_INT32:
            return static_cast<float>(arg.asInt32);
        case OFX_OSC_TYPE_INT64:
            return static_cast<float>(arg.asInt64);
        case OFX_OSC_TYPE_FLOAT:
            return arg.asFloat;
        case OFX_OSC_TYPE_DOUBLE:
            return static_cast<float>(arg.asDouble);
        default:
            throw std::runtime_error("Cannot convert argument to float");
    }
}

double ofxOscMessage::getArgAsDouble(size_t index) const {
    checkArgumentIndex(index);
    const Argument& arg = arguments_[index];

    switch (arg.type) {
        case OFX_OSC_TYPE_INT32:
            return static_cast<double>(arg.asInt32);
        case OFX_OSC_TYPE_INT64:
            return static_cast<double>(arg.asInt64);
        case OFX_OSC_TYPE_FLOAT:
            return static_cast<double>(arg.asFloat);
        case OFX_OSC_TYPE_DOUBLE:
            return arg.asDouble;
        default:
            throw std::runtime_error("Cannot convert argument to double");
    }
}

std::string ofxOscMessage::getArgAsString(size_t index) const {
    checkArgumentIndex(index);
    const Argument& arg = arguments_[index];

    if (arg.type == OFX_OSC_TYPE_STRING) {
        return arg.asString;
    } else {
        // Convert numeric types to string
        std::ostringstream oss;
        switch (arg.type) {
            case OFX_OSC_TYPE_INT32:
                oss << arg.asInt32;
                return oss.str();
            case OFX_OSC_TYPE_INT64:
                oss << arg.asInt64;
                return oss.str();
            case OFX_OSC_TYPE_FLOAT:
                oss << arg.asFloat;
                return oss.str();
            case OFX_OSC_TYPE_DOUBLE:
                oss << arg.asDouble;
                return oss.str();
            case OFX_OSC_TYPE_CHAR:
                return std::string(1, arg.asChar);
            case OFX_OSC_TYPE_TRUE:
                return "true";
            case OFX_OSC_TYPE_FALSE:
                return "false";
            case OFX_OSC_TYPE_NONE:
                return "nil";
            case OFX_OSC_TYPE_TRIGGER:
                return "trigger";
            default:
                throw std::runtime_error("Cannot convert argument to string");
        }
    }
}

std::vector<uint8_t> ofxOscMessage::getArgAsBlob(size_t index) const {
    checkArgumentIndex(index);
    const Argument& arg = arguments_[index];

    if (arg.type != OFX_OSC_TYPE_BLOB) {
        throw std::runtime_error("Argument is not a blob");
    }

    return arg.asBlob;
}

bool ofxOscMessage::getArgAsBool(size_t index) const {
    checkArgumentIndex(index);
    const Argument& arg = arguments_[index];

    switch (arg.type) {
        case OFX_OSC_TYPE_TRUE:
            return true;
        case OFX_OSC_TYPE_FALSE:
            return false;
        case OFX_OSC_TYPE_INT32:
            return arg.asInt32 != 0;
        case OFX_OSC_TYPE_INT64:
            return arg.asInt64 != 0;
        case OFX_OSC_TYPE_FLOAT:
            return arg.asFloat != 0.0f;
        case OFX_OSC_TYPE_DOUBLE:
            return arg.asDouble != 0.0;
        default:
            throw std::runtime_error("Cannot convert argument to bool");
    }
}

char ofxOscMessage::getArgAsChar(size_t index) const {
    checkArgumentIndex(index);
    const Argument& arg = arguments_[index];

    if (arg.type != OFX_OSC_TYPE_CHAR) {
        throw std::runtime_error("Argument is not a char");
    }

    return arg.asChar;
}

uint32_t ofxOscMessage::getArgAsRgbaColor(size_t index) const {
    checkArgumentIndex(index);
    const Argument& arg = arguments_[index];

    if (arg.type != OFX_OSC_TYPE_RGBA_COLOR) {
        throw std::runtime_error("Argument is not an RGBA color");
    }

    return arg.asRgbaColor;
}

uint32_t ofxOscMessage::getArgAsMidiMessage(size_t index) const {
    checkArgumentIndex(index);
    const Argument& arg = arguments_[index];

    if (arg.type != OFX_OSC_TYPE_MIDI_MESSAGE) {
        throw std::runtime_error("Argument is not a MIDI message");
    }

    return arg.asMidiMessage;
}

uint64_t ofxOscMessage::getArgAsTimeTag(size_t index) const {
    checkArgumentIndex(index);
    const Argument& arg = arguments_[index];

    if (arg.type != OFX_OSC_TYPE_TIMETAG) {
        throw std::runtime_error("Argument is not a time tag");
    }

    return arg.asTimeTag;
}

void ofxOscMessage::clear() {
    address_.clear();
    arguments_.clear();
}

const ofxOscMessage::Argument& ofxOscMessage::getArgument(size_t index) const {
    checkArgumentIndex(index);
    return arguments_[index];
}

void ofxOscMessage::checkArgumentIndex(size_t index) const {
    if (index >= arguments_.size()) {
        throw std::out_of_range("Argument index out of range");
    }
}
