#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <simd/simd.h>

#include "MetalRenderer.h"
#include "../DrawCommand.h"
#include <vector>
#include <cstring>

namespace render {
namespace metal {

// ============================================================================
// Constants
// ============================================================================

constexpr uint32_t kMaxFramesInFlight = 3;  // Triple buffering
constexpr uint32_t kMaxVertices2D = 65536;   // Max 2D vertices per frame
constexpr uint32_t kMaxVertices3D = 32768;   // Max 3D vertices per frame
constexpr uint32_t kMaxIndices = 98304;      // Max indices per frame

// ============================================================================
// MetalRenderer::Impl
// ============================================================================

struct MetalRenderer::Impl {
    // Metal objects
    id<MTLDevice> device = nil;
    id<MTLCommandQueue> commandQueue = nil;
    MTKView* view = nil;
    MTKTextureLoader* textureLoader = nil;

    // Pipeline states
    id<MTLRenderPipelineState> pipeline2D = nil;
    id<MTLRenderPipelineState> pipeline3D = nil;

    // Depth/stencil states
    id<MTLDepthStencilState> depthEnabledState = nil;
    id<MTLDepthStencilState> depthDisabledState = nil;

    // Triple buffering
    uint32_t currentFrameIndex = 0;
    dispatch_semaphore_t frameSemaphore;

    // Vertex buffers (triple buffered)
    id<MTLBuffer> vertexBuffer2D[kMaxFramesInFlight] = {nil, nil, nil};
    id<MTLBuffer> vertexBuffer3D[kMaxFramesInFlight] = {nil, nil, nil};
    id<MTLBuffer> indexBuffer[kMaxFramesInFlight] = {nil, nil, nil};

    // Current frame state
    id<MTLCommandBuffer> currentCommandBuffer = nil;
    id<MTLRenderCommandEncoder> currentEncoder = nil;
    MTLRenderPassDescriptor* currentRenderPass = nil;

    // Current render state
    BlendMode currentBlendMode = BlendMode::Alpha;
    bool depthTestEnabled = false;
    bool depthWriteEnabled = true;
    bool scissorEnabled = false;
    MTLViewport currentViewport;
    MTLScissorRect currentScissor;

    // Statistics
    uint32_t frameDrawCalls = 0;
    uint32_t frameVertices = 0;

    // Initialization flag
    bool initialized = false;

    Impl(void* dev, void* v)
        : device((__bridge id<MTLDevice>)dev)
        , view((__bridge MTKView*)v)
        , frameSemaphore(dispatch_semaphore_create(kMaxFramesInFlight))
    {}

    ~Impl() {
        shutdown();
    }

    bool initialize();
    void shutdown();
    bool createPipelines();
    bool createBuffers();
    bool createDepthStencilStates();

    // Command execution
    bool executeCommand(const DrawCommand& cmd, const DrawList& drawList);
    bool executeDraw2D(const DrawCommand2D& cmd, const DrawList& drawList);
    bool executeDraw3D(const DrawCommand3D& cmd, const DrawList& drawList);
    bool executeSetViewport(const SetViewportCommand& cmd);
    bool executeSetScissor(const SetScissorCommand& cmd);
    bool executeClear(const SetClearCommand& cmd);

    // State management
    void applyBlendMode(BlendMode mode);
    void applyDepthState(bool enabled);

    // Helper functions
    void endCurrentEncoder();
    MTLRenderPassDescriptor* getCurrentRenderPassDescriptor();
};

// ============================================================================
// Initialization
// ============================================================================

bool MetalRenderer::Impl::initialize() {
    if (initialized) {
        return true;
    }

    @autoreleasepool {
        // Create command queue
        commandQueue = [device newCommandQueue];
        if (!commandQueue) {
            NSLog(@"MetalRenderer: Failed to create command queue");
            return false;
        }

        // Create texture loader
        textureLoader = [[MTKTextureLoader alloc] initWithDevice:device];

        // Create pipelines
        if (!createPipelines()) {
            return false;
        }

        // Create buffers
        if (!createBuffers()) {
            return false;
        }

        // Create depth/stencil states
        if (!createDepthStencilStates()) {
            return false;
        }

        // Set initial viewport to view size
        currentViewport.originX = 0;
        currentViewport.originY = 0;
        currentViewport.width = view.drawableSize.width;
        currentViewport.height = view.drawableSize.height;
        currentViewport.znear = 0.0;
        currentViewport.zfar = 1.0;

        initialized = true;
        NSLog(@"MetalRenderer: Initialized successfully");
        return true;
    }
}

void MetalRenderer::Impl::shutdown() {
    if (!initialized) {
        return;
    }

    @autoreleasepool {
        // Wait for all frames to complete
        for (uint32_t i = 0; i < kMaxFramesInFlight; i++) {
            dispatch_semaphore_wait(frameSemaphore, DISPATCH_TIME_FOREVER);
        }

        // Release buffers
        for (uint32_t i = 0; i < kMaxFramesInFlight; i++) {
            vertexBuffer2D[i] = nil;
            vertexBuffer3D[i] = nil;
            indexBuffer[i] = nil;
        }

        // Release other objects
        pipeline2D = nil;
        pipeline3D = nil;
        depthEnabledState = nil;
        depthDisabledState = nil;
        textureLoader = nil;
        commandQueue = nil;

        initialized = false;
        NSLog(@"MetalRenderer: Shutdown complete");
    }
}

bool MetalRenderer::Impl::createPipelines() {
    @autoreleasepool {
        NSError* error = nil;

        // Load default shader library
        id<MTLLibrary> library = [device newDefaultLibrary];
        if (!library) {
            NSLog(@"MetalRenderer: Failed to load shader library");
            return false;
        }

        // TODO: For now, create placeholder pipeline
        // In Phase 4.7, we'll add actual shaders
        // For now, we'll just mark the renderer as ready

        NSLog(@"MetalRenderer: Pipelines created (placeholder)");
        return true;
    }
}

bool MetalRenderer::Impl::createBuffers() {
    @autoreleasepool {
        // Create triple-buffered vertex and index buffers
        for (uint32_t i = 0; i < kMaxFramesInFlight; i++) {
            // 2D vertex buffer
            vertexBuffer2D[i] = [device newBufferWithLength:kMaxVertices2D * sizeof(Vertex2D)
                                                   options:MTLResourceStorageModeShared];
            if (!vertexBuffer2D[i]) {
                NSLog(@"MetalRenderer: Failed to create 2D vertex buffer %u", i);
                return false;
            }
            vertexBuffer2D[i].label = [NSString stringWithFormat:@"VertexBuffer2D_%u", i];

            // 3D vertex buffer
            vertexBuffer3D[i] = [device newBufferWithLength:kMaxVertices3D * sizeof(Vertex3D)
                                                   options:MTLResourceStorageModeShared];
            if (!vertexBuffer3D[i]) {
                NSLog(@"MetalRenderer: Failed to create 3D vertex buffer %u", i);
                return false;
            }
            vertexBuffer3D[i].label = [NSString stringWithFormat:@"VertexBuffer3D_%u", i];

            // Index buffer
            indexBuffer[i] = [device newBufferWithLength:kMaxIndices * sizeof(uint32_t)
                                                options:MTLResourceStorageModeShared];
            if (!indexBuffer[i]) {
                NSLog(@"MetalRenderer: Failed to create index buffer %u", i);
                return false;
            }
            indexBuffer[i].label = [NSString stringWithFormat:@"IndexBuffer_%u", i];
        }

        NSLog(@"MetalRenderer: Buffers created successfully");
        return true;
    }
}

bool MetalRenderer::Impl::createDepthStencilStates() {
    @autoreleasepool {
        // Depth enabled state
        MTLDepthStencilDescriptor* depthEnabledDesc = [[MTLDepthStencilDescriptor alloc] init];
        depthEnabledDesc.depthCompareFunction = MTLCompareFunctionLess;
        depthEnabledDesc.depthWriteEnabled = YES;
        depthEnabledState = [device newDepthStencilStateWithDescriptor:depthEnabledDesc];
        if (!depthEnabledState) {
            NSLog(@"MetalRenderer: Failed to create depth enabled state");
            return false;
        }

        // Depth disabled state
        MTLDepthStencilDescriptor* depthDisabledDesc = [[MTLDepthStencilDescriptor alloc] init];
        depthDisabledDesc.depthCompareFunction = MTLCompareFunctionAlways;
        depthDisabledDesc.depthWriteEnabled = NO;
        depthDisabledState = [device newDepthStencilStateWithDescriptor:depthDisabledDesc];
        if (!depthDisabledState) {
            NSLog(@"MetalRenderer: Failed to create depth disabled state");
            return false;
        }

        NSLog(@"MetalRenderer: Depth/stencil states created");
        return true;
    }
}

// ============================================================================
// Frame Management
// ============================================================================

bool MetalRenderer::Impl::beginFrame() {
    if (!initialized) {
        return false;
    }

    @autoreleasepool {
        // Wait for a frame slot to be available
        dispatch_semaphore_wait(frameSemaphore, DISPATCH_TIME_FOREVER);

        // Reset frame statistics
        frameDrawCalls = 0;
        frameVertices = 0;

        // Create command buffer
        currentCommandBuffer = [commandQueue commandBuffer];
        if (!currentCommandBuffer) {
            NSLog(@"MetalRenderer: Failed to create command buffer");
            dispatch_semaphore_signal(frameSemaphore);
            return false;
        }
        currentCommandBuffer.label = @"FrameCommandBuffer";

        // Signal semaphore when frame completes
        __block dispatch_semaphore_t blockSemaphore = frameSemaphore;
        [currentCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
            dispatch_semaphore_signal(blockSemaphore);
        }];

        return true;
    }
}

bool MetalRenderer::Impl::endFrame() {
    if (!initialized || !currentCommandBuffer) {
        return false;
    }

    @autoreleasepool {
        // End current encoder if active
        endCurrentEncoder();

        // Present drawable
        id<CAMetalDrawable> drawable = view.currentDrawable;
        if (drawable) {
            [currentCommandBuffer presentDrawable:drawable];
        }

        // Commit command buffer
        [currentCommandBuffer commit];

        // Advance frame index
        currentFrameIndex = (currentFrameIndex + 1) % kMaxFramesInFlight;

        // Clear current state
        currentCommandBuffer = nil;
        currentEncoder = nil;
        currentRenderPass = nil;

        return true;
    }
}

void MetalRenderer::Impl::endCurrentEncoder() {
    if (currentEncoder) {
        [currentEncoder endEncoding];
        currentEncoder = nil;
    }
}

MTLRenderPassDescriptor* MetalRenderer::Impl::getCurrentRenderPassDescriptor() {
    if (currentRenderPass) {
        return currentRenderPass;
    }

    // Get default render pass from view
    currentRenderPass = view.currentRenderPassDescriptor;
    if (!currentRenderPass) {
        NSLog(@"MetalRenderer: No render pass descriptor available");
        return nil;
    }

    return currentRenderPass;
}

// ============================================================================
// DrawList Execution
// ============================================================================

bool MetalRenderer::Impl::executeCommand(const DrawCommand& cmd, const DrawList& drawList) {
    switch (cmd.type) {
        case CommandType::Draw2D:
            return executeDraw2D(cmd.draw2D, drawList);

        case CommandType::Draw3D:
            return executeDraw3D(cmd.draw3D, drawList);

        case CommandType::SetViewport:
            return executeSetViewport(cmd.setViewport);

        case CommandType::SetScissor:
            return executeSetScissor(cmd.setScissor);

        case CommandType::Clear:
            return executeClear(cmd.setClear);

        case CommandType::SetRenderTarget:
            // TODO: Implement in Phase 11 (FBO)
            NSLog(@"MetalRenderer: SetRenderTarget not yet implemented");
            return true;

        default:
            NSLog(@"MetalRenderer: Unknown command type %u", (uint32_t)cmd.type);
            return false;
    }
}

bool MetalRenderer::Impl::executeDraw2D(const DrawCommand2D& cmd, const DrawList& drawList) {
    @autoreleasepool {
        // TODO: Phase 4.7 - Implement with actual shaders
        // For now, just track statistics
        frameDrawCalls++;
        frameVertices += cmd.vertexCount;
        return true;
    }
}

bool MetalRenderer::Impl::executeDraw3D(const DrawCommand3D& cmd, const DrawList& drawList) {
    @autoreleasepool {
        // TODO: Phase 4.7 - Implement with actual shaders
        // For now, just track statistics
        frameDrawCalls++;
        frameVertices += cmd.vertexCount;
        return true;
    }
}

bool MetalRenderer::Impl::executeSetViewport(const SetViewportCommand& cmd) {
    currentViewport.originX = cmd.viewport.x;
    currentViewport.originY = cmd.viewport.y;
    currentViewport.width = cmd.viewport.width;
    currentViewport.height = cmd.viewport.height;

    if (currentEncoder) {
        [currentEncoder setViewport:currentViewport];
    }

    return true;
}

bool MetalRenderer::Impl::executeSetScissor(const SetScissorCommand& cmd) {
    scissorEnabled = cmd.enabled;

    if (cmd.enabled) {
        currentScissor.x = (NSUInteger)cmd.scissor.x;
        currentScissor.y = (NSUInteger)cmd.scissor.y;
        currentScissor.width = (NSUInteger)cmd.scissor.width;
        currentScissor.height = (NSUInteger)cmd.scissor.height;

        if (currentEncoder) {
            [currentEncoder setScissorRect:currentScissor];
        }
    }

    return true;
}

bool MetalRenderer::Impl::executeClear(const SetClearCommand& cmd) {
    @autoreleasepool {
        // End current encoder
        endCurrentEncoder();

        // Get render pass descriptor
        MTLRenderPassDescriptor* renderPass = getCurrentRenderPassDescriptor();
        if (!renderPass) {
            return false;
        }

        // Configure clear operation
        if (cmd.clear.clearColor) {
            renderPass.colorAttachments[0].loadAction = MTLLoadActionClear;
            renderPass.colorAttachments[0].clearColor = MTLClearColorMake(
                cmd.clear.color.x,
                cmd.clear.color.y,
                cmd.clear.color.z,
                cmd.clear.color.w
            );
        } else {
            renderPass.colorAttachments[0].loadAction = MTLLoadActionLoad;
        }

        if (cmd.clear.clearDepth && renderPass.depthAttachment.texture) {
            renderPass.depthAttachment.loadAction = MTLLoadActionClear;
            renderPass.depthAttachment.clearDepth = cmd.clear.depth;
        }

        // Create new encoder with clear
        currentEncoder = [currentCommandBuffer renderCommandEncoderWithDescriptor:renderPass];
        if (!currentEncoder) {
            NSLog(@"MetalRenderer: Failed to create render encoder for clear");
            return false;
        }

        // Apply current state
        [currentEncoder setViewport:currentViewport];
        if (scissorEnabled) {
            [currentEncoder setScissorRect:currentScissor];
        }
        applyDepthState(depthTestEnabled);

        return true;
    }
}

// ============================================================================
// State Management
// ============================================================================

void MetalRenderer::Impl::applyBlendMode(BlendMode mode) {
    currentBlendMode = mode;
    // TODO: Blend mode will be applied via pipeline state in Phase 4.7
}

void MetalRenderer::Impl::applyDepthState(bool enabled) {
    depthTestEnabled = enabled;
    if (currentEncoder) {
        if (enabled) {
            [currentEncoder setDepthStencilState:depthEnabledState];
        } else {
            [currentEncoder setDepthStencilState:depthDisabledState];
        }
    }
}

// ============================================================================
// MetalRenderer Public Interface
// ============================================================================

MetalRenderer::MetalRenderer(void* device, void* view)
    : impl_(std::make_unique<Impl>(device, view))
{}

MetalRenderer::~MetalRenderer() = default;

bool MetalRenderer::initialize() {
    return impl_->initialize();
}

void MetalRenderer::shutdown() {
    impl_->shutdown();
}

bool MetalRenderer::isInitialized() const {
    return impl_->initialized;
}

bool MetalRenderer::beginFrame() {
    return impl_->beginFrame();
}

bool MetalRenderer::endFrame() {
    return impl_->endFrame();
}

uint32_t MetalRenderer::getCurrentFrameIndex() const {
    return impl_->currentFrameIndex;
}

bool MetalRenderer::executeDrawList(const DrawList& drawList) {
    if (!impl_->initialized) {
        return false;
    }

    @autoreleasepool {
        // Upload vertex data to GPU buffers
        uint32_t frameIdx = impl_->currentFrameIndex;

        // Upload 2D vertices
        if (drawList.getVertex2DCount() > 0) {
            size_t dataSize = drawList.getVertex2DDataSize();
            if (dataSize > kMaxVertices2D * sizeof(Vertex2D)) {
                NSLog(@"MetalRenderer: 2D vertex count exceeds buffer size");
                return false;
            }
            void* bufferData = [impl_->vertexBuffer2D[frameIdx] contents];
            std::memcpy(bufferData, drawList.getVertex2DData(), dataSize);
        }

        // Upload 3D vertices
        if (drawList.getVertex3DCount() > 0) {
            size_t dataSize = drawList.getVertex3DDataSize();
            if (dataSize > kMaxVertices3D * sizeof(Vertex3D)) {
                NSLog(@"MetalRenderer: 3D vertex count exceeds buffer size");
                return false;
            }
            void* bufferData = [impl_->vertexBuffer3D[frameIdx] contents];
            std::memcpy(bufferData, drawList.getVertex3DData(), dataSize);
        }

        // Upload indices
        if (drawList.getIndexCount() > 0) {
            size_t dataSize = drawList.getIndexDataSize();
            if (dataSize > kMaxIndices * sizeof(uint32_t)) {
                NSLog(@"MetalRenderer: Index count exceeds buffer size");
                return false;
            }
            void* bufferData = [impl_->indexBuffer[frameIdx] contents];
            std::memcpy(bufferData, drawList.getIndexData(), dataSize);
        }

        // Execute all commands
        const auto& commands = drawList.getCommands();
        for (const auto& cmd : commands) {
            if (!impl_->executeCommand(cmd, drawList)) {
                NSLog(@"MetalRenderer: Command execution failed");
                return false;
            }
        }

        return true;
    }
}

void MetalRenderer::setViewport(float x, float y, float width, float height) {
    impl_->currentViewport.originX = x;
    impl_->currentViewport.originY = y;
    impl_->currentViewport.width = width;
    impl_->currentViewport.height = height;

    if (impl_->currentEncoder) {
        [impl_->currentEncoder setViewport:impl_->currentViewport];
    }
}

void MetalRenderer::setScissor(float x, float y, float width, float height) {
    impl_->currentScissor.x = (NSUInteger)x;
    impl_->currentScissor.y = (NSUInteger)y;
    impl_->currentScissor.width = (NSUInteger)width;
    impl_->currentScissor.height = (NSUInteger)height;

    if (impl_->scissorEnabled && impl_->currentEncoder) {
        [impl_->currentEncoder setScissorRect:impl_->currentScissor];
    }
}

void MetalRenderer::setScissorEnabled(bool enabled) {
    impl_->scissorEnabled = enabled;
}

void MetalRenderer::clear(const simd_float4& color, bool clearColor,
                          bool clearDepth, float depth) {
    SetClearCommand cmd;
    cmd.clear.color = color;
    cmd.clear.clearColor = clearColor;
    cmd.clear.clearDepth = clearDepth;
    cmd.clear.depth = depth;
    impl_->executeClear(cmd);
}

void MetalRenderer::setBlendMode(BlendMode mode) {
    impl_->applyBlendMode(mode);
}

void MetalRenderer::setDepthTestEnabled(bool enabled) {
    impl_->applyDepthState(enabled);
}

void MetalRenderer::setDepthWriteEnabled(bool enabled) {
    impl_->depthWriteEnabled = enabled;
}

void MetalRenderer::setCullingMode(bool cullBack, bool enabled) {
    // TODO: Implement culling in Phase 8 (3D)
}

bool MetalRenderer::setRenderTarget(void* renderTarget) {
    // TODO: Implement in Phase 11 (FBO)
    return true;
}

void* MetalRenderer::getDefaultRenderTarget() const {
    return (__bridge void*)impl_->view.currentDrawable.texture;
}

void* MetalRenderer::createTexture(uint32_t width, uint32_t height, const void* data) {
    @autoreleasepool {
        MTLTextureDescriptor* desc = [MTLTextureDescriptor
            texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
            width:width
            height:height
            mipmapped:NO];
        desc.usage = MTLTextureUsageShaderRead;

        id<MTLTexture> texture = [impl_->device newTextureWithDescriptor:desc];
        if (!texture) {
            return nullptr;
        }

        if (data) {
            [texture replaceRegion:MTLRegionMake2D(0, 0, width, height)
                       mipmapLevel:0
                         withBytes:data
                       bytesPerRow:width * 4];
        }

        return (__bridge_retained void*)texture;
    }
}

void* MetalRenderer::loadTexture(const char* path) {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:path];
        NSURL* url = [NSURL fileURLWithPath:nsPath];

        NSError* error = nil;
        NSDictionary* options = @{
            MTKTextureLoaderOptionSRGB: @NO,
            MTKTextureLoaderOptionGenerateMipmaps: @YES
        };

        id<MTLTexture> texture = [impl_->textureLoader
            newTextureWithContentsOfURL:url
            options:options
            error:&error];

        if (error) {
            NSLog(@"MetalRenderer: Failed to load texture: %@", error.localizedDescription);
            return nullptr;
        }

        return (__bridge_retained void*)texture;
    }
}

void MetalRenderer::destroyTexture(void* texture) {
    if (texture) {
        id<MTLTexture> tex = (__bridge_transfer id<MTLTexture>)texture;
        tex = nil;
    }
}

const char* MetalRenderer::getRendererName() const {
    return "Metal";
}

uint32_t MetalRenderer::getViewportWidth() const {
    return (uint32_t)impl_->currentViewport.width;
}

uint32_t MetalRenderer::getViewportHeight() const {
    return (uint32_t)impl_->currentViewport.height;
}

void MetalRenderer::getStatistics(uint32_t& outDrawCalls, uint32_t& outVertices) const {
    outDrawCalls = impl_->frameDrawCalls;
    outVertices = impl_->frameVertices;
}

} // namespace metal
} // namespace render
