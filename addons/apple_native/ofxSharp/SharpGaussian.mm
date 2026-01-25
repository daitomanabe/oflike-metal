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

    // DEFERRED: Full spherical harmonics (SH) evaluation for view-dependent appearance
    //
    // Rationale:
    // - DC component (degree 0) provides view-independent base color (sufficient for many scenes)
    // - Full SH evaluation requires basis function computation: Y_l^m(θ,φ) for l=1..3
    // - Formula: color = SH_DC + Σ(SH_l^m * Y_l^m(viewDir)) for degrees 1-3
    // - GPU-side SH evaluation is more efficient (per-fragment in shader)
    // - CPU-side evaluation only needed for preview/debugging
    // - Deferred until view-dependent effects are validated as necessary
    //
    // Current: DC component provides acceptable results for static scenes
    // For view-dependent effects: Implement in Metal shader with sh_rest coefficients

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
