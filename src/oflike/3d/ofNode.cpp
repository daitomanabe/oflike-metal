#include "ofNode.h"
#include <algorithm>

namespace oflike {

// ========================================================================
// Construction / Destruction
// ========================================================================

ofNode::ofNode()
    : position_(0, 0, 0)
    , orientation_(ofQuaternion::identity())
    , scale_(1, 1, 1)
    , parent_(nullptr)
{
}

ofNode::~ofNode() {
    // Remove from parent
    if (parent_) {
        parent_->removeChild(this);
    }

    // Clear all children's parent reference
    for (ofNode* child : children_) {
        child->parent_ = nullptr;
    }
    children_.clear();
}

// ========================================================================
// Position
// ========================================================================

void ofNode::setPosition(float x, float y, float z) {
    position_.set(x, y, z);
    notifyTransformChanged();
}

void ofNode::setPosition(const ofVec3f& p) {
    position_ = p;
    notifyTransformChanged();
}

ofVec3f ofNode::getPosition() const {
    return position_;
}

ofVec3f ofNode::getGlobalPosition() const {
    if (!parent_) {
        return position_;
    }

    // Transform local position by parent's global transform
    ofMatrix4x4 parentGlobal = parent_->getGlobalTransformMatrix();
    return parentGlobal * position_;
}

void ofNode::setGlobalPosition(const ofVec3f& p) {
    if (!parent_) {
        setPosition(p);
        return;
    }

    // Transform world position to local space
    ofMatrix4x4 parentGlobalInverse = parent_->getGlobalTransformMatrix().getInverse();
    ofVec3f localPos = parentGlobalInverse * p;
    setPosition(localPos);
}

void ofNode::setGlobalPosition(float x, float y, float z) {
    setGlobalPosition(ofVec3f(x, y, z));
}

void ofNode::move(float x, float y, float z) {
    position_.x += x;
    position_.y += y;
    position_.z += z;
    notifyTransformChanged();
}

void ofNode::move(const ofVec3f& offset) {
    position_ += offset;
    notifyTransformChanged();
}

// ========================================================================
// Orientation
// ========================================================================

void ofNode::setOrientation(const ofQuaternion& q) {
    orientation_ = q;
    notifyTransformChanged();
}

ofQuaternion ofNode::getOrientation() const {
    return orientation_;
}

ofQuaternion ofNode::getGlobalOrientation() const {
    if (!parent_) {
        return orientation_;
    }

    // Combine with parent's global orientation
    return parent_->getGlobalOrientation() * orientation_;
}

void ofNode::setGlobalOrientation(const ofQuaternion& q) {
    if (!parent_) {
        setOrientation(q);
        return;
    }

    // Convert global orientation to local
    ofQuaternion parentGlobalInverse = parent_->getGlobalOrientation().getInverse();
    ofQuaternion localOrientation = parentGlobalInverse * q;
    setOrientation(localOrientation);
}

void ofNode::setOrientation(float x, float y, float z) {
    // Convert Euler angles (degrees) to quaternion using ZYX order
    orientation_ = ofQuaternion::makeFromEuler(z, y, x);
    notifyTransformChanged();
}

ofVec3f ofNode::getOrientationEuler() const {
    return orientation_.getEuler();
}

void ofNode::lookAt(const ofVec3f& target, const ofVec3f& up) {
    ofVec3f globalPos = getGlobalPosition();

    // Calculate forward direction
    ofVec3f forward = target - globalPos;
    if (forward.length() < 0.0001f) {
        return;  // Target is too close
    }
    forward.normalize();

    // Calculate right and up vectors
    ofVec3f right = forward.cross(up);
    if (right.length() < 0.0001f) {
        // Forward is parallel to up, use a different up vector
        right = forward.cross(ofVec3f(0, 0, 1));
        if (right.length() < 0.0001f) {
            right = forward.cross(ofVec3f(1, 0, 0));
        }
    }
    right.normalize();

    ofVec3f upDir = right.cross(forward);
    upDir.normalize();

    // Build rotation matrix (looking down -Z)
    ofMatrix4x4 rotMatrix;
    rotMatrix.mat.columns[0] = simd_make_float4(right.x, right.y, right.z, 0);
    rotMatrix.mat.columns[1] = simd_make_float4(upDir.x, upDir.y, upDir.z, 0);
    rotMatrix.mat.columns[2] = simd_make_float4(-forward.x, -forward.y, -forward.z, 0);
    rotMatrix.mat.columns[3] = simd_make_float4(0, 0, 0, 1);

    // Convert to quaternion
    simd_quatf q = simd_quaternion(rotMatrix.mat);
    ofQuaternion globalOrientation(q);

    // Set as global orientation (will convert to local if has parent)
    setGlobalOrientation(globalOrientation);
}

void ofNode::rotate(float angle, const ofVec3f& axis) {
    ofQuaternion rotation = ofQuaternion::makeRotate(angle, axis);
    orientation_ = orientation_ * rotation;
    notifyTransformChanged();
}

void ofNode::rotate(float angle, float x, float y, float z) {
    rotate(angle, ofVec3f(x, y, z));
}

void ofNode::rotateX(float angle) {
    rotate(angle, 1, 0, 0);
}

void ofNode::rotateY(float angle) {
    rotate(angle, 0, 1, 0);
}

void ofNode::rotateZ(float angle) {
    rotate(angle, 0, 0, 1);
}

void ofNode::rotateAround(float angle, const ofVec3f& axis, const ofVec3f& point) {
    // Get current global position
    ofVec3f globalPos = getGlobalPosition();

    // Translate to origin relative to point
    ofVec3f offset = globalPos - point;

    // Create rotation quaternion
    ofQuaternion rotation = ofQuaternion::makeRotate(angle, axis);

    // Rotate the offset
    ofVec3f newOffset = rotation * offset;

    // New position
    ofVec3f newGlobalPos = point + newOffset;
    setGlobalPosition(newGlobalPos);

    // Also rotate orientation
    orientation_ = orientation_ * rotation;
    notifyTransformChanged();
}

// ========================================================================
// Scale
// ========================================================================

void ofNode::setScale(float scale) {
    scale_.set(scale, scale, scale);
    notifyTransformChanged();
}

void ofNode::setScale(float x, float y, float z) {
    scale_.set(x, y, z);
    notifyTransformChanged();
}

void ofNode::setScale(const ofVec3f& s) {
    scale_ = s;
    notifyTransformChanged();
}

ofVec3f ofNode::getScale() const {
    return scale_;
}

ofVec3f ofNode::getGlobalScale() const {
    if (!parent_) {
        return scale_;
    }

    ofVec3f parentScale = parent_->getGlobalScale();
    return ofVec3f(
        scale_.x * parentScale.x,
        scale_.y * parentScale.y,
        scale_.z * parentScale.z
    );
}

// ========================================================================
// Transform Matrices
// ========================================================================

ofMatrix4x4 ofNode::getLocalTransformMatrix() const {
    // Build TRS matrix: Translation * Rotation * Scale
    ofMatrix4x4 translation = ofMatrix4x4::newTranslationMatrix(position_);
    ofMatrix4x4 rotation = orientation_.getMatrix();
    ofMatrix4x4 scale = ofMatrix4x4::newScaleMatrix(scale_);

    return translation * rotation * scale;
}

ofMatrix4x4 ofNode::getGlobalTransformMatrix() const {
    ofMatrix4x4 localMatrix = getLocalTransformMatrix();

    if (!parent_) {
        return localMatrix;
    }

    // Multiply by parent's global transform
    return parent_->getGlobalTransformMatrix() * localMatrix;
}

void ofNode::resetTransform() {
    position_.set(0, 0, 0);
    orientation_ = ofQuaternion::identity();
    scale_.set(1, 1, 1);
    notifyTransformChanged();
}

// ========================================================================
// Direction Vectors
// ========================================================================

ofVec3f ofNode::getForwardDir() const {
    ofQuaternion globalOrientation = getGlobalOrientation();
    return globalOrientation * ofVec3f(0, 0, -1);
}

ofVec3f ofNode::getUpDir() const {
    ofQuaternion globalOrientation = getGlobalOrientation();
    return globalOrientation * ofVec3f(0, 1, 0);
}

ofVec3f ofNode::getRightDir() const {
    ofQuaternion globalOrientation = getGlobalOrientation();
    return globalOrientation * ofVec3f(1, 0, 0);
}

// ========================================================================
// Parent-Child Hierarchy
// ========================================================================

void ofNode::setParent(ofNode* parent, bool keepGlobalTransform) {
    if (parent_ == parent) {
        return;
    }

    // Store global transform if needed
    ofVec3f globalPos;
    ofQuaternion globalOrientation;
    ofVec3f globalScale;

    if (keepGlobalTransform) {
        globalPos = getGlobalPosition();
        globalOrientation = getGlobalOrientation();
        globalScale = getGlobalScale();
    }

    // Remove from old parent
    if (parent_) {
        parent_->removeChild(this);
    }

    // Set new parent
    parent_ = parent;

    // Add to new parent
    if (parent_) {
        parent_->addChild(this);
    }

    // Restore global transform
    if (keepGlobalTransform) {
        setGlobalPosition(globalPos);
        setGlobalOrientation(globalOrientation);

        // Adjust local scale to match global scale
        if (parent_) {
            ofVec3f parentScale = parent_->getGlobalScale();
            scale_.x = (parentScale.x != 0) ? globalScale.x / parentScale.x : globalScale.x;
            scale_.y = (parentScale.y != 0) ? globalScale.y / parentScale.y : globalScale.y;
            scale_.z = (parentScale.z != 0) ? globalScale.z / parentScale.z : globalScale.z;
        } else {
            scale_ = globalScale;
        }
    }

    notifyTransformChanged();
}

void ofNode::setParent(ofNode& parent, bool keepGlobalTransform) {
    setParent(&parent, keepGlobalTransform);
}

void ofNode::clearParent(bool keepGlobalTransform) {
    setParent(nullptr, keepGlobalTransform);
}

ofNode* ofNode::getParent() const {
    return parent_;
}

bool ofNode::hasParent() const {
    return parent_ != nullptr;
}

const std::vector<ofNode*>& ofNode::getChildren() const {
    return children_;
}

size_t ofNode::getNumChildren() const {
    return children_.size();
}

// ========================================================================
// Internal Methods
// ========================================================================

void ofNode::addChild(ofNode* child) {
    if (!child) return;

    // Check if already a child
    auto it = std::find(children_.begin(), children_.end(), child);
    if (it == children_.end()) {
        children_.push_back(child);
    }
}

void ofNode::removeChild(ofNode* child) {
    if (!child) return;

    auto it = std::find(children_.begin(), children_.end(), child);
    if (it != children_.end()) {
        children_.erase(it);
    }
}

void ofNode::notifyTransformChanged() {
    // Call virtual callback
    onTransformChanged();

    // Notify all children
    for (ofNode* child : children_) {
        child->notifyTransformChanged();
    }
}

void ofNode::onTransformChanged() {
    // Default implementation does nothing
    // Subclasses can override to respond to transform changes
}

} // namespace oflike
