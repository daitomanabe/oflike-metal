#include "ofLight.h"
#include <vector>
#include <algorithm>

namespace oflike {

// ============================================================================
// Construction / Destruction
// ============================================================================

ofLight::ofLight()
    : type_(ofLightType::Point)
    , enabled_(false)
    , position_(0.0f, 0.0f, 0.0f)
    , direction_(0.0f, 0.0f, -1.0f)
    , ambientColor_(0.0f, 0.0f, 0.0f, 1.0f)
    , diffuseColor_(1.0f, 1.0f, 1.0f, 1.0f)
    , specularColor_(1.0f, 1.0f, 1.0f, 1.0f)
    , attenuationConstant_(1.0f)
    , attenuationLinear_(0.0f)
    , attenuationQuadratic_(0.0f)
    , spotCutoff_(45.0f)
    , spotConcentration_(0.0f)
{
}

ofLight::~ofLight() {
}

// ============================================================================
// Enable / Disable
// ============================================================================

void ofLight::enable() {
    enabled_ = true;
}

void ofLight::disable() {
    enabled_ = false;
}

bool ofLight::isEnabled() const {
    return enabled_;
}

// ============================================================================
// Light Type
// ============================================================================

void ofLight::setPointLight() {
    type_ = ofLightType::Point;
}

void ofLight::setDirectional() {
    type_ = ofLightType::Directional;
}

void ofLight::setSpotlight() {
    type_ = ofLightType::Spot;
}

ofLightType ofLight::getLightType() const {
    return type_;
}

// ============================================================================
// Position & Direction
// ============================================================================

void ofLight::setPosition(float x, float y, float z) {
    position_.set(x, y, z);
}

void ofLight::setPosition(const ofVec3f& p) {
    position_ = p;
}

ofVec3f ofLight::getPosition() const {
    return position_;
}

void ofLight::setDirection(float x, float y, float z) {
    direction_.set(x, y, z);
    direction_.normalize();
}

void ofLight::setDirection(const ofVec3f& dir) {
    direction_ = dir;
    direction_.normalize();
}

ofVec3f ofLight::getDirection() const {
    return direction_;
}

// ============================================================================
// Colors
// ============================================================================

void ofLight::setAmbientColor(int r, int g, int b) {
    ambientColor_.set(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
}

void ofLight::setAmbientColor(const ofColor& c) {
    ambientColor_.set(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f);
}

void ofLight::setAmbientColor(const ofFloatColor& c) {
    ambientColor_ = c;
}

ofFloatColor ofLight::getAmbientColor() const {
    return ambientColor_;
}

void ofLight::setDiffuseColor(int r, int g, int b) {
    diffuseColor_.set(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
}

void ofLight::setDiffuseColor(const ofColor& c) {
    diffuseColor_.set(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f);
}

void ofLight::setDiffuseColor(const ofFloatColor& c) {
    diffuseColor_ = c;
}

ofFloatColor ofLight::getDiffuseColor() const {
    return diffuseColor_;
}

void ofLight::setSpecularColor(int r, int g, int b) {
    specularColor_.set(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
}

void ofLight::setSpecularColor(const ofColor& c) {
    specularColor_.set(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f);
}

void ofLight::setSpecularColor(const ofFloatColor& c) {
    specularColor_ = c;
}

ofFloatColor ofLight::getSpecularColor() const {
    return specularColor_;
}

// ============================================================================
// Attenuation
// ============================================================================

void ofLight::setAttenuation(float constant, float linear, float quadratic) {
    attenuationConstant_ = constant;
    attenuationLinear_ = linear;
    attenuationQuadratic_ = quadratic;
}

float ofLight::getAttenuationConstant() const {
    return attenuationConstant_;
}

float ofLight::getAttenuationLinear() const {
    return attenuationLinear_;
}

float ofLight::getAttenuationQuadratic() const {
    return attenuationQuadratic_;
}

// ============================================================================
// Spotlight Parameters
// ============================================================================

void ofLight::setSpotlightCutOff(float cutoff) {
    spotCutoff_ = std::clamp(cutoff, 0.0f, 90.0f);
}

float ofLight::getSpotlightCutOff() const {
    return spotCutoff_;
}

void ofLight::setSpotConcentration(float concentration) {
    spotConcentration_ = std::clamp(concentration, 0.0f, 128.0f);
}

float ofLight::getSpotConcentration() const {
    return spotConcentration_;
}

// ============================================================================
// Internal State
// ============================================================================

std::vector<float> ofLight::getUniformData() const {
    std::vector<float> data;
    data.reserve(32); // Reserve space for all parameters

    // Light type (0=Point, 1=Directional, 2=Spot)
    data.push_back(static_cast<float>(type_));

    // Position
    data.push_back(position_.x);
    data.push_back(position_.y);
    data.push_back(position_.z);

    // Direction
    data.push_back(direction_.x);
    data.push_back(direction_.y);
    data.push_back(direction_.z);

    // Ambient color
    data.push_back(ambientColor_.r);
    data.push_back(ambientColor_.g);
    data.push_back(ambientColor_.b);

    // Diffuse color
    data.push_back(diffuseColor_.r);
    data.push_back(diffuseColor_.g);
    data.push_back(diffuseColor_.b);

    // Specular color
    data.push_back(specularColor_.r);
    data.push_back(specularColor_.g);
    data.push_back(specularColor_.b);

    // Attenuation
    data.push_back(attenuationConstant_);
    data.push_back(attenuationLinear_);
    data.push_back(attenuationQuadratic_);

    // Spotlight parameters
    data.push_back(spotCutoff_);
    data.push_back(spotConcentration_);

    // Enabled state
    data.push_back(enabled_ ? 1.0f : 0.0f);

    return data;
}

} // namespace oflike
