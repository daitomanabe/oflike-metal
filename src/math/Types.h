#pragma once

// oflike-metal Math Types - simd based
// Pure C++ header with Apple simd type aliases

#include <simd/simd.h>

namespace oflike {

// ============================================================================
// Vector types (simd aliases)
// ============================================================================

using float2 = simd_float2;
using float3 = simd_float3;
using float4 = simd_float4;

using double2 = simd_double2;
using double3 = simd_double3;
using double4 = simd_double4;

using int2 = simd_int2;
using int3 = simd_int3;
using int4 = simd_int4;

using uint2 = simd_uint2;
using uint3 = simd_uint3;
using uint4 = simd_uint4;

// ============================================================================
// Matrix types (simd aliases)
// ============================================================================

using float2x2 = simd_float2x2;
using float3x3 = simd_float3x3;
using float4x4 = simd_float4x4;

using double2x2 = simd_double2x2;
using double3x3 = simd_double3x3;
using double4x4 = simd_double4x4;

// ============================================================================
// Quaternion types (simd aliases)
// ============================================================================

using quatf = simd_quatf;
using quatd = simd_quatd;

} // namespace oflike
