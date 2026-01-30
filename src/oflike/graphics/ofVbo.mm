#import "ofVbo.h"
#import "../3d/ofMesh.h"
#import "ofGraphics.h"
#import "../../core/Context.h"
#import "../../render/DrawList.h"
#import "../../render/DrawCommand.h"
#import "../../render/RenderTypes.h"
#import <Metal/Metal.h>
#import <simd/simd.h>
#include <cstring>

namespace oflike {

// ============================================================================
// ofVbo::Impl
// ============================================================================

struct ofVbo::Impl {
    // Metal buffers
    id<MTLBuffer> vertexBuffer = nil;
    id<MTLBuffer> normalBuffer = nil;
    id<MTLBuffer> colorBuffer = nil;
    id<MTLBuffer> texCoordBuffer = nil;
    id<MTLBuffer> indexBuffer = nil;

    // Counts
    int numVertices = 0;
    int numNormals = 0;
    int numColors = 0;
    int numTexCoords = 0;
    int numIndices = 0;

    // Usage hints (for future optimization)
    int vertexUsage = 0;
    int normalUsage = 0;
    int colorUsage = 0;
    int texCoordUsage = 0;
    int indexUsage = 0;

    // Bound state
    bool bound = false;

    // Helper: Get Metal device
    id<MTLDevice> getDevice() const {
        void* devicePtr = Context::instance().getMetalDevice();
        return devicePtr ? (__bridge id<MTLDevice>)devicePtr : nil;
    }

    // Create or resize a buffer
    id<MTLBuffer> createBuffer(const void* data, size_t size, int usage) {
        @autoreleasepool {
            id<MTLDevice> device = getDevice();
            if (!device || size == 0) {
                return nil;
            }

            // Metal storage mode based on usage hint
            // usage == 0 or GL_STATIC_DRAW → shared memory (CPU+GPU)
            // usage == 1 or GL_DYNAMIC_DRAW → shared memory with write combine
            MTLResourceOptions options = MTLResourceStorageModeShared;

            id<MTLBuffer> buffer = [device newBufferWithBytes:data
                                                       length:size
                                                      options:options];
            return buffer;
        }
    }

    // Update buffer contents
    void updateBuffer(id<MTLBuffer> buffer, int offset, const void* data, size_t size) {
        if (!buffer || !data || size == 0) {
            return;
        }

        void* contents = [buffer contents];
        if (contents) {
            std::memcpy(static_cast<uint8_t*>(contents) + offset, data, size);
        }
    }

    Impl() = default;

    ~Impl() {
        clear();
    }

    void clear() {
        @autoreleasepool {
            vertexBuffer = nil;
            normalBuffer = nil;
            colorBuffer = nil;
            texCoordBuffer = nil;
            indexBuffer = nil;

            numVertices = 0;
            numNormals = 0;
            numColors = 0;
            numTexCoords = 0;
            numIndices = 0;
        }
    }
};

// ============================================================================
// ofVbo Implementation
// ============================================================================

ofVbo::ofVbo()
    : impl_(std::make_unique<Impl>()) {
}

ofVbo::~ofVbo() = default;

ofVbo::ofVbo(ofVbo&& other) noexcept = default;

ofVbo& ofVbo::operator=(ofVbo&& other) noexcept = default;

void ofVbo::ensureImpl() {
    if (!impl_) {
        impl_ = std::make_unique<Impl>();
    }
}

// ============================================================================
// Vertex Data
// ============================================================================

void ofVbo::setVertexData(const float* vertices, int total, int usage) {
    ensureImpl();
    impl_->vertexUsage = usage;
    impl_->numVertices = total;

    size_t size = total * 3 * sizeof(float);
    impl_->vertexBuffer = impl_->createBuffer(vertices, size, usage);
}

void ofVbo::setVertexData(const ofVec3f* vertices, int total, int usage) {
    setVertexData(reinterpret_cast<const float*>(vertices), total, usage);
}

void ofVbo::setVertexData(const std::vector<ofVec3f>& vertices, int usage) {
    if (vertices.empty()) {
        clearVertices();
        return;
    }
    setVertexData(vertices.data(), static_cast<int>(vertices.size()), usage);
}

int ofVbo::getNumVertices() const {
    return impl_ ? impl_->numVertices : 0;
}

bool ofVbo::hasVertices() const {
    return impl_ && impl_->vertexBuffer != nil;
}

// ============================================================================
// Normal Data
// ============================================================================

void ofVbo::setNormalData(const float* normals, int total, int usage) {
    ensureImpl();
    impl_->normalUsage = usage;
    impl_->numNormals = total;

    size_t size = total * 3 * sizeof(float);
    impl_->normalBuffer = impl_->createBuffer(normals, size, usage);
}

void ofVbo::setNormalData(const ofVec3f* normals, int total, int usage) {
    setNormalData(reinterpret_cast<const float*>(normals), total, usage);
}

void ofVbo::setNormalData(const std::vector<ofVec3f>& normals, int usage) {
    if (normals.empty()) {
        clearNormals();
        return;
    }
    setNormalData(normals.data(), static_cast<int>(normals.size()), usage);
}

int ofVbo::getNumNormals() const {
    return impl_ ? impl_->numNormals : 0;
}

bool ofVbo::hasNormals() const {
    return impl_ && impl_->normalBuffer != nil;
}

// ============================================================================
// Color Data
// ============================================================================

void ofVbo::setColorData(const float* colors, int total, int usage) {
    ensureImpl();
    impl_->colorUsage = usage;
    impl_->numColors = total;

    size_t size = total * 4 * sizeof(float);
    impl_->colorBuffer = impl_->createBuffer(colors, size, usage);
}

void ofVbo::setColorData(const ofFloatColor* colors, int total, int usage) {
    setColorData(reinterpret_cast<const float*>(colors), total, usage);
}

void ofVbo::setColorData(const std::vector<ofFloatColor>& colors, int usage) {
    if (colors.empty()) {
        clearColors();
        return;
    }
    setColorData(colors.data(), static_cast<int>(colors.size()), usage);
}

int ofVbo::getNumColors() const {
    return impl_ ? impl_->numColors : 0;
}

bool ofVbo::hasColors() const {
    return impl_ && impl_->colorBuffer != nil;
}

// ============================================================================
// Texture Coordinate Data
// ============================================================================

void ofVbo::setTexCoordData(const float* texCoords, int total, int usage) {
    ensureImpl();
    impl_->texCoordUsage = usage;
    impl_->numTexCoords = total;

    size_t size = total * 2 * sizeof(float);
    impl_->texCoordBuffer = impl_->createBuffer(texCoords, size, usage);
}

void ofVbo::setTexCoordData(const ofVec2f* texCoords, int total, int usage) {
    setTexCoordData(reinterpret_cast<const float*>(texCoords), total, usage);
}

void ofVbo::setTexCoordData(const std::vector<ofVec2f>& texCoords, int usage) {
    if (texCoords.empty()) {
        clearTexCoords();
        return;
    }
    setTexCoordData(texCoords.data(), static_cast<int>(texCoords.size()), usage);
}

int ofVbo::getNumTexCoords() const {
    return impl_ ? impl_->numTexCoords : 0;
}

bool ofVbo::hasTexCoords() const {
    return impl_ && impl_->texCoordBuffer != nil;
}

// ============================================================================
// Index Data
// ============================================================================

void ofVbo::setIndexData(const unsigned int* indices, int total, int usage) {
    ensureImpl();
    impl_->indexUsage = usage;
    impl_->numIndices = total;

    size_t size = total * sizeof(unsigned int);
    impl_->indexBuffer = impl_->createBuffer(indices, size, usage);
}

void ofVbo::setIndexData(const std::vector<unsigned int>& indices, int usage) {
    if (indices.empty()) {
        clearIndices();
        return;
    }
    setIndexData(indices.data(), static_cast<int>(indices.size()), usage);
}

int ofVbo::getNumIndices() const {
    return impl_ ? impl_->numIndices : 0;
}

bool ofVbo::hasIndices() const {
    return impl_ && impl_->indexBuffer != nil;
}

// ============================================================================
// Update Data
// ============================================================================

void ofVbo::updateVertexData(int offset, const ofVec3f* vertices, int count) {
    if (!impl_ || !impl_->vertexBuffer || !vertices || count <= 0) {
        return;
    }
    size_t byteOffset = offset * 3 * sizeof(float);
    size_t size = count * 3 * sizeof(float);
    impl_->updateBuffer(impl_->vertexBuffer, byteOffset, vertices, size);
}

void ofVbo::updateNormalData(int offset, const ofVec3f* normals, int count) {
    if (!impl_ || !impl_->normalBuffer || !normals || count <= 0) {
        return;
    }
    size_t byteOffset = offset * 3 * sizeof(float);
    size_t size = count * 3 * sizeof(float);
    impl_->updateBuffer(impl_->normalBuffer, byteOffset, normals, size);
}

void ofVbo::updateColorData(int offset, const ofFloatColor* colors, int count) {
    if (!impl_ || !impl_->colorBuffer || !colors || count <= 0) {
        return;
    }
    size_t byteOffset = offset * 4 * sizeof(float);
    size_t size = count * 4 * sizeof(float);
    impl_->updateBuffer(impl_->colorBuffer, byteOffset, colors, size);
}

void ofVbo::updateTexCoordData(int offset, const ofVec2f* texCoords, int count) {
    if (!impl_ || !impl_->texCoordBuffer || !texCoords || count <= 0) {
        return;
    }
    size_t byteOffset = offset * 2 * sizeof(float);
    size_t size = count * 2 * sizeof(float);
    impl_->updateBuffer(impl_->texCoordBuffer, byteOffset, texCoords, size);
}

void ofVbo::updateIndexData(int offset, const unsigned int* indices, int count) {
    if (!impl_ || !impl_->indexBuffer || !indices || count <= 0) {
        return;
    }
    size_t byteOffset = offset * sizeof(unsigned int);
    size_t size = count * sizeof(unsigned int);
    impl_->updateBuffer(impl_->indexBuffer, byteOffset, indices, size);
}

// ============================================================================
// Drawing
// ============================================================================

void ofVbo::draw(int drawMode, int first, int total) const {
    if (!impl_ || !impl_->vertexBuffer || total <= 0) {
        return;
    }

    auto& ctx = Context::instance();
    auto& drawList = ctx.getDrawList();

    // Convert VBO data to DrawList vertices
    // This is a temporary solution - ideally we'd pass the MTLBuffer directly
    void* vertexContents = [impl_->vertexBuffer contents];
    const float* verts = static_cast<const float*>(vertexContents);

    void* colorContents = impl_->colorBuffer ? [impl_->colorBuffer contents] : nullptr;
    const float* colors = static_cast<const float*>(colorContents);

    void* texCoordContents = impl_->texCoordBuffer ? [impl_->texCoordBuffer contents] : nullptr;
    const float* texCoords = static_cast<const float*>(texCoordContents);

    // Build vertices for DrawList
    std::vector<render::Vertex3D> vertices;
    vertices.reserve(total);

    uint8_t r, g, b, a;
    ::ofGetColor(r, g, b, a);
    ofFloatColor currentColor(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);

    for (int i = first; i < first + total; ++i) {
        render::Vertex3D v;
        v.position = simd_make_float3(verts[i * 3], verts[i * 3 + 1], verts[i * 3 + 2]);

        if (colors) {
            v.color = simd_make_float4(colors[i * 4], colors[i * 4 + 1],
                                        colors[i * 4 + 2], colors[i * 4 + 3]);
        } else {
            v.color = simd_make_float4(currentColor.r, currentColor.g,
                                        currentColor.b, currentColor.a);
        }

        if (texCoords) {
            v.texCoord = simd_make_float2(texCoords[i * 2], texCoords[i * 2 + 1]);
        } else {
            v.texCoord = simd_make_float2(0, 0);
        }

        v.normal = simd_make_float3(0, 0, 1);  // Default normal

        vertices.push_back(v);
    }

    // Add vertices to draw list
    uint32_t vertexOffset = drawList.addVertices3D(vertices);

    // Create draw command
    render::DrawCommand3D cmd;
    cmd.vertexOffset = vertexOffset;
    cmd.vertexCount = static_cast<uint32_t>(total);

    // Set primitive type based on draw mode
    switch (drawMode) {
        case OF_MESH_POINTS:
            cmd.primitiveType = render::PrimitiveType::Point;
            break;
        case OF_MESH_WIREFRAME:
            cmd.primitiveType = render::PrimitiveType::Line;
            break;
        case OF_MESH_FILL:
        default:
            cmd.primitiveType = render::PrimitiveType::Triangle;
            break;
    }

    // Get current matrices
    cmd.modelViewMatrix = ctx.getViewMatrix();
    cmd.projectionMatrix = ctx.getProjectionMatrix();
    cmd.depthTestEnabled = true;  // Default to enabled
    cmd.depthWriteEnabled = true; // Default to enabled

    drawList.addCommand(cmd);
}

void ofVbo::drawElements(int drawMode, int first, int total) const {
    if (!impl_ || !impl_->vertexBuffer || !impl_->indexBuffer || total <= 0) {
        return;
    }

    auto& ctx = Context::instance();
    auto& drawList = ctx.getDrawList();

    // Get buffer contents
    void* vertexContents = [impl_->vertexBuffer contents];
    const float* verts = static_cast<const float*>(vertexContents);

    void* indexContents = [impl_->indexBuffer contents];
    const unsigned int* indices = static_cast<const unsigned int*>(indexContents);

    void* colorContents = impl_->colorBuffer ? [impl_->colorBuffer contents] : nullptr;
    const float* colors = static_cast<const float*>(colorContents);

    void* texCoordContents = impl_->texCoordBuffer ? [impl_->texCoordBuffer contents] : nullptr;
    const float* texCoords = static_cast<const float*>(texCoordContents);

    // Build vertices for DrawList (indexed)
    std::vector<render::Vertex3D> vertices;
    vertices.reserve(total);

    uint8_t r, g, b, a;
    ::ofGetColor(r, g, b, a);
    ofFloatColor currentColor(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);

    for (int i = first; i < first + total; ++i) {
        unsigned int idx = indices[i];
        render::Vertex3D v;
        v.position = simd_make_float3(verts[idx * 3], verts[idx * 3 + 1], verts[idx * 3 + 2]);

        if (colors) {
            v.color = simd_make_float4(colors[idx * 4], colors[idx * 4 + 1],
                                        colors[idx * 4 + 2], colors[idx * 4 + 3]);
        } else {
            v.color = simd_make_float4(currentColor.r, currentColor.g,
                                        currentColor.b, currentColor.a);
        }

        if (texCoords) {
            v.texCoord = simd_make_float2(texCoords[idx * 2], texCoords[idx * 2 + 1]);
        } else {
            v.texCoord = simd_make_float2(0, 0);
        }

        v.normal = simd_make_float3(0, 0, 1);

        vertices.push_back(v);
    }

    // Add vertices to draw list
    uint32_t vertexOffset = drawList.addVertices3D(vertices);

    // Create draw command
    render::DrawCommand3D cmd;
    cmd.vertexOffset = vertexOffset;
    cmd.vertexCount = static_cast<uint32_t>(total);

    switch (drawMode) {
        case OF_MESH_POINTS:
            cmd.primitiveType = render::PrimitiveType::Point;
            break;
        case OF_MESH_WIREFRAME:
            cmd.primitiveType = render::PrimitiveType::Line;
            break;
        case OF_MESH_FILL:
        default:
            cmd.primitiveType = render::PrimitiveType::Triangle;
            break;
    }

    cmd.modelViewMatrix = ctx.getViewMatrix();
    cmd.projectionMatrix = ctx.getProjectionMatrix();
    cmd.depthTestEnabled = true;  // Default to enabled
    cmd.depthWriteEnabled = true; // Default to enabled

    drawList.addCommand(cmd);
}

void ofVbo::drawInstanced(int drawMode, int first, int total, int instanceCount) const {
    // For now, just draw multiple times
    // TODO: Implement true instanced rendering with MTLRenderCommandEncoder
    for (int i = 0; i < instanceCount; ++i) {
        draw(drawMode, first, total);
    }
}

// ============================================================================
// Mesh Integration
// ============================================================================

void ofVbo::setMesh(const ofMesh& mesh, int usage) {
    // Set vertices
    if (mesh.getNumVertices() > 0) {
        setVertexData(mesh.getVertices(), usage);
    }

    // Set normals
    if (mesh.hasNormals()) {
        setNormalData(mesh.getNormals(), usage);
    }

    // Set colors - convert ofColor to ofFloatColor
    if (mesh.hasColors()) {
        const auto& colors = mesh.getColors();
        std::vector<ofFloatColor> floatColors;
        floatColors.reserve(colors.size());
        for (const auto& c : colors) {
            floatColors.push_back(ofFloatColor(
                c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f
            ));
        }
        setColorData(floatColors, usage);
    }

    // Set texture coordinates
    if (mesh.hasTexCoords()) {
        setTexCoordData(mesh.getTexCoords(), usage);
    }

    // Set indices
    if (mesh.hasIndices()) {
        setIndexData(mesh.getIndices(), usage);
    }
}

// ============================================================================
// Clear / Reset
// ============================================================================

void ofVbo::clear() {
    if (impl_) {
        impl_->clear();
    }
}

void ofVbo::clearVertices() {
    if (impl_) {
        impl_->vertexBuffer = nil;
        impl_->numVertices = 0;
    }
}

void ofVbo::clearNormals() {
    if (impl_) {
        impl_->normalBuffer = nil;
        impl_->numNormals = 0;
    }
}

void ofVbo::clearColors() {
    if (impl_) {
        impl_->colorBuffer = nil;
        impl_->numColors = 0;
    }
}

void ofVbo::clearTexCoords() {
    if (impl_) {
        impl_->texCoordBuffer = nil;
        impl_->numTexCoords = 0;
    }
}

void ofVbo::clearIndices() {
    if (impl_) {
        impl_->indexBuffer = nil;
        impl_->numIndices = 0;
    }
}

// ============================================================================
// Binding
// ============================================================================

void ofVbo::bind() const {
    if (impl_) {
        impl_->bound = true;
    }
}

void ofVbo::unbind() const {
    if (impl_) {
        impl_->bound = false;
    }
}

// ============================================================================
// Native Access
// ============================================================================

void* ofVbo::getNativeVertexBuffer() const {
    if (!impl_ || !impl_->vertexBuffer) {
        return nullptr;
    }
    return (__bridge void*)impl_->vertexBuffer;
}

void* ofVbo::getNativeNormalBuffer() const {
    if (!impl_ || !impl_->normalBuffer) {
        return nullptr;
    }
    return (__bridge void*)impl_->normalBuffer;
}

void* ofVbo::getNativeColorBuffer() const {
    if (!impl_ || !impl_->colorBuffer) {
        return nullptr;
    }
    return (__bridge void*)impl_->colorBuffer;
}

void* ofVbo::getNativeTexCoordBuffer() const {
    if (!impl_ || !impl_->texCoordBuffer) {
        return nullptr;
    }
    return (__bridge void*)impl_->texCoordBuffer;
}

void* ofVbo::getNativeIndexBuffer() const {
    if (!impl_ || !impl_->indexBuffer) {
        return nullptr;
    }
    return (__bridge void*)impl_->indexBuffer;
}

} // namespace oflike
