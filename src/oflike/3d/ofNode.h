#pragma once

// oflike-metal ofNode - openFrameworks API compatible scene graph node
// Base class for objects with position, orientation, and scale in 3D space

#include "../math/ofVec3f.h"
#include "../math/ofMatrix4x4.h"
#include "../math/ofQuaternion.h"
#include <vector>

namespace oflike {

/// \brief Scene graph node with position, orientation, and scale
/// \details ofNode provides openFrameworks-compatible 3D transformations with
/// parent-child hierarchy support. Child nodes inherit their parent's transforms,
/// allowing complex scene hierarchies.
///
/// Features:
/// - Position, rotation (quaternion), scale
/// - Parent-child hierarchy with transform inheritance
/// - Local and global transform matrices
/// - lookAt for easy orientation
///
/// Example:
/// \code
///     ofNode parent;
///     ofNode child;
///     child.setParent(parent);
///
///     parent.setPosition(100, 0, 0);
///     child.setPosition(50, 0, 0);  // Local position
///
///     // child.getGlobalPosition() == (150, 0, 0)
/// \endcode
class ofNode {
public:
    // ========================================================================
    // Construction / Destruction
    // ========================================================================

    /// \brief Default constructor
    /// Creates a node at origin with no rotation and uniform scale of 1
    ofNode();

    /// \brief Virtual destructor for inheritance
    virtual ~ofNode();

    // ========================================================================
    // Position
    // ========================================================================

    /// \brief Set local position
    /// \param x X coordinate
    /// \param y Y coordinate
    /// \param z Z coordinate
    void setPosition(float x, float y, float z);

    /// \brief Set local position
    /// \param p Position vector
    void setPosition(const ofVec3f& p);

    /// \brief Get local position
    /// \return Local position relative to parent
    ofVec3f getPosition() const;

    /// \brief Get global position (world space)
    /// \return Position in world coordinates
    ofVec3f getGlobalPosition() const;

    /// \brief Set global position (world space)
    /// \param p World position
    void setGlobalPosition(const ofVec3f& p);

    /// \brief Set global position (world space)
    /// \param x X coordinate
    /// \param y Y coordinate
    /// \param z Z coordinate
    void setGlobalPosition(float x, float y, float z);

    /// \brief Move by offset
    /// \param x X offset
    /// \param y Y offset
    /// \param z Z offset
    void move(float x, float y, float z);

    /// \brief Move by offset vector
    /// \param offset Offset vector
    void move(const ofVec3f& offset);

    // ========================================================================
    // Orientation
    // ========================================================================

    /// \brief Set orientation as quaternion
    /// \param q Orientation quaternion
    void setOrientation(const ofQuaternion& q);

    /// \brief Get orientation as quaternion
    /// \return Local orientation
    ofQuaternion getOrientation() const;

    /// \brief Get global orientation (world space)
    /// \return Orientation in world coordinates
    ofQuaternion getGlobalOrientation() const;

    /// \brief Set global orientation (world space)
    /// \param q World orientation
    void setGlobalOrientation(const ofQuaternion& q);

    /// \brief Set orientation from Euler angles (degrees)
    /// \param x Rotation around X axis
    /// \param y Rotation around Y axis
    /// \param z Rotation around Z axis
    void setOrientation(float x, float y, float z);

    /// \brief Get orientation as Euler angles (degrees)
    /// \return Euler angles (x, y, z)
    ofVec3f getOrientationEuler() const;

    /// \brief Point the node at a target position
    /// \param target Target position to look at
    /// \param up Up direction (default: Y-up)
    void lookAt(const ofVec3f& target, const ofVec3f& up = ofVec3f(0, 1, 0));

    /// \brief Rotate around an axis
    /// \param angle Rotation angle in degrees
    /// \param axis Rotation axis
    void rotate(float angle, const ofVec3f& axis);

    /// \brief Rotate around an axis
    /// \param angle Rotation angle in degrees
    /// \param x Axis X component
    /// \param y Axis Y component
    /// \param z Axis Z component
    void rotate(float angle, float x, float y, float z);

    /// \brief Rotate around local X axis
    /// \param angle Angle in degrees
    void rotateX(float angle);

    /// \brief Rotate around local Y axis
    /// \param angle Angle in degrees
    void rotateY(float angle);

    /// \brief Rotate around local Z axis
    /// \param angle Angle in degrees
    void rotateZ(float angle);

    /// \brief Rotate around a point
    /// \param angle Angle in degrees
    /// \param axis Rotation axis
    /// \param point Point to rotate around
    void rotateAround(float angle, const ofVec3f& axis, const ofVec3f& point);

    // ========================================================================
    // Scale
    // ========================================================================

    /// \brief Set uniform scale
    /// \param scale Scale factor
    void setScale(float scale);

    /// \brief Set non-uniform scale
    /// \param x X scale
    /// \param y Y scale
    /// \param z Z scale
    void setScale(float x, float y, float z);

    /// \brief Set scale from vector
    /// \param s Scale vector
    void setScale(const ofVec3f& s);

    /// \brief Get local scale
    /// \return Scale vector
    ofVec3f getScale() const;

    /// \brief Get global scale (including parent scales)
    /// \return Global scale vector
    ofVec3f getGlobalScale() const;

    // ========================================================================
    // Transform Matrices
    // ========================================================================

    /// \brief Get local transform matrix
    /// \return 4x4 transformation matrix (position, rotation, scale)
    ofMatrix4x4 getLocalTransformMatrix() const;

    /// \brief Get global transform matrix (world space)
    /// \return 4x4 world transformation matrix
    ofMatrix4x4 getGlobalTransformMatrix() const;

    /// \brief Reset all transforms to default
    void resetTransform();

    // ========================================================================
    // Direction Vectors
    // ========================================================================

    /// \brief Get forward direction (-Z in local space)
    /// \return Forward direction in world space
    ofVec3f getForwardDir() const;

    /// \brief Get up direction (+Y in local space)
    /// \return Up direction in world space
    ofVec3f getUpDir() const;

    /// \brief Get right direction (+X in local space)
    /// \return Right direction in world space
    ofVec3f getRightDir() const;

    // ========================================================================
    // Parent-Child Hierarchy
    // ========================================================================

    /// \brief Set parent node
    /// \param parent Parent node (nullptr to clear parent)
    /// \param keepGlobalTransform If true, adjust local transform to maintain global position
    void setParent(ofNode* parent, bool keepGlobalTransform = false);

    /// \brief Set parent node (reference version)
    /// \param parent Parent node
    /// \param keepGlobalTransform If true, adjust local transform to maintain global position
    void setParent(ofNode& parent, bool keepGlobalTransform = false);

    /// \brief Clear parent
    /// \param keepGlobalTransform If true, adjust local transform to maintain global position
    void clearParent(bool keepGlobalTransform = false);

    /// \brief Get parent node
    /// \return Pointer to parent, or nullptr if no parent
    ofNode* getParent() const;

    /// \brief Check if this node has a parent
    /// \return true if has parent
    bool hasParent() const;

    /// \brief Get all child nodes
    /// \return Vector of child node pointers
    const std::vector<ofNode*>& getChildren() const;

    /// \brief Get number of children
    /// \return Number of child nodes
    size_t getNumChildren() const;

    // ========================================================================
    // Callbacks (for subclass customization)
    // ========================================================================

    /// \brief Called when transform changes
    /// \details Override in subclasses to respond to transform updates
    virtual void onTransformChanged();

protected:
    // ========================================================================
    // Internal State
    // ========================================================================

    /// Local position
    ofVec3f position_;

    /// Local orientation (quaternion)
    ofQuaternion orientation_;

    /// Local scale
    ofVec3f scale_;

    /// Parent node (nullptr if root)
    ofNode* parent_;

    /// Child nodes
    std::vector<ofNode*> children_;

    // ========================================================================
    // Internal Methods
    // ========================================================================

    /// \brief Add a child node (called by setParent)
    void addChild(ofNode* child);

    /// \brief Remove a child node (called by clearParent)
    void removeChild(ofNode* child);

    /// \brief Notify transform changed (calls onTransformChanged and propagates to children)
    void notifyTransformChanged();
};

} // namespace oflike
