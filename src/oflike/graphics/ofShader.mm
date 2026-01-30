#import "ofShader.h"
#import "../../core/Context.h"
#import "../../render/DrawList.h"
#import "../../render/DrawCommand.h"
#import "../image/ofTexture.h"
#import <Metal/Metal.h>
#import <Foundation/Foundation.h>
#include <unordered_map>
#include <vector>
#include <cstring>

namespace oflike {

// ============================================================================
// ofShader::Impl
// ============================================================================

struct ofShader::Impl {
    // Shader state
    bool loaded = false;
    bool active = false;

    // Function names
    std::string vertexFunctionName = "vertex_main";
    std::string fragmentFunctionName = "fragment_main";
    std::string shaderPath;

    // Metal objects
    id<MTLLibrary> library = nil;
    id<MTLFunction> vertexFunction = nil;
    id<MTLFunction> fragmentFunction = nil;
    id<MTLRenderPipelineState> pipelineState = nil;

    // Uniform buffer data
    std::unordered_map<std::string, size_t> uniformOffsets;
    std::vector<uint8_t> uniformBuffer;
    size_t uniformBufferSize = 0;
    bool uniformsDirty = false;

    // Texture bindings
    struct TextureBinding {
        void* textureHandle = nullptr;
        int index = 0;
    };
    std::unordered_map<std::string, TextureBinding> textureBindings;

    // Helper: Get Metal device
    id<MTLDevice> getDevice() const {
        void* devicePtr = Context::instance().getMetalDevice();
        return devicePtr ? (__bridge id<MTLDevice>)devicePtr : nil;
    }

    Impl() {
        // Reserve space for uniforms (4KB default)
        uniformBuffer.resize(4096, 0);
        uniformBufferSize = 4096;
    }

    ~Impl() {
        unload();
    }

    void unload() {
        @autoreleasepool {
            pipelineState = nil;
            vertexFunction = nil;
            fragmentFunction = nil;
            library = nil;
            loaded = false;
            active = false;
            uniformOffsets.clear();
            textureBindings.clear();
        }
    }

    bool loadFromFile(const std::string& path,
                      const std::string& vertName,
                      const std::string& fragName) {
        @autoreleasepool {
            unload();

            id<MTLDevice> device = getDevice();
            if (!device) {
                NSLog(@"ofShader: No Metal device available");
                return false;
            }

            vertexFunctionName = vertName;
            fragmentFunctionName = fragName;
            shaderPath = path;

            // Try the default library first (shaders compiled into the app bundle)
            library = [device newDefaultLibrary];
            if (library) {
                // Check if the functions exist in the default library
                id<MTLFunction> testVert = [library newFunctionWithName:
                    [NSString stringWithUTF8String:vertName.c_str()]];
                id<MTLFunction> testFrag = [library newFunctionWithName:
                    [NSString stringWithUTF8String:fragName.c_str()]];
                if (!testVert || !testFrag) {
                    // Functions not in default library, try other sources
                    library = nil;
                }
            }

            // Try to load pre-compiled metallib first
            if (!library) {
                NSString* metallibPath = [NSString stringWithFormat:@"%s.metallib",
                                           path.c_str()];
                if ([[NSFileManager defaultManager] fileExistsAtPath:metallibPath]) {
                    NSError* error = nil;
                    library = [device newLibraryWithFile:metallibPath error:&error];
                    if (!library) {
                        NSLog(@"ofShader: Failed to load metallib: %@",
                              error.localizedDescription);
                    }
                }
            }

            // Try to load from bundle resources
            if (!library) {
                NSBundle* bundle = [NSBundle mainBundle];
                NSString* baseName = [[NSString stringWithUTF8String:path.c_str()]
                                       lastPathComponent];
                NSString* bundlePath = [bundle pathForResource:baseName ofType:@"metallib"];
                if (bundlePath) {
                    NSError* error = nil;
                    library = [device newLibraryWithFile:bundlePath error:&error];
                    if (!library) {
                        NSLog(@"ofShader: Failed to load bundled metallib: %@",
                              error.localizedDescription);
                    }
                }
            }

            // Try to compile from source
            if (!library) {
                NSString* sourcePath = [NSString stringWithFormat:@"%s.metal",
                                         path.c_str()];
                // Also check bundle
                if (![[NSFileManager defaultManager] fileExistsAtPath:sourcePath]) {
                    NSBundle* bundle = [NSBundle mainBundle];
                    NSString* baseName = [[NSString stringWithUTF8String:path.c_str()]
                                           lastPathComponent];
                    sourcePath = [bundle pathForResource:baseName ofType:@"metal"];
                }

                if (sourcePath && [[NSFileManager defaultManager] fileExistsAtPath:sourcePath]) {
                    NSError* readError = nil;
                    NSString* source = [NSString stringWithContentsOfFile:sourcePath
                                                                 encoding:NSUTF8StringEncoding
                                                                    error:&readError];
                    if (source) {
                        MTLCompileOptions* options = [[MTLCompileOptions alloc] init];
                        options.fastMathEnabled = YES;

                        NSError* compileError = nil;
                        library = [device newLibraryWithSource:source
                                                       options:options
                                                         error:&compileError];
                        if (!library) {
                            NSLog(@"ofShader: Compilation failed: %@",
                                  compileError.localizedDescription);
                        }
                    } else {
                        NSLog(@"ofShader: Failed to read source: %@",
                              readError.localizedDescription);
                    }
                }
            }

            if (!library) {
                NSLog(@"ofShader: Could not load shader from: %s", path.c_str());
                return false;
            }

            // Get functions
            vertexFunction = [library newFunctionWithName:
                              [NSString stringWithUTF8String:vertexFunctionName.c_str()]];
            fragmentFunction = [library newFunctionWithName:
                                [NSString stringWithUTF8String:fragmentFunctionName.c_str()]];

            if (!vertexFunction) {
                NSLog(@"ofShader: Vertex function '%s' not found", vertexFunctionName.c_str());
                unload();
                return false;
            }

            if (!fragmentFunction) {
                NSLog(@"ofShader: Fragment function '%s' not found", fragmentFunctionName.c_str());
                unload();
                return false;
            }

            // Create pipeline state
            if (!createPipeline()) {
                unload();
                return false;
            }

            loaded = true;
            return true;
        }
    }

    bool loadFromSource(const std::string& source,
                        const std::string& vertName,
                        const std::string& fragName) {
        @autoreleasepool {
            unload();

            id<MTLDevice> device = getDevice();
            if (!device) {
                NSLog(@"ofShader: No Metal device available");
                return false;
            }

            vertexFunctionName = vertName;
            fragmentFunctionName = fragName;
            shaderPath = "<source>";

            // Compile source
            MTLCompileOptions* options = [[MTLCompileOptions alloc] init];
            options.fastMathEnabled = YES;

            NSError* error = nil;
            library = [device newLibraryWithSource:[NSString stringWithUTF8String:source.c_str()]
                                           options:options
                                             error:&error];
            if (!library) {
                NSLog(@"ofShader: Compilation failed: %@", error.localizedDescription);
                return false;
            }

            // Get functions
            vertexFunction = [library newFunctionWithName:
                              [NSString stringWithUTF8String:vertexFunctionName.c_str()]];
            fragmentFunction = [library newFunctionWithName:
                                [NSString stringWithUTF8String:fragmentFunctionName.c_str()]];

            if (!vertexFunction || !fragmentFunction) {
                NSLog(@"ofShader: Function not found");
                unload();
                return false;
            }

            // Create pipeline state
            if (!createPipeline()) {
                unload();
                return false;
            }

            loaded = true;
            return true;
        }
    }

    bool createPipeline() {
        @autoreleasepool {
            id<MTLDevice> device = getDevice();
            if (!device || !vertexFunction || !fragmentFunction) {
                return false;
            }

            MTLRenderPipelineDescriptor* desc = [[MTLRenderPipelineDescriptor alloc] init];
            desc.vertexFunction = vertexFunction;
            desc.fragmentFunction = fragmentFunction;

            // Configure for standard BGRA output (matching MTKView)
            desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
            desc.colorAttachments[0].blendingEnabled = YES;
            desc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
            desc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
            desc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
            desc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

            // Depth format (optional)
            desc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;

            NSError* error = nil;
            pipelineState = [device newRenderPipelineStateWithDescriptor:desc error:&error];

            if (!pipelineState) {
                NSLog(@"ofShader: Pipeline creation failed: %@", error.localizedDescription);
                return false;
            }

            return true;
        }
    }

    void begin() {
        if (!loaded || active) {
            return;
        }

        auto& ctx = Context::instance();
        auto& drawList = ctx.getDrawList();

        // Add custom shader command
        render::SetCustomShaderCommand cmd;
        cmd.pipelineState = (__bridge void*)pipelineState;
        cmd.uniformData = uniformBuffer.data();
        cmd.uniformSize = uniformBuffer.size();

        // Copy texture bindings
        for (const auto& [name, binding] : textureBindings) {
            if (binding.textureHandle) {
                cmd.textures[binding.index] = binding.textureHandle;
            }
        }

        drawList.addCommand(cmd);

        active = true;
        uniformsDirty = false;
    }

    void end() {
        if (!active) {
            return;
        }

        auto& ctx = Context::instance();
        auto& drawList = ctx.getDrawList();

        // Restore default shader
        render::SetCustomShaderCommand cmd;
        cmd.pipelineState = nullptr;  // nullptr means use default
        drawList.addCommand(cmd);

        active = false;
    }

    // Uniform setters
    template<typename T>
    void setUniform(const std::string& name, const T& value) {
        // Find or allocate offset for this uniform
        auto it = uniformOffsets.find(name);
        size_t offset;

        if (it == uniformOffsets.end()) {
            // Align to 16 bytes for Metal requirements
            size_t currentSize = 0;
            for (const auto& [_, off] : uniformOffsets) {
                currentSize = std::max(currentSize, off + sizeof(T));
            }
            offset = (currentSize + 15) & ~15;  // Align to 16

            // Grow buffer if needed
            if (offset + sizeof(T) > uniformBufferSize) {
                size_t newSize = uniformBufferSize * 2;
                while (offset + sizeof(T) > newSize) {
                    newSize *= 2;
                }
                uniformBuffer.resize(newSize, 0);
                uniformBufferSize = newSize;
            }

            uniformOffsets[name] = offset;
        } else {
            offset = it->second;
        }

        // Copy value
        std::memcpy(uniformBuffer.data() + offset, &value, sizeof(T));
        uniformsDirty = true;
    }

    void setTexture(const std::string& name, void* texture, int index) {
        textureBindings[name] = {texture, index};
    }
};

// ============================================================================
// ofShader Implementation
// ============================================================================

ofShader::ofShader()
    : impl_(std::make_unique<Impl>()) {
}

ofShader::~ofShader() = default;

ofShader::ofShader(ofShader&& other) noexcept = default;

ofShader& ofShader::operator=(ofShader&& other) noexcept = default;

void ofShader::ensureImpl() {
    if (!impl_) {
        impl_ = std::make_unique<Impl>();
    }
}

// Loading
bool ofShader::load(const std::string& shaderName) {
    return load(shaderName, "vertex_main", "fragment_main");
}

bool ofShader::load(const std::string& shaderName,
                    const std::string& vertexFunctionName,
                    const std::string& fragmentFunctionName) {
    ensureImpl();
    return impl_->loadFromFile(shaderName, vertexFunctionName, fragmentFunctionName);
}

bool ofShader::loadFromSource(const std::string& source,
                              const std::string& vertexFunctionName,
                              const std::string& fragmentFunctionName) {
    ensureImpl();
    return impl_->loadFromSource(source, vertexFunctionName, fragmentFunctionName);
}

void ofShader::unload() {
    if (impl_) {
        impl_->unload();
    }
}

bool ofShader::isLoaded() const {
    return impl_ && impl_->loaded;
}

// Activation
void ofShader::begin() {
    ensureImpl();
    impl_->begin();
}

void ofShader::end() {
    if (impl_) {
        impl_->end();
    }
}

bool ofShader::isActive() const {
    return impl_ && impl_->active;
}

// Uniform setters - Scalars
void ofShader::setUniform1f(const std::string& name, float v) {
    ensureImpl();
    impl_->setUniform(name, v);
}

void ofShader::setUniform1i(const std::string& name, int v) {
    ensureImpl();
    impl_->setUniform(name, v);
}

void ofShader::setUniform1b(const std::string& name, bool v) {
    setUniform1i(name, v ? 1 : 0);
}

// Uniform setters - Vectors
void ofShader::setUniform2f(const std::string& name, float x, float y) {
    ensureImpl();
    simd_float2 v = simd_make_float2(x, y);
    impl_->setUniform(name, v);
}

void ofShader::setUniform2f(const std::string& name, const ofVec2f& v) {
    setUniform2f(name, v.x, v.y);
}

void ofShader::setUniform3f(const std::string& name, float x, float y, float z) {
    ensureImpl();
    simd_float3 v = simd_make_float3(x, y, z);
    impl_->setUniform(name, v);
}

void ofShader::setUniform3f(const std::string& name, const ofVec3f& v) {
    setUniform3f(name, v.x, v.y, v.z);
}

void ofShader::setUniform4f(const std::string& name, float x, float y, float z, float w) {
    ensureImpl();
    simd_float4 v = simd_make_float4(x, y, z, w);
    impl_->setUniform(name, v);
}

void ofShader::setUniform4f(const std::string& name, const ofVec4f& v) {
    setUniform4f(name, v.x, v.y, v.z, v.w);
}

void ofShader::setUniform4f(const std::string& name, const ofFloatColor& c) {
    setUniform4f(name, c.r, c.g, c.b, c.a);
}

// Uniform setters - Integer Vectors
void ofShader::setUniform2i(const std::string& name, int x, int y) {
    ensureImpl();
    simd_int2 v = simd_make_int2(x, y);
    impl_->setUniform(name, v);
}

void ofShader::setUniform3i(const std::string& name, int x, int y, int z) {
    ensureImpl();
    simd_int3 v = simd_make_int3(x, y, z);
    impl_->setUniform(name, v);
}

void ofShader::setUniform4i(const std::string& name, int x, int y, int z, int w) {
    ensureImpl();
    simd_int4 v = simd_make_int4(x, y, z, w);
    impl_->setUniform(name, v);
}

// Uniform setters - Matrices
void ofShader::setUniformMatrix3f(const std::string& name, const float* m) {
    ensureImpl();
    simd_float3x3 mat;
    mat.columns[0] = simd_make_float3(m[0], m[1], m[2]);
    mat.columns[1] = simd_make_float3(m[3], m[4], m[5]);
    mat.columns[2] = simd_make_float3(m[6], m[7], m[8]);
    impl_->setUniform(name, mat);
}

void ofShader::setUniformMatrix4f(const std::string& name, const float* m) {
    ensureImpl();
    simd_float4x4 mat;
    mat.columns[0] = simd_make_float4(m[0], m[1], m[2], m[3]);
    mat.columns[1] = simd_make_float4(m[4], m[5], m[6], m[7]);
    mat.columns[2] = simd_make_float4(m[8], m[9], m[10], m[11]);
    mat.columns[3] = simd_make_float4(m[12], m[13], m[14], m[15]);
    impl_->setUniform(name, mat);
}

void ofShader::setUniformMatrix4f(const std::string& name, const ofMatrix4x4& m) {
    ensureImpl();
    impl_->setUniform(name, m.mat);
}

// Uniform setters - Arrays
void ofShader::setUniform1fv(const std::string& name, const float* values, size_t count) {
    ensureImpl();
    // Store as array in uniform buffer
    for (size_t i = 0; i < count; ++i) {
        std::string indexedName = name + "[" + std::to_string(i) + "]";
        impl_->setUniform(indexedName, values[i]);
    }
}

void ofShader::setUniform2fv(const std::string& name, const float* values, size_t count) {
    ensureImpl();
    for (size_t i = 0; i < count; ++i) {
        std::string indexedName = name + "[" + std::to_string(i) + "]";
        simd_float2 v = simd_make_float2(values[i*2], values[i*2+1]);
        impl_->setUniform(indexedName, v);
    }
}

void ofShader::setUniform3fv(const std::string& name, const float* values, size_t count) {
    ensureImpl();
    for (size_t i = 0; i < count; ++i) {
        std::string indexedName = name + "[" + std::to_string(i) + "]";
        simd_float3 v = simd_make_float3(values[i*3], values[i*3+1], values[i*3+2]);
        impl_->setUniform(indexedName, v);
    }
}

void ofShader::setUniform4fv(const std::string& name, const float* values, size_t count) {
    ensureImpl();
    for (size_t i = 0; i < count; ++i) {
        std::string indexedName = name + "[" + std::to_string(i) + "]";
        simd_float4 v = simd_make_float4(values[i*4], values[i*4+1], values[i*4+2], values[i*4+3]);
        impl_->setUniform(indexedName, v);
    }
}

// Texture setters
void ofShader::setUniformTexture(const std::string& name, const ofTexture& texture, int textureIndex) {
    // Get native handle from ofTexture
    void* handle = texture.getNativeHandle();
    setUniformTexture(name, handle, textureIndex);
}

void ofShader::setUniformTexture(const std::string& name, void* textureHandle, int textureIndex) {
    ensureImpl();
    impl_->setTexture(name, textureHandle, textureIndex);
}

// Information
std::string ofShader::getVertexFunctionName() const {
    return impl_ ? impl_->vertexFunctionName : "";
}

std::string ofShader::getFragmentFunctionName() const {
    return impl_ ? impl_->fragmentFunctionName : "";
}

std::string ofShader::getShaderPath() const {
    return impl_ ? impl_->shaderPath : "";
}

// Native access
void* ofShader::getNativePipelineState() const {
    if (!impl_ || !impl_->pipelineState) {
        return nullptr;
    }
    return (__bridge void*)impl_->pipelineState;
}

void* ofShader::getNativeVertexFunction() const {
    if (!impl_ || !impl_->vertexFunction) {
        return nullptr;
    }
    return (__bridge void*)impl_->vertexFunction;
}

void* ofShader::getNativeFragmentFunction() const {
    if (!impl_ || !impl_->fragmentFunction) {
        return nullptr;
    }
    return (__bridge void*)impl_->fragmentFunction;
}

} // namespace oflike
