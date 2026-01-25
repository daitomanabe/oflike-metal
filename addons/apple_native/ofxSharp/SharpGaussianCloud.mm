#import <Metal/Metal.h>
#import <simd/simd.h>
#include "SharpGaussianCloud.h"
#include "math/ofMatrix4x4.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <numeric>

namespace Sharp {

// ============================================================================
// Private Implementation
// ============================================================================

class GaussianCloud::Impl {
public:
    // Gaussian data
    std::vector<Gaussian> gaussians;

    // Bounding box cache
    oflike::float3 boundsMin{INFINITY, INFINITY, INFINITY};
    oflike::float3 boundsMax{-INFINITY, -INFINITY, -INFINITY};
    bool boundsDirty = true;

    // Metal resources
    id<MTLDevice> device = nil;
    id<MTLBuffer> gaussianBuffer = nil;
    bool bufferDirty = true;

    Impl() {
        @autoreleasepool {
            device = MTLCreateSystemDefaultDevice();
            if (!device) {
                // Fallback: no Metal support (shouldn't happen on modern macOS)
                NSLog(@"[ofxSharp] Warning: Metal device not available");
            }
        }
    }

    ~Impl() {
        @autoreleasepool {
            gaussianBuffer = nil;
            device = nil;
        }
    }

    void markDirty() {
        boundsDirty = true;
        bufferDirty = true;
    }

    void updateBoundsIfNeeded() {
        if (!boundsDirty || gaussians.empty()) {
            return;
        }

        boundsMin = oflike::float3{INFINITY, INFINITY, INFINITY};
        boundsMax = oflike::float3{-INFINITY, -INFINITY, -INFINITY};

        for (const auto& g : gaussians) {
            // Consider Gaussian position + radius for bounds
            float radius = g.getRadius();
            oflike::float3 minPt = g.position - oflike::float3{radius, radius, radius};
            oflike::float3 maxPt = g.position + oflike::float3{radius, radius, radius};

            boundsMin = simd_min(boundsMin, minPt);
            boundsMax = simd_max(boundsMax, maxPt);
        }

        boundsDirty = false;
    }

    bool updateMetalBufferImpl() {
        if (!device || gaussians.empty()) {
            return false;
        }

        @autoreleasepool {
            size_t bufferSize = gaussians.size() * sizeof(Gaussian);

            // Create or resize buffer
            if (!gaussianBuffer || gaussianBuffer.length != bufferSize) {
                gaussianBuffer = [device newBufferWithLength:bufferSize
                                                     options:MTLResourceStorageModeShared];
                if (!gaussianBuffer) {
                    NSLog(@"[ofxSharp] Error: Failed to create Metal buffer");
                    return false;
                }
            }

            // Copy data to buffer
            memcpy(gaussianBuffer.contents, gaussians.data(), bufferSize);

            bufferDirty = false;
            return true;
        }
    }

    bool loadPLYImpl(const std::string& filepath) {
        // Simple PLY loader for Gaussian Splatting format
        // Expected PLY format:
        // - Header with element count
        // - Binary data with: x, y, z, scale_x, scale_y, scale_z, rot_0, rot_1, rot_2, rot_3, opacity, sh_dc_r, sh_dc_g, sh_dc_b, ...

        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }

        // Parse PLY header
        std::string line;
        size_t vertexCount = 0;
        bool isBinary = false;

        while (std::getline(file, line)) {
            if (line.find("element vertex") != std::string::npos) {
                std::istringstream iss(line);
                std::string dummy1, dummy2;
                iss >> dummy1 >> dummy2 >> vertexCount;
            } else if (line.find("format binary") != std::string::npos) {
                isBinary = true;
            } else if (line == "end_header") {
                break;
            }
        }

        if (vertexCount == 0 || !isBinary) {
            // For now, only support binary PLY
            return false;
        }

        // Reserve space
        gaussians.clear();
        gaussians.reserve(vertexCount);

        // Read binary data
        // Note: This is a simplified version. Real implementation should parse
        // property list from header and read accordingly.
        for (size_t i = 0; i < vertexCount; ++i) {
            Gaussian g;

            // Read position (3 floats)
            file.read(reinterpret_cast<char*>(&g.position.x), sizeof(float));
            file.read(reinterpret_cast<char*>(&g.position.y), sizeof(float));
            file.read(reinterpret_cast<char*>(&g.position.z), sizeof(float));

            // Read scale (3 floats)
            file.read(reinterpret_cast<char*>(&g.scale.x), sizeof(float));
            file.read(reinterpret_cast<char*>(&g.scale.y), sizeof(float));
            file.read(reinterpret_cast<char*>(&g.scale.z), sizeof(float));

            // Read rotation (4 floats - quaternion)
            float qx, qy, qz, qw;
            file.read(reinterpret_cast<char*>(&qx), sizeof(float));
            file.read(reinterpret_cast<char*>(&qy), sizeof(float));
            file.read(reinterpret_cast<char*>(&qz), sizeof(float));
            file.read(reinterpret_cast<char*>(&qw), sizeof(float));
            g.rotation = simd_quaternion(qx, qy, qz, qw);

            // Read opacity (1 float)
            file.read(reinterpret_cast<char*>(&g.opacity), sizeof(float));

            // Read sh_dc (3 floats - RGB)
            file.read(reinterpret_cast<char*>(&g.sh_dc.x), sizeof(float));
            file.read(reinterpret_cast<char*>(&g.sh_dc.y), sizeof(float));
            file.read(reinterpret_cast<char*>(&g.sh_dc.z), sizeof(float));

            // Read sh_rest (15 * 3 floats)
            // Simplified: skip for now or read zeros
            // In a real implementation, parse all SH coefficients

            gaussians.push_back(g);
        }

        markDirty();
        return true;
    }

    bool savePLYImpl(const std::string& filepath) const {
        // Simple PLY saver for Gaussian Splatting format

        std::ofstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }

        // Write PLY header
        file << "ply\n";
        file << "format binary_little_endian 1.0\n";
        file << "element vertex " << gaussians.size() << "\n";
        file << "property float x\n";
        file << "property float y\n";
        file << "property float z\n";
        file << "property float scale_x\n";
        file << "property float scale_y\n";
        file << "property float scale_z\n";
        file << "property float rot_0\n";
        file << "property float rot_1\n";
        file << "property float rot_2\n";
        file << "property float rot_3\n";
        file << "property float opacity\n";
        file << "property float sh_dc_r\n";
        file << "property float sh_dc_g\n";
        file << "property float sh_dc_b\n";
        file << "end_header\n";

        // Write binary data
        for (const auto& g : gaussians) {
            // Write position
            file.write(reinterpret_cast<const char*>(&g.position.x), sizeof(float));
            file.write(reinterpret_cast<const char*>(&g.position.y), sizeof(float));
            file.write(reinterpret_cast<const char*>(&g.position.z), sizeof(float));

            // Write scale
            file.write(reinterpret_cast<const char*>(&g.scale.x), sizeof(float));
            file.write(reinterpret_cast<const char*>(&g.scale.y), sizeof(float));
            file.write(reinterpret_cast<const char*>(&g.scale.z), sizeof(float));

            // Write rotation (quaternion components)
            float qx = g.rotation.vector.x;
            float qy = g.rotation.vector.y;
            float qz = g.rotation.vector.z;
            float qw = g.rotation.vector.w;
            file.write(reinterpret_cast<const char*>(&qx), sizeof(float));
            file.write(reinterpret_cast<const char*>(&qy), sizeof(float));
            file.write(reinterpret_cast<const char*>(&qz), sizeof(float));
            file.write(reinterpret_cast<const char*>(&qw), sizeof(float));

            // Write opacity
            file.write(reinterpret_cast<const char*>(&g.opacity), sizeof(float));

            // Write sh_dc
            file.write(reinterpret_cast<const char*>(&g.sh_dc.x), sizeof(float));
            file.write(reinterpret_cast<const char*>(&g.sh_dc.y), sizeof(float));
            file.write(reinterpret_cast<const char*>(&g.sh_dc.z), sizeof(float));
        }

        return true;
    }
};

// ============================================================================
// Public Interface Implementation
// ============================================================================

GaussianCloud::GaussianCloud()
    : impl_(std::make_unique<Impl>()) {
}

GaussianCloud::~GaussianCloud() = default;

GaussianCloud::GaussianCloud(GaussianCloud&& other) noexcept = default;
GaussianCloud& GaussianCloud::operator=(GaussianCloud&& other) noexcept = default;

// ============================================================================
// Data Management
// ============================================================================

void GaussianCloud::addGaussian(const Gaussian& gaussian) {
    impl_->gaussians.push_back(gaussian);
    impl_->markDirty();
}

void GaussianCloud::addGaussians(const std::vector<Gaussian>& gaussians) {
    impl_->gaussians.insert(impl_->gaussians.end(), gaussians.begin(), gaussians.end());
    impl_->markDirty();
}

void GaussianCloud::reserve(size_t count) {
    impl_->gaussians.reserve(count);
}

void GaussianCloud::clear() {
    impl_->gaussians.clear();
    impl_->markDirty();
}

size_t GaussianCloud::size() const {
    return impl_->gaussians.size();
}

bool GaussianCloud::empty() const {
    return impl_->gaussians.empty();
}

const Gaussian& GaussianCloud::getGaussian(size_t index) const {
    return impl_->gaussians.at(index);
}

Gaussian& GaussianCloud::getGaussian(size_t index) {
    impl_->bufferDirty = true;
    return impl_->gaussians.at(index);
}

const std::vector<Gaussian>& GaussianCloud::getGaussians() const {
    return impl_->gaussians;
}

// ============================================================================
// Spatial Transformations
// ============================================================================

void GaussianCloud::translate(const oflike::float3& offset) {
    for (auto& g : impl_->gaussians) {
        g.position += offset;
    }
    impl_->markDirty();
}

void GaussianCloud::rotate(const oflike::quatf& rotation) {
    oflike::float3 center = getCenter();
    rotateAround(rotation, center);
}

void GaussianCloud::rotateAround(const oflike::quatf& rotation, const oflike::float3& center) {
    for (auto& g : impl_->gaussians) {
        // Rotate position around center
        oflike::float3 offset = g.position - center;
        offset = simd_act(rotation, offset);
        g.position = center + offset;

        // Rotate Gaussian orientation
        g.rotation = simd_mul(rotation, g.rotation);
    }
    impl_->markDirty();
}

void GaussianCloud::scale(float factor) {
    scale(oflike::float3{factor, factor, factor});
}

void GaussianCloud::scale(const oflike::float3& factors) {
    oflike::float3 center = getCenter();
    for (auto& g : impl_->gaussians) {
        // Scale position relative to center
        oflike::float3 offset = g.position - center;
        offset *= factors;
        g.position = center + offset;

        // Scale Gaussian size
        g.scale *= factors;
    }
    impl_->markDirty();
}

void GaussianCloud::transform(const oflike::ofMatrix4x4& matrix) {
    // Convert ofMatrix4x4 to simd_float4x4
    simd_float4x4 m = matrix.getMatrix();

    // Extract rotation component for Gaussian orientations
    // Upper-left 3x3 is rotation+scale
    simd_float3x3 rotScale = simd_matrix3x3(m.columns[0].xyz, m.columns[1].xyz, m.columns[2].xyz);

    // Extract scale factors
    oflike::float3 scaleFactors = {
        simd_length(m.columns[0].xyz),
        simd_length(m.columns[1].xyz),
        simd_length(m.columns[2].xyz)
    };

    // Normalize to get pure rotation
    simd_float3x3 rotation = simd_matrix3x3(
        simd_normalize(m.columns[0].xyz),
        simd_normalize(m.columns[1].xyz),
        simd_normalize(m.columns[2].xyz)
    );
    oflike::quatf quat = simd_quaternion(rotation);

    for (auto& g : impl_->gaussians) {
        // Transform position
        simd_float4 pos4 = simd_make_float4(g.position.x, g.position.y, g.position.z, 1.0f);
        simd_float4 transformedPos = simd_mul(m, pos4);
        g.position = simd_make_float3(transformedPos.x, transformedPos.y, transformedPos.z);

        // Transform rotation
        g.rotation = simd_mul(quat, g.rotation);

        // Transform scale
        g.scale *= scaleFactors;
    }
    impl_->markDirty();
}

// ============================================================================
// Filtering & Culling
// ============================================================================

void GaussianCloud::filterByOpacity(float minOpacity) {
    auto& gaussians = impl_->gaussians;
    gaussians.erase(
        std::remove_if(gaussians.begin(), gaussians.end(),
                       [minOpacity](const Gaussian& g) { return g.opacity < minOpacity; }),
        gaussians.end()
    );
    impl_->markDirty();
}

void GaussianCloud::filterBySize(float minSize, float maxSize) {
    auto& gaussians = impl_->gaussians;
    gaussians.erase(
        std::remove_if(gaussians.begin(), gaussians.end(),
                       [minSize, maxSize](const Gaussian& g) {
                           float radius = g.getRadius();
                           return radius < minSize || radius > maxSize;
                       }),
        gaussians.end()
    );
    impl_->markDirty();
}

void GaussianCloud::filterByBounds(const oflike::float3& minBounds, const oflike::float3& maxBounds) {
    auto& gaussians = impl_->gaussians;
    gaussians.erase(
        std::remove_if(gaussians.begin(), gaussians.end(),
                       [&minBounds, &maxBounds](const Gaussian& g) {
                           return simd_any(g.position < minBounds) || simd_any(g.position > maxBounds);
                       }),
        gaussians.end()
    );
    impl_->markDirty();
}

void GaussianCloud::removeInvisible(float threshold) {
    filterByOpacity(threshold);
}

// ============================================================================
// Bounding Box
// ============================================================================

oflike::float3 GaussianCloud::getBoundsMin() const {
    impl_->updateBoundsIfNeeded();
    return impl_->boundsMin;
}

oflike::float3 GaussianCloud::getBoundsMax() const {
    impl_->updateBoundsIfNeeded();
    return impl_->boundsMax;
}

oflike::float3 GaussianCloud::getCenter() const {
    impl_->updateBoundsIfNeeded();
    return (impl_->boundsMin + impl_->boundsMax) * 0.5f;
}

oflike::float3 GaussianCloud::getSize() const {
    impl_->updateBoundsIfNeeded();
    return impl_->boundsMax - impl_->boundsMin;
}

void GaussianCloud::updateBounds() {
    impl_->boundsDirty = true;
    impl_->updateBoundsIfNeeded();
}

// ============================================================================
// Metal Buffer Management
// ============================================================================

bool GaussianCloud::updateMetalBuffer() {
    return impl_->updateMetalBufferImpl();
}

void* GaussianCloud::getMetalBuffer() const {
    return (__bridge void*)impl_->gaussianBuffer;
}

size_t GaussianCloud::getBufferSize() const {
    return impl_->gaussians.size() * sizeof(Gaussian);
}

bool GaussianCloud::isBufferDirty() const {
    return impl_->bufferDirty;
}

// ============================================================================
// Import / Export
// ============================================================================

bool GaussianCloud::loadFromPLY(const std::string& filepath) {
    return impl_->loadPLYImpl(filepath);
}

bool GaussianCloud::saveToPLY(const std::string& filepath) const {
    return impl_->savePLYImpl(filepath);
}

// ============================================================================
// Statistics
// ============================================================================

size_t GaussianCloud::getMemoryUsage() const {
    size_t cpuMemory = impl_->gaussians.capacity() * sizeof(Gaussian);
    size_t gpuMemory = impl_->gaussianBuffer ? impl_->gaussianBuffer.length : 0;
    return cpuMemory + gpuMemory;
}

float GaussianCloud::getAverageOpacity() const {
    if (impl_->gaussians.empty()) {
        return 0.0f;
    }
    float sum = std::accumulate(impl_->gaussians.begin(), impl_->gaussians.end(), 0.0f,
                                [](float acc, const Gaussian& g) { return acc + g.opacity; });
    return sum / impl_->gaussians.size();
}

float GaussianCloud::getAverageScale() const {
    if (impl_->gaussians.empty()) {
        return 0.0f;
    }
    float sum = std::accumulate(impl_->gaussians.begin(), impl_->gaussians.end(), 0.0f,
                                [](float acc, const Gaussian& g) { return acc + g.getRadius(); });
    return sum / impl_->gaussians.size();
}

} // namespace Sharp
