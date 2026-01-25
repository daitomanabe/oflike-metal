#pragma once

// SharpGaussian - Core 3D Gaussian representation for Gaussian Splatting
//
// This struct represents a single 3D Gaussian primitive used in Gaussian Splatting.
// Each Gaussian is defined by:
// - position: 3D center point in world space
// - scale: 3D anisotropic scale (ellipsoid dimensions)
// - rotation: orientation as quaternion
// - opacity: transparency value [0,1]
// - spherical harmonics: view-dependent color representation
//
// Reference: https://github.com/graphdeco-inria/gaussian-splatting
// Paper: "3D Gaussian Splatting for Real-Time Radiance Field Rendering"

#include "math/Types.h"
#include <array>

namespace Sharp {

// Maximum degree of spherical harmonics (degree 3 = 16 coefficients)
// Degree 0: 1 coefficient (DC component)
// Degree 1: 3 coefficients
// Degree 2: 5 coefficients
// Degree 3: 7 coefficients
// Total: 1 + 3 + 5 + 7 = 16 coefficients per color channel
constexpr int kMaxSHDegree = 3;
constexpr int kNumSHCoefficients = 16; // (degree + 1)^2

struct Gaussian {
    // ============================================================================
    // Geometric properties
    // ============================================================================

    // Position in world space (xyz)
    oflike::float3 position{0.0f, 0.0f, 0.0f};

    // 3D scale (anisotropic ellipsoid dimensions)
    // Each component represents the scale along the local axis after rotation
    oflike::float3 scale{1.0f, 1.0f, 1.0f};

    // Rotation as quaternion (x, y, z, w)
    // Defines the orientation of the Gaussian ellipsoid
    oflike::quatf rotation = simd_quaternion(0.0f, 0.0f, 0.0f, 1.0f); // identity quaternion

    // Opacity/alpha in range [0,1]
    // 0 = fully transparent, 1 = fully opaque
    float opacity{1.0f};

    // ============================================================================
    // Appearance properties (Spherical Harmonics)
    // ============================================================================

    // DC component (degree 0) - base color in RGB
    // This is the view-independent diffuse color
    oflike::float3 sh_dc{0.5f, 0.5f, 0.5f}; // default gray

    // Spherical harmonics coefficients for view-dependent appearance
    // Stores 15 coefficients (degrees 1-3) for each RGB channel
    // Layout: [R0, R1, ..., R14, G0, G1, ..., G14, B0, B1, ..., B14]
    std::array<oflike::float3, kNumSHCoefficients - 1> sh_rest{};

    // ============================================================================
    // Constructors
    // ============================================================================

    Gaussian() = default;

    Gaussian(const oflike::float3& pos,
             const oflike::float3& scl,
             const oflike::quatf& rot,
             float opac,
             const oflike::float3& color)
        : position(pos)
        , scale(scl)
        , rotation(rot)
        , opacity(opac)
        , sh_dc(color) {
        // sh_rest is zero-initialized by default
    }

    // ============================================================================
    // Utility methods
    // ============================================================================

    // Get the covariance matrix in world space
    // Covariance = R * S * S^T * R^T
    // where R is rotation matrix and S is diagonal scale matrix
    oflike::float3x3 getCovariance() const;

    // Get color for a given viewing direction (normalized)
    // Evaluates spherical harmonics to compute view-dependent color
    oflike::float3 evaluateColor(const oflike::float3& viewDir) const;

    // Get the effective radius for culling/sorting
    // Returns the maximum extent of the Gaussian ellipsoid
    float getRadius() const;

    // Check if this Gaussian is visible (opacity > threshold)
    bool isVisible(float threshold = 0.01f) const {
        return opacity > threshold;
    }
};

} // namespace Sharp
