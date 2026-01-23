#pragma once

// oflike-metal ofMaterial - openFrameworks API compatible material class
// Provides material properties for 3D rendering with lighting

#include <vector>
#include "../types/ofColor.h"

namespace oflike {

/// \brief Material properties for 3D rendering
/// \details ofMaterial provides an openFrameworks-compatible material interface
/// for defining surface properties in lit 3D scenes. Materials control how
/// surfaces respond to light through ambient, diffuse, specular, and emissive colors,
/// plus a shininess parameter for specular highlights.
class ofMaterial {
public:
    // ========================================================================
    // Construction / Destruction
    // ========================================================================

    /// \brief Default constructor
    /// Creates a material with default gray surface properties
    ofMaterial();

    /// \brief Destructor
    ~ofMaterial();

    // ========================================================================
    // Material Scope
    // ========================================================================

    /// \brief Begin using this material
    /// \details Activates this material for subsequent rendering. All geometry
    /// drawn after begin() and before end() will use this material's properties.
    void begin();

    /// \brief End using this material
    /// \details Deactivates this material and restores the previous material state.
    void end();

    // ========================================================================
    // Ambient Color
    // ========================================================================

    /// \brief Set the ambient color
    /// \param r Red component (0-255)
    /// \param g Green component (0-255)
    /// \param b Blue component (0-255)
    /// \details Ambient color defines the surface's response to ambient lighting.
    /// It represents the color of the surface in shadow or under uniform lighting.
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

    // ========================================================================
    // Diffuse Color
    // ========================================================================

    /// \brief Set the diffuse color
    /// \param r Red component (0-255)
    /// \param g Green component (0-255)
    /// \param b Blue component (0-255)
    /// \details Diffuse color defines the surface's main color under direct lighting.
    /// It determines how the surface scatters light based on the angle between
    /// the surface normal and light direction (Lambertian reflection).
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

    // ========================================================================
    // Specular Color
    // ========================================================================

    /// \brief Set the specular color
    /// \param r Red component (0-255)
    /// \param g Green component (0-255)
    /// \param b Blue component (0-255)
    /// \details Specular color defines the color of highlights on shiny surfaces.
    /// Combined with shininess, it controls the appearance of reflective highlights
    /// based on view angle relative to reflected light direction.
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
    // Emissive Color
    // ========================================================================

    /// \brief Set the emissive color
    /// \param r Red component (0-255)
    /// \param g Green component (0-255)
    /// \param b Blue component (0-255)
    /// \details Emissive color makes the surface appear to emit light.
    /// This color is added to the final surface color regardless of lighting,
    /// making the surface glow. Does not actually illuminate other objects.
    void setEmissiveColor(int r, int g, int b);

    /// \brief Set the emissive color
    /// \param c Color value
    void setEmissiveColor(const ofColor& c);

    /// \brief Set the emissive color
    /// \param c Float color value (0.0-1.0)
    void setEmissiveColor(const ofFloatColor& c);

    /// \brief Get the emissive color
    /// \return Current emissive color
    ofFloatColor getEmissiveColor() const;

    // ========================================================================
    // Shininess
    // ========================================================================

    /// \brief Set the material shininess
    /// \param shininess Shininess exponent (0-128)
    /// \details Controls the size and sharpness of specular highlights.
    /// Higher values create smaller, sharper highlights (more mirror-like).
    /// Lower values create larger, softer highlights (more matte).
    /// Range: 0 (very soft/matte) to 128 (very sharp/shiny).
    /// Default: 64.0
    void setShininess(float shininess);

    /// \brief Get the material shininess
    /// \return Current shininess exponent
    float getShininess() const;

    // ========================================================================
    // Internal State
    // ========================================================================

    /// \brief Get the material's uniform data as a vector
    /// \return A vector containing all material parameters for shader uniform upload
    /// Format: [ambient.rgb, diffuse.rgb, specular.rgb, emissive.rgb, shininess]
    std::vector<float> getUniformData() const;

private:
    // Colors (stored as float 0.0-1.0)
    ofFloatColor ambientColor_;
    ofFloatColor diffuseColor_;
    ofFloatColor specularColor_;
    ofFloatColor emissiveColor_;

    // Shininess parameter
    float shininess_;

    // Previous material state for begin/end nesting
    static ofMaterial* currentMaterial_;
    ofMaterial* previousMaterial_;
};

} // namespace oflike

// Global namespace aliases for openFrameworks compatibility
using ofMaterial = oflike::ofMaterial;
