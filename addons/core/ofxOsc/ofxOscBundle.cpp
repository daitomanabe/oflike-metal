#include "ofxOscBundle.h"
#include <stdexcept>

// OSC time tag constants
// 0x0000000000000001 = immediate execution (special value in OSC spec)
static const uint64_t OSC_IMMEDIATE_TIMETAG = 0x0000000000000001ULL;

ofxOscBundle::ofxOscBundle()
    : timeTag_(OSC_IMMEDIATE_TIMETAG) {
}

ofxOscBundle::~ofxOscBundle() {
}

ofxOscBundle::ofxOscBundle(const ofxOscBundle& other)
    : timeTag_(other.timeTag_)
    , messages_(other.messages_)
    , bundles_(other.bundles_) {
}

ofxOscBundle& ofxOscBundle::operator=(const ofxOscBundle& other) {
    if (this != &other) {
        timeTag_ = other.timeTag_;
        messages_ = other.messages_;
        bundles_ = other.bundles_;
    }
    return *this;
}

void ofxOscBundle::setTimeTag(uint64_t timeTag) {
    timeTag_ = timeTag;
}

uint64_t ofxOscBundle::getTimeTag() const {
    return timeTag_;
}

void ofxOscBundle::addMessage(const ofxOscMessage& message) {
    messages_.push_back(message);
}

size_t ofxOscBundle::getMessageCount() const {
    return messages_.size();
}

const ofxOscMessage& ofxOscBundle::getMessageAt(size_t index) const {
    checkMessageIndex(index);
    return messages_[index];
}

void ofxOscBundle::addBundle(const ofxOscBundle& bundle) {
    bundles_.push_back(bundle);
}

size_t ofxOscBundle::getBundleCount() const {
    return bundles_.size();
}

const ofxOscBundle& ofxOscBundle::getBundleAt(size_t index) const {
    checkBundleIndex(index);
    return bundles_[index];
}

void ofxOscBundle::clear() {
    messages_.clear();
    bundles_.clear();
    timeTag_ = OSC_IMMEDIATE_TIMETAG;
}

bool ofxOscBundle::isEmpty() const {
    return messages_.empty() && bundles_.empty();
}

size_t ofxOscBundle::getTotalItemCount() const {
    return messages_.size() + bundles_.size();
}

void ofxOscBundle::checkMessageIndex(size_t index) const {
    if (index >= messages_.size()) {
        throw std::out_of_range("ofxOscBundle: message index out of range");
    }
}

void ofxOscBundle::checkBundleIndex(size_t index) const {
    if (index >= bundles_.size()) {
        throw std::out_of_range("ofxOscBundle: bundle index out of range");
    }
}
