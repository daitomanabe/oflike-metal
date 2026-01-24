#import "SharpRenderer.h"
#import "SharpGaussian.h"
#import "ofCamera.h"
#import "ofMatrix4x4.h"
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <simd/simd.h>
#import <Accelerate/Accelerate.h>
#include <algorithm>
#include <vector>
#include <chrono>

namespace Sharp {

// ============================================================================
// Metal Shader Structures (must match shader code)
// ============================================================================

struct GaussianVertex {
    simd_float3 position;      // 3D position in world space
    simd_float3 scale;         // 3D scale (ellipsoid)
    simd_float4 rotation;      // Quaternion rotation
    float opacity;             // Opacity [0,1]
    simd_float3 sh_dc;         // DC component (diffuse color)
    float padding1;
    // SH coefficients (45 floats = 15 coefficients × 3 RGB)
    float sh_coefficients[45];
};

struct GaussianUniforms {
    simd_float4x4 viewMatrix;
    simd_float4x4 projectionMatrix;
    simd_float4x4 viewProjectionMatrix;
    simd_float3 cameraPosition;
    float splatScale;
    float opacityScale;
    int maxSHDegree;
    int enableSphericalHarmonics;
    int screenWidth;
    int screenHeight;
    float padding[3];
};

struct SortData {
    uint32_t index;     // Original Gaussian index
    float depth;        // Depth in camera space (for sorting)
};

// ============================================================================
// Private Implementation
// ============================================================================

class SharpRenderer::Impl {
public:
    Impl()
        : device_(nil)
        , commandQueue_(nil)
        , renderPipelineState_(nil)
        , sortComputePipelineState_(nil)
        , gaussianBuffer_(nil)
        , sortDataBuffer_(nil)
        , indexBuffer_(nil)
        , uniformBuffer_(nil)
        , config_()
        , stats_()
        , initialized_(false)
        , frameIndex_(0) {
    }

    ~Impl() {
        @autoreleasepool {
            shutdown();
        }
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    bool initialize(void* device, void* commandQueue) {
        @autoreleasepool {
            if (initialized_) {
                shutdown();
            }

            device_ = (__bridge id<MTLDevice>)device;
            commandQueue_ = (__bridge id<MTLCommandQueue>)commandQueue;

            if (!device_ || !commandQueue_) {
                return false;
            }

            // Create render pipeline state
            if (!createRenderPipeline()) {
                return false;
            }

            // Create compute pipeline for depth sorting
            if (!createSortPipeline()) {
                return false;
            }

            initialized_ = true;
            return true;
        }
    }

    bool isInitialized() const {
        return initialized_;
    }

    void shutdown() {
        @autoreleasepool {
            renderPipelineState_ = nil;
            sortComputePipelineState_ = nil;
            gaussianBuffer_ = nil;
            sortDataBuffer_ = nil;
            indexBuffer_ = nil;
            uniformBuffer_ = nil;
            device_ = nil;
            commandQueue_ = nil;
            initialized_ = false;
        }
    }

    // ========================================================================
    // Rendering
    // ========================================================================

    bool render(const GaussianCloud& cloud,
                const oflike::ofMatrix4x4& viewMatrix,
                const oflike::ofMatrix4x4& projectionMatrix,
                void* renderTarget,
                void* commandBuffer) {
        @autoreleasepool {
            if (!initialized_) {
                return false;
            }

            if (cloud.size() == 0) {
                return true; // Nothing to render
            }

            auto startTime = std::chrono::high_resolution_clock::now();

            id<MTLTexture> targetTexture = (__bridge id<MTLTexture>)renderTarget;
            id<MTLCommandBuffer> cmdBuffer = (__bridge id<MTLCommandBuffer>)commandBuffer;

            // Update statistics
            stats_.totalGaussians = static_cast<uint32_t>(cloud.size());
            stats_.frameIndex = frameIndex_++;

            // Upload Gaussian data to GPU
            if (!uploadGaussianData(cloud)) {
                return false;
            }

            // Prepare uniforms
            GaussianUniforms uniforms;
            uniforms.viewMatrix = toSimdMatrix(viewMatrix);
            uniforms.projectionMatrix = toSimdMatrix(projectionMatrix);
            uniforms.viewProjectionMatrix = simd_mul(uniforms.projectionMatrix, uniforms.viewMatrix);
            uniforms.cameraPosition = extractCameraPosition(viewMatrix);
            uniforms.splatScale = config_.splatScale;
            uniforms.opacityScale = config_.opacityScale;
            uniforms.maxSHDegree = config_.enableSphericalHarmonics ? config_.maxSHDegree : 0;
            uniforms.enableSphericalHarmonics = config_.enableSphericalHarmonics ? 1 : 0;
            uniforms.screenWidth = static_cast<int>(targetTexture.width);
            uniforms.screenHeight = static_cast<int>(targetTexture.height);

            // Upload uniforms
            if (!uploadUniforms(uniforms)) {
                return false;
            }

            // Depth sort if enabled
            double sortTime = 0.0;
            if (config_.enableDepthSort) {
                auto sortStart = std::chrono::high_resolution_clock::now();
                if (!depthSort(cloud, viewMatrix, cmdBuffer)) {
                    return false;
                }
                auto sortEnd = std::chrono::high_resolution_clock::now();
                sortTime = std::chrono::duration<double, std::milli>(sortEnd - sortStart).count();
            }

            // Render Gaussians
            auto renderStart = std::chrono::high_resolution_clock::now();
            if (!renderGaussians(targetTexture, cmdBuffer)) {
                return false;
            }
            auto renderEnd = std::chrono::high_resolution_clock::now();
            double renderTime = std::chrono::duration<double, std::milli>(renderEnd - renderStart).count();

            auto endTime = std::chrono::high_resolution_clock::now();
            double totalTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();

            // Update statistics
            stats_.sortTimeMs = sortTime;
            stats_.renderTimeMs = renderTime;
            stats_.totalTimeMs = totalTime;
            stats_.visibleGaussians = stats_.totalGaussians; // TODO: Implement frustum culling
            stats_.culledGaussians = 0;

            return true;
        }
    }

    // ========================================================================
    // Configuration
    // ========================================================================

    void setConfig(const RenderConfig& config) {
        config_ = config;
    }

    const RenderConfig& getConfig() const {
        return config_;
    }

    const RenderStats& getStats() const {
        return stats_;
    }

    void resetStats() {
        stats_ = RenderStats();
        frameIndex_ = 0;
    }

private:
    // ========================================================================
    // Pipeline Creation
    // ========================================================================

    bool createRenderPipeline() {
        @autoreleasepool {
            NSError* error = nil;

            // Load shader library
            id<MTLLibrary> library = [device_ newDefaultLibrary];
            if (!library) {
                return false;
            }

            // Load Gaussian Splatting shaders (Phase 25.5)
            id<MTLFunction> vertexFunction = [library newFunctionWithName:@"gaussianSplattingVertex"];
            id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"gaussianSplattingFragment"];

            if (!vertexFunction || !fragmentFunction) {
                NSLog(@"[SharpRenderer] Error: Failed to load Gaussian Splatting shaders");
                NSLog(@"[SharpRenderer] Ensure GaussianSplatting.metal is compiled into default.metallib");
                return false;
            }

            // Create render pipeline descriptor
            MTLRenderPipelineDescriptor* pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
            pipelineDescriptor.label = @"Gaussian Splatting Pipeline";
            pipelineDescriptor.vertexFunction = vertexFunction;
            pipelineDescriptor.fragmentFunction = fragmentFunction;
            pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

            // Enable alpha blending for transparency
            pipelineDescriptor.colorAttachments[0].blendingEnabled = YES;
            pipelineDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
            pipelineDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
            pipelineDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
            pipelineDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
            pipelineDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
            pipelineDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

            // Enable depth testing
            pipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;

            // Create pipeline state
            renderPipelineState_ = [device_ newRenderPipelineStateWithDescriptor:pipelineDescriptor
                                                                           error:&error];
            if (!renderPipelineState_) {
                return false;
            }

            return true;
        }
    }

    bool createSortPipeline() {
        @autoreleasepool {
            NSError* error = nil;

            // Load shader library
            id<MTLLibrary> library = [device_ newDefaultLibrary];
            if (!library) {
                return false;
            }

            // Load GPU depth sort shader (Phase 25.5)
            id<MTLFunction> sortFunction = [library newFunctionWithName:@"bitonicSort"];

            if (!sortFunction) {
                NSLog(@"[SharpRenderer] Warning: GPU sort shader not found, using CPU fallback");
                // CPU sorting fallback is still available
                return true;
            }

            sortComputePipelineState_ = [device_ newComputePipelineStateWithFunction:sortFunction
                                                                                error:&error];
            if (!sortComputePipelineState_) {
                return false;
            }

            return true;
        }
    }

    // ========================================================================
    // Data Upload
    // ========================================================================

    bool uploadGaussianData(const GaussianCloud& cloud) {
        @autoreleasepool {
            size_t count = cloud.size();
            if (count == 0) {
                return true;
            }

            // Allocate buffer if needed
            size_t bufferSize = count * sizeof(GaussianVertex);
            if (!gaussianBuffer_ || gaussianBuffer_.length < bufferSize) {
                gaussianBuffer_ = [device_ newBufferWithLength:bufferSize
                                                       options:MTLResourceStorageModeShared];
                if (!gaussianBuffer_) {
                    return false;
                }
                gaussianBuffer_.label = @"Gaussian Data";
            }

            // Copy Gaussian data to buffer
            GaussianVertex* vertices = static_cast<GaussianVertex*>(gaussianBuffer_.contents);
            const auto& gaussians = cloud.getGaussians();

            for (size_t i = 0; i < count; ++i) {
                const Gaussian& g = gaussians[i];

                vertices[i].position = simd_make_float3(g.position.x, g.position.y, g.position.z);
                vertices[i].scale = simd_make_float3(g.scale.x, g.scale.y, g.scale.z);
                vertices[i].rotation = simd_make_float4(g.rotation.vector.x, g.rotation.vector.y,
                                                        g.rotation.vector.z, g.rotation.vector.w);
                vertices[i].opacity = g.opacity;
                vertices[i].sh_dc = simd_make_float3(g.sh_dc.x, g.sh_dc.y, g.sh_dc.z);

                // Copy SH coefficients
                std::memcpy(vertices[i].sh_coefficients, g.sh_coefficients.data(),
                           sizeof(float) * 45);
            }

            return true;
        }
    }

    bool uploadUniforms(const GaussianUniforms& uniforms) {
        @autoreleasepool {
            size_t bufferSize = sizeof(GaussianUniforms);
            if (!uniformBuffer_ || uniformBuffer_.length < bufferSize) {
                uniformBuffer_ = [device_ newBufferWithLength:bufferSize
                                                      options:MTLResourceStorageModeShared];
                if (!uniformBuffer_) {
                    return false;
                }
                uniformBuffer_.label = @"Gaussian Uniforms";
            }

            std::memcpy(uniformBuffer_.contents, &uniforms, bufferSize);
            return true;
        }
    }

    // ========================================================================
    // Depth Sorting
    // ========================================================================

    bool depthSort(const GaussianCloud& cloud,
                   const oflike::ofMatrix4x4& viewMatrix,
                   id<MTLCommandBuffer> commandBuffer) {
        @autoreleasepool {
            size_t count = cloud.size();
            if (count == 0) {
                return true;
            }

            // Use CPU-based sorting as fallback
            // GPU sorting available if compute shader loaded (Phase 25.5)
            if (sortComputePipelineState_) {
                return depthSortGPU(cloud, viewMatrix);
            }
            return depthSortCPU(cloud, viewMatrix);
        }
    }

    // GPU-based bitonic sort for depth ordering
    bool depthSortGPU(const GaussianCloud& cloud, const oflike::ofMatrix4x4& viewMatrix) {
        @autoreleasepool {
            if (!sortComputePipelineState_) {
                return false;
            }

            size_t count = cloud.size();
            if (count == 0) {
                return true;
            }

            // Create sort data with depths
            SortData* sortData = new SortData[count];
            const auto& gaussians = cloud.getGaussians();
            simd_float4x4 view = toSimdMatrix(viewMatrix);

            for (size_t i = 0; i < count; ++i) {
                const Gaussian& g = gaussians[i];
                simd_float4 pos = simd_make_float4(g.position.x, g.position.y, g.position.z, 1.0f);
                simd_float4 viewPos = simd_mul(view, pos);

                sortData[i].index = static_cast<uint32_t>(i);
                sortData[i].depth = viewPos.z;
            }

            // Upload to GPU
            size_t sortBufferSize = count * sizeof(SortData);
            if (!sortDataBuffer_ || sortDataBuffer_.length < sortBufferSize) {
                sortDataBuffer_ = [device_ newBufferWithLength:sortBufferSize
                                                       options:MTLResourceStorageModeShared];
                if (!sortDataBuffer_) {
                    delete[] sortData;
                    return false;
                }
            }
            memcpy(sortDataBuffer_.contents, sortData, sortBufferSize);
            delete[] sortData;

            // Bitonic sort requires power-of-2 size, pad if needed
            uint32_t paddedCount = 1;
            while (paddedCount < count) {
                paddedCount <<= 1;
            }

            // Create command buffer for sorting
            id<MTLCommandBuffer> commandBuffer = [commandQueue_ commandBuffer];
            id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
            [computeEncoder setComputePipelineState:sortComputePipelineState_];
            [computeEncoder setBuffer:sortDataBuffer_ offset:0 atIndex:0];

            // Bitonic sort: log2(N) stages
            for (uint32_t stage = 2; stage <= paddedCount; stage <<= 1) {
                for (uint32_t step = stage >> 1; step > 0; step >>= 1) {
                    [computeEncoder setBytes:&paddedCount length:sizeof(uint32_t) atIndex:1];
                    [computeEncoder setBytes:&stage length:sizeof(uint32_t) atIndex:2];
                    [computeEncoder setBytes:&step length:sizeof(uint32_t) atIndex:3];

                    // Dispatch threads
                    NSUInteger threadsPerGroup = 256;
                    NSUInteger numThreadgroups = (paddedCount / 2 + threadsPerGroup - 1) / threadsPerGroup;
                    [computeEncoder dispatchThreadgroups:MTLSizeMake(numThreadgroups, 1, 1)
                                   threadsPerThreadgroup:MTLSizeMake(threadsPerGroup, 1, 1)];

                    // Memory barrier between passes
                    [computeEncoder memoryBarrierWithScope:MTLBarrierScopeBuffers];
                }
            }

            [computeEncoder endEncoding];
            [commandBuffer commit];
            [commandBuffer waitUntilCompleted];

            // Copy sorted indices
            SortData* sortedData = (SortData*)sortDataBuffer_.contents;
            size_t indexBufferSize = count * sizeof(uint32_t);
            if (!indexBuffer_ || indexBuffer_.length < indexBufferSize) {
                indexBuffer_ = [device_ newBufferWithLength:indexBufferSize
                                                    options:MTLResourceStorageModeShared];
                if (!indexBuffer_) {
                    return false;
                }
            }

            uint32_t* indices = (uint32_t*)indexBuffer_.contents;
            for (size_t i = 0; i < count; ++i) {
                indices[i] = sortedData[i].index;
            }

            return true;
        }
    }

    bool depthSortCPU(const GaussianCloud& cloud, const oflike::ofMatrix4x4& viewMatrix) {
        @autoreleasepool {
            size_t count = cloud.size();

            // Allocate sort data buffer if needed
            size_t bufferSize = count * sizeof(SortData);
            if (!sortDataBuffer_ || sortDataBuffer_.length < bufferSize) {
                sortDataBuffer_ = [device_ newBufferWithLength:bufferSize
                                                       options:MTLResourceStorageModeShared];
                if (!sortDataBuffer_) {
                    return false;
                }
                sortDataBuffer_.label = @"Sort Data";
            }

            // Compute depth for each Gaussian
            SortData* sortData = static_cast<SortData*>(sortDataBuffer_.contents);
            const auto& gaussians = cloud.getGaussians();

            simd_float4x4 view = toSimdMatrix(viewMatrix);

            for (size_t i = 0; i < count; ++i) {
                const Gaussian& g = gaussians[i];
                simd_float4 pos = simd_make_float4(g.position.x, g.position.y, g.position.z, 1.0f);
                simd_float4 viewPos = simd_mul(view, pos);

                sortData[i].index = static_cast<uint32_t>(i);
                sortData[i].depth = viewPos.z; // Depth in camera space
            }

            // Sort by depth (back-to-front for alpha blending)
            std::sort(sortData, sortData + count,
                     [](const SortData& a, const SortData& b) {
                         return a.depth > b.depth; // Back-to-front
                     });

            // Create index buffer with sorted indices
            size_t indexBufferSize = count * sizeof(uint32_t);
            if (!indexBuffer_ || indexBuffer_.length < indexBufferSize) {
                indexBuffer_ = [device_ newBufferWithLength:indexBufferSize
                                                    options:MTLResourceStorageModeShared];
                if (!indexBuffer_) {
                    return false;
                }
                indexBuffer_.label = @"Sorted Indices";
            }

            uint32_t* indices = static_cast<uint32_t*>(indexBuffer_.contents);
            for (size_t i = 0; i < count; ++i) {
                indices[i] = sortData[i].index;
            }

            return true;
        }
    }

    // ========================================================================
    // Rendering
    // ========================================================================

    bool renderGaussians(id<MTLTexture> renderTarget, id<MTLCommandBuffer> commandBuffer) {
        @autoreleasepool {
            if (!renderPipelineState_) {
                // Pipeline not created yet (shaders not implemented)
                // This is expected for Phase 25.1 - just return success
                return true;
            }

            // Create render pass descriptor
            MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
            renderPassDescriptor.colorAttachments[0].texture = renderTarget;
            renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
            renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

            // Create render command encoder
            id<MTLRenderCommandEncoder> renderEncoder =
                [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
            if (!renderEncoder) {
                return false;
            }

            renderEncoder.label = @"Gaussian Splatting Render";

            // Set pipeline state
            [renderEncoder setRenderPipelineState:renderPipelineState_];

            // Set buffers
            [renderEncoder setVertexBuffer:gaussianBuffer_ offset:0 atIndex:0];
            [renderEncoder setVertexBuffer:uniformBuffer_ offset:0 atIndex:1];
            [renderEncoder setFragmentBuffer:uniformBuffer_ offset:0 atIndex:0];

            // Draw Gaussians
            // Each Gaussian is rendered as a billboard quad (6 vertices)
            size_t gaussianCount = gaussianBuffer_.length / sizeof(GaussianVertex);
            [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle
                              vertexStart:0
                              vertexCount:6 * gaussianCount];

            [renderEncoder endEncoding];

            return true;
        }
    }

    // ========================================================================
    // Utility Functions
    // ========================================================================

    simd_float4x4 toSimdMatrix(const oflike::ofMatrix4x4& m) {
        // Convert oflike::ofMatrix4x4 to simd_float4x4
        // ofMatrix4x4 has a mat member which is already simd_float4x4
        return m.toSimd();
    }

    simd_float3 extractCameraPosition(const oflike::ofMatrix4x4& viewMatrix) {
        // Extract camera position from view matrix
        // Camera position is the inverse translation
        oflike::ofMatrix4x4 invView = viewMatrix.getInverse();
        simd_float4x4 invSimd = invView.toSimd();
        return simd_make_float3(invSimd.columns[3].x, invSimd.columns[3].y, invSimd.columns[3].z);
    }

    // ========================================================================
    // Member Variables
    // ========================================================================

    id<MTLDevice> device_;
    id<MTLCommandQueue> commandQueue_;
    id<MTLRenderPipelineState> renderPipelineState_;
    id<MTLComputePipelineState> sortComputePipelineState_;

    id<MTLBuffer> gaussianBuffer_;
    id<MTLBuffer> sortDataBuffer_;
    id<MTLBuffer> indexBuffer_;
    id<MTLBuffer> uniformBuffer_;

    RenderConfig config_;
    RenderStats stats_;

    bool initialized_;
    uint32_t frameIndex_;
};

// ============================================================================
// Public API Implementation
// ============================================================================

SharpRenderer::SharpRenderer()
    : impl_(std::make_unique<Impl>()) {
}

SharpRenderer::~SharpRenderer() = default;

SharpRenderer::SharpRenderer(SharpRenderer&& other) noexcept = default;
SharpRenderer& SharpRenderer::operator=(SharpRenderer&& other) noexcept = default;

bool SharpRenderer::initialize(void* device, void* commandQueue) {
    return impl_->initialize(device, commandQueue);
}

bool SharpRenderer::isInitialized() const {
    return impl_->isInitialized();
}

void SharpRenderer::shutdown() {
    impl_->shutdown();
}

bool SharpRenderer::render(const GaussianCloud& cloud,
                          const oflike::ofMatrix4x4& viewMatrix,
                          const oflike::ofMatrix4x4& projectionMatrix,
                          void* renderTarget,
                          void* commandBuffer) {
    return impl_->render(cloud, viewMatrix, projectionMatrix, renderTarget, commandBuffer);
}

bool SharpRenderer::render(const GaussianCloud& cloud,
                          const oflike::ofCamera& camera,
                          void* renderTarget,
                          void* commandBuffer) {
    // Extract view and projection matrices from camera
    oflike::ofMatrix4x4 viewMatrix = camera.getModelViewMatrix();
    oflike::ofMatrix4x4 projectionMatrix = camera.getProjectionMatrix();
    return impl_->render(cloud, viewMatrix, projectionMatrix, renderTarget, commandBuffer);
}

void SharpRenderer::setConfig(const RenderConfig& config) {
    impl_->setConfig(config);
}

const RenderConfig& SharpRenderer::getConfig() const {
    return impl_->getConfig();
}

void SharpRenderer::setDepthSortEnabled(bool enabled) {
    RenderConfig config = impl_->getConfig();
    config.enableDepthSort = enabled;
    impl_->setConfig(config);
}

void SharpRenderer::setSplatScale(float scale) {
    RenderConfig config = impl_->getConfig();
    config.splatScale = scale;
    impl_->setConfig(config);
}

void SharpRenderer::setOpacityScale(float scale) {
    RenderConfig config = impl_->getConfig();
    config.opacityScale = scale;
    impl_->setConfig(config);
}

void SharpRenderer::setSphericalHarmonicsEnabled(bool enabled) {
    RenderConfig config = impl_->getConfig();
    config.enableSphericalHarmonics = enabled;
    impl_->setConfig(config);
}

void SharpRenderer::setMaxSHDegree(int degree) {
    RenderConfig config = impl_->getConfig();
    config.maxSHDegree = std::clamp(degree, 0, 3);
    impl_->setConfig(config);
}

const RenderStats& SharpRenderer::getStats() const {
    return impl_->getStats();
}

void SharpRenderer::resetStats() {
    impl_->resetStats();
}

} // namespace Sharp
