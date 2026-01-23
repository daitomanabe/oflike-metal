#include "ofMaterial.h"
#include <vector>
#include <algorithm>

namespace oflike {

// Static member for material stack
ofMaterial* ofMaterial::currentMaterial_ = nullptr;

// ============================================================================
// Construction / Destruction
// ============================================================================

ofMaterial::ofMaterial()
    : ambientColor_(0.2f, 0.2f, 0.2f, 1.0f)
    , diffuseColor_(0.8f, 0.8f, 0.8f, 1.0f)
    , specularColor_(1.0f, 1.0f, 1.0f, 1.0f)
    , emissiveColor_(0.0f, 0.0f, 0.0f, 1.0f)
    , shininess_(64.0f)
    , previousMaterial_(nullptr)
{
}

ofMaterial::~ofMaterial() {
}

// ============================================================================
// Material Scope
// ============================================================================

void ofMaterial::begin() {
    // Save the previous material for end()
    previousMaterial_ = currentMaterial_;
    currentMaterial_ = this;

    // TODO: When rendering system is integrated, upload material uniforms here
    // For now, materials are managed through getUniformData() calls
}

void ofMaterial::end() {
    // Restore the previous material
    currentMaterial_ = previousMaterial_;
    previousMaterial_ = nullptr;

    // TODO: When rendering system is integrated, restore previous material uniforms
}

// ============================================================================
// Ambient Color
// ============================================================================

void ofMaterial::setAmbientColor(int r, int g, int b) {
    ambientColor_.set(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
}

void ofMaterial::setAmbientColor(const ofColor& c) {
    ambientColor_ = c;
}

void ofMaterial::setAmbientColor(const ofFloatColor& c) {
    ambientColor_ = c;
}

ofFloatColor ofMaterial::getAmbientColor() const {
    return ambientColor_;
}

// ============================================================================
// Diffuse Color
// ============================================================================

void ofMaterial::setDiffuseColor(int r, int g, int b) {
    diffuseColor_.set(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
}

void ofMaterial::setDiffuseColor(const ofColor& c) {
    diffuseColor_ = c;
}

void ofMaterial::setDiffuseColor(const ofFloatColor& c) {
    diffuseColor_ = c;
}

ofFloatColor ofMaterial::getDiffuseColor() const {
    return diffuseColor_;
}

// ============================================================================
// Specular Color
// ============================================================================

void ofMaterial::setSpecularColor(int r, int g, int b) {
    specularColor_.set(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
}

void ofMaterial::setSpecularColor(const ofColor& c) {
    specularColor_ = c;
}

void ofMaterial::setSpecularColor(const ofFloatColor& c) {
    specularColor_ = c;
}

ofFloatColor ofMaterial::getSpecularColor() const {
    return specularColor_;
}

// ============================================================================
// Emissive Color
// ============================================================================

void ofMaterial::setEmissiveColor(int r, int g, int b) {
    emissiveColor_.set(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
}

void ofMaterial::setEmissiveColor(const ofColor& c) {
    emissiveColor_ = c;
}

void ofMaterial::setEmissiveColor(const ofFloatColor& c) {
    emissiveColor_ = c;
}

ofFloatColor ofMaterial::getEmissiveColor() const {
    return emissiveColor_;
}

// ============================================================================
// Shininess
// ============================================================================

void ofMaterial::setShininess(float shininess) {
    // Clamp to valid range (0-128)
    shininess_ = std::max(0.0f, std::min(128.0f, shininess));
}

float ofMaterial::getShininess() const {
    return shininess_;
}

// ============================================================================
// Internal State
// ============================================================================

std::vector<float> ofMaterial::getUniformData() const {
    std::vector<float> data;
    data.reserve(13); // 3 + 3 + 3 + 3 + 1 = 13 floats

    // Ambient color (RGB)
    data.push_back(ambientColor_.r);
    data.push_back(ambientColor_.g);
    data.push_back(ambientColor_.b);

    // Diffuse color (RGB)
    data.push_back(diffuseColor_.r);
    data.push_back(diffuseColor_.g);
    data.push_back(diffuseColor_.b);

    // Specular color (RGB)
    data.push_back(specularColor_.r);
    data.push_back(specularColor_.g);
    data.push_back(specularColor_.b);

    // Emissive color (RGB)
    data.push_back(emissiveColor_.r);
    data.push_back(emissiveColor_.g);
    data.push_back(emissiveColor_.b);

    // Shininess
    data.push_back(shininess_);

    return data;
}

} // namespace oflike
