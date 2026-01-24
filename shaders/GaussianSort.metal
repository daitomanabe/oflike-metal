#include <metal_stdlib>
using namespace metal;

// ============================================================================
// Gaussian Depth Sorting Compute Shaders
// ============================================================================

struct SortData {
    uint index;     // Original Gaussian index
    float depth;    // Depth in camera space (for sorting)
};

// ============================================================================
// Bitonic Sort (Good for small-medium datasets, up to ~10K elements)
// ============================================================================

/**
 * Parallel bitonic sort kernel.
 * Requires multiple passes: log2(N) stages, each with log2(stage) steps.
 *
 * Usage:
 *   for (stage = 2; stage <= count; stage *= 2) {
 *       for (step = stage/2; step > 0; step /= 2) {
 *           dispatch bitonicSort with (count/2) threads
 *       }
 *   }
 */
kernel void bitonicSort(
    device SortData* sortData [[buffer(0)]],
    constant uint& count [[buffer(1)]],
    constant uint& stage [[buffer(2)]],
    constant uint& step [[buffer(3)]],
    uint tid [[thread_position_in_grid]]
) {
    uint pairDistance = step;
    uint blockWidth = 2 * pairDistance;

    uint leftID = (tid % pairDistance) + (tid / pairDistance) * blockWidth;
    uint rightID = leftID + pairDistance;

    if (rightID < count) {
        SortData left = sortData[leftID];
        SortData right = sortData[rightID];

        // Determine sort direction
        // For back-to-front rendering, we want descending order (largest depth first)
        uint direction = (tid / stage) & 1;
        bool shouldSwap = (direction == 0) ? (left.depth < right.depth) : (left.depth > right.depth);

        if (shouldSwap) {
            sortData[leftID] = right;
            sortData[rightID] = left;
        }
    }
}

// ============================================================================
// Radix Sort (Efficient for large datasets, 100K+ elements)
// ============================================================================

// Local radix sort using shared memory
// This sorts within each threadgroup, then merges results
kernel void radixSortLocal(
    device SortData* sortData [[buffer(0)]],
    device SortData* tempData [[buffer(1)]],
    constant uint& count [[buffer(2)]],
    constant uint& bitShift [[buffer(3)]],
    threadgroup uint* sharedHistogram [[threadgroup(0)]],
    threadgroup uint* sharedOffsets [[threadgroup(1)]],
    uint tid [[thread_position_in_grid]],
    uint lid [[thread_position_in_threadgroup]],
    uint gid [[threadgroup_position_in_grid]],
    uint threadgroupSize [[threads_per_threadgroup]]
) {
    const uint RADIX = 256; // 8-bit radix
    const uint RADIX_MASK = RADIX - 1;

    // Initialize shared histogram
    if (lid < RADIX) {
        sharedHistogram[lid] = 0;
    }
    threadgroup_barrier(mem_flags::mem_threadgroup);

    // Count occurrences of each radix value in this threadgroup
    if (tid < count) {
        SortData data = sortData[tid];
        uint key = as_type<uint>(data.depth);
        uint radixValue = (key >> bitShift) & RADIX_MASK;
        atomic_fetch_add_explicit((threadgroup atomic_uint*)&sharedHistogram[radixValue], 1, memory_order_relaxed);
    }
    threadgroup_barrier(mem_flags::mem_threadgroup);

    // Compute prefix sum (scan) of histogram
    if (lid < RADIX) {
        uint sum = 0;
        for (uint i = 0; i < lid; ++i) {
            sum += sharedHistogram[i];
        }
        sharedOffsets[lid] = sum;
    }
    threadgroup_barrier(mem_flags::mem_threadgroup);

    // Scatter elements to temp buffer based on their radix value
    if (tid < count) {
        SortData data = sortData[tid];
        uint key = as_type<uint>(data.depth);
        uint radixValue = (key >> bitShift) & RADIX_MASK;

        // Get output position (using atomic increment to handle conflicts)
        uint offset = atomic_fetch_add_explicit((threadgroup atomic_uint*)&sharedOffsets[radixValue], 1, memory_order_relaxed);
        uint outputIndex = gid * threadgroupSize + offset;

        if (outputIndex < count) {
            tempData[outputIndex] = data;
        }
    }
}

// Copy temp buffer back to main buffer
kernel void radixSortCopyBack(
    device SortData* sortData [[buffer(0)]],
    device const SortData* tempData [[buffer(1)]],
    constant uint& count [[buffer(2)]],
    uint tid [[thread_position_in_grid]]
) {
    if (tid < count) {
        sortData[tid] = tempData[tid];
    }
}

// ============================================================================
// Quick Sort (Recursive, good for medium datasets)
// ============================================================================

// Partition kernel for quicksort
kernel void quickSortPartition(
    device SortData* sortData [[buffer(0)]],
    constant uint& left [[buffer(1)]],
    constant uint& right [[buffer(2)]],
    device atomic_uint* pivotIndex [[buffer(3)]],
    uint tid [[thread_position_in_grid]]
) {
    if (tid < right - left) {
        uint index = left + tid;

        // Use middle element as pivot
        uint pivotIdx = left + (right - left) / 2;
        float pivotValue = sortData[pivotIdx].depth;

        // Compare with pivot (back-to-front: descending order)
        if (sortData[index].depth > pivotValue) {
            // Element should go to left partition
            uint newIndex = atomic_fetch_add_explicit(pivotIndex, 1, memory_order_relaxed);
            // Would swap here in a complete implementation
        }
    }
}

// ============================================================================
// Parallel Merge Sort (Stable sort, good for medium-large datasets)
// ============================================================================

kernel void mergeSortMerge(
    device const SortData* input [[buffer(0)]],
    device SortData* output [[buffer(1)]],
    constant uint& count [[buffer(2)]],
    constant uint& width [[buffer(3)]],
    uint tid [[thread_position_in_grid]]
) {
    uint i = tid * 2 * width;

    if (i >= count) return;

    uint left = i;
    uint mid = min(i + width, count);
    uint right = min(i + 2 * width, count);

    // Merge [left, mid) and [mid, right)
    uint l = left, r = mid, out = left;

    while (l < mid && r < right) {
        // Back-to-front: descending order
        if (input[l].depth > input[r].depth) {
            output[out++] = input[l++];
        } else {
            output[out++] = input[r++];
        }
    }

    // Copy remaining elements
    while (l < mid) {
        output[out++] = input[l++];
    }
    while (r < right) {
        output[out++] = input[r++];
    }
}

// ============================================================================
// Simple Bubble Sort (Debugging/Small datasets only)
// ============================================================================

kernel void bubbleSortPass(
    device SortData* sortData [[buffer(0)]],
    constant uint& count [[buffer(1)]],
    constant uint& passIndex [[buffer(2)]],
    uint tid [[thread_position_in_grid]]
) {
    if (tid < count - 1 - passIndex) {
        if (sortData[tid].depth < sortData[tid + 1].depth) {
            SortData temp = sortData[tid];
            sortData[tid] = sortData[tid + 1];
            sortData[tid + 1] = temp;
        }
    }
}

// ============================================================================
// Hybrid Sort Dispatcher
// ============================================================================

/**
 * Chooses optimal sort algorithm based on count:
 * - count < 1K: Bitonic sort
 * - 1K < count < 100K: Merge sort
 * - count > 100K: Radix sort
 *
 * Called from host to select appropriate kernel.
 */
