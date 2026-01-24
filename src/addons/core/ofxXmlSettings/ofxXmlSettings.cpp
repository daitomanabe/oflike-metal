#include "ofxXmlSettings.h"
#include "../../../third_party/pugixml/src/pugixml.hpp"
#include <sstream>
#include <stack>

class ofxXmlSettings::Impl {
public:
    pugi::xml_document doc_;
    std::stack<pugi::xml_node> nodeStack_;

    Impl() {
        // Initialize with root node
        nodeStack_.push(doc_);
    }

    pugi::xml_node getCurrentNode() const {
        if (nodeStack_.empty()) {
            return doc_;
        }
        return nodeStack_.top();
    }

    pugi::xml_node findChildNode(const std::string& name, int which) const {
        pugi::xml_node current = getCurrentNode();
        int count = 0;

        for (pugi::xml_node child : current.children(name.c_str())) {
            if (count == which) {
                return child;
            }
            count++;
        }

        return pugi::xml_node();
    }
};

ofxXmlSettings::ofxXmlSettings()
    : impl_(std::make_unique<Impl>()) {
}

ofxXmlSettings::~ofxXmlSettings() = default;

ofxXmlSettings::ofxXmlSettings(const ofxXmlSettings& other)
    : impl_(std::make_unique<Impl>()) {
    impl_->doc_.reset(other.impl_->doc_);
    // Copy stack state
    impl_->nodeStack_ = other.impl_->nodeStack_;
}

ofxXmlSettings& ofxXmlSettings::operator=(const ofxXmlSettings& other) {
    if (this != &other) {
        impl_->doc_.reset(other.impl_->doc_);
        impl_->nodeStack_ = other.impl_->nodeStack_;
    }
    return *this;
}

bool ofxXmlSettings::load(const std::string& path) {
    pugi::xml_parse_result result = impl_->doc_.load_file(path.c_str());

    if (result) {
        // Reset navigation stack
        while (!impl_->nodeStack_.empty()) {
            impl_->nodeStack_.pop();
        }
        impl_->nodeStack_.push(impl_->doc_);
        return true;
    }

    return false;
}

bool ofxXmlSettings::save(const std::string& path) {
    return impl_->doc_.save_file(path.c_str(), "  ");
}

bool ofxXmlSettings::loadFromString(const std::string& xmlStr) {
    pugi::xml_parse_result result = impl_->doc_.load_string(xmlStr.c_str());

    if (result) {
        // Reset navigation stack
        while (!impl_->nodeStack_.empty()) {
            impl_->nodeStack_.pop();
        }
        impl_->nodeStack_.push(impl_->doc_);
        return true;
    }

    return false;
}

std::string ofxXmlSettings::toString() const {
    std::ostringstream oss;
    impl_->doc_.save(oss, "  ");
    return oss.str();
}

void ofxXmlSettings::clear() {
    impl_->doc_.reset();

    // Reset navigation stack
    while (!impl_->nodeStack_.empty()) {
        impl_->nodeStack_.pop();
    }
    impl_->nodeStack_.push(impl_->doc_);
}

bool ofxXmlSettings::pushTag(const std::string& name, int which) {
    pugi::xml_node child = impl_->findChildNode(name, which);

    if (child) {
        impl_->nodeStack_.push(child);
        return true;
    }

    return false;
}

bool ofxXmlSettings::popTag() {
    if (impl_->nodeStack_.size() > 1) {
        impl_->nodeStack_.pop();
        return true;
    }

    return false;
}

int ofxXmlSettings::getLevel() const {
    return static_cast<int>(impl_->nodeStack_.size()) - 1;
}

bool ofxXmlSettings::tagExists(const std::string& name, int which) const {
    pugi::xml_node node = impl_->findChildNode(name, which);
    return static_cast<bool>(node);
}

int ofxXmlSettings::getNumTags(const std::string& name) const {
    pugi::xml_node current = impl_->getCurrentNode();
    int count = 0;

    for (pugi::xml_node child : current.children(name.c_str())) {
        (void)child; // Suppress unused variable warning
        count++;
    }

    return count;
}

std::string ofxXmlSettings::getCurrentTag() const {
    pugi::xml_node current = impl_->getCurrentNode();
    return current.name();
}

int ofxXmlSettings::getValue(const std::string& name, int defaultValue, int which) const {
    pugi::xml_node node = impl_->findChildNode(name, which);

    if (node) {
        return node.text().as_int(defaultValue);
    }

    return defaultValue;
}

float ofxXmlSettings::getValue(const std::string& name, float defaultValue, int which) const {
    pugi::xml_node node = impl_->findChildNode(name, which);

    if (node) {
        return node.text().as_float(defaultValue);
    }

    return defaultValue;
}

double ofxXmlSettings::getValue(const std::string& name, double defaultValue, int which) const {
    pugi::xml_node node = impl_->findChildNode(name, which);

    if (node) {
        return node.text().as_double(defaultValue);
    }

    return defaultValue;
}

std::string ofxXmlSettings::getValue(const std::string& name, const std::string& defaultValue, int which) const {
    pugi::xml_node node = impl_->findChildNode(name, which);

    if (node) {
        const char* text = node.text().as_string();
        return text ? text : defaultValue;
    }

    return defaultValue;
}

bool ofxXmlSettings::getValue(const std::string& name, bool defaultValue, int which) const {
    pugi::xml_node node = impl_->findChildNode(name, which);

    if (node) {
        return node.text().as_bool(defaultValue);
    }

    return defaultValue;
}

bool ofxXmlSettings::setValue(const std::string& name, int value, int which) {
    pugi::xml_node current = impl_->getCurrentNode();
    pugi::xml_node node = impl_->findChildNode(name, which);

    if (!node) {
        // Create new node if it doesn't exist
        if (which == 0) {
            node = current.append_child(name.c_str());
        } else {
            return false;
        }
    }

    if (node) {
        node.text().set(value);
        return true;
    }

    return false;
}

bool ofxXmlSettings::setValue(const std::string& name, float value, int which) {
    pugi::xml_node current = impl_->getCurrentNode();
    pugi::xml_node node = impl_->findChildNode(name, which);

    if (!node) {
        if (which == 0) {
            node = current.append_child(name.c_str());
        } else {
            return false;
        }
    }

    if (node) {
        node.text().set(value);
        return true;
    }

    return false;
}

bool ofxXmlSettings::setValue(const std::string& name, double value, int which) {
    pugi::xml_node current = impl_->getCurrentNode();
    pugi::xml_node node = impl_->findChildNode(name, which);

    if (!node) {
        if (which == 0) {
            node = current.append_child(name.c_str());
        } else {
            return false;
        }
    }

    if (node) {
        node.text().set(value);
        return true;
    }

    return false;
}

bool ofxXmlSettings::setValue(const std::string& name, const std::string& value, int which) {
    pugi::xml_node current = impl_->getCurrentNode();
    pugi::xml_node node = impl_->findChildNode(name, which);

    if (!node) {
        if (which == 0) {
            node = current.append_child(name.c_str());
        } else {
            return false;
        }
    }

    if (node) {
        node.text().set(value.c_str());
        return true;
    }

    return false;
}

bool ofxXmlSettings::setValue(const std::string& name, bool value, int which) {
    pugi::xml_node current = impl_->getCurrentNode();
    pugi::xml_node node = impl_->findChildNode(name, which);

    if (!node) {
        if (which == 0) {
            node = current.append_child(name.c_str());
        } else {
            return false;
        }
    }

    if (node) {
        node.text().set(value);
        return true;
    }

    return false;
}

int ofxXmlSettings::addTag(const std::string& name) {
    pugi::xml_node current = impl_->getCurrentNode();
    current.append_child(name.c_str());

    // Return the index of the newly added tag
    return getNumTags(name) - 1;
}

bool ofxXmlSettings::removeTag(const std::string& name, int which) {
    pugi::xml_node node = impl_->findChildNode(name, which);

    if (node) {
        pugi::xml_node current = impl_->getCurrentNode();
        current.remove_child(node);
        return true;
    }

    return false;
}

std::string ofxXmlSettings::getAttribute(const std::string& tag, const std::string& attribute,
                                         const std::string& defaultValue, int which) const {
    pugi::xml_node node = impl_->findChildNode(tag, which);

    if (node) {
        pugi::xml_attribute attr = node.attribute(attribute.c_str());
        if (attr) {
            return attr.as_string(defaultValue.c_str());
        }
    }

    return defaultValue;
}

bool ofxXmlSettings::setAttribute(const std::string& tag, const std::string& attribute,
                                  const std::string& value, int which) {
    pugi::xml_node node = impl_->findChildNode(tag, which);

    if (node) {
        pugi::xml_attribute attr = node.attribute(attribute.c_str());
        if (!attr) {
            attr = node.append_attribute(attribute.c_str());
        }
        attr.set_value(value.c_str());
        return true;
    }

    return false;
}
