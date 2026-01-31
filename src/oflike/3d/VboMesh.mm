// VboMesh.mm - Modern Metal GPU Mesh Implementation
// Leverages Metal 3 features for high-performance rendering

#import <Metal/Metal.h>
#import <simd/simd.h>
#import <dispatch/dispatch.h>
#include "VboMesh.h"
#include "../../core/Context.h"
#include "../../render/metal/MetalRenderer.h"
#include "../../render/DrawList.h"
#include "../graphics/ofGraphics.h"  // For ofGetCurrentMatrix, ofGetColor, ofGetFill
#include "../math/ofMatrix4x4.h"     // Full definition for ofMatrix4x4

namespace oflike {

// ============================================================================
// Constants
// ============================================================================

static constexpr uint32_t kMaxFramesInFlight = 3;
static constexpr size_t kMinBufferSize = 4096;  // Minimum buffer allocation

// ============================================================================
// Interleaved Vertex Structure
// ============================================================================

struct InterleavedVertex {
    simd_float3 position;   // 12 bytes + 4 padding = 16
    simd_float3 normal;     // 12 bytes + 4 padding = 16
    simd_float2 texCoord;   // 8 bytes
    simd_float4 color;      // 16 bytes
};                          // Total: 56 bytes (with alignment may be 64)

// Note: Size may be larger due to alignment requirements

// ============================================================================
// VboMesh Implementation
// ============================================================================

struct VboMesh::Impl {
    // Metal device reference
    id<MTLDevice> device = nil;

    // Buffer configuration
    VboStorageMode storageMode = VboStorageMode::Auto;
    VboUsageHint usageHint = VboUsageHint::Static;
    VboAttributeLayout layout = VboAttributeLayout::Interleaved;
    ofPrimitiveMode primitiveMode = OF_PRIMITIVE_TRIANGLES;

    // Vertex data counts
    size_t numVertices = 0;
    size_t numIndices = 0;
    size_t maxVertices = 0;
    size_t maxIndices = 0;

    // Attribute flags
    bool hasNormals_ = false;
    bool hasTexCoords_ = false;
    bool hasColors_ = false;

    // Triple-buffered vertex buffers (for dynamic meshes)
    id<MTLBuffer> vertexBuffers[kMaxFramesInFlight] = {nil, nil, nil};
    id<MTLBuffer> indexBuffers[kMaxFramesInFlight] = {nil, nil, nil};

    // Separate attribute buffers (if using Separate layout)
    id<MTLBuffer> positionBuffers[kMaxFramesInFlight] = {nil, nil, nil};
    id<MTLBuffer> normalBuffers[kMaxFramesInFlight] = {nil, nil, nil};
    id<MTLBuffer> texCoordBuffers[kMaxFramesInFlight] = {nil, nil, nil};
    id<MTLBuffer> colorBuffers[kMaxFramesInFlight] = {nil, nil, nil};

    // Instance data buffers
    id<MTLBuffer> instanceBuffers[kMaxFramesInFlight] = {nil, nil, nil};
    size_t numInstances = 0;
    size_t maxInstances = 0;

    // Indirect draw argument buffer
    id<MTLBuffer> indirectArgumentBuffer = nil;

    // Dirty flags for lazy upload
    bool dirty = false;
    uint32_t dirtyFrameMask = 0;  // Bitmask of frames that need update

    // CPU-side data (for managed/shared modes)
    std::vector<InterleavedVertex> cpuVertices;
    std::vector<uint32_t> cpuIndices;
    std::vector<VboInstanceData> cpuInstances;

    // Frame synchronization
    dispatch_semaphore_t frameSemaphore = nullptr;
    uint32_t currentFrameIndex = 0;

    // ========================================================================
    // Initialization
    // ========================================================================

    Impl() {
        // Device will be acquired lazily when needed
        device = nil;

        // Create semaphore for triple buffering
        frameSemaphore = dispatch_semaphore_create(kMaxFramesInFlight);
    }

    // Lazy device acquisition
    bool ensureDevice() {
        if (device) return true;

        auto& ctx = Context::instance();
        if (!ctx.isInitialized()) {
            return false;
        }

        void* metalDevice = ctx.getMetalDevice();
        if (!metalDevice) {
            return false;
        }

        device = (__bridge id<MTLDevice>)metalDevice;
        return device != nil;
    }

    ~Impl() {
        clear();
    }

    void clear() {
        for (uint32_t i = 0; i < kMaxFramesInFlight; i++) {
            vertexBuffers[i] = nil;
            indexBuffers[i] = nil;
            positionBuffers[i] = nil;
            normalBuffers[i] = nil;
            texCoordBuffers[i] = nil;
            colorBuffers[i] = nil;
            instanceBuffers[i] = nil;
        }
        indirectArgumentBuffer = nil;

        cpuVertices.clear();
        cpuIndices.clear();
        cpuInstances.clear();

        numVertices = 0;
        numIndices = 0;
        maxVertices = 0;
        maxIndices = 0;
        numInstances = 0;
        maxInstances = 0;
        dirty = false;
        dirtyFrameMask = 0;
    }

    // ========================================================================
    // Storage Mode Selection
    // ========================================================================

    MTLResourceOptions getResourceOptions() const {
        VboStorageMode mode = storageMode;

        // Auto-select based on usage hint
        if (mode == VboStorageMode::Auto) {
            switch (usageHint) {
                case VboUsageHint::Static:
                    mode = VboStorageMode::Private;
                    break;
                case VboUsageHint::Dynamic:
                    mode = VboStorageMode::Managed;
                    break;
                case VboUsageHint::Stream:
                    mode = VboStorageMode::Shared;
                    break;
            }
        }

        switch (mode) {
            case VboStorageMode::Private:
                return MTLResourceStorageModePrivate;
            case VboStorageMode::Shared:
                return MTLResourceStorageModeShared | MTLResourceCPUCacheModeWriteCombined;
            case VboStorageMode::Managed:
                return MTLResourceStorageModeManaged;
            default:
                return MTLResourceStorageModeShared;
        }
    }

    bool isPrivateStorage() const {
        VboStorageMode mode = storageMode;
        if (mode == VboStorageMode::Auto) {
            mode = (usageHint == VboUsageHint::Static) ? VboStorageMode::Private : VboStorageMode::Shared;
        }
        return mode == VboStorageMode::Private;
    }

    // ========================================================================
    // Buffer Creation
    // ========================================================================

    bool createBuffers(size_t vertexCount, size_t indexCount) {
        if (!ensureDevice()) return false;

        maxVertices = vertexCount;
        maxIndices = indexCount;

        MTLResourceOptions options = getResourceOptions();
        bool usePrivate = isPrivateStorage();

        // For static meshes, we only need one buffer (no triple buffering)
        uint32_t numBuffers = (usageHint == VboUsageHint::Static) ? 1 : kMaxFramesInFlight;

        if (layout == VboAttributeLayout::Interleaved) {
            // Single interleaved buffer per frame
            size_t vertexBufferSize = std::max(maxVertices * sizeof(InterleavedVertex), kMinBufferSize);

            for (uint32_t i = 0; i < numBuffers; i++) {
                if (usePrivate) {
                    // Private storage: need staging buffer for upload
                    vertexBuffers[i] = [device newBufferWithLength:vertexBufferSize
                                                           options:MTLResourceStorageModePrivate];
                } else {
                    vertexBuffers[i] = [device newBufferWithLength:vertexBufferSize
                                                           options:options];
                }
                if (!vertexBuffers[i]) return false;
                vertexBuffers[i].label = [NSString stringWithFormat:@"VboMesh Vertex %u", i];
            }
        } else {
            // Separate buffers for each attribute
            size_t posSize = std::max(maxVertices * sizeof(simd_float3), kMinBufferSize);
            size_t normSize = std::max(maxVertices * sizeof(simd_float3), kMinBufferSize);
            size_t texSize = std::max(maxVertices * sizeof(simd_float2), kMinBufferSize);
            size_t colSize = std::max(maxVertices * sizeof(simd_float4), kMinBufferSize);

            for (uint32_t i = 0; i < numBuffers; i++) {
                positionBuffers[i] = [device newBufferWithLength:posSize options:options];
                normalBuffers[i] = [device newBufferWithLength:normSize options:options];
                texCoordBuffers[i] = [device newBufferWithLength:texSize options:options];
                colorBuffers[i] = [device newBufferWithLength:colSize options:options];

                if (!positionBuffers[i]) return false;

                positionBuffers[i].label = [NSString stringWithFormat:@"VboMesh Position %u", i];
                normalBuffers[i].label = [NSString stringWithFormat:@"VboMesh Normal %u", i];
                texCoordBuffers[i].label = [NSString stringWithFormat:@"VboMesh TexCoord %u", i];
                colorBuffers[i].label = [NSString stringWithFormat:@"VboMesh Color %u", i];
            }
        }

        // Index buffer
        if (maxIndices > 0) {
            size_t indexBufferSize = std::max(maxIndices * sizeof(uint32_t), kMinBufferSize);

            for (uint32_t i = 0; i < numBuffers; i++) {
                if (usePrivate) {
                    indexBuffers[i] = [device newBufferWithLength:indexBufferSize
                                                          options:MTLResourceStorageModePrivate];
                } else {
                    indexBuffers[i] = [device newBufferWithLength:indexBufferSize
                                                          options:options];
                }
                if (!indexBuffers[i]) return false;
                indexBuffers[i].label = [NSString stringWithFormat:@"VboMesh Index %u", i];
            }
        }

        return true;
    }

    // ========================================================================
    // Data Upload
    // ========================================================================

    void uploadToGPU(id<MTLCommandQueue> commandQueue, uint32_t frameIndex) {
        if (!dirty || cpuVertices.empty()) return;
        if (!ensureDevice()) return;

        bool usePrivate = isPrivateStorage();

        // For private storage without a command queue, skip upload
        // Data will be uploaded when draw() is called and we have access to a command queue
        if (usePrivate && !commandQueue) {
            return;
        }

        if (usePrivate && commandQueue) {
            // Private storage: use blit command to upload
            @autoreleasepool {
                id<MTLCommandBuffer> blitCommandBuffer = [commandQueue commandBuffer];
                id<MTLBlitCommandEncoder> blitEncoder = [blitCommandBuffer blitCommandEncoder];

                // Create staging buffers
                size_t vertexSize = cpuVertices.size() * sizeof(InterleavedVertex);
                id<MTLBuffer> stagingVertex = [device newBufferWithBytes:cpuVertices.data()
                                                                  length:vertexSize
                                                                 options:MTLResourceStorageModeShared];

                [blitEncoder copyFromBuffer:stagingVertex
                               sourceOffset:0
                                   toBuffer:vertexBuffers[frameIndex]
                          destinationOffset:0
                                       size:vertexSize];

                if (!cpuIndices.empty() && indexBuffers[frameIndex]) {
                    size_t indexSize = cpuIndices.size() * sizeof(uint32_t);
                    id<MTLBuffer> stagingIndex = [device newBufferWithBytes:cpuIndices.data()
                                                                     length:indexSize
                                                                    options:MTLResourceStorageModeShared];

                    [blitEncoder copyFromBuffer:stagingIndex
                                   sourceOffset:0
                                       toBuffer:indexBuffers[frameIndex]
                              destinationOffset:0
                                           size:indexSize];
                }

                [blitEncoder endEncoding];
                [blitCommandBuffer commit];
                [blitCommandBuffer waitUntilCompleted];
            }
        } else {
            // Shared/Managed storage: direct memory copy
            if (layout == VboAttributeLayout::Interleaved) {
                if (!vertexBuffers[frameIndex]) return;
                memcpy([vertexBuffers[frameIndex] contents],
                       cpuVertices.data(),
                       cpuVertices.size() * sizeof(InterleavedVertex));

                // For managed storage, sync to GPU
                if (storageMode == VboStorageMode::Managed ||
                    (storageMode == VboStorageMode::Auto && usageHint == VboUsageHint::Dynamic)) {
                    [vertexBuffers[frameIndex] didModifyRange:NSMakeRange(0, cpuVertices.size() * sizeof(InterleavedVertex))];
                }
            }

            if (!cpuIndices.empty() && indexBuffers[frameIndex]) {
                void* indexContents = [indexBuffers[frameIndex] contents];
                if (indexContents) {
                    memcpy(indexContents,
                           cpuIndices.data(),
                           cpuIndices.size() * sizeof(uint32_t));

                    if (storageMode == VboStorageMode::Managed ||
                        (storageMode == VboStorageMode::Auto && usageHint == VboUsageHint::Dynamic)) {
                        [indexBuffers[frameIndex] didModifyRange:NSMakeRange(0, cpuIndices.size() * sizeof(uint32_t))];
                    }
                }
            }
        }

        dirtyFrameMask &= ~(1 << frameIndex);
        if (dirtyFrameMask == 0) {
            dirty = false;
        }
    }

    // ========================================================================
    // Instance Buffer Management
    // ========================================================================

    bool createInstanceBuffers(size_t count) {
        if (!ensureDevice() || count == 0) return false;

        maxInstances = count;
        size_t bufferSize = std::max(count * sizeof(VboInstanceData), kMinBufferSize);

        MTLResourceOptions options = MTLResourceStorageModeShared | MTLResourceCPUCacheModeWriteCombined;

        for (uint32_t i = 0; i < kMaxFramesInFlight; i++) {
            instanceBuffers[i] = [device newBufferWithLength:bufferSize options:options];
            if (!instanceBuffers[i]) return false;
            instanceBuffers[i].label = [NSString stringWithFormat:@"VboMesh Instance %u", i];
        }

        return true;
    }

    void uploadInstances(uint32_t frameIndex) {
        if (cpuInstances.empty() || !instanceBuffers[frameIndex]) return;

        memcpy([instanceBuffers[frameIndex] contents],
               cpuInstances.data(),
               cpuInstances.size() * sizeof(VboInstanceData));
    }

    // ========================================================================
    // Indirect Argument Buffer
    // ========================================================================

    bool createIndirectArgumentBuffer() {
        if (!ensureDevice()) return false;

        indirectArgumentBuffer = [device newBufferWithLength:sizeof(VboIndirectArguments)
                                                     options:MTLResourceStorageModeShared];
        if (!indirectArgumentBuffer) return false;

        indirectArgumentBuffer.label = @"VboMesh Indirect Arguments";
        return true;
    }

    void setIndirectArgs(const VboIndirectArguments& args) {
        if (!indirectArgumentBuffer) {
            createIndirectArgumentBuffer();
        }

        if (indirectArgumentBuffer) {
            memcpy([indirectArgumentBuffer contents], &args, sizeof(VboIndirectArguments));
        }
    }
};

// ============================================================================
// VboMesh Public Interface
// ============================================================================

VboMesh::VboMesh()
    : impl_(std::make_unique<Impl>()) {}

VboMesh::~VboMesh() = default;

VboMesh::VboMesh(VboMesh&& other) noexcept = default;
VboMesh& VboMesh::operator=(VboMesh&& other) noexcept = default;

bool VboMesh::setMesh(const ofMesh& mesh, VboUsageHint usage, VboAttributeLayout layout) {
    impl_->usageHint = usage;
    impl_->layout = layout;
    impl_->primitiveMode = mesh.getMode();

    const auto& vertices = mesh.getVertices();
    const auto& normals = mesh.getNormals();
    const auto& texCoords = mesh.getTexCoords();
    const auto& colors = mesh.getColors();
    const auto& indices = mesh.getIndices();

    if (vertices.empty()) return false;

    impl_->hasNormals_ = !normals.empty();
    impl_->hasTexCoords_ = !texCoords.empty();
    impl_->hasColors_ = !colors.empty();

    // Allocate buffers
    if (!impl_->createBuffers(vertices.size(), indices.size())) {
        return false;
    }

    // Convert to interleaved format
    impl_->cpuVertices.resize(vertices.size());
    for (size_t i = 0; i < vertices.size(); i++) {
        InterleavedVertex& v = impl_->cpuVertices[i];
        v.position = simd_make_float3(vertices[i].x, vertices[i].y, vertices[i].z);

        if (i < normals.size()) {
            v.normal = simd_make_float3(normals[i].x, normals[i].y, normals[i].z);
        } else {
            v.normal = simd_make_float3(0, 0, 1);
        }

        if (i < texCoords.size()) {
            v.texCoord = simd_make_float2(texCoords[i].x, texCoords[i].y);
        } else {
            v.texCoord = simd_make_float2(0, 0);
        }

        if (i < colors.size()) {
            v.color = simd_make_float4(colors[i].r / 255.0f, colors[i].g / 255.0f,
                                       colors[i].b / 255.0f, colors[i].a / 255.0f);
        } else {
            v.color = simd_make_float4(1, 1, 1, 1);
        }
    }

    impl_->cpuIndices = indices;
    impl_->numVertices = vertices.size();
    impl_->numIndices = indices.size();

    // Mark all frames as dirty
    impl_->dirty = true;
    impl_->dirtyFrameMask = (1 << kMaxFramesInFlight) - 1;

    // For static meshes, upload immediately
    if (usage == VboUsageHint::Static) {
        impl_->uploadToGPU(nil, 0);
    }

    return true;
}

bool VboMesh::allocate(size_t maxVertices, size_t maxIndices, VboUsageHint usage, VboAttributeLayout layout) {
    impl_->usageHint = usage;
    impl_->layout = layout;

    if (!impl_->createBuffers(maxVertices, maxIndices)) {
        return false;
    }

    impl_->cpuVertices.reserve(maxVertices);
    if (maxIndices > 0) {
        impl_->cpuIndices.reserve(maxIndices);
    }

    return true;
}

void VboMesh::setStorageMode(VboStorageMode mode) {
    impl_->storageMode = mode;
}

bool VboMesh::isAllocated() const {
    return impl_->maxVertices > 0 && (impl_->vertexBuffers[0] != nil || impl_->positionBuffers[0] != nil);
}

void VboMesh::clear() {
    impl_->clear();
}

void VboMesh::updateMesh(const ofMesh& mesh) {
    const auto& vertices = mesh.getVertices();
    const auto& normals = mesh.getNormals();
    const auto& texCoords = mesh.getTexCoords();
    const auto& colors = mesh.getColors();
    const auto& indices = mesh.getIndices();

    if (vertices.size() > impl_->maxVertices) {
        // Need to reallocate
        setMesh(mesh, impl_->usageHint, impl_->layout);
        return;
    }

    // Update CPU data
    impl_->cpuVertices.resize(vertices.size());
    for (size_t i = 0; i < vertices.size(); i++) {
        InterleavedVertex& v = impl_->cpuVertices[i];
        v.position = simd_make_float3(vertices[i].x, vertices[i].y, vertices[i].z);

        if (i < normals.size()) {
            v.normal = simd_make_float3(normals[i].x, normals[i].y, normals[i].z);
        }

        if (i < texCoords.size()) {
            v.texCoord = simd_make_float2(texCoords[i].x, texCoords[i].y);
        }

        if (i < colors.size()) {
            v.color = simd_make_float4(colors[i].r / 255.0f, colors[i].g / 255.0f,
                                       colors[i].b / 255.0f, colors[i].a / 255.0f);
        }
    }

    impl_->cpuIndices = indices;
    impl_->numVertices = vertices.size();
    impl_->numIndices = indices.size();
    impl_->primitiveMode = mesh.getMode();

    markDirty();
}

void VboMesh::updateVertices(const ofVec3f* data, size_t count, size_t offset) {
    size_t oldSize = impl_->cpuVertices.size();
    if (offset + count > oldSize) {
        impl_->cpuVertices.resize(offset + count);
        // Initialize new vertices with default values (white color, forward normal)
        for (size_t i = oldSize; i < impl_->cpuVertices.size(); i++) {
            impl_->cpuVertices[i].normal = simd_make_float3(0, 0, 1);
            impl_->cpuVertices[i].texCoord = simd_make_float2(0, 0);
            impl_->cpuVertices[i].color = simd_make_float4(1, 1, 1, 1);  // White, opaque
        }
    }

    for (size_t i = 0; i < count; i++) {
        impl_->cpuVertices[offset + i].position = simd_make_float3(data[i].x, data[i].y, data[i].z);
    }

    impl_->numVertices = std::max(impl_->numVertices, offset + count);
    markDirty();
}

void VboMesh::updateNormals(const ofVec3f* data, size_t count, size_t offset) {
    for (size_t i = 0; i < count && (offset + i) < impl_->cpuVertices.size(); i++) {
        impl_->cpuVertices[offset + i].normal = simd_make_float3(data[i].x, data[i].y, data[i].z);
    }
    impl_->hasNormals_ = true;
    markDirty();
}

void VboMesh::updateTexCoords(const ofVec2f* data, size_t count, size_t offset) {
    for (size_t i = 0; i < count && (offset + i) < impl_->cpuVertices.size(); i++) {
        impl_->cpuVertices[offset + i].texCoord = simd_make_float2(data[i].x, data[i].y);
    }
    impl_->hasTexCoords_ = true;
    markDirty();
}

void VboMesh::updateColors(const ofColor* data, size_t count, size_t offset) {
    for (size_t i = 0; i < count && (offset + i) < impl_->cpuVertices.size(); i++) {
        impl_->cpuVertices[offset + i].color = simd_make_float4(
            data[i].r / 255.0f, data[i].g / 255.0f,
            data[i].b / 255.0f, data[i].a / 255.0f);
    }
    impl_->hasColors_ = true;
    markDirty();
}

void VboMesh::updateIndices(const uint32_t* data, size_t count, size_t offset) {
    if (offset + count > impl_->cpuIndices.size()) {
        impl_->cpuIndices.resize(offset + count);
    }

    memcpy(impl_->cpuIndices.data() + offset, data, count * sizeof(uint32_t));
    impl_->numIndices = std::max(impl_->numIndices, offset + count);
    markDirty();
}

void VboMesh::setVertexCount(size_t count) {
    impl_->numVertices = std::min(count, impl_->maxVertices);
}

void VboMesh::setIndexCount(size_t count) {
    impl_->numIndices = std::min(count, impl_->maxIndices);
}

void VboMesh::sync() {
    // For managed storage, synchronize all buffers
    uint32_t numBuffers = (impl_->usageHint == VboUsageHint::Static) ? 1 : kMaxFramesInFlight;

    for (uint32_t i = 0; i < numBuffers; i++) {
        if (impl_->vertexBuffers[i]) {
            [impl_->vertexBuffers[i] didModifyRange:NSMakeRange(0, impl_->numVertices * sizeof(InterleavedVertex))];
        }
        if (impl_->indexBuffers[i] && impl_->numIndices > 0) {
            [impl_->indexBuffers[i] didModifyRange:NSMakeRange(0, impl_->numIndices * sizeof(uint32_t))];
        }
    }
}

void VboMesh::setInstances(const VboInstanceData* data, size_t count) {
    if (count > impl_->maxInstances) {
        impl_->createInstanceBuffers(count);
    }

    impl_->cpuInstances.assign(data, data + count);
    impl_->numInstances = count;

    // Upload immediately to all frames
    for (uint32_t i = 0; i < kMaxFramesInFlight; i++) {
        impl_->uploadInstances(i);
    }
}

void VboMesh::updateInstances(const VboInstanceData* data, size_t count, size_t offset) {
    if (offset + count > impl_->cpuInstances.size()) {
        impl_->cpuInstances.resize(offset + count);
    }

    memcpy(impl_->cpuInstances.data() + offset, data, count * sizeof(VboInstanceData));
    impl_->numInstances = std::max(impl_->numInstances, offset + count);

    for (uint32_t i = 0; i < kMaxFramesInFlight; i++) {
        impl_->uploadInstances(i);
    }
}

size_t VboMesh::getMaxInstances() const {
    return impl_->maxInstances;
}

void VboMesh::setIndirectArguments(const VboIndirectArguments& args) {
    impl_->setIndirectArgs(args);
}

void* VboMesh::getIndirectArgumentBuffer() {
    if (!impl_->indirectArgumentBuffer) {
        impl_->createIndirectArgumentBuffer();
    }
    return (__bridge void*)impl_->indirectArgumentBuffer;
}

void VboMesh::draw() const {
    draw(impl_->primitiveMode);
}

void VboMesh::draw(ofPrimitiveMode mode) const {
    uploadIfNeeded();

    auto& ctx = Context::instance();
    auto renderer = ctx.renderer();
    if (!renderer) return;

    // Check fill mode
    bool fillEnabled = ofGetFill();

    // Determine if we need wireframe rendering
    bool needsWireframe = !fillEnabled && (mode == OF_PRIMITIVE_TRIANGLES ||
                                            mode == OF_PRIMITIVE_TRIANGLE_STRIP ||
                                            mode == OF_PRIMITIVE_TRIANGLE_FAN);

    // Convert primitive mode
    render::PrimitiveType primType;
    if (needsWireframe) {
        primType = render::PrimitiveType::Line;
    } else {
        switch (mode) {
            case OF_PRIMITIVE_POINTS:
                primType = render::PrimitiveType::Point;
                break;
            case OF_PRIMITIVE_LINES:
            case OF_PRIMITIVE_LINE_LOOP:
                primType = render::PrimitiveType::Line;
                break;
            case OF_PRIMITIVE_LINE_STRIP:
                primType = render::PrimitiveType::LineStrip;
                break;
            case OF_PRIMITIVE_TRIANGLE_STRIP:
            case OF_PRIMITIVE_TRIANGLE_FAN:
                primType = render::PrimitiveType::TriangleStrip;
                break;
            default:
                primType = render::PrimitiveType::Triangle;
                break;
        }
    }

    // Get current color from graphics state
    uint8_t r, g, b, a;
    ofGetColor(r, g, b, a);
    simd_float4 currentColor = simd_make_float4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);

    // Convert interleaved vertices to Vertex3D for DrawList
    // Tint vertex colors with current color
    std::vector<render::Vertex3D> vertices(impl_->numVertices);
    for (size_t i = 0; i < impl_->numVertices; i++) {
        const InterleavedVertex& v = impl_->cpuVertices[i];
        vertices[i].position = v.position;
        vertices[i].normal = v.normal;
        vertices[i].texCoord = v.texCoord;
        // Tint vertex color with current color (multiply)
        vertices[i].color = simd_make_float4(
            v.color.x * currentColor.x,
            v.color.y * currentColor.y,
            v.color.z * currentColor.z,
            v.color.w * currentColor.w
        );
    }

    // Get the current draw list from Context
    render::DrawList& drawList = ctx.getDrawList();

    // Add vertices to draw list
    uint32_t vertexOffset = drawList.addVertices3D(vertices);

    // Generate indices - for wireframe, convert triangle indices to edge indices
    uint32_t indexOffset = 0;
    uint32_t indexCount = 0;

    if (needsWireframe && impl_->numIndices > 0) {
        // Convert triangle indices to line indices (edges)
        // For each triangle [a,b,c], create edges [a,b], [b,c], [c,a]
        std::vector<uint32_t> lineIndices;
        lineIndices.reserve(impl_->numIndices * 2);

        for (size_t i = 0; i + 2 < impl_->numIndices; i += 3) {
            uint32_t a = impl_->cpuIndices[i];
            uint32_t b = impl_->cpuIndices[i + 1];
            uint32_t c = impl_->cpuIndices[i + 2];

            // Edge a-b
            lineIndices.push_back(a);
            lineIndices.push_back(b);
            // Edge b-c
            lineIndices.push_back(b);
            lineIndices.push_back(c);
            // Edge c-a
            lineIndices.push_back(c);
            lineIndices.push_back(a);
        }

        indexOffset = drawList.addIndices(lineIndices);
        indexCount = static_cast<uint32_t>(lineIndices.size());
    } else if (impl_->numIndices > 0) {
        indexOffset = drawList.addIndices(impl_->cpuIndices);
        indexCount = static_cast<uint32_t>(impl_->numIndices);
    }

    // Create DrawCommand3D
    render::DrawCommand3D cmd;
    cmd.vertexOffset = vertexOffset;
    cmd.vertexCount = static_cast<uint32_t>(vertices.size());
    cmd.indexOffset = indexOffset;
    cmd.indexCount = indexCount;
    cmd.primitiveType = primType;
    cmd.blendMode = render::BlendMode::Alpha;
    cmd.texture = nullptr;

    // Get view matrix from Context (set by camera)
    simd_float4x4 viewMatrix = ctx.getViewMatrix();

    // Get model matrix from current transform stack (ofPushMatrix/ofTranslate/etc.)
    ofMatrix4x4 m = ofGetCurrentMatrix();
    simd_float4x4 modelMatrix = simd_matrix(
        simd_make_float4(m(0,0), m(1,0), m(2,0), m(3,0)),
        simd_make_float4(m(0,1), m(1,1), m(2,1), m(3,1)),
        simd_make_float4(m(0,2), m(1,2), m(2,2), m(3,2)),
        simd_make_float4(m(0,3), m(1,3), m(2,3), m(3,3))
    );

    // ModelView = View * Model
    cmd.modelViewMatrix = simd_mul(viewMatrix, modelMatrix);
    cmd.projectionMatrix = ctx.getProjectionMatrix();

    // Calculate normal matrix from model-view matrix
    simd_float3x3 normalMatrix;
    normalMatrix.columns[0] = simd_make_float3(cmd.modelViewMatrix.columns[0].x,
                                                 cmd.modelViewMatrix.columns[0].y,
                                                 cmd.modelViewMatrix.columns[0].z);
    normalMatrix.columns[1] = simd_make_float3(cmd.modelViewMatrix.columns[1].x,
                                                 cmd.modelViewMatrix.columns[1].y,
                                                 cmd.modelViewMatrix.columns[1].z);
    normalMatrix.columns[2] = simd_make_float3(cmd.modelViewMatrix.columns[2].x,
                                                 cmd.modelViewMatrix.columns[2].y,
                                                 cmd.modelViewMatrix.columns[2].z);
    cmd.normalMatrix = normalMatrix;

    // Enable depth testing for 3D rendering
    cmd.depthTestEnabled = true;
    cmd.depthWriteEnabled = true;
    cmd.cullBackFace = false;

    // Capture lighting state at command creation time
    cmd.useLighting = ctx.hasMaterial() && ctx.isLightingEnabled();
    if (cmd.useLighting) {
        cmd.lightCount = ctx.getLightCount();
        auto matData = ctx.getMaterialData();
        size_t matSize = std::min(matData.size(), size_t(16));
        for (size_t i = 0; i < matSize; ++i) {
            cmd.materialData[i] = matData[i];
        }
        auto lightData = ctx.getAllLightData();
        size_t lightSize = std::min(lightData.size(), size_t(256));
        for (size_t i = 0; i < lightSize; ++i) {
            cmd.lightData[i] = lightData[i];
        }
    }

    // Add command to draw list
    drawList.addCommand(cmd);
}

void VboMesh::drawRange(size_t start, size_t count) const {
    // TODO: Implement range drawing
    draw();
}

void VboMesh::drawInstanced(size_t instanceCount) const {
    drawInstanced(instanceCount, impl_->primitiveMode);
}

void VboMesh::drawInstanced(size_t instanceCount, ofPrimitiveMode mode) const {
    // TODO: Implement instanced rendering with Metal encoder directly
    // For now, fall back to multiple draws
    for (size_t i = 0; i < std::min(instanceCount, impl_->numInstances); i++) {
        draw(mode);
    }
}

void VboMesh::drawIndirect() const {
    if (impl_->indirectArgumentBuffer) {
        drawIndirect((__bridge void*)impl_->indirectArgumentBuffer, 0);
    }
}

void VboMesh::drawIndirect(void* argumentBuffer, size_t argumentOffset) const {
    // TODO: Implement indirect rendering with Metal encoder directly
    draw();
}

size_t VboMesh::getNumVertices() const {
    return impl_->numVertices;
}

size_t VboMesh::getNumIndices() const {
    return impl_->numIndices;
}

ofPrimitiveMode VboMesh::getMode() const {
    return impl_->primitiveMode;
}

void VboMesh::setMode(ofPrimitiveMode mode) {
    impl_->primitiveMode = mode;
}

bool VboMesh::hasIndices() const {
    return impl_->numIndices > 0;
}

bool VboMesh::hasNormals() const {
    return impl_->hasNormals_;
}

bool VboMesh::hasTexCoords() const {
    return impl_->hasTexCoords_;
}

bool VboMesh::hasColors() const {
    return impl_->hasColors_;
}

VboStorageMode VboMesh::getStorageMode() const {
    return impl_->storageMode;
}

VboAttributeLayout VboMesh::getAttributeLayout() const {
    return impl_->layout;
}

void* VboMesh::getVertexBuffer(uint32_t frameIndex) const {
    return (__bridge void*)impl_->vertexBuffers[frameIndex % kMaxFramesInFlight];
}

void* VboMesh::getIndexBuffer(uint32_t frameIndex) const {
    return (__bridge void*)impl_->indexBuffers[frameIndex % kMaxFramesInFlight];
}

void* VboMesh::getInstanceBuffer(uint32_t frameIndex) const {
    return (__bridge void*)impl_->instanceBuffers[frameIndex % kMaxFramesInFlight];
}

size_t VboMesh::getVertexBufferOffset() const {
    return 0;
}

size_t VboMesh::getVertexStride() const {
    return sizeof(InterleavedVertex);
}

void VboMesh::markDirty() {
    impl_->dirty = true;
    impl_->dirtyFrameMask = (1 << kMaxFramesInFlight) - 1;
}

void VboMesh::uploadIfNeeded() const {
    if (!impl_->dirty) return;

    auto& ctx = Context::instance();
    auto renderer = ctx.renderer();
    if (!renderer) return;

    uint32_t frameIndex = renderer->getCurrentFrameIndex() % kMaxFramesInFlight;

    if (impl_->dirtyFrameMask & (1 << frameIndex)) {
        const_cast<Impl*>(impl_.get())->uploadToGPU(nil, frameIndex);
    }
}

// ============================================================================
// Static Utility Functions
// ============================================================================

VboMesh VboMesh::createPlane(float width, float height, int columns, int rows) {
    VboMesh vbo;
    ofMesh mesh = ofMesh::plane(width, height, columns, rows);
    vbo.setMesh(mesh, VboUsageHint::Dynamic);
    return vbo;
}

VboMesh VboMesh::createSphere(float radius, int resolution) {
    VboMesh vbo;
    ofMesh mesh = ofMesh::sphere(radius, resolution);
    vbo.setMesh(mesh, VboUsageHint::Dynamic);
    return vbo;
}

VboMesh VboMesh::createBox(float width, float height, float depth) {
    VboMesh vbo;
    ofMesh mesh = ofMesh::box(width, height, depth);
    vbo.setMesh(mesh, VboUsageHint::Dynamic);
    return vbo;
}

VboMesh VboMesh::createCone(float radius, float height, int segments) {
    VboMesh vbo;
    ofMesh mesh = ofMesh::cone(radius, height, segments);
    vbo.setMesh(mesh, VboUsageHint::Dynamic);
    return vbo;
}

VboMesh VboMesh::createCylinder(float radius, float height, int segments) {
    VboMesh vbo;
    ofMesh mesh = ofMesh::cylinder(radius, height, segments);
    vbo.setMesh(mesh, VboUsageHint::Dynamic);
    return vbo;
}

VboMesh VboMesh::createIcosphere(float radius, int subdivisions) {
    VboMesh vbo;
    ofMesh mesh = ofMesh::icosphere(radius, subdivisions);
    vbo.setMesh(mesh, VboUsageHint::Dynamic);
    return vbo;
}

// ============================================================================
// VboMeshBatch Implementation
// ============================================================================

struct VboMeshBatch::Impl {
    struct BatchEntry {
        VboMesh* mesh;
        simd_float4x4 transform;
    };

    std::vector<BatchEntry> entries;
};

VboMeshBatch::VboMeshBatch()
    : impl_(std::make_unique<Impl>()) {}

VboMeshBatch::~VboMeshBatch() = default;

void VboMeshBatch::add(VboMesh& mesh, const simd_float4x4& transform) {
    impl_->entries.push_back({&mesh, transform});
}

void VboMeshBatch::clear() {
    impl_->entries.clear();
}

void VboMeshBatch::draw() {
    // TODO: Implement efficient batched rendering
    // For now, draw each mesh individually
    for (auto& entry : impl_->entries) {
        // TODO: Apply transform
        entry.mesh->draw();
    }
}

size_t VboMeshBatch::getCount() const {
    return impl_->entries.size();
}

} // namespace oflike
