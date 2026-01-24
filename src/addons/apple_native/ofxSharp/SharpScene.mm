#import "SharpScene.h"
#import "SharpRenderer.h"
#import "oflike/3d/ofCamera.h"
#import "oflike/math/ofMatrix4x4.h"
#import <Foundation/Foundation.h>
#import <simd/simd.h>
#import <map>
#import <fstream>

namespace Sharp {

// ============================================================================
// SceneObject Implementation
// ============================================================================

oflike::float4x4 SceneObject::getMatrix() const {
    using namespace oflike;

    // Create transformation matrix: T * R * S
    float4x4 translationMatrix = matrix_identity_float4x4;
    translationMatrix.columns[3] = simd_make_float4(position.x, position.y, position.z, 1.0f);

    float4x4 rotationMatrix = matrix_identity_float4x4;
    rotationMatrix = simd_matrix4x4(rotation);

    float4x4 scaleMatrix = matrix_identity_float4x4;
    scaleMatrix.columns[0].x = scale.x;
    scaleMatrix.columns[1].y = scale.y;
    scaleMatrix.columns[2].z = scale.z;

    return simd_mul(simd_mul(translationMatrix, rotationMatrix), scaleMatrix);
}

// ============================================================================
// SharpScene::Impl
// ============================================================================

struct SharpScene::Impl {
    // Scene objects
    struct Object {
        ObjectID id;
        GaussianCloud cloud;
        SceneObject metadata;
    };

    std::map<ObjectID, Object> objects_;
    ObjectID nextID_ = 0;

    // Find object by ID
    Object* findObject(ObjectID id) {
        auto it = objects_.find(id);
        return (it != objects_.end()) ? &it->second : nullptr;
    }

    const Object* findObject(ObjectID id) const {
        auto it = objects_.find(id);
        return (it != objects_.end()) ? &it->second : nullptr;
    }

    // Generate unique ID
    ObjectID generateID() {
        return nextID_++;
    }
};

// ============================================================================
// SharpScene Implementation
// ============================================================================

SharpScene::SharpScene()
    : impl_(std::make_unique<Impl>()) {
}

SharpScene::~SharpScene() = default;

SharpScene::SharpScene(SharpScene&& other) noexcept = default;
SharpScene& SharpScene::operator=(SharpScene&& other) noexcept = default;

// ============================================================================
// Object Management
// ============================================================================

ObjectID SharpScene::addCloud(GaussianCloud&& cloud, const std::string& name) {
    ObjectID id = impl_->generateID();

    Impl::Object obj;
    obj.id = id;
    obj.cloud = std::move(cloud);
    obj.metadata.id = id;
    obj.metadata.name = name.empty() ? "Object_" + std::to_string(id) : name;
    obj.metadata.position = {0.0f, 0.0f, 0.0f};
    obj.metadata.rotation = simd_quaternion(0.0f, 0.0f, 0.0f, 1.0f); // identity
    obj.metadata.scale = {1.0f, 1.0f, 1.0f};
    obj.metadata.visible = true;

    impl_->objects_[id] = std::move(obj);
    return id;
}

bool SharpScene::removeObject(ObjectID id) {
    auto it = impl_->objects_.find(id);
    if (it == impl_->objects_.end()) {
        return false;
    }
    impl_->objects_.erase(it);
    return true;
}

void SharpScene::clear() {
    impl_->objects_.clear();
}

size_t SharpScene::getObjectCount() const {
    return impl_->objects_.size();
}

bool SharpScene::hasObject(ObjectID id) const {
    return impl_->findObject(id) != nullptr;
}

std::vector<ObjectID> SharpScene::getObjectIDs() const {
    std::vector<ObjectID> ids;
    ids.reserve(impl_->objects_.size());
    for (const auto& pair : impl_->objects_) {
        ids.push_back(pair.first);
    }
    return ids;
}

const SceneObject* SharpScene::getObject(ObjectID id) const {
    const auto* obj = impl_->findObject(id);
    return obj ? &obj->metadata : nullptr;
}

const GaussianCloud* SharpScene::getCloud(ObjectID id) const {
    const auto* obj = impl_->findObject(id);
    return obj ? &obj->cloud : nullptr;
}

GaussianCloud* SharpScene::getCloud(ObjectID id) {
    auto* obj = impl_->findObject(id);
    return obj ? &obj->cloud : nullptr;
}

// ============================================================================
// Transformations
// ============================================================================

void SharpScene::setPosition(ObjectID id, const oflike::float3& position) {
    auto* obj = impl_->findObject(id);
    if (obj) {
        obj->metadata.position = position;
    }
}

oflike::float3 SharpScene::getPosition(ObjectID id) const {
    const auto* obj = impl_->findObject(id);
    return obj ? obj->metadata.position : oflike::float3{0, 0, 0};
}

void SharpScene::setRotation(ObjectID id, const oflike::quatf& rotation) {
    auto* obj = impl_->findObject(id);
    if (obj) {
        obj->metadata.rotation = rotation;
    }
}

void SharpScene::setRotation(ObjectID id, float angle, const oflike::float3& axis) {
    auto* obj = impl_->findObject(id);
    if (obj) {
        obj->metadata.rotation = simd_quaternion(angle, axis);
    }
}

oflike::quatf SharpScene::getRotation(ObjectID id) const {
    const auto* obj = impl_->findObject(id);
    return obj ? obj->metadata.rotation : simd_quaternion(0.0f, 0.0f, 0.0f, 1.0f);
}

void SharpScene::setScale(ObjectID id, float scale) {
    auto* obj = impl_->findObject(id);
    if (obj) {
        obj->metadata.scale = {scale, scale, scale};
    }
}

void SharpScene::setScale(ObjectID id, const oflike::float3& scale) {
    auto* obj = impl_->findObject(id);
    if (obj) {
        obj->metadata.scale = scale;
    }
}

oflike::float3 SharpScene::getScale(ObjectID id) const {
    const auto* obj = impl_->findObject(id);
    return obj ? obj->metadata.scale : oflike::float3{1, 1, 1};
}

void SharpScene::setTransform(ObjectID id, const oflike::float4x4& matrix) {
    auto* obj = impl_->findObject(id);
    if (!obj) return;

    // Decompose matrix into T, R, S
    // Translation: extract from column 3
    obj->metadata.position = {matrix.columns[3].x, matrix.columns[3].y, matrix.columns[3].z};

    // Extract scale
    oflike::float3 scaleX = {matrix.columns[0].x, matrix.columns[0].y, matrix.columns[0].z};
    oflike::float3 scaleY = {matrix.columns[1].x, matrix.columns[1].y, matrix.columns[1].z};
    oflike::float3 scaleZ = {matrix.columns[2].x, matrix.columns[2].y, matrix.columns[2].z};
    obj->metadata.scale = {
        simd_length(scaleX),
        simd_length(scaleY),
        simd_length(scaleZ)
    };

    // Extract rotation (normalize scale out)
    oflike::float4x4 rotMatrix = matrix;
    if (obj->metadata.scale.x != 0) rotMatrix.columns[0] /= obj->metadata.scale.x;
    if (obj->metadata.scale.y != 0) rotMatrix.columns[1] /= obj->metadata.scale.y;
    if (obj->metadata.scale.z != 0) rotMatrix.columns[2] /= obj->metadata.scale.z;
    rotMatrix.columns[3] = {0, 0, 0, 1};

    obj->metadata.rotation = simd_quaternion(rotMatrix);
}

oflike::float4x4 SharpScene::getTransform(ObjectID id) const {
    const auto* obj = impl_->findObject(id);
    if (!obj) {
        return matrix_identity_float4x4;
    }
    return obj->metadata.getMatrix();
}

// ============================================================================
// Visibility
// ============================================================================

void SharpScene::setVisible(ObjectID id, bool visible) {
    auto* obj = impl_->findObject(id);
    if (obj) {
        obj->metadata.visible = visible;
    }
}

bool SharpScene::isVisible(ObjectID id) const {
    const auto* obj = impl_->findObject(id);
    return obj ? obj->metadata.visible : false;
}

void SharpScene::showAll() {
    for (auto& pair : impl_->objects_) {
        pair.second.metadata.visible = true;
    }
}

void SharpScene::hideAll() {
    for (auto& pair : impl_->objects_) {
        pair.second.metadata.visible = false;
    }
}

// ============================================================================
// Naming
// ============================================================================

void SharpScene::setName(ObjectID id, const std::string& name) {
    auto* obj = impl_->findObject(id);
    if (obj) {
        obj->metadata.name = name;
    }
}

std::string SharpScene::getName(ObjectID id) const {
    const auto* obj = impl_->findObject(id);
    return obj ? obj->metadata.name : "";
}

ObjectID SharpScene::findByName(const std::string& name) const {
    for (const auto& pair : impl_->objects_) {
        if (pair.second.metadata.name == name) {
            return pair.first;
        }
    }
    return kInvalidObjectID;
}

// ============================================================================
// Rendering
// ============================================================================

void SharpScene::render(SharpRenderer& renderer, const oflike::ofCamera& camera) const {
    using namespace oflike;

    // Get view and projection matrices from camera
    float4x4 viewMatrix = camera.getModelViewMatrix().toSimd();
    float4x4 projectionMatrix = camera.getProjectionMatrix().toSimd();

    render(renderer, viewMatrix, projectionMatrix);
}

void SharpScene::render(SharpRenderer& renderer,
                        const oflike::float4x4& viewMatrix,
                        const oflike::float4x4& projectionMatrix) const {
    // NOTE: Rendering implementation requires renderer to support per-object transforms
    // For Phase 25.6, we document the intended behavior:
    // - Iterate through all visible objects
    // - For each object:
    //   1. Get object's transformation matrix
    //   2. Create transformed copy of cloud OR pass transform to renderer
    //   3. Render with renderer

    // This is a placeholder - full implementation requires renderer API updates
    // which will be completed in Phase 25.7 (Camera Path integration)

    // TODO(Phase 25.7): Implement scene rendering with per-object transforms
    // Options:
    // 1. Transform each cloud's Gaussians by object matrix (CPU-intensive)
    // 2. Pass object matrix to renderer as uniform (GPU-efficient)
    // 3. Batch render with instancing (most efficient)
}

void SharpScene::renderObject(ObjectID id, SharpRenderer& renderer, const oflike::ofCamera& camera) const {
    const auto* obj = impl_->findObject(id);
    if (!obj || !obj->metadata.visible) {
        return;
    }

    // TODO(Phase 25.7): Implement single object rendering
}

// ============================================================================
// Bounding Box
// ============================================================================

oflike::float3 SharpScene::getBoundsMin() const {
    if (impl_->objects_.empty()) {
        return {0, 0, 0};
    }

    oflike::float3 minBounds = {INFINITY, INFINITY, INFINITY};

    for (const auto& pair : impl_->objects_) {
        const auto& obj = pair.second;
        if (!obj.metadata.visible) continue;

        // Get cloud bounds in object space
        oflike::float3 cloudMin = obj.cloud.getBoundsMin();
        oflike::float3 cloudMax = obj.cloud.getBoundsMax();

        // Transform bounds to world space (simplified - just apply translation + scale)
        cloudMin = cloudMin * obj.metadata.scale + obj.metadata.position;
        cloudMax = cloudMax * obj.metadata.scale + obj.metadata.position;

        minBounds = simd_min(minBounds, cloudMin);
        minBounds = simd_min(minBounds, cloudMax);
    }

    return minBounds;
}

oflike::float3 SharpScene::getBoundsMax() const {
    if (impl_->objects_.empty()) {
        return {0, 0, 0};
    }

    oflike::float3 maxBounds = {-INFINITY, -INFINITY, -INFINITY};

    for (const auto& pair : impl_->objects_) {
        const auto& obj = pair.second;
        if (!obj.metadata.visible) continue;

        // Get cloud bounds in object space
        oflike::float3 cloudMin = obj.cloud.getBoundsMin();
        oflike::float3 cloudMax = obj.cloud.getBoundsMax();

        // Transform bounds to world space
        cloudMin = cloudMin * obj.metadata.scale + obj.metadata.position;
        cloudMax = cloudMax * obj.metadata.scale + obj.metadata.position;

        maxBounds = simd_max(maxBounds, cloudMin);
        maxBounds = simd_max(maxBounds, cloudMax);
    }

    return maxBounds;
}

oflike::float3 SharpScene::getCenter() const {
    oflike::float3 min = getBoundsMin();
    oflike::float3 max = getBoundsMax();
    return (min + max) * 0.5f;
}

oflike::float3 SharpScene::getSize() const {
    oflike::float3 min = getBoundsMin();
    oflike::float3 max = getBoundsMax();
    return max - min;
}

// ============================================================================
// Statistics
// ============================================================================

size_t SharpScene::getTotalGaussianCount() const {
    size_t total = 0;
    for (const auto& pair : impl_->objects_) {
        total += pair.second.cloud.size();
    }
    return total;
}

size_t SharpScene::getTotalMemoryUsage() const {
    size_t total = 0;
    for (const auto& pair : impl_->objects_) {
        total += pair.second.cloud.getMemoryUsage();
    }
    return total;
}

// ============================================================================
// Serialization
// ============================================================================

bool SharpScene::save(const std::string& filepath) const {
    @autoreleasepool {
        // Create JSON metadata
        NSMutableDictionary* sceneDict = [NSMutableDictionary dictionary];
        sceneDict[@"version"] = @"1.0.0";
        sceneDict[@"objectCount"] = @(impl_->objects_.size());

        NSMutableArray* objectsArray = [NSMutableArray array];

        // Save each object
        for (const auto& pair : impl_->objects_) {
            const auto& obj = pair.second;

            // Save cloud to PLY file
            std::string plyFilename = "object_" + std::to_string(obj.id) + ".ply";
            NSString* basePath = [NSString stringWithUTF8String:filepath.c_str()];
            NSString* dirPath = [basePath stringByDeletingLastPathComponent];
            NSString* plyPath = [dirPath stringByAppendingPathComponent:
                                 [NSString stringWithUTF8String:plyFilename.c_str()]];

            if (!obj.cloud.saveToPLY([plyPath UTF8String])) {
                return false;
            }

            // Save object metadata
            NSMutableDictionary* objDict = [NSMutableDictionary dictionary];
            objDict[@"id"] = @(obj.id);
            objDict[@"name"] = [NSString stringWithUTF8String:obj.metadata.name.c_str()];
            objDict[@"plyFile"] = [NSString stringWithUTF8String:plyFilename.c_str()];
            objDict[@"visible"] = @(obj.metadata.visible);

            // Position
            objDict[@"position"] = @[
                @(obj.metadata.position.x),
                @(obj.metadata.position.y),
                @(obj.metadata.position.z)
            ];

            // Rotation (quaternion)
            objDict[@"rotation"] = @[
                @(obj.metadata.rotation.vector.x),
                @(obj.metadata.rotation.vector.y),
                @(obj.metadata.rotation.vector.z),
                @(obj.metadata.rotation.vector.w)
            ];

            // Scale
            objDict[@"scale"] = @[
                @(obj.metadata.scale.x),
                @(obj.metadata.scale.y),
                @(obj.metadata.scale.z)
            ];

            [objectsArray addObject:objDict];
        }

        sceneDict[@"objects"] = objectsArray;

        // Write JSON to file
        NSError* error = nil;
        NSData* jsonData = [NSJSONSerialization dataWithJSONObject:sceneDict
                                                           options:NSJSONWritingPrettyPrinted
                                                             error:&error];
        if (error) {
            return false;
        }

        NSString* nsFilepath = [NSString stringWithUTF8String:filepath.c_str()];
        return [jsonData writeToFile:nsFilepath atomically:YES];
    }
}

bool SharpScene::load(const std::string& filepath) {
    @autoreleasepool {
        NSString* nsFilepath = [NSString stringWithUTF8String:filepath.c_str()];
        NSData* jsonData = [NSData dataWithContentsOfFile:nsFilepath];
        if (!jsonData) {
            return false;
        }

        NSError* error = nil;
        NSDictionary* sceneDict = [NSJSONSerialization JSONObjectWithData:jsonData
                                                                  options:0
                                                                    error:&error];
        if (error || !sceneDict) {
            return false;
        }

        // Clear existing scene
        clear();

        // Load objects
        NSArray* objectsArray = sceneDict[@"objects"];
        if (!objectsArray) {
            return false;
        }

        NSString* dirPath = [nsFilepath stringByDeletingLastPathComponent];

        for (NSDictionary* objDict in objectsArray) {
            // Load PLY file
            NSString* plyFilename = objDict[@"plyFile"];
            NSString* plyPath = [dirPath stringByAppendingPathComponent:plyFilename];

            GaussianCloud cloud;
            if (!cloud.loadFromPLY([plyPath UTF8String])) {
                return false;
            }

            // Add to scene
            NSString* name = objDict[@"name"];
            ObjectID id = addCloud(std::move(cloud), [name UTF8String]);

            // Restore metadata
            auto* obj = impl_->findObject(id);
            if (!obj) continue;

            obj->metadata.visible = [objDict[@"visible"] boolValue];

            // Position
            NSArray* pos = objDict[@"position"];
            if (pos.count == 3) {
                obj->metadata.position = {
                    [pos[0] floatValue],
                    [pos[1] floatValue],
                    [pos[2] floatValue]
                };
            }

            // Rotation
            NSArray* rot = objDict[@"rotation"];
            if (rot.count == 4) {
                obj->metadata.rotation = simd_quaternion(
                    [rot[0] floatValue],
                    [rot[1] floatValue],
                    [rot[2] floatValue],
                    [rot[3] floatValue]
                );
            }

            // Scale
            NSArray* scale = objDict[@"scale"];
            if (scale.count == 3) {
                obj->metadata.scale = {
                    [scale[0] floatValue],
                    [scale[1] floatValue],
                    [scale[2] floatValue]
                };
            }
        }

        return true;
    }
}

} // namespace Sharp
