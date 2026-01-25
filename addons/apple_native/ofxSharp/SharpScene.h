#pragma once

// SharpScene - Scene management for multiple Gaussian clouds
//
// This class provides a high-level scene graph for managing multiple 3D Gaussian Splatting
// clouds with independent transformations. Features include:
// - Multiple object management with unique IDs
// - Per-object transformation (position, rotation, scale)
// - Scene serialization (save/load)
// - Batch rendering
// - Visibility control
// - Bounding box computation
//
// Usage:
//   Sharp::SharpScene scene;
//   int id1 = scene.addCloud(cloud1);
//   int id2 = scene.addCloud(cloud2);
//   scene.setPosition(id1, {0, 0, 0});
//   scene.setRotation(id2, M_PI/2, {0, 1, 0});
//   scene.render(renderer, camera);
//   scene.save("scene.sharp");

#include "SharpGaussianCloud.h"
#include "math/Types.h"
#include <string>
#include <memory>
#include <vector>

// Forward declarations
namespace oflike {
    class ofMatrix4x4;
    class ofCamera;
}

namespace Sharp {

// Forward declaration
class SharpRenderer;

// Scene object handle
using ObjectID = int;

// Invalid object ID constant
constexpr ObjectID kInvalidObjectID = -1;

// Scene object metadata
struct SceneObject {
    ObjectID id = kInvalidObjectID;
    std::string name;
    oflike::float3 position = {0.0f, 0.0f, 0.0f};
    oflike::quatf rotation; // identity quaternion (initialized in constructor)
    oflike::float3 scale = {1.0f, 1.0f, 1.0f};
    bool visible = true;

    // Constructor
    SceneObject() : rotation(simd_quaternion(0.0f, 0.0f, 0.0f, 1.0f)) {}

    // Get transformation matrix
    oflike::float4x4 getMatrix() const;
};

class SharpScene {
public:
    // ============================================================================
    // Constructors / Destructor
    // ============================================================================

    SharpScene();
    ~SharpScene();

    // Move semantics
    SharpScene(SharpScene&& other) noexcept;
    SharpScene& operator=(SharpScene&& other) noexcept;

    // Disable copy (contains unique GaussianCloud objects)
    SharpScene(const SharpScene&) = delete;
    SharpScene& operator=(const SharpScene&) = delete;

    // ============================================================================
    // Object Management
    // ============================================================================

    /**
     * Add a Gaussian cloud to the scene.
     * The cloud is moved into the scene (ownership transferred).
     * @param cloud Gaussian cloud to add
     * @param name Optional name for the object
     * @return Object ID for the added cloud
     */
    ObjectID addCloud(GaussianCloud&& cloud, const std::string& name = "");

    /**
     * Remove an object from the scene.
     * @param id Object ID to remove
     * @return true if object was found and removed
     */
    bool removeObject(ObjectID id);

    /**
     * Remove all objects from the scene.
     */
    void clear();

    /**
     * Get number of objects in the scene.
     */
    size_t getObjectCount() const;

    /**
     * Check if object exists in the scene.
     */
    bool hasObject(ObjectID id) const;

    /**
     * Get all object IDs in the scene.
     */
    std::vector<ObjectID> getObjectIDs() const;

    /**
     * Get object metadata.
     * @param id Object ID
     * @return Object metadata, or nullptr if not found
     */
    const SceneObject* getObject(ObjectID id) const;

    /**
     * Get Gaussian cloud for an object.
     * @param id Object ID
     * @return Pointer to GaussianCloud, or nullptr if not found
     */
    const GaussianCloud* getCloud(ObjectID id) const;
    GaussianCloud* getCloud(ObjectID id);

    // ============================================================================
    // Transformations
    // ============================================================================

    /**
     * Set object position.
     */
    void setPosition(ObjectID id, const oflike::float3& position);

    /**
     * Get object position.
     */
    oflike::float3 getPosition(ObjectID id) const;

    /**
     * Set object rotation (quaternion).
     */
    void setRotation(ObjectID id, const oflike::quatf& rotation);

    /**
     * Set object rotation (axis-angle).
     * @param id Object ID
     * @param angle Rotation angle in radians
     * @param axis Rotation axis (normalized)
     */
    void setRotation(ObjectID id, float angle, const oflike::float3& axis);

    /**
     * Get object rotation.
     */
    oflike::quatf getRotation(ObjectID id) const;

    /**
     * Set object scale (uniform).
     */
    void setScale(ObjectID id, float scale);

    /**
     * Set object scale (non-uniform).
     */
    void setScale(ObjectID id, const oflike::float3& scale);

    /**
     * Get object scale.
     */
    oflike::float3 getScale(ObjectID id) const;

    /**
     * Set object transformation matrix directly.
     */
    void setTransform(ObjectID id, const oflike::float4x4& matrix);

    /**
     * Get object transformation matrix.
     */
    oflike::float4x4 getTransform(ObjectID id) const;

    // ============================================================================
    // Visibility
    // ============================================================================

    /**
     * Set object visibility.
     */
    void setVisible(ObjectID id, bool visible);

    /**
     * Get object visibility.
     */
    bool isVisible(ObjectID id) const;

    /**
     * Show all objects.
     */
    void showAll();

    /**
     * Hide all objects.
     */
    void hideAll();

    // ============================================================================
    // Naming
    // ============================================================================

    /**
     * Set object name.
     */
    void setName(ObjectID id, const std::string& name);

    /**
     * Get object name.
     */
    std::string getName(ObjectID id) const;

    /**
     * Find object by name.
     * @return Object ID, or kInvalidObjectID if not found
     */
    ObjectID findByName(const std::string& name) const;

    // ============================================================================
    // Rendering
    // ============================================================================

    /**
     * Render entire scene using the given renderer and camera.
     * This applies per-object transformations and renders all visible objects.
     * @param renderer SharpRenderer instance
     * @param camera Camera for view/projection matrices
     */
    void render(SharpRenderer& renderer, const oflike::ofCamera& camera) const;

    /**
     * Render entire scene with custom view/projection matrices.
     */
    void render(SharpRenderer& renderer,
                const oflike::float4x4& viewMatrix,
                const oflike::float4x4& projectionMatrix) const;

    /**
     * Render a single object.
     */
    void renderObject(ObjectID id, SharpRenderer& renderer, const oflike::ofCamera& camera) const;

    // ============================================================================
    // Bounding Box
    // ============================================================================

    /**
     * Get scene bounding box minimum point (world space).
     */
    oflike::float3 getBoundsMin() const;

    /**
     * Get scene bounding box maximum point (world space).
     */
    oflike::float3 getBoundsMax() const;

    /**
     * Get scene center (world space).
     */
    oflike::float3 getCenter() const;

    /**
     * Get scene bounding box size.
     */
    oflike::float3 getSize() const;

    // ============================================================================
    // Statistics
    // ============================================================================

    /**
     * Get total number of Gaussians in the scene (all objects).
     */
    size_t getTotalGaussianCount() const;

    /**
     * Get total memory usage (CPU + GPU) for all objects.
     */
    size_t getTotalMemoryUsage() const;

    // ============================================================================
    // Serialization
    // ============================================================================

    /**
     * Save scene to file.
     * File format: .sharp (custom binary format with JSON metadata)
     * Includes:
     * - All Gaussian clouds (PLY format embedded)
     * - Object transformations
     * - Object names and visibility
     * @param filepath Path to save file
     * @return true if successful
     */
    bool save(const std::string& filepath) const;

    /**
     * Load scene from file.
     * Clears existing scene content before loading.
     * @param filepath Path to load file
     * @return true if successful
     */
    bool load(const std::string& filepath);

private:
    // pImpl pattern to hide implementation details
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace Sharp
