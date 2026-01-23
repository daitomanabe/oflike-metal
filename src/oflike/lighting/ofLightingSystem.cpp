#include "ofLightingSystem.h"
#include <algorithm>

namespace oflike {

// ============================================================================
// Singleton Access
// ============================================================================

ofLightingSystem& ofLightingSystem::instance() {
    static ofLightingSystem inst;
    return inst;
}

// ============================================================================
// Light Registration
// ============================================================================

int ofLightingSystem::registerLight(ofLight* light) {
    if (!light) {
        return -1;
    }

    // Find first available slot
    for (int i = 0; i < MAX_LIGHTS; ++i) {
        if (lights_[i] == nullptr) {
            lights_[i] = light;
            return i;
        }
    }

    // No slots available
    return -1;
}

void ofLightingSystem::unregisterLight(int lightId) {
    if (lightId >= 0 && lightId < MAX_LIGHTS) {
        lights_[lightId] = nullptr;
    }
}

// ============================================================================
// Active Light Queries
// ============================================================================

int ofLightingSystem::getActiveLightCount() const {
    int count = 0;
    for (int i = 0; i < MAX_LIGHTS; ++i) {
        if (lights_[i] != nullptr && lights_[i]->isEnabled()) {
            ++count;
        }
    }
    return count;
}

std::vector<ofLight*> ofLightingSystem::getActiveLights() const {
    std::vector<ofLight*> activeLights;
    activeLights.reserve(MAX_LIGHTS);

    for (int i = 0; i < MAX_LIGHTS; ++i) {
        if (lights_[i] != nullptr && lights_[i]->isEnabled()) {
            activeLights.push_back(lights_[i]);
        }
    }

    return activeLights;
}

// ============================================================================
// Uniform Buffer Data
// ============================================================================

std::vector<float> ofLightingSystem::getLightUniformData() const {
    std::vector<float> uniformData;

    // Reserve space for all light slots (even if empty, for consistent buffer layout)
    // Each light uses a fixed layout for easy indexing in shaders
    // We'll pack the data tightly with only enabled lights

    for (int i = 0; i < MAX_LIGHTS; ++i) {
        if (lights_[i] != nullptr) {
            // Get uniform data from the light
            std::vector<float> lightData = lights_[i]->getUniformData();
            uniformData.insert(uniformData.end(), lightData.begin(), lightData.end());
        } else {
            // Empty slot - push disabled light data (type=-1, enabled=0)
            // This maintains consistent buffer layout if shader expects fixed array
            // For now, we'll only include active lights (sparse array)
            // If fixed layout is needed, uncomment below and pad with zeros

            // uniformData.push_back(-1.0f); // type = invalid
            // for (int j = 0; j < 47; ++j) {
            //     uniformData.push_back(0.0f);
            // }
        }
    }

    return uniformData;
}

// ============================================================================
// Clear
// ============================================================================

void ofLightingSystem::clear() {
    for (int i = 0; i < MAX_LIGHTS; ++i) {
        lights_[i] = nullptr;
    }
}

} // namespace oflike
