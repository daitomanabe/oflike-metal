#pragma once

// oflike-metal ofLight - openFrameworks API compatible light class
// Provides point, directional, and spot light sources for 3D rendering

#include <vector>
#include "../math/ofVec3f.h"
#include "../types/ofColor.h"

namespace oflike {

/// \brief Light type enumeration
enum class ofLightType {
    Point,        ///< Point light (omnidirectional)
    Directional,  ///< Directional light (sun-like)
    Spot,         ///< Spotlight (cone-shaped)
    Area          ///< Area light (not yet implemented)
};

/// \brief 3D light source for rendering
/// \details ofLight provides an openFrameworks-compatible lighting interface
/// supporting point, directional, and spot lights with configurable colors,
/// attenuation, and spotlight parameters.
class ofLight {
public:
    // ========================================================================
    // Construction / Destruction
    // ========================================================================

    /// \brief Default constructor
    /// Creates a point light at the origin with white color
    ofLight();

    /// \brief Destructor
    ~ofLight();

    // ========================================================================
    // Enable / Disable
    // ========================================================================

    /// \brief Enable the light
    /// \details Activates this light for rendering. The light must be enabled
    /// to affect the scene.
    void enable();

    /// \brief Disable the light
    /// \details Deactivates this light. Disabled lights do not affect rendering.
    void disable();

    /// \brief Check if the light is enabled
    /// \return true if enabled, false otherwise
    bool isEnabled() const;

    // ========================================================================
    // Light Type
    // ========================================================================

    /// \brief Set the light as a point light
    /// \details Point lights emit light equally in all directions from a position.
    /// Attenuation applies based on distance from the light position.
    void setPointLight();

    /// \brief Set the light as a directional light
    /// \details Directional lights emit parallel rays in a single direction,
    /// like sunlight. Position is ignored; only direction matters.
    void setDirectional();

    /// \brief Set the light as a spotlight
    /// \details Spotlights emit light in a cone shape from a position along a direction.
    /// Use setSpotlightCutOff() and setSpotConcentration() to control the cone.
    void setSpotlight();

    /// \brief Get the current light type
    /// \return The light type (Point, Directional, or Spot)
    ofLightType getLightType() const;

    // ========================================================================
    // Position & Direction
    // ========================================================================

    /// \brief Set the light position
    /// \param x X coordinate
    /// \param y Y coordinate
    /// \param z Z coordinate
    /// \details For point and spot lights, this is the light's location in world space.
    /// For directional lights, position is ignored.
    void setPosition(float x, float y, float z);

    /// \brief Set the light position
    /// \param p Position vector
    void setPosition(const ofVec3f& p);

    /// \brief Get the light position
    /// \return Current position
    ofVec3f getPosition() const;

    /// \brief Set the light direction
    /// \param x X component of direction
    /// \param y Y component of direction
    /// \param z Z component of direction
    /// \details For directional lights, this is the direction light travels.
    /// For spotlights, this is the cone's axis. For point lights, direction is ignored.
    void setDirection(float x, float y, float z);

    /// \brief Set the light direction
    /// \param dir Direction vector (will be normalized)
    void setDirection(const ofVec3f& dir);

    /// \brief Get the light direction
    /// \return Current direction (normalized)
    ofVec3f getDirection() const;

    // ========================================================================
    // Colors
    // ========================================================================

    /// \brief Set the ambient color
    /// \param r Red component (0-255)
    /// \param g Green component (0-255)
    /// \param b Blue component (0-255)
    /// \details Ambient color is the light's contribution to overall scene illumination,
    /// affecting all surfaces equally regardless of position or angle.
    void setAmbientColor(int r, int g, int b);

    /// \brief Set the ambient color
    /// \param c Color value
    void setAmbientColor(const ofColor& c);

    /// \brief Set the ambient color
    /// \param c Float color value (0.0-1.0)
    void setAmbientColor(const ofFloatColor& c);

    /// \brief Get the ambient color
    /// \return Current ambient color
    ofFloatColor getAmbientColor() const;

    /// \brief Set the diffuse color
    /// \param r Red component (0-255)
    /// \param g Green component (0-255)
    /// \param b Blue component (0-255)
    /// \details Diffuse color is the main light color, affected by surface angle
    /// relative to the light direction (Lambertian reflection).
    void setDiffuseColor(int r, int g, int b);

    /// \brief Set the diffuse color
    /// \param c Color value
    void setDiffuseColor(const ofColor& c);

    /// \brief Set the diffuse color
    /// \param c Float color value (0.0-1.0)
    void setDiffuseColor(const ofFloatColor& c);

    /// \brief Get the diffuse color
    /// \return Current diffuse color
    ofFloatColor getDiffuseColor() const;

    /// \brief Set the specular color
    /// \param r Red component (0-255)
    /// \param g Green component (0-255)
    /// \param b Blue component (0-255)
    /// \details Specular color creates highlights on shiny surfaces based on
    /// view angle and surface shininess.
    void setSpecularColor(int r, int g, int b);

    /// \brief Set the specular color
    /// \param c Color value
    void setSpecularColor(const ofColor& c);

    /// \brief Set the specular color
    /// \param c Float color value (0.0-1.0)
    void setSpecularColor(const ofFloatColor& c);

    /// \brief Get the specular color
    /// \return Current specular color
    ofFloatColor getSpecularColor() const;

    // ========================================================================
    // Attenuation
    // ========================================================================

    /// \brief Set the attenuation parameters
    /// \param constant Constant attenuation factor
    /// \param linear Linear attenuation factor
    /// \param quadratic Quadratic attenuation factor
    /// \details Attenuation controls how light intensity decreases with distance.
    /// Final attenuation = 1.0 / (constant + linear*d + quadratic*d^2)
    /// where d is the distance from the light.
    /// Default: (1.0, 0.0, 0.0) for no attenuation.
    void setAttenuation(float constant = 1.0f, float linear = 0.0f, float quadratic = 0.0f);

    /// \brief Get the constant attenuation factor
    /// \return Constant attenuation
    float getAttenuationConstant() const;

    /// \brief Get the linear attenuation factor
    /// \return Linear attenuation
    float getAttenuationLinear() const;

    /// \brief Get the quadratic attenuation factor
    /// \return Quadratic attenuation
    float getAttenuationQuadratic() const;

    // ========================================================================
    // Spotlight Parameters
    // ========================================================================

    /// \brief Set the spotlight cutoff angle
    /// \param cutoff Cutoff angle in degrees (0-90)
    /// \details Controls the spotlight cone's outer angle. Light intensity
    /// fades to zero beyond this angle from the spotlight direction.
    /// Only applies to spotlights.
    void setSpotlightCutOff(float cutoff);

    /// \brief Get the spotlight cutoff angle
    /// \return Cutoff angle in degrees
    float getSpotlightCutOff() const;

    /// \brief Set the spotlight concentration (exponent)
    /// \param concentration Concentration exponent (0-128)
    /// \details Controls how focused the spotlight is. Higher values create
    /// a tighter, more focused beam. 0 = uniform intensity within cone.
    /// Only applies to spotlights.
    void setSpotConcentration(float concentration);

    /// \brief Get the spotlight concentration
    /// \return Concentration exponent
    float getSpotConcentration() const;

    // ========================================================================
    // Internal State
    // ========================================================================

    /// \brief Get the light's uniform data as a vector
    /// \return A vector containing all light parameters for shader uniform upload
    /// Format: [type, position.xyz, direction.xyz, ambient.rgb, diffuse.rgb, specular.rgb,
    ///          attenuation.xyz, spotCutoff, spotExponent, enabled]
    std::vector<float> getUniformData() const;

private:
    // Light type
    ofLightType type_;

    // Enable state
    bool enabled_;

    // Position and direction
    ofVec3f position_;
    ofVec3f direction_;

    // Colors (stored as float 0.0-1.0)
    ofFloatColor ambientColor_;
    ofFloatColor diffuseColor_;
    ofFloatColor specularColor_;

    // Attenuation parameters
    float attenuationConstant_;
    float attenuationLinear_;
    float attenuationQuadratic_;

    // Spotlight parameters
    float spotCutoff_;        // In degrees (0-90)
    float spotConcentration_; // Exponent (0-128)
};

} // namespace oflike

// Global namespace aliases for openFrameworks compatibility
using ofLight = oflike::ofLight;
using ofLightType = oflike::ofLightType;
