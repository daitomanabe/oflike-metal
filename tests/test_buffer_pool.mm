// ============================================================================
// test_buffer_pool.cpp
// Tests for MetalBufferPool optimization
// ============================================================================

#include "render/metal/MetalBuffer.h"
#include <iostream>
#include <cassert>
#include <cstring>

// Mock Metal device for testing (uses Metal framework on macOS)
#ifdef __APPLE__
#import <Metal/Metal.h>
#endif

using namespace render::metal;

// Test utilities
void log_test(const char* test_name, bool passed) {
    std::cout << "[" << (passed ? "PASS" : "FAIL") << "] " << test_name << std::endl;
    if (!passed) {
        exit(1);
    }
}

#ifdef __APPLE__

// ============================================================================
// Test 1: Pool creation and cleanup
// ============================================================================
void test_pool_creation() {
    @autoreleasepool {
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        assert(device != nil && "Failed to create Metal device");

        MetalBufferPool pool((__bridge void*)device, 3);

        bool passed = (pool.getPoolSize() == 0 &&
                      pool.getAcquiredCount() == 0 &&
                      pool.getAvailableCount() == 0);

        log_test("Pool creation", passed);
    }
}

// ============================================================================
// Test 2: Acquire and release
// ============================================================================
void test_acquire_release() {
    @autoreleasepool {
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        assert(device != nil && "Failed to create Metal device");

        MetalBufferPool pool((__bridge void*)device, 3);

        // Acquire a buffer
        MetalBuffer* buffer = pool.acquire(1024, 0);
        bool acquired = (buffer != nullptr &&
                        pool.getPoolSize() == 1 &&
                        pool.getAcquiredCount() == 1 &&
                        pool.getAvailableCount() == 0);

        // Release the buffer
        pool.release(buffer);
        bool released = (pool.getPoolSize() == 1 &&
                        pool.getAcquiredCount() == 0 &&
                        pool.getAvailableCount() == 1);

        log_test("Acquire and release", acquired && released);
    }
}

// ============================================================================
// Test 3: Buffer reuse
// ============================================================================
void test_buffer_reuse() {
    @autoreleasepool {
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        assert(device != nil && "Failed to create Metal device");

        MetalBufferPool pool((__bridge void*)device, 3);

        // Acquire and release a buffer
        MetalBuffer* buffer1 = pool.acquire(1024, 0);
        pool.release(buffer1);

        // Acquire again - should reuse the same buffer
        MetalBuffer* buffer2 = pool.acquire(1024, 0);

        bool reused = (buffer1 == buffer2 &&
                      pool.getPoolSize() == 1 &&
                      pool.getAcquiredCount() == 1);

        pool.release(buffer2);

        log_test("Buffer reuse", reused);
    }
}

// ============================================================================
// Test 4: Multiple buffers
// ============================================================================
void test_multiple_buffers() {
    @autoreleasepool {
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        assert(device != nil && "Failed to create Metal device");

        MetalBufferPool pool((__bridge void*)device, 3);

        // Acquire multiple buffers
        MetalBuffer* b1 = pool.acquire(1024, 0);
        MetalBuffer* b2 = pool.acquire(2048, 0);
        MetalBuffer* b3 = pool.acquire(4096, 0);

        bool acquired = (pool.getPoolSize() == 3 &&
                        pool.getAcquiredCount() == 3 &&
                        pool.getAvailableCount() == 0);

        // Release all
        pool.release(b1);
        pool.release(b2);
        pool.release(b3);

        bool released = (pool.getPoolSize() == 3 &&
                        pool.getAcquiredCount() == 0 &&
                        pool.getAvailableCount() == 3);

        log_test("Multiple buffers", acquired && released);
    }
}

// ============================================================================
// Test 5: Size-based selection
// ============================================================================
void test_size_selection() {
    @autoreleasepool {
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        assert(device != nil && "Failed to create Metal device");

        MetalBufferPool pool((__bridge void*)device, 3);

        // Create buffers of different sizes
        MetalBuffer* small = pool.acquire(1024, 0);
        MetalBuffer* medium = pool.acquire(2048, 0);
        MetalBuffer* large = pool.acquire(4096, 0);

        pool.release(small);
        pool.release(medium);
        pool.release(large);

        // Request small size - should get the small buffer (first fit)
        MetalBuffer* reused = pool.acquire(512, 0);
        bool correct_size = (reused == small);

        pool.release(reused);

        log_test("Size-based selection", correct_size);
    }
}

// ============================================================================
// Test 6: Clear pool
// ============================================================================
void test_clear() {
    @autoreleasepool {
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        assert(device != nil && "Failed to create Metal device");

        MetalBufferPool pool((__bridge void*)device, 3);

        // Acquire some buffers
        MetalBuffer* b1 = pool.acquire(1024, 0);
        MetalBuffer* b2 = pool.acquire(2048, 0);
        pool.release(b1);
        pool.release(b2);

        // Clear the pool
        pool.clear();

        bool cleared = (pool.getPoolSize() == 0 &&
                       pool.getAcquiredCount() == 0 &&
                       pool.getAvailableCount() == 0);

        log_test("Clear pool", cleared);
    }
}

// ============================================================================
// Test 7: Frame index handling
// ============================================================================
void test_frame_indices() {
    @autoreleasepool {
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        assert(device != nil && "Failed to create Metal device");

        MetalBufferPool pool((__bridge void*)device, 3);

        // Acquire buffers for different frames
        MetalBuffer* b0 = pool.acquire(1024, 0);
        MetalBuffer* b1 = pool.acquire(1024, 1);
        MetalBuffer* b2 = pool.acquire(1024, 2);

        // Should create separate buffers for each frame
        bool all_valid = (b0 != nullptr && b1 != nullptr && b2 != nullptr);

        // All buffers should be allocated
        bool all_allocated = (b0->getSize(0) >= 1024 &&
                             b1->getSize(1) >= 1024 &&
                             b2->getSize(2) >= 1024);

        pool.release(b0);
        pool.release(b1);
        pool.release(b2);

        log_test("Frame index handling", all_valid && all_allocated);
    }
}

// ============================================================================
// Test 8: Reset frame
// ============================================================================
void test_reset_frame() {
    @autoreleasepool {
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        assert(device != nil && "Failed to create Metal device");

        MetalBufferPool pool((__bridge void*)device, 3);

        // Acquire and use buffer
        MetalBuffer* buffer = pool.acquire(1024, 0);
        void* contents = buffer->getContents(0);
        std::memset(contents, 0xFF, 1024);

        // Reset frame
        pool.resetFrame(0);

        // Buffer should still exist
        bool buffer_exists = (pool.getPoolSize() == 1);

        pool.release(buffer);

        log_test("Reset frame", buffer_exists);
    }
}

#endif // __APPLE__

// ============================================================================
// Main
// ============================================================================
int main() {
    std::cout << "=== MetalBufferPool Tests ===" << std::endl;

#ifdef __APPLE__
    test_pool_creation();
    test_acquire_release();
    test_buffer_reuse();
    test_multiple_buffers();
    test_size_selection();
    test_clear();
    test_frame_indices();
    test_reset_frame();

    std::cout << "\n✅ All buffer pool tests passed!" << std::endl;
#else
    std::cout << "⚠️  Tests skipped (requires macOS/Metal)" << std::endl;
#endif

    return 0;
}
