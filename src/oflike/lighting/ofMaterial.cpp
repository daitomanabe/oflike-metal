#include "ofMaterial.h"

ofMaterial::ofMaterial() {
    // Default material: gray diffuse, black specular, dark gray ambient
    diffuse_ = ofFloatColor(0.8f, 0.8f, 0.8f, 1.0f);
    specular_ = ofFloatColor(0.0f, 0.0f, 0.0f, 1.0f);
    ambient_ = ofFloatColor(0.2f, 0.2f, 0.2f, 1.0f);
    emissive_ = ofFloatColor(0.0f, 0.0f, 0.0f, 1.0f);
    shininess_ = 0.2f;
}

void ofMaterial::begin() const {
    // TODO: Push material state to DrawList/MetalRenderer
    // Will be implemented when integrating with Metal shader pipeline
    // This should set the current material uniforms for subsequent draw calls
}

void ofMaterial::end() const {
    // TODO: Pop material state from DrawList/MetalRenderer
    // Restore previous material state
}

void ofMaterial::setColors(const ofFloatColor& diffuse,
                            const ofFloatColor& ambient,
                            const ofFloatColor& specular,
                            const ofFloatColor& emissive) {
    diffuse_ = diffuse;
    ambient_ = ambient;
    specular_ = specular;
    emissive_ = emissive;
}
