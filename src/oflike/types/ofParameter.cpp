#include "ofParameter.h"
#include <algorithm>

// MARK: - ofParameterGroup::Impl

class ofParameterGroup::Impl {
public:
    std::string name;
    std::vector<std::shared_ptr<ofAbstractParameter>> parameters;
};

// MARK: - ofParameterGroup

ofParameterGroup::ofParameterGroup()
    : impl_(std::make_unique<Impl>()) {
    impl_->name = "group";
}

ofParameterGroup::ofParameterGroup(const std::string& name)
    : impl_(std::make_unique<Impl>()) {
    impl_->name = name;
}

ofParameterGroup::~ofParameterGroup() = default;

std::string ofParameterGroup::getName() const {
    return impl_->name;
}

void ofParameterGroup::setName(const std::string& name) {
    impl_->name = name;
}

std::string ofParameterGroup::getValueAsString() const {
    std::string result = impl_->name + " {";
    for (size_t i = 0; i < impl_->parameters.size(); ++i) {
        if (i > 0) result += ", ";
        result += impl_->parameters[i]->getName();
        result += ": ";
        result += impl_->parameters[i]->getValueAsString();
    }
    result += "}";
    return result;
}

void ofParameterGroup::setFromString(const std::string&) {
    // Not implemented - groups don't set from string
}

void ofParameterGroup::add(std::shared_ptr<ofAbstractParameter> param) {
    impl_->parameters.push_back(param);
}

size_t ofParameterGroup::size() const {
    return impl_->parameters.size();
}

std::shared_ptr<ofAbstractParameter> ofParameterGroup::get(size_t index) const {
    if (index < impl_->parameters.size()) {
        return impl_->parameters[index];
    }
    return nullptr;
}

std::shared_ptr<ofAbstractParameter> ofParameterGroup::get(const std::string& name) const {
    auto it = std::find_if(impl_->parameters.begin(), impl_->parameters.end(),
        [&name](const std::shared_ptr<ofAbstractParameter>& param) {
            return param->getName() == name;
        });

    if (it != impl_->parameters.end()) {
        return *it;
    }
    return nullptr;
}

void ofParameterGroup::clear() {
    impl_->parameters.clear();
}
