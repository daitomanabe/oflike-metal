#pragma once

#include <cstdint>
#include <memory>

namespace render {
namespace metal {

// ============================================================================
// MetalBuffer - Dynamic Metal Buffer Management
// ============================================================================

/**
 * Manages dynamic Metal buffers with automatic growth and pooling.
 *
 * Features:
 * - Dynamic buffer allocation with automatic growth
 * - Triple buffering support for concurrent frame rendering
 * - Buffer pooling to reduce allocation overhead
 * - Shared storage mode for CPU-GPU shared memory
 *
 * Implementation:
 * - Uses pImpl pattern to hide Objective-C++ Metal code
 * - Thread-safety: Main thread only
 * - Automatic cleanup on destruction
 *
 * Usage:
 *   MetalBuffer buffer(device, initialSize);
 *   void* data = buffer.allocate(size, frameIndex);
 *   std::memcpy(data, myData, size);
 *   id<MTLBuffer> mtlBuffer = buffer.getBuffer(frameIndex);
 */
class MetalBuffer {
public:
    /**
     * Constructor.
     * @param device Metal device (id<MTLDevice>)
     * @param initialSize Initial buffer size in bytes
     * @param maxFramesInFlight Number of frames in flight (default: 3)
     */
    MetalBuffer(void* device, size_t initialSize, uint32_t maxFramesInFlight = 3);

    ~MetalBuffer();

    // Disable copy and move
    MetalBuffer(const MetalBuffer&) = delete;
    MetalBuffer& operator=(const MetalBuffer&) = delete;
    MetalBuffer(MetalBuffer&&) = delete;
    MetalBuffer& operator=(MetalBuffer&&) = delete;

    // ========================================================================
    // Buffer Allocation
    // ========================================================================

    /**
     * Allocate space in the buffer for the current frame.
     * If the requested size exceeds current buffer capacity, reallocates.
     * @param size Size in bytes to allocate
     * @param frameIndex Frame index (0 to maxFramesInFlight-1)
     * @return Pointer to buffer contents, or nullptr on failure
     */
    void* allocate(size_t size, uint32_t frameIndex);

    /**
     * Get the Metal buffer handle for a specific frame.
     * @param frameIndex Frame index (0 to maxFramesInFlight-1)
     * @return MTLBuffer handle (id<MTLBuffer>), or nullptr if not allocated
     */
    void* getBuffer(uint32_t frameIndex) const;

    /**
     * Get the current allocated size for a specific frame.
     * @param frameIndex Frame index (0 to maxFramesInFlight-1)
     * @return Allocated size in bytes
     */
    size_t getSize(uint32_t frameIndex) const;

    /**
     * Get buffer contents pointer (for direct memory access).
     * @param frameIndex Frame index (0 to maxFramesInFlight-1)
     * @return Pointer to buffer contents, or nullptr if not allocated
     */
    void* getContents(uint32_t frameIndex) const;

    // ========================================================================
    // Buffer Management
    // ========================================================================

    /**
     * Reset allocation tracking for a specific frame.
     * Does not deallocate the buffer, just resets internal tracking.
     * @param frameIndex Frame index (0 to maxFramesInFlight-1)
     */
    void reset(uint32_t frameIndex);

    /**
     * Reset all frames.
     */
    void resetAll();

    /**
     * Get maximum frames in flight.
     */
    uint32_t getMaxFramesInFlight() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// ============================================================================
// MetalBufferPool - Buffer Pool Management
// ============================================================================

/**
 * Manages a pool of MetalBuffer objects for efficient reuse.
 *
 * Features:
 * - Reduces allocation overhead by reusing buffers
 * - Automatic buffer cleanup when unused
 * - Size-based buffer selection
 *
 * Implementation:
 * - Uses pImpl pattern
 * - Thread-safety: Main thread only
 *
 * Usage:
 *   MetalBufferPool pool(device);
 *   MetalBuffer* buffer = pool.acquire(size, frameIndex);
 *   // Use buffer...
 *   pool.release(buffer);
 */
class MetalBufferPool {
public:
    /**
     * Constructor.
     * @param device Metal device (id<MTLDevice>)
     * @param maxFramesInFlight Number of frames in flight (default: 3)
     */
    MetalBufferPool(void* device, uint32_t maxFramesInFlight = 3);

    ~MetalBufferPool();

    // Disable copy and move
    MetalBufferPool(const MetalBufferPool&) = delete;
    MetalBufferPool& operator=(const MetalBufferPool&) = delete;
    MetalBufferPool(MetalBufferPool&&) = delete;
    MetalBufferPool& operator=(MetalBufferPool&&) = delete;

    // ========================================================================
    // Pool Management
    // ========================================================================

    /**
     * Acquire a buffer from the pool.
     * Returns an existing buffer if available, or creates a new one.
     * @param size Minimum size in bytes
     * @param frameIndex Frame index (0 to maxFramesInFlight-1)
     * @return MetalBuffer pointer, or nullptr on failure
     */
    MetalBuffer* acquire(size_t size, uint32_t frameIndex);

    /**
     * Release a buffer back to the pool.
     * @param buffer Buffer to release
     */
    void release(MetalBuffer* buffer);

    /**
     * Reset all buffers in the pool for a specific frame.
     * @param frameIndex Frame index (0 to maxFramesInFlight-1)
     */
    void resetFrame(uint32_t frameIndex);

    /**
     * Clear the pool and deallocate all buffers.
     */
    void clear();

    /**
     * Get number of buffers currently in the pool.
     */
    size_t getPoolSize() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace metal
} // namespace render
