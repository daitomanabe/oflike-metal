#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;

// ============================================================================
// Gaussian Splatting Structures
// ============================================================================

struct Gaussian {
    float3 position;
    float3 scale;
    simd_quatf rotation;
    float opacity;
    float3 sh_dc;           // Spherical harmonics DC (degree 0)
    float3 sh_rest[15];     // SH coefficients degrees 1-3
};

struct GaussianUniforms {
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 viewProjectionMatrix;
    float3 cameraPosition;
    float splatScale;
    float opacityScale;
    int maxSHDegree;
    float2 viewportSize;
};

// Vertex output / fragment input
struct GaussianVertex {
    float4 position [[position]];
    float2 uv;              // Local quad coordinates (-1 to 1)
    float3 color;           // Color from spherical harmonics
    float opacity;
    float2 cov2D[2];        // 2D covariance matrix (2x2 symmetric)
};

// ============================================================================
// Spherical Harmonics Evaluation
// ============================================================================

// SH basis functions for degree 0-3
// Reference: https://github.com/graphdeco-inria/gaussian-splatting

float3 evaluateSH0(float3 sh_dc) {
    // Degree 0 (DC component)
    const float C0 = 0.28209479177387814;
    return sh_dc * C0;
}

float3 evaluateSH1(float3 sh[3], float3 dir) {
    // Degree 1
    const float C1 = 0.4886025119029199;
    float3 result = float3(0.0);
    result += sh[0] * (-C1 * dir.y);
    result += sh[1] * (C1 * dir.z);
    result += sh[2] * (-C1 * dir.x);
    return result;
}

float3 evaluateSH2(float3 sh[5], float3 dir) {
    // Degree 2
    const float C2_0 = 1.0925484305920792;
    const float C2_1 = -1.0925484305920792;
    const float C2_2 = 0.31539156525252005;
    const float C2_3 = -1.0925484305920792;
    const float C2_4 = 0.5462742152960396;

    float x = dir.x, y = dir.y, z = dir.z;
    float x2 = x * x, y2 = y * y, z2 = z * z;

    float3 result = float3(0.0);
    result += sh[0] * (C2_0 * x * y);
    result += sh[1] * (C2_1 * y * z);
    result += sh[2] * (C2_2 * (2.0 * z2 - x2 - y2));
    result += sh[3] * (C2_3 * x * z);
    result += sh[4] * (C2_4 * (x2 - y2));
    return result;
}

float3 evaluateSH3(float3 sh[7], float3 dir) {
    // Degree 3
    const float C3_0 = -0.5900435899266435;
    const float C3_1 = 2.890611442640554;
    const float C3_2 = -0.4570457994644658;
    const float C3_3 = 0.3731763325901154;
    const float C3_4 = -0.4570457994644658;
    const float C3_5 = 1.445305721320277;
    const float C3_6 = -0.5900435899266435;

    float x = dir.x, y = dir.y, z = dir.z;
    float x2 = x * x, y2 = y * y, z2 = z * z;

    float3 result = float3(0.0);
    result += sh[0] * (C3_0 * y * (3.0 * x2 - y2));
    result += sh[1] * (C3_1 * x * y * z);
    result += sh[2] * (C3_2 * y * (4.0 * z2 - x2 - y2));
    result += sh[3] * (C3_3 * z * (2.0 * z2 - 3.0 * x2 - 3.0 * y2));
    result += sh[4] * (C3_4 * x * (4.0 * z2 - x2 - y2));
    result += sh[5] * (C3_5 * z * (x2 - y2));
    result += sh[6] * (C3_6 * x * (x2 - 3.0 * y2));
    return result;
}

float3 evaluateSphericalHarmonics(constant Gaussian& gaussian,
                                   float3 viewDir,
                                   int maxDegree) {
    // Start with DC component (degree 0)
    float3 color = evaluateSH0(gaussian.sh_dc);

    // Add higher degrees if enabled
    if (maxDegree >= 1 && length(gaussian.sh_rest[0]) > 0.0) {
        color += evaluateSH1((thread float3*)&gaussian.sh_rest[0], viewDir);
    }
    if (maxDegree >= 2 && length(gaussian.sh_rest[3]) > 0.0) {
        color += evaluateSH2((thread float3*)&gaussian.sh_rest[3], viewDir);
    }
    if (maxDegree >= 3 && length(gaussian.sh_rest[8]) > 0.0) {
        color += evaluateSH3((thread float3*)&gaussian.sh_rest[8], viewDir);
    }

    // Clamp to valid range
    color = clamp(color, 0.0, 1.0);
    return color;
}

// ============================================================================
// Covariance 3D → 2D Projection
// ============================================================================

// Compute 3D covariance matrix from scale and rotation
float3x3 computeCovariance3D(constant Gaussian& gaussian) {
    // Create scale matrix
    float3x3 S = float3x3(
        gaussian.scale.x, 0, 0,
        0, gaussian.scale.y, 0,
        0, 0, gaussian.scale.z
    );

    // Convert quaternion to rotation matrix
    float4 q = float4(gaussian.rotation.vector.x,
                      gaussian.rotation.vector.y,
                      gaussian.rotation.vector.z,
                      gaussian.rotation.vector.w);

    float xx = q.x * q.x, yy = q.y * q.y, zz = q.z * q.z;
    float xy = q.x * q.y, xz = q.x * q.z, yz = q.y * q.z;
    float wx = q.w * q.x, wy = q.w * q.y, wz = q.w * q.z;

    float3x3 R = float3x3(
        1.0 - 2.0 * (yy + zz), 2.0 * (xy - wz), 2.0 * (xz + wy),
        2.0 * (xy + wz), 1.0 - 2.0 * (xx + zz), 2.0 * (yz - wx),
        2.0 * (xz - wy), 2.0 * (yz + wx), 1.0 - 2.0 * (xx + yy)
    );

    // Covariance = R * S * S^T * R^T
    float3x3 RS = R * S;
    float3x3 Sigma3D = RS * transpose(RS);

    return Sigma3D;
}

// Project 3D covariance to 2D screen space
void projectCovariance3Dto2D(float3x3 cov3D,
                              float4x4 viewMatrix,
                              float4x4 projMatrix,
                              float3 position,
                              float2 viewportSize,
                              thread float2 cov2D[2]) {
    // Transform position to view space
    float4 viewPos = viewMatrix * float4(position, 1.0);

    // Compute Jacobian of perspective projection
    float focal_x = projMatrix[0][0] * viewportSize.x * 0.5;
    float focal_y = projMatrix[1][1] * viewportSize.y * 0.5;
    float z = viewPos.z;
    float z2 = z * z;

    // Jacobian of perspective projection
    float3x2 J = float3x2(
        focal_x / z, 0.0,
        0.0, focal_y / z,
        -focal_x * viewPos.x / z2, -focal_y * viewPos.y / z2
    );

    // Transform covariance to view space
    float3x3 viewCov3D = (float3x3)viewMatrix * cov3D * transpose((float3x3)viewMatrix);

    // Project to 2D: Sigma2D = J * Sigma3D * J^T
    float2x2 Sigma2D;
    Sigma2D[0][0] = J[0][0] * (viewCov3D[0][0] * J[0][0] + viewCov3D[0][1] * J[1][0] + viewCov3D[0][2] * J[2][0]) +
                    J[1][0] * (viewCov3D[1][0] * J[0][0] + viewCov3D[1][1] * J[1][0] + viewCov3D[1][2] * J[2][0]) +
                    J[2][0] * (viewCov3D[2][0] * J[0][0] + viewCov3D[2][1] * J[1][0] + viewCov3D[2][2] * J[2][0]);

    Sigma2D[0][1] = J[0][0] * (viewCov3D[0][0] * J[0][1] + viewCov3D[0][1] * J[1][1] + viewCov3D[0][2] * J[2][1]) +
                    J[1][0] * (viewCov3D[1][0] * J[0][1] + viewCov3D[1][1] * J[1][1] + viewCov3D[1][2] * J[2][1]) +
                    J[2][0] * (viewCov3D[2][0] * J[0][1] + viewCov3D[2][1] * J[1][1] + viewCov3D[2][2] * J[2][1]);

    Sigma2D[1][0] = Sigma2D[0][1]; // Symmetric

    Sigma2D[1][1] = J[0][1] * (viewCov3D[0][0] * J[0][1] + viewCov3D[0][1] * J[1][1] + viewCov3D[0][2] * J[2][1]) +
                    J[1][1] * (viewCov3D[1][0] * J[0][1] + viewCov3D[1][1] * J[1][1] + viewCov3D[1][2] * J[2][1]) +
                    J[2][1] * (viewCov3D[2][0] * J[0][1] + viewCov3D[2][1] * J[1][1] + viewCov3D[2][2] * J[2][1]);

    // Add small epsilon for numerical stability
    Sigma2D[0][0] += 0.1;
    Sigma2D[1][1] += 0.1;

    // Store as two float2 vectors
    cov2D[0] = Sigma2D[0];
    cov2D[1] = Sigma2D[1];
}

// ============================================================================
// Vertex Shader
// ============================================================================

vertex GaussianVertex gaussianSplattingVertex(
    uint vertexID [[vertex_id]],
    uint instanceID [[instance_id]],
    constant Gaussian* gaussians [[buffer(0)]],
    constant uint* sortedIndices [[buffer(1)]],
    constant GaussianUniforms& uniforms [[buffer(2)]]
) {
    GaussianVertex out;

    // Get Gaussian for this instance (sorted order)
    uint gaussianIndex = sortedIndices[instanceID];
    constant Gaussian& gaussian = gaussians[gaussianIndex];

    // Billboard quad vertices (4 vertices per Gaussian)
    // vertexID: 0, 1, 2, 3 for quad corners
    float2 quadVertices[4] = {
        float2(-1.0, -1.0),
        float2( 1.0, -1.0),
        float2(-1.0,  1.0),
        float2( 1.0,  1.0)
    };
    float2 localPos = quadVertices[vertexID];

    // Transform Gaussian center to clip space
    float4 clipPos = uniforms.viewProjectionMatrix * float4(gaussian.position, 1.0);
    float3 ndcPos = clipPos.xyz / clipPos.w;

    // Compute 3D covariance
    float3x3 cov3D = computeCovariance3D(gaussian);

    // Project to 2D screen space
    float2 cov2D[2];
    projectCovariance3Dto2D(cov3D, uniforms.viewMatrix, uniforms.projectionMatrix,
                            gaussian.position, uniforms.viewportSize, cov2D);

    // Compute eigenvalues/eigenvectors for billboard orientation
    // For a 2x2 symmetric matrix: [[a, b], [b, d]]
    float a = cov2D[0].x;
    float b = cov2D[0].y;
    float d = cov2D[1].y;

    // Eigenvalues
    float trace = a + d;
    float det = a * d - b * b;
    float lambda1 = trace * 0.5 + sqrt(max(0.0, trace * trace * 0.25 - det));
    float lambda2 = trace * 0.5 - sqrt(max(0.0, trace * trace * 0.25 - det));

    // Eigenvector for lambda1 (major axis)
    float2 eigenvec1 = normalize(float2(lambda1 - d, b));
    float2 eigenvec2 = float2(-eigenvec1.y, eigenvec1.x); // Perpendicular

    // Billboard size (3 sigma for 99.7% coverage)
    float radius1 = 3.0 * sqrt(lambda1) * uniforms.splatScale;
    float radius2 = 3.0 * sqrt(lambda2) * uniforms.splatScale;

    // Construct billboard in screen space
    float2 screenOffset = localPos.x * eigenvec1 * radius1 + localPos.y * eigenvec2 * radius2;

    // Convert to NDC offset
    float2 ndcOffset = screenOffset / (uniforms.viewportSize * 0.5);

    // Final position
    out.position = float4(ndcPos.xy + ndcOffset, ndcPos.z, 1.0);
    out.uv = localPos;

    // Evaluate spherical harmonics for color
    float3 viewDir = normalize(gaussian.position - uniforms.cameraPosition);
    out.color = evaluateSphericalHarmonics(gaussian, viewDir, uniforms.maxSHDegree);

    // Opacity
    out.opacity = gaussian.opacity * uniforms.opacityScale;

    // Pass 2D covariance to fragment shader
    out.cov2D[0] = cov2D[0];
    out.cov2D[1] = cov2D[1];

    return out;
}

// ============================================================================
// Fragment Shader
// ============================================================================

fragment float4 gaussianSplattingFragment(
    GaussianVertex in [[stage_in]]
) {
    // Evaluate 2D Gaussian function
    // G(x) = exp(-0.5 * x^T * Sigma^{-1} * x)

    // Invert 2D covariance matrix
    float a = in.cov2D[0].x;
    float b = in.cov2D[0].y;
    float d = in.cov2D[1].y;

    float det = a * d - b * b;
    if (det <= 1e-6) {
        discard_fragment(); // Degenerate Gaussian
    }

    float invDet = 1.0 / det;
    float2x2 invCov;
    invCov[0][0] = d * invDet;
    invCov[0][1] = -b * invDet;
    invCov[1][0] = -b * invDet;
    invCov[1][1] = a * invDet;

    // Evaluate Gaussian: exp(-0.5 * uv^T * invCov * uv)
    float2 uv = in.uv;
    float exponent = -0.5 * (invCov[0][0] * uv.x * uv.x +
                             2.0 * invCov[0][1] * uv.x * uv.y +
                             invCov[1][1] * uv.y * uv.y);

    float alpha = exp(exponent) * in.opacity;

    // Early discard for transparent fragments (performance optimization)
    if (alpha < 0.01) {
        discard_fragment();
    }

    // Output color with alpha
    // Note: The Gaussian splatting itself provides natural anti-aliasing
    return float4(in.color * alpha, alpha);
}

// ============================================================================
// Compute Shader: GPU Depth Sorting
// ============================================================================

struct SortData {
    uint index;
    float depth;
};

// Parallel bitonic sort for depth sorting
kernel void gaussianDepthSort(
    device SortData* sortData [[buffer(0)]],
    constant uint& count [[buffer(1)]],
    constant uint& stage [[buffer(2)]],
    constant uint& step [[buffer(3)]],
    uint tid [[thread_position_in_grid]]
) {
    // Bitonic sort implementation
    // stage: current stage of the sort (0 to log2(count)-1)
    // step: current step within the stage

    uint pairDistance = 1 << (stage - step);
    uint blockWidth = 2 * pairDistance;

    uint leftID = (tid % pairDistance) + (tid / pairDistance) * blockWidth;
    uint rightID = leftID + pairDistance;

    if (rightID < count) {
        SortData left = sortData[leftID];
        SortData right = sortData[rightID];

        // Determine sort direction (descending for back-to-front)
        uint direction = (tid / (1 << stage)) & 1;
        bool shouldSwap = (direction == 0) ? (left.depth < right.depth) : (left.depth > right.depth);

        if (shouldSwap) {
            sortData[leftID] = right;
            sortData[rightID] = left;
        }
    }
}

// Radix sort variant (more efficient for large datasets)
kernel void gaussianRadixSort(
    device SortData* sortData [[buffer(0)]],
    device SortData* tempData [[buffer(1)]],
    device uint* histograms [[buffer(2)]],
    constant uint& count [[buffer(3)]],
    constant uint& bit [[buffer(4)]],
    uint tid [[thread_position_in_grid]],
    uint lid [[thread_position_in_threadgroup]],
    uint gid [[threadgroup_position_in_grid]]
) {
    // Radix sort pass for a single bit
    // This is a simplified version; full implementation would use scan/prefix sum

    const uint radix = 2; // Binary radix
    const uint mask = 1 << bit;

    if (tid < count) {
        SortData data = sortData[tid];
        uint bitValue = (as_type<uint>(data.depth) & mask) ? 1 : 0;

        // Count occurrences (simplified - would use shared memory in practice)
        atomic_fetch_add_explicit((device atomic_uint*)&histograms[bitValue], 1, memory_order_relaxed);
    }

    // Full radix sort would continue with prefix sum and reordering
    // This is a placeholder showing the structure
}
