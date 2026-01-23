#import "MetalBuffer.h"
#import <Metal/Metal.h>
#import <Foundation/Foundation.h>
#import <vector>

namespace render {
namespace metal {

// ============================================================================
// MetalBuffer Implementation
// ============================================================================

struct MetalBuffer::Impl {
    id<MTLDevice> device = nil;
    uint32_t maxFramesInFlight = 3;

    // Per-frame buffers
    std::vector<id<MTLBuffer>> buffers;
    std::vector<size_t> sizes;

    Impl(id<MTLDevice> dev, size_t initialSize, uint32_t maxFrames)
        : device(dev), maxFramesInFlight(maxFrames) {
        @autoreleasepool {
            buffers.resize(maxFramesInFlight, nil);
            sizes.resize(maxFramesInFlight, 0);

            // Allocate initial buffers
            for (uint32_t i = 0; i < maxFramesInFlight; i++) {
                if (initialSize > 0) {
                    buffers[i] = [device newBufferWithLength:initialSize
                                                     options:MTLResourceStorageModeShared];
                    if (buffers[i]) {
                        sizes[i] = initialSize;
                        buffers[i].label = [NSString stringWithFormat:@"MetalBuffer_%u", i];
                    }
                }
            }
        }
    }

    ~Impl() {
        @autoreleasepool {
            for (auto& buffer : buffers) {
                buffer = nil;
            }
            buffers.clear();
            sizes.clear();
        }
    }
};

MetalBuffer::MetalBuffer(void* device, size_t initialSize, uint32_t maxFramesInFlight)
    : impl_(std::make_unique<Impl>((__bridge id<MTLDevice>)device, initialSize, maxFramesInFlight)) {
}

MetalBuffer::~MetalBuffer() = default;

void* MetalBuffer::allocate(size_t size, uint32_t frameIndex) {
    if (!impl_ || frameIndex >= impl_->maxFramesInFlight) {
        return nullptr;
    }

    @autoreleasepool {
        // Check if we need to reallocate
        if (size > impl_->sizes[frameIndex]) {
            // Grow buffer with some overhead (1.5x growth factor)
            size_t newSize = size + (size / 2);

            id<MTLBuffer> newBuffer = [impl_->device newBufferWithLength:newSize
                                                                 options:MTLResourceStorageModeShared];
            if (!newBuffer) {
                NSLog(@"MetalBuffer: Failed to allocate buffer of size %zu", newSize);
                return nullptr;
            }

            newBuffer.label = [NSString stringWithFormat:@"MetalBuffer_%u", frameIndex];

            // Replace old buffer
            impl_->buffers[frameIndex] = newBuffer;
            impl_->sizes[frameIndex] = newSize;
        }

        return [impl_->buffers[frameIndex] contents];
    }
}

void* MetalBuffer::getBuffer(uint32_t frameIndex) const {
    if (!impl_ || frameIndex >= impl_->maxFramesInFlight) {
        return nullptr;
    }
    return (__bridge void*)impl_->buffers[frameIndex];
}

size_t MetalBuffer::getSize(uint32_t frameIndex) const {
    if (!impl_ || frameIndex >= impl_->maxFramesInFlight) {
        return 0;
    }
    return impl_->sizes[frameIndex];
}

void* MetalBuffer::getContents(uint32_t frameIndex) const {
    if (!impl_ || frameIndex >= impl_->maxFramesInFlight) {
        return nullptr;
    }

    @autoreleasepool {
        if (impl_->buffers[frameIndex]) {
            return [impl_->buffers[frameIndex] contents];
        }
        return nullptr;
    }
}

void MetalBuffer::reset(uint32_t frameIndex) {
    // Reset is a no-op for now since we reuse buffers
    // Could be used for internal tracking if needed
}

void MetalBuffer::resetAll() {
    // Reset all frames
    for (uint32_t i = 0; i < impl_->maxFramesInFlight; i++) {
        reset(i);
    }
}

uint32_t MetalBuffer::getMaxFramesInFlight() const {
    return impl_ ? impl_->maxFramesInFlight : 0;
}

// ============================================================================
// MetalBufferPool Implementation
// ============================================================================

struct MetalBufferPool::Impl {
    id<MTLDevice> device = nil;
    uint32_t maxFramesInFlight = 3;

    // Pool of all buffers (owns them)
    std::vector<std::unique_ptr<MetalBuffer>> allBuffers;

    // Tracking acquired/available state (does not own)
    std::vector<MetalBuffer*> availableBuffers;
    std::vector<MetalBuffer*> acquiredBuffers;

    Impl(id<MTLDevice> dev, uint32_t maxFrames)
        : device(dev), maxFramesInFlight(maxFrames) {
    }

    ~Impl() {
        @autoreleasepool {
            allBuffers.clear();
            availableBuffers.clear();
            acquiredBuffers.clear();
        }
    }
};

MetalBufferPool::MetalBufferPool(void* device, uint32_t maxFramesInFlight)
    : impl_(std::make_unique<Impl>((__bridge id<MTLDevice>)device, maxFramesInFlight)) {
}

MetalBufferPool::~MetalBufferPool() = default;

MetalBuffer* MetalBufferPool::acquire(size_t size, uint32_t frameIndex) {
    if (!impl_ || frameIndex >= impl_->maxFramesInFlight) {
        return nullptr;
    }

    @autoreleasepool {
        // Try to find a suitable buffer in the available pool
        for (auto it = impl_->availableBuffers.begin(); it != impl_->availableBuffers.end(); ++it) {
            MetalBuffer* buffer = *it;
            if (buffer->getSize(frameIndex) >= size) {
                // Found a suitable buffer - move to acquired
                impl_->availableBuffers.erase(it);
                impl_->acquiredBuffers.push_back(buffer);
                return buffer;
            }
        }

        // No suitable buffer found, create a new one
        auto newBuffer = std::make_unique<MetalBuffer>(
            (__bridge void*)impl_->device, size, impl_->maxFramesInFlight);

        MetalBuffer* bufferPtr = newBuffer.get();
        impl_->allBuffers.push_back(std::move(newBuffer));
        impl_->acquiredBuffers.push_back(bufferPtr);

        return bufferPtr;
    }
}

void MetalBufferPool::release(MetalBuffer* buffer) {
    if (!impl_ || !buffer) {
        return;
    }

    @autoreleasepool {
        // Find buffer in acquired list
        auto it = std::find(impl_->acquiredBuffers.begin(),
                           impl_->acquiredBuffers.end(),
                           buffer);

        if (it != impl_->acquiredBuffers.end()) {
            // Move from acquired to available
            impl_->acquiredBuffers.erase(it);
            impl_->availableBuffers.push_back(buffer);
        }
    }
}

void MetalBufferPool::resetFrame(uint32_t frameIndex) {
    if (!impl_ || frameIndex >= impl_->maxFramesInFlight) {
        return;
    }

    @autoreleasepool {
        // Reset all buffers in the pool
        for (auto& buffer : impl_->allBuffers) {
            buffer->reset(frameIndex);
        }
    }
}

void MetalBufferPool::clear() {
    if (!impl_) {
        return;
    }

    @autoreleasepool {
        impl_->allBuffers.clear();
        impl_->availableBuffers.clear();
        impl_->acquiredBuffers.clear();
    }
}

size_t MetalBufferPool::getPoolSize() const {
    if (!impl_) {
        return 0;
    }
    return impl_->allBuffers.size();
}

size_t MetalBufferPool::getAcquiredCount() const {
    if (!impl_) {
        return 0;
    }
    return impl_->acquiredBuffers.size();
}

size_t MetalBufferPool::getAvailableCount() const {
    if (!impl_) {
        return 0;
    }
    return impl_->availableBuffers.size();
}

} // namespace metal
} // namespace render
