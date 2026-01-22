#pragma once

#include "../graphics/ofColor.h"

// ofMaterial: Material properties for Phong shading
// Defines how a surface responds to light (ambient, diffuse, specular, emissive)
class ofMaterial {
public:
    ofMaterial();
    ~ofMaterial() = default;

    // Apply material (sets material state for subsequent draw calls)
    void begin() const;
    void end() const;

    // Color Setters
    void setDiffuseColor(const ofFloatColor& c) { diffuse_ = c; }
    void setSpecularColor(const ofFloatColor& c) { specular_ = c; }
    void setAmbientColor(const ofFloatColor& c) { ambient_ = c; }
    void setEmissiveColor(const ofFloatColor& c) { emissive_ = c; }
    void setShininess(float shininess) { shininess_ = shininess; }

    // Color Getters
    ofFloatColor getDiffuseColor() const { return diffuse_; }
    ofFloatColor getSpecularColor() const { return specular_; }
    ofFloatColor getAmbientColor() const { return ambient_; }
    ofFloatColor getEmissiveColor() const { return emissive_; }
    float getShininess() const { return shininess_; }

    // Batch Color Setter
    void setColors(const ofFloatColor& diffuse,
                   const ofFloatColor& ambient,
                   const ofFloatColor& specular,
                   const ofFloatColor& emissive);

protected:
    ofFloatColor diffuse_{0.8f, 0.8f, 0.8f, 1.0f};
    ofFloatColor specular_{0.0f, 0.0f, 0.0f, 1.0f};
    ofFloatColor ambient_{0.2f, 0.2f, 0.2f, 1.0f};
    ofFloatColor emissive_{0.0f, 0.0f, 0.0f, 1.0f};
    float shininess_ = 0.2f;
};
