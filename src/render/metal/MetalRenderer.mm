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

    // Pipeline states (cached by blend mode)
    id<MTLRenderPipelineState> pipeline2D[8] = {nil};  // One per BlendMode
    id<MTLRenderPipelineState> pipeline3D[8] = {nil};  // One per BlendMode

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
    bool cullingEnabled = false;
    bool cullBackFaces = true;  // true = back, false = front
    bool scissorEnabled = false;
    MTLViewport currentViewport;
    MTLScissorRect currentScissor;

    // Render target state
    id<MTLTexture> currentRenderTarget = nil;  // nil = render to view (default)
    id<MTLTexture> depthTarget = nil;          // Depth buffer for current render target

    // Statistics
    uint32_t frameDrawCalls = 0;
    uint32_t frameVertices = 0;
    double lastGPUTime = 0.0;  // Last measured GPU time in milliseconds
    CFTimeInterval frameStartTime = 0.0;  // CPU frame start time

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
    id<MTLRenderPipelineState> createPipelineVariant(id<MTLLibrary> library, const char* vertexFunc,
                                                       const char* fragmentFunc, BlendMode blendMode);
    bool createBuffers();
    bool createDepthStencilStates();

    // Frame management
    bool beginFrame();
    bool endFrame();

    // Command execution
    bool executeCommand(const DrawCommand& cmd, const DrawList& drawList);
    bool executeDraw2D(const DrawCommand2D& cmd, const DrawList& drawList);
    bool executeDraw3D(const DrawCommand3D& cmd, const DrawList& drawList);
    bool executeSetViewport(const SetViewportCommand& cmd);
    bool executeSetScissor(const SetScissorCommand& cmd);
    bool executeClear(const SetClearCommand& cmd);
    bool executeSetRenderTarget(const SetRenderTargetCommand& cmd);

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

        // Release pipeline variants
        for (int i = 0; i < 8; i++) {
            pipeline2D[i] = nil;
            pipeline3D[i] = nil;
        }

        depthEnabledState = nil;
        depthDisabledState = nil;
        textureLoader = nil;
        commandQueue = nil;

        // Release render target state
        currentRenderTarget = nil;
        depthTarget = nil;

        initialized = false;
        NSLog(@"MetalRenderer: Shutdown complete");
    }
}

id<MTLRenderPipelineState> MetalRenderer::Impl::createPipelineVariant(id<MTLLibrary> library,
                                                                        const char* vertexFunc,
                                                                        const char* fragmentFunc,
                                                                        BlendMode blendMode) {
    @autoreleasepool {
        NSError* error = nil;

        id<MTLFunction> vertFunc = [library newFunctionWithName:@(vertexFunc)];
        id<MTLFunction> fragFunc = [library newFunctionWithName:@(fragmentFunc)];

        if (!vertFunc || !fragFunc) {
            NSLog(@"MetalRenderer: Failed to find shader functions: %s / %s", vertexFunc, fragmentFunc);
            return nil;
        }

        MTLRenderPipelineDescriptor* pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineDesc.label = [NSString stringWithFormat:@"Pipeline_%s_Blend%d", vertexFunc, (int)blendMode];
        pipelineDesc.vertexFunction = vertFunc;
        pipelineDesc.fragmentFunction = fragFunc;
        pipelineDesc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
        pipelineDesc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;

        // Apply blend configuration
        BlendConfig blendConfig = BlendConfig::forMode(blendMode);
        pipelineDesc.colorAttachments[0].blendingEnabled = blendConfig.blendingEnabled;
        pipelineDesc.colorAttachments[0].rgbBlendOperation = (MTLBlendOperation)blendConfig.rgbBlendOperation;
        pipelineDesc.colorAttachments[0].alphaBlendOperation = (MTLBlendOperation)blendConfig.alphaBlendOperation;
        pipelineDesc.colorAttachments[0].sourceRGBBlendFactor = (MTLBlendFactor)blendConfig.sourceRGBBlendFactor;
        pipelineDesc.colorAttachments[0].destinationRGBBlendFactor = (MTLBlendFactor)blendConfig.destinationRGBBlendFactor;
        pipelineDesc.colorAttachments[0].sourceAlphaBlendFactor = (MTLBlendFactor)blendConfig.sourceAlphaBlendFactor;
        pipelineDesc.colorAttachments[0].destinationAlphaBlendFactor = (MTLBlendFactor)blendConfig.destinationAlphaBlendFactor;

        id<MTLRenderPipelineState> pipeline = [device newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];
        if (!pipeline) {
            NSLog(@"MetalRenderer: Failed to create pipeline %s (blend %d): %@",
                  vertexFunc, (int)blendMode, error.localizedDescription);
            return nil;
        }

        return pipeline;
    }
}

bool MetalRenderer::Impl::createPipelines() {
    @autoreleasepool {
        NSError* error = nil;

        // Try to load default shader library first
        id<MTLLibrary> library = [device newDefaultLibrary];

        // If default library not found, compile shaders from source
        if (!library) {
            NSLog(@"MetalRenderer: Default library not found, compiling shaders from source...");

            // Embedded shader source (Common.h + Basic2D.metal combined)
            NSString* shaderSource = @R"(
#include <metal_stdlib>
using namespace metal;

// Vertex Definitions
struct Vertex2D {
    float2 position [[attribute(0)]];
    float2 texCoord [[attribute(1)]];
    float4 color    [[attribute(2)]];
};

struct Vertex3D {
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
    float2 texCoord [[attribute(2)]];
    float4 color    [[attribute(3)]];
};

// Uniform Buffers
struct Uniforms2D {
    float4x4 projectionMatrix;
    float4x4 modelViewMatrix;
};

struct Uniforms3D {
    float4x4 projectionMatrix;
    float4x4 modelViewMatrix;
    float4x4 normalMatrix;
};

// Rasterizer Data
struct RasterizerData2D {
    float4 position [[position]];
    float2 texCoord;
    float4 color;
};

struct RasterizerData3D {
    float4 position [[position]];
    float3 normal;
    float2 texCoord;
    float4 color;
    float3 worldPosition;
};

// 2D Vertex Shader
vertex RasterizerData2D vertex2D(
    uint vertexID [[vertex_id]],
    constant Vertex2D* vertices [[buffer(0)]],
    constant Uniforms2D& uniforms [[buffer(1)]]
) {
    RasterizerData2D out;
    Vertex2D in = vertices[vertexID];
    float4 position = float4(in.position, 0.0, 1.0);
    out.position = uniforms.projectionMatrix * uniforms.modelViewMatrix * position;
    out.texCoord = in.texCoord;
    out.color = in.color;
    return out;
}

// 2D Fragment Shader (solid color)
fragment float4 fragment2D(RasterizerData2D in [[stage_in]]) {
    return in.color;
}

// 2D Fragment Shader (textured)
fragment float4 fragment2DTextured(
    RasterizerData2D in [[stage_in]],
    texture2d<float> colorTexture [[texture(0)]],
    sampler textureSampler [[sampler(0)]]
) {
    float4 texColor = colorTexture.sample(textureSampler, in.texCoord);
    return texColor * in.color;
}

// 3D Vertex Shader
vertex RasterizerData3D vertex3D(
    uint vertexID [[vertex_id]],
    constant Vertex3D* vertices [[buffer(0)]],
    constant Uniforms3D& uniforms [[buffer(1)]]
) {
    RasterizerData3D out;
    Vertex3D in = vertices[vertexID];
    float4 position = float4(in.position, 1.0);
    out.position = uniforms.projectionMatrix * uniforms.modelViewMatrix * position;
    out.worldPosition = in.position;
    out.normal = (uniforms.normalMatrix * float4(in.normal, 0.0)).xyz;
    out.texCoord = in.texCoord;
    out.color = in.color;
    return out;
}

// 3D Fragment Shader (solid color)
fragment float4 fragment3D(RasterizerData3D in [[stage_in]]) {
    return in.color;
}
)";

            MTLCompileOptions* options = [[MTLCompileOptions alloc] init];
            options.languageVersion = MTLLanguageVersion2_4;

            library = [device newLibraryWithSource:shaderSource options:options error:&error];
            if (!library) {
                NSLog(@"MetalRenderer: Failed to compile shaders: %@", error.localizedDescription);
                return false;
            }
            NSLog(@"MetalRenderer: Shaders compiled successfully from source");
        }

        // Create pipeline variants for all blend modes
        // We create 7 variants (all enum values except PremultipliedAlpha which is less common)
        // On-demand creation would be more efficient but requires more complex caching logic

        NSLog(@"MetalRenderer: Creating pipeline variants...");

        // Create 2D pipeline variants for common blend modes
        for (int i = 0; i <= 6; i++) {
            BlendMode mode = (BlendMode)i;
            pipeline2D[i] = createPipelineVariant(library, "vertex2D", "fragment2D", mode);
            if (!pipeline2D[i]) {
                NSLog(@"MetalRenderer: Failed to create 2D pipeline variant for blend mode %d", i);
                return false;
            }
        }

        // Create 3D pipeline variants for common blend modes
        for (int i = 0; i <= 6; i++) {
            BlendMode mode = (BlendMode)i;
            pipeline3D[i] = createPipelineVariant(library, "vertex3D", "fragment3D", mode);
            if (!pipeline3D[i]) {
                NSLog(@"MetalRenderer: Failed to create 3D pipeline variant for blend mode %d", i);
                return false;
            }
        }

        NSLog(@"MetalRenderer: All pipeline variants created successfully");
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
        frameStartTime = CACurrentMediaTime();

        // Create command buffer
        currentCommandBuffer = [commandQueue commandBuffer];
        if (!currentCommandBuffer) {
            NSLog(@"MetalRenderer: Failed to create command buffer");
            dispatch_semaphore_signal(frameSemaphore);
            return false;
        }
        currentCommandBuffer.label = @"FrameCommandBuffer";

        // Capture GPU timing info
        __block double* gpuTimePtr = &lastGPUTime;
        __block CFTimeInterval startTime = frameStartTime;

        // Signal semaphore when frame completes
        __block dispatch_semaphore_t blockSemaphore = frameSemaphore;
        [currentCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
            // Calculate GPU time (kernel + GPU time)
            CFTimeInterval gpuStart = buffer.GPUStartTime;
            CFTimeInterval gpuEnd = buffer.GPUEndTime;

            if (gpuStart > 0 && gpuEnd > 0) {
                *gpuTimePtr = (gpuEnd - gpuStart) * 1000.0;  // Convert to milliseconds
            }

            dispatch_semaphore_signal(blockSemaphore);
        }];

        return true;
    }
}

bool MetalRenderer::Impl::endFrame() {
    if (!initialized || !currentCommandBuffer || !view) {
        return false;
    }

    @autoreleasepool {
        // End current encoder if active
        endCurrentEncoder();

        // Present drawable
        id<CAMetalDrawable> drawable = view.currentDrawable;
        if (drawable && currentCommandBuffer) {
            [currentCommandBuffer presentDrawable:drawable];
        }

        // Commit command buffer
        if (currentCommandBuffer) {
            [currentCommandBuffer commit];
        }

        // Advance frame index
        currentFrameIndex = (currentFrameIndex + 1) % kMaxFramesInFlight;

        // Clear current state
        currentCommandBuffer = nil;
        currentEncoder = nil;
        currentRenderPass = nil;

        // Reset render target state to default (screen)
        currentRenderTarget = nil;
        // Keep depthTarget allocated for reuse in next frame

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

    @autoreleasepool {
        // Check if rendering to a custom render target (FBO) or the view
        if (currentRenderTarget) {
            // Create render pass for custom render target (FBO)
            currentRenderPass = [MTLRenderPassDescriptor renderPassDescriptor];

            // Set color attachment
            currentRenderPass.colorAttachments[0].texture = currentRenderTarget;
            currentRenderPass.colorAttachments[0].loadAction = MTLLoadActionLoad;
            currentRenderPass.colorAttachments[0].storeAction = MTLStoreActionStore;

            // Set depth attachment if available
            if (depthTarget) {
                currentRenderPass.depthAttachment.texture = depthTarget;
                currentRenderPass.depthAttachment.loadAction = MTLLoadActionLoad;
                currentRenderPass.depthAttachment.storeAction = MTLStoreActionStore;
            }
        } else {
            // Get default render pass from view (render to screen)
            currentRenderPass = view.currentRenderPassDescriptor;
            if (!currentRenderPass) {
                NSLog(@"MetalRenderer: No render pass descriptor available");
                return nil;
            }
        }

        return currentRenderPass;
    }
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
            return executeSetViewport(cmd.viewport);

        case CommandType::SetScissor:
            return executeSetScissor(cmd.scissor);

        case CommandType::Clear:
            return executeClear(cmd.clear);

        case CommandType::SetRenderTarget:
            return executeSetRenderTarget(cmd.renderTarget);

        default:
            NSLog(@"MetalRenderer: Unknown command type %u", (uint32_t)cmd.type);
            return false;
    }
}

bool MetalRenderer::Impl::executeDraw2D(const DrawCommand2D& cmd, const DrawList& drawList) {
    @autoreleasepool {
        // Ensure we have an encoder
        if (!currentEncoder) {
            MTLRenderPassDescriptor* renderPass = getCurrentRenderPassDescriptor();
            if (!renderPass) {
                NSLog(@"MetalRenderer: Failed to get render pass for draw2D");
                return false;
            }

            currentEncoder = [currentCommandBuffer renderCommandEncoderWithDescriptor:renderPass];
            if (!currentEncoder) {
                NSLog(@"MetalRenderer: Failed to create render encoder for draw2D");
                return false;
            }

            // Apply current viewport and scissor state
            [currentEncoder setViewport:currentViewport];
            if (scissorEnabled) {
                [currentEncoder setScissorRect:currentScissor];
            }
            applyDepthState(depthTestEnabled);
        }

        // Upload vertex data to buffer
        const Vertex2D* vertices = drawList.getVertex2DData();
        if (!vertices || cmd.vertexCount == 0) {
            NSLog(@"MetalRenderer: No vertices to draw in draw2D");
            return false;
        }

        const size_t vertexDataSize = cmd.vertexCount * sizeof(Vertex2D);
        const size_t bufferOffset = cmd.vertexOffset * sizeof(Vertex2D);

        // Check buffer bounds
        if (bufferOffset + vertexDataSize > kMaxVertices2D * sizeof(Vertex2D)) {
            NSLog(@"MetalRenderer: Vertex data exceeds buffer size in draw2D");
            return false;
        }

        // Copy vertex data into current frame's buffer
        id<MTLBuffer> currentBuffer = vertexBuffer2D[currentFrameIndex];
        uint8_t* bufferPtr = (uint8_t*)[currentBuffer contents];
        memcpy(bufferPtr + bufferOffset, vertices + cmd.vertexOffset, vertexDataSize);

        // Set pipeline (select variant based on blend mode)
        uint32_t blendIndex = (uint32_t)cmd.blendMode;
        if (blendIndex > 6) blendIndex = 1; // Default to Alpha if out of range
        [currentEncoder setRenderPipelineState:pipeline2D[blendIndex]];

        // Set vertex buffer
        [currentEncoder setVertexBuffer:currentBuffer offset:bufferOffset atIndex:0];

        // Set uniforms (projection + modelView matrices)
        struct Uniforms2D {
            simd_float4x4 projectionMatrix;
            simd_float4x4 modelViewMatrix;
        };

        // Get viewport dimensions for projection matrix
        float width = currentViewport.width;
        float height = currentViewport.height;

        // Create orthographic projection matrix: left-top origin (0,0) to (width, height)
        // Maps to Metal NDC: (-1, -1) to (+1, +1)
        // Y-axis flipped: oF Y↓ to Metal Y↑
        simd_float4x4 projection = {
            simd_make_float4(2.0f / width,  0.0f,            0.0f, 0.0f),
            simd_make_float4(0.0f,         -2.0f / height,   0.0f, 0.0f),
            simd_make_float4(0.0f,          0.0f,            1.0f, 0.0f),
            simd_make_float4(-1.0f,         1.0f,            0.0f, 1.0f)
        };

        Uniforms2D uniforms;
        uniforms.projectionMatrix = projection;
        uniforms.modelViewMatrix = cmd.transform;

        [currentEncoder setVertexBytes:&uniforms length:sizeof(Uniforms2D) atIndex:1];

        // Set texture if present
        if (cmd.texture) {
            id<MTLTexture> texture = (__bridge id<MTLTexture>)cmd.texture;
            [currentEncoder setFragmentTexture:texture atIndex:0];

            // Create sampler state (linear filtering, clamp to edge)
            MTLSamplerDescriptor* samplerDesc = [[MTLSamplerDescriptor alloc] init];
            samplerDesc.minFilter = MTLSamplerMinMagFilterLinear;
            samplerDesc.magFilter = MTLSamplerMinMagFilterLinear;
            samplerDesc.sAddressMode = MTLSamplerAddressModeClampToEdge;
            samplerDesc.tAddressMode = MTLSamplerAddressModeClampToEdge;
            samplerDesc.mipFilter = MTLSamplerMipFilterNotMipmapped;

            id<MTLSamplerState> sampler = [device newSamplerStateWithDescriptor:samplerDesc];
            [currentEncoder setFragmentSamplerState:sampler atIndex:0];
        }

        // Convert PrimitiveType to MTLPrimitiveType
        MTLPrimitiveType mtlPrimitive;
        switch (cmd.primitiveType) {
            case PrimitiveType::Point:
                mtlPrimitive = MTLPrimitiveTypePoint;
                break;
            case PrimitiveType::Line:
                mtlPrimitive = MTLPrimitiveTypeLine;
                break;
            case PrimitiveType::LineStrip:
                mtlPrimitive = MTLPrimitiveTypeLineStrip;
                break;
            case PrimitiveType::Triangle:
                mtlPrimitive = MTLPrimitiveTypeTriangle;
                break;
            case PrimitiveType::TriangleStrip:
                mtlPrimitive = MTLPrimitiveTypeTriangleStrip;
                break;
            default:
                mtlPrimitive = MTLPrimitiveTypeTriangle;
                break;
        }

        // Execute draw call
        if (cmd.indexCount > 0) {
            // Indexed draw
            const uint32_t* indices = drawList.getIndexData();
            if (!indices) {
                NSLog(@"MetalRenderer: No indices for indexed draw2D");
                return false;
            }

            const size_t indexDataSize = cmd.indexCount * sizeof(uint32_t);
            const size_t indexBufferOffset = cmd.indexOffset * sizeof(uint32_t);

            // Check buffer bounds
            if (indexBufferOffset + indexDataSize > kMaxIndices * sizeof(uint32_t)) {
                NSLog(@"MetalRenderer: Index data exceeds buffer size in draw2D");
                return false;
            }

            // Copy index data into current frame's buffer
            id<MTLBuffer> currentIndexBuffer = indexBuffer[currentFrameIndex];
            uint8_t* indexBufferPtr = (uint8_t*)[currentIndexBuffer contents];
            memcpy(indexBufferPtr + indexBufferOffset, indices + cmd.indexOffset, indexDataSize);

            [currentEncoder drawIndexedPrimitives:mtlPrimitive
                                       indexCount:cmd.indexCount
                                        indexType:MTLIndexTypeUInt32
                                      indexBuffer:currentIndexBuffer
                                indexBufferOffset:indexBufferOffset];
        } else {
            // Non-indexed draw
            [currentEncoder drawPrimitives:mtlPrimitive
                               vertexStart:0
                               vertexCount:cmd.vertexCount];
        }

        // Update statistics
        frameDrawCalls++;
        frameVertices += cmd.vertexCount;

        return true;
    }
}

bool MetalRenderer::Impl::executeDraw3D(const DrawCommand3D& cmd, const DrawList& drawList) {
    @autoreleasepool {
        // Ensure we have an encoder
        if (!currentEncoder) {
            MTLRenderPassDescriptor* renderPass = getCurrentRenderPassDescriptor();
            if (!renderPass) {
                NSLog(@"MetalRenderer: Failed to get render pass for draw3D");
                return false;
            }

            currentEncoder = [currentCommandBuffer renderCommandEncoderWithDescriptor:renderPass];
            if (!currentEncoder) {
                NSLog(@"MetalRenderer: Failed to create render encoder for draw3D");
                return false;
            }

            // Apply current viewport and scissor state
            [currentEncoder setViewport:currentViewport];
            if (scissorEnabled) {
                [currentEncoder setScissorRect:currentScissor];
            }
        }

        // Upload vertex data to buffer
        const Vertex3D* vertices = drawList.getVertex3DData();
        if (!vertices || cmd.vertexCount == 0) {
            NSLog(@"MetalRenderer: No vertices to draw in draw3D");
            return false;
        }

        const size_t vertexDataSize = cmd.vertexCount * sizeof(Vertex3D);
        const size_t bufferOffset = cmd.vertexOffset * sizeof(Vertex3D);

        // Check buffer bounds
        if (bufferOffset + vertexDataSize > kMaxVertices3D * sizeof(Vertex3D)) {
            NSLog(@"MetalRenderer: Vertex data exceeds buffer size in draw3D");
            return false;
        }

        // Copy vertex data into current frame's buffer
        id<MTLBuffer> currentBuffer = vertexBuffer3D[currentFrameIndex];
        uint8_t* bufferPtr = (uint8_t*)[currentBuffer contents];
        memcpy(bufferPtr + bufferOffset, vertices + cmd.vertexOffset, vertexDataSize);

        // Set pipeline (select variant based on blend mode)
        uint32_t blendIndex = (uint32_t)cmd.blendMode;
        if (blendIndex > 6) blendIndex = 1; // Default to Alpha if out of range
        [currentEncoder setRenderPipelineState:pipeline3D[blendIndex]];

        // Set vertex buffer
        [currentEncoder setVertexBuffer:currentBuffer offset:bufferOffset atIndex:0];

        // Set uniforms (projection + modelView + normal matrices)
        struct Uniforms3D {
            simd_float4x4 projectionMatrix;
            simd_float4x4 modelViewMatrix;
            simd_float4x4 normalMatrix;
        };

        Uniforms3D uniforms;
        uniforms.projectionMatrix = cmd.projectionMatrix;
        uniforms.modelViewMatrix = cmd.modelViewMatrix;

        // Convert 3x3 normal matrix to 4x4 for uniform buffer
        simd_float4x4 normalMatrix4x4 = {
            simd_make_float4(cmd.normalMatrix.columns[0], 0.0f),
            simd_make_float4(cmd.normalMatrix.columns[1], 0.0f),
            simd_make_float4(cmd.normalMatrix.columns[2], 0.0f),
            simd_make_float4(0.0f, 0.0f, 0.0f, 1.0f)
        };
        uniforms.normalMatrix = normalMatrix4x4;

        [currentEncoder setVertexBytes:&uniforms length:sizeof(Uniforms3D) atIndex:1];

        // Apply depth state
        applyDepthState(cmd.depthTestEnabled);

        // Apply culling mode
        if (cmd.cullBackFace) {
            [currentEncoder setCullMode:MTLCullModeBack];
        } else {
            [currentEncoder setCullMode:MTLCullModeNone];
        }

        // Set texture if present
        if (cmd.texture) {
            id<MTLTexture> texture = (__bridge id<MTLTexture>)cmd.texture;
            [currentEncoder setFragmentTexture:texture atIndex:0];

            // Create sampler state (linear filtering, clamp to edge)
            MTLSamplerDescriptor* samplerDesc = [[MTLSamplerDescriptor alloc] init];
            samplerDesc.minFilter = MTLSamplerMinMagFilterLinear;
            samplerDesc.magFilter = MTLSamplerMinMagFilterLinear;
            samplerDesc.sAddressMode = MTLSamplerAddressModeClampToEdge;
            samplerDesc.tAddressMode = MTLSamplerAddressModeClampToEdge;
            samplerDesc.mipFilter = MTLSamplerMipFilterNotMipmapped;

            id<MTLSamplerState> sampler = [device newSamplerStateWithDescriptor:samplerDesc];
            [currentEncoder setFragmentSamplerState:sampler atIndex:0];
        }

        // Convert PrimitiveType to MTLPrimitiveType
        MTLPrimitiveType mtlPrimitive;
        switch (cmd.primitiveType) {
            case PrimitiveType::Point:
                mtlPrimitive = MTLPrimitiveTypePoint;
                break;
            case PrimitiveType::Line:
                mtlPrimitive = MTLPrimitiveTypeLine;
                break;
            case PrimitiveType::LineStrip:
                mtlPrimitive = MTLPrimitiveTypeLineStrip;
                break;
            case PrimitiveType::Triangle:
                mtlPrimitive = MTLPrimitiveTypeTriangle;
                break;
            case PrimitiveType::TriangleStrip:
                mtlPrimitive = MTLPrimitiveTypeTriangleStrip;
                break;
            default:
                mtlPrimitive = MTLPrimitiveTypeTriangle;
                break;
        }

        // Execute draw call
        if (cmd.indexCount > 0) {
            // Indexed draw
            const uint32_t* indices = drawList.getIndexData();
            if (!indices) {
                NSLog(@"MetalRenderer: No indices for indexed draw3D");
                return false;
            }

            const size_t indexDataSize = cmd.indexCount * sizeof(uint32_t);
            const size_t indexBufferOffset = cmd.indexOffset * sizeof(uint32_t);

            // Check buffer bounds
            if (indexBufferOffset + indexDataSize > kMaxIndices * sizeof(uint32_t)) {
                NSLog(@"MetalRenderer: Index data exceeds buffer size in draw3D");
                return false;
            }

            // Copy index data into current frame's buffer
            id<MTLBuffer> currentIndexBuffer = indexBuffer[currentFrameIndex];
            uint8_t* indexBufferPtr = (uint8_t*)[currentIndexBuffer contents];
            memcpy(indexBufferPtr + indexBufferOffset, indices + cmd.indexOffset, indexDataSize);

            [currentEncoder drawIndexedPrimitives:mtlPrimitive
                                       indexCount:cmd.indexCount
                                        indexType:MTLIndexTypeUInt32
                                      indexBuffer:currentIndexBuffer
                                indexBufferOffset:indexBufferOffset];
        } else {
            // Non-indexed draw
            [currentEncoder drawPrimitives:mtlPrimitive
                               vertexStart:0
                               vertexCount:cmd.vertexCount];
        }

        // Update statistics
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
        if (cmd.clearData.clearColor) {
            renderPass.colorAttachments[0].loadAction = MTLLoadActionClear;
            renderPass.colorAttachments[0].clearColor = MTLClearColorMake(
                cmd.clearData.color.x,
                cmd.clearData.color.y,
                cmd.clearData.color.z,
                cmd.clearData.color.w
            );
        } else {
            renderPass.colorAttachments[0].loadAction = MTLLoadActionLoad;
        }

        if (cmd.clearData.clearDepth && renderPass.depthAttachment.texture) {
            renderPass.depthAttachment.loadAction = MTLLoadActionClear;
            renderPass.depthAttachment.clearDepth = cmd.clearData.depth;
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

bool MetalRenderer::Impl::executeSetRenderTarget(const SetRenderTargetCommand& cmd) {
    @autoreleasepool {
        // End current encoder - switching render targets requires a new encoder
        endCurrentEncoder();

        // Clear current render pass descriptor - will be recreated with new target
        currentRenderPass = nil;

        // Set new render target
        if (cmd.renderTarget) {
            // Render to custom texture (FBO)
            id<MTLTexture> targetTexture = (__bridge id<MTLTexture>)cmd.renderTarget;
            currentRenderTarget = targetTexture;

            // Try to find or create a matching depth buffer
            // For now, we'll check if there's a depth texture with matching dimensions
            // This is a simplified approach; a full implementation would maintain
            // a depth buffer pool or allow explicit depth buffer specification
            NSUInteger width = targetTexture.width;
            NSUInteger height = targetTexture.height;

            // Check if we need to create a depth buffer
            if (!depthTarget || depthTarget.width != width || depthTarget.height != height) {
                // Create depth texture matching the render target dimensions
                MTLTextureDescriptor* depthDesc = [MTLTextureDescriptor
                    texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                    width:width
                    height:height
                    mipmapped:NO];
                depthDesc.usage = MTLTextureUsageRenderTarget;
                depthDesc.storageMode = MTLStorageModePrivate;

                depthTarget = [device newTextureWithDescriptor:depthDesc];
                if (!depthTarget) {
                    NSLog(@"MetalRenderer: Failed to create depth buffer for FBO");
                    // Continue without depth buffer
                }
            }

            NSLog(@"MetalRenderer: Switched to FBO render target (%lu x %lu)",
                  (unsigned long)width, (unsigned long)height);
        } else {
            // Render to screen (default)
            currentRenderTarget = nil;
            depthTarget = nil;  // View provides its own depth buffer

            NSLog(@"MetalRenderer: Switched to screen render target");
        }

        return true;
    }
}

// ============================================================================
// State Management
// ============================================================================

void MetalRenderer::Impl::applyBlendMode(BlendMode mode) {
    currentBlendMode = mode;
    // Blend mode is applied via pipeline state selection in executeDraw2D/3D
    // No runtime state change needed - Metal requires blend state in PSO
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
    if (!impl_) return false;
    return impl_->beginFrame();
}

bool MetalRenderer::endFrame() {
    if (!impl_) return false;
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
    cmd.clearData.color = color;
    cmd.clearData.clearColor = clearColor;
    cmd.clearData.clearDepth = clearDepth;
    cmd.clearData.depth = depth;
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
    impl_->cullingEnabled = enabled;
    impl_->cullBackFaces = cullBack;
    // Culling is applied dynamically in executeDraw3D via setCullMode()
    // 2D rendering always uses MTLCullModeNone
}

bool MetalRenderer::setRenderTarget(void* renderTarget) {
    if (!impl_->initialized) {
        return false;
    }

    SetRenderTargetCommand cmd;
    cmd.renderTarget = renderTarget;
    return impl_->executeSetRenderTarget(cmd);
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

double MetalRenderer::getLastGPUTime() const {
    return impl_->lastGPUTime;
}

void* MetalRenderer::getDevice() const {
    return (__bridge void*)impl_->device;
}

} // namespace metal
} // namespace render
