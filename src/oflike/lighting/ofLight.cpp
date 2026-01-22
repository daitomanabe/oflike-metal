#include "ofLight.h"

ofLight::ofLight() {
    // Default: point light with white diffuse
    type_ = POINT;
    diffuse_ = ofFloatColor(1.0f, 1.0f, 1.0f, 1.0f);
    specular_ = ofFloatColor(1.0f, 1.0f, 1.0f, 1.0f);
    ambient_ = ofFloatColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void ofLight::enable() {
    enabled_ = true;
    // TODO: Register this light with the rendering system
    // Will be implemented when integrating with DrawList/MetalRenderer
}

void ofLight::disable() {
    enabled_ = false;
    // TODO: Unregister this light from the rendering system
}

void ofLight::setPointLight() {
    type_ = POINT;
}

void ofLight::setDirectional() {
    type_ = DIRECTIONAL;
}

void ofLight::setSpotlight(float spotCutOff, float exponent) {
    type_ = SPOT;
    spotCutoff_ = spotCutOff;
    spotConcentration_ = exponent;
}

void ofLight::setAreaLight(float width, float height) {
    type_ = AREA;
    areaWidth_ = width;
    areaHeight_ = height;
}

void ofLight::setAttenuation(float constant, float linear, float quadratic) {
    attenuation_constant_ = constant;
    attenuation_linear_ = linear;
    attenuation_quadratic_ = quadratic;
}

void ofLight::draw() const {
    // Debug visualization of light
    // TODO: Implement debug drawing
    // - Point light: draw small sphere
    // - Directional light: draw arrow
    // - Spot light: draw cone wireframe
    // - Area light: draw rectangle
}
