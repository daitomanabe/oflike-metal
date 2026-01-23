#pragma once

// oflike-metal Lighting System - Global light array management
// Manages up to 8 active lights and uniform buffer data

#include "ofLight.h"
#include <vector>
#include <array>

namespace oflike {

/// \brief Global lighting system manager
/// \details Manages active lights and provides uniform data for shaders.
/// Supports up to 8 simultaneous lights (hardware typical limit).
class ofLightingSystem {
public:
    /// Maximum number of lights supported (typical GPU limit)
    static constexpr int MAX_LIGHTS = 8;

    /// \brief Get the singleton instance
    /// \return Reference to the global lighting system
    static ofLightingSystem& instance();

    /// \brief Register a light with the system
    /// \param light Pointer to the light to register
    /// \return Light ID (0-7) or -1 if no slots available
    int registerLight(ofLight* light);

    /// \brief Unregister a light from the system
    /// \param lightId The ID returned by registerLight
    void unregisterLight(int lightId);

    /// \brief Get the number of active (enabled) lights
    /// \return Count of enabled lights
    int getActiveLightCount() const;

    /// \brief Get all active lights
    /// \return Vector of pointers to enabled lights
    std::vector<ofLight*> getActiveLights() const;

    /// \brief Get light uniform buffer data for all active lights
    /// \return Flat array of floats suitable for shader uniform buffer
    /// \details Format per light (48 floats):
    /// [type, position.xyz, direction.xyz, ambient.rgb, diffuse.rgb,
    ///  specular.rgb, attenuation.xyz, spotCutoff, spotExponent, enabled]
    std::vector<float> getLightUniformData() const;

    /// \brief Clear all registered lights
    void clear();

private:
    ofLightingSystem() = default;
    ~ofLightingSystem() = default;

    // Prevent copying
    ofLightingSystem(const ofLightingSystem&) = delete;
    ofLightingSystem& operator=(const ofLightingSystem&) = delete;

    // Light slots (nullptr = empty slot)
    std::array<ofLight*, MAX_LIGHTS> lights_ = {{nullptr}};
};

} // namespace oflike

// Global namespace aliases for openFrameworks compatibility
using ofLightingSystem = oflike::ofLightingSystem;
