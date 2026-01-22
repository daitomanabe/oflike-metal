#pragma once

#include "../3d/ofNode.h"
#include "../graphics/ofColor.h"
#include "../math/ofVectorMath.h"

// ofLight: Light source management
// Supports Point, Directional, Spot, and Area lights
// Inherits from ofNode for position and orientation
class ofLight : public ofNode {
public:
    enum Type {
        POINT,        // Point light (omnidirectional)
        DIRECTIONAL,  // Directional light (infinite distance, like sun)
        SPOT,         // Spotlight (cone-shaped)
        AREA          // Area light (planar light source)
    };

    ofLight();
    ~ofLight() = default;

    // Enable/Disable
    void enable();
    void disable();
    bool isEnabled() const { return enabled_; }

    // Light Type
    void setType(Type type) { type_ = type; }
    Type getType() const { return type_; }

    // Convenience type setters
    void setPointLight();
    void setDirectional();
    void setSpotlight(float spotCutOff = 45.0f, float exponent = 0.0f);
    void setAreaLight(float width, float height);

    // Color Properties
    void setAmbientColor(const ofFloatColor& c) { ambient_ = c; }
    void setDiffuseColor(const ofFloatColor& c) { diffuse_ = c; }
    void setSpecularColor(const ofFloatColor& c) { specular_ = c; }

    ofFloatColor getAmbientColor() const { return ambient_; }
    ofFloatColor getDiffuseColor() const { return diffuse_; }
    ofFloatColor getSpecularColor() const { return specular_; }

    // Attenuation (distance falloff)
    void setAttenuation(float constant = 1.0f, float linear = 0.0f, float quadratic = 0.0f);
    float getAttenuationConstant() const { return attenuation_constant_; }
    float getAttenuationLinear() const { return attenuation_linear_; }
    float getAttenuationQuadratic() const { return attenuation_quadratic_; }

    // Spotlight Parameters
    void setSpotConcentration(float concentration) { spotConcentration_ = concentration; }
    void setSpotlightCutOff(float cutoff) { spotCutoff_ = cutoff; }
    float getSpotConcentration() const { return spotConcentration_; }
    float getSpotlightCutOff() const { return spotCutoff_; }

    // Area Light Parameters
    float getWidth() const { return areaWidth_; }
    float getHeight() const { return areaHeight_; }

    // Debug Visualization
    void draw() const;

protected:
    Type type_ = POINT;

    ofFloatColor ambient_{0.0f, 0.0f, 0.0f, 1.0f};
    ofFloatColor diffuse_{1.0f, 1.0f, 1.0f, 1.0f};
    ofFloatColor specular_{1.0f, 1.0f, 1.0f, 1.0f};

    // Attenuation parameters (for point and spot lights)
    float attenuation_constant_ = 1.0f;
    float attenuation_linear_ = 0.0f;
    float attenuation_quadratic_ = 0.0f;

    // Spotlight parameters
    float spotCutoff_ = 45.0f;
    float spotConcentration_ = 0.0f;

    // Area light parameters
    float areaWidth_ = 100.0f;
    float areaHeight_ = 100.0f;

    bool enabled_ = false;
};
