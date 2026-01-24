#include "SharpGaussian.h"
#include <simd/simd.h>
#include <cmath>

namespace Sharp {

// ============================================================================
// Gaussian utility method implementations
// ============================================================================

oflike::float3x3 Gaussian::getCovariance() const {
    // Convert quaternion to rotation matrix
    // Using simd_matrix3x3(quaternion) for direct conversion
    oflike::float3x3 R = simd_matrix3x3(rotation);

    // Create diagonal scale matrix S manually
    // S = diag(scale.x, scale.y, scale.z)
    oflike::float3x3 S = simd_matrix(
        simd_make_float3(scale.x, 0.0f, 0.0f),
        simd_make_float3(0.0f, scale.y, 0.0f),
        simd_make_float3(0.0f, 0.0f, scale.z)
    );

    // Covariance = R * (S * S^T) * R^T
    oflike::float3x3 SST = simd_mul(S, simd_transpose(S));
    oflike::float3x3 cov = simd_mul(simd_mul(R, SST), simd_transpose(R));

    return cov;
}

oflike::float3 Gaussian::evaluateColor(const oflike::float3& viewDir) const {
    // Start with DC component (degree 0)
    oflike::float3 color = sh_dc;

    // For now, return just the DC component
    // Full spherical harmonics evaluation would evaluate all coefficients
    // based on viewing direction using spherical harmonics basis functions
    //
    // TODO: Implement full SH evaluation if needed for view-dependent appearance
    // Formula: color = SH_0 + sum(SH_l * Y_l(viewDir)) for l=1..3

    return color;
}

float Gaussian::getRadius() const {
    // Return the maximum extent of the ellipsoid
    // This is useful for culling and sorting
    float maxScale = std::max({scale.x, scale.y, scale.z});

    // Use 3-sigma rule (99.7% of Gaussian mass is within 3 standard deviations)
    return maxScale * 3.0f;
}

} // namespace Sharp
