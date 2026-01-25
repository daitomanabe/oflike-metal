#include "ofCamera.h"
#include "../../core/Context.h"
#include <cmath>

namespace oflike {

// ========================================================================
// Construction / Destruction
// ========================================================================

ofCamera::ofCamera()
    : position_(0, 0, 0)
    , orientation_(ofQuaternion::identity())
    , fov_(60.0f)
    , nearClip_(0.1f)
    , farClip_(10000.0f)
    , aspectRatio_(4.0f / 3.0f)  // Default 4:3 aspect ratio
    , isActive_(false)
    , projectionMatrix_(ofMatrix4x4::identity())
    , projectionDirty_(true)
{
}

ofCamera::~ofCamera() {
}

// ========================================================================
// Begin / End Rendering
// ========================================================================

void ofCamera::begin() {
    if (isActive_) {
        // Already active, warn and ignore
        return;
    }

    // Push current view and projection matrices to render context (Phase 8.1)
    Context::instance().pushView();

    // Update projection matrix if needed
    updateProjectionMatrix();

    // Apply this camera's matrices
    Context::instance().setViewMatrix(calculateViewMatrix().toSimd());
    Context::instance().setProjectionMatrix(projectionMatrix_.toSimd());

    isActive_ = true;
}

void ofCamera::end() {
    if (!isActive_) {
        // Not active, warn and ignore
        return;
    }

    // Restore previous view and projection matrices (Phase 8.1)
    Context::instance().popView();

    isActive_ = false;
}

// ========================================================================
// Position & Orientation
// ========================================================================

void ofCamera::setPosition(float x, float y, float z) {
    position_.set(x, y, z);
}

void ofCamera::setPosition(const ofVec3f& p) {
    position_ = p;
}

ofVec3f ofCamera::getPosition() const {
    return position_;
}

void ofCamera::setOrientation(const ofQuaternion& q) {
    orientation_ = q;
}

ofQuaternion ofCamera::getOrientation() const {
    return orientation_;
}

void ofCamera::lookAt(const ofVec3f& target, const ofVec3f& up) {
    // Calculate forward direction (from camera to target)
    ofVec3f forward = target - position_;
    forward.normalize();

    // Calculate right direction (perpendicular to forward and up)
    ofVec3f right = forward.cross(up);
    right.normalize();

    // Recalculate up direction (perpendicular to right and forward)
    ofVec3f upDir = right.cross(forward);
    upDir.normalize();

    // Create rotation matrix from basis vectors
    // Note: OpenGL/Metal uses -Z as forward, so negate forward
    ofMatrix4x4 lookMatrix;
    lookMatrix.mat.columns[0] = simd_make_float4(right.x, right.y, right.z, 0);
    lookMatrix.mat.columns[1] = simd_make_float4(upDir.x, upDir.y, upDir.z, 0);
    lookMatrix.mat.columns[2] = simd_make_float4(-forward.x, -forward.y, -forward.z, 0);
    lookMatrix.mat.columns[3] = simd_make_float4(0, 0, 0, 1);

    // Convert rotation matrix to quaternion
    // Use simd to convert matrix to quaternion
    simd_quatf q = simd_quaternion(lookMatrix.mat);
    orientation_ = ofQuaternion(q);
}

void ofCamera::lookAt(float x, float y, float z, float upX, float upY, float upZ) {
    lookAt(ofVec3f(x, y, z), ofVec3f(upX, upY, upZ));
}

// ========================================================================
// Projection Parameters
// ========================================================================

void ofCamera::setNearClip(float near) {
    if (near > 0.0f) {
        nearClip_ = near;
        projectionDirty_ = true;
    }
}

float ofCamera::getNearClip() const {
    return nearClip_;
}

void ofCamera::setFarClip(float far) {
    if (far > nearClip_) {
        farClip_ = far;
        projectionDirty_ = true;
    }
}

float ofCamera::getFarClip() const {
    return farClip_;
}

void ofCamera::setFov(float fov) {
    fov_ = fov;
    projectionDirty_ = true;
}

float ofCamera::getFov() const {
    return fov_;
}

void ofCamera::setAspectRatio(float ratio) {
    aspectRatio_ = ratio;
    projectionDirty_ = true;
}

float ofCamera::getAspectRatio() const {
    return aspectRatio_;
}

// ========================================================================
// Matrix Access
// ========================================================================

ofMatrix4x4 ofCamera::getModelViewMatrix() const {
    return calculateViewMatrix();
}

ofMatrix4x4 ofCamera::getProjectionMatrix() const {
    updateProjectionMatrix();
    return projectionMatrix_;
}

ofMatrix4x4 ofCamera::getModelViewProjectionMatrix() const {
    return getProjectionMatrix() * getModelViewMatrix();
}

// ========================================================================
// Screen-World Coordinate Conversion
// ========================================================================

ofVec3f ofCamera::worldToScreen(const ofVec3f& world) const {
    // Get viewport dimensions from Context (Phase 8.1)
    simd_float4 viewport = Context::instance().getViewport();
    float viewportX = viewport.x;
    float viewportY = viewport.y;
    float viewportWidth = viewport.z;
    float viewportHeight = viewport.w;

    // Transform world -> clip space
    ofMatrix4x4 mvp = getModelViewProjectionMatrix();
    ofVec4f clipPos = mvp * ofVec4f(world.x, world.y, world.z, 1.0f);

    // Perspective divide to get NDC (-1 to +1)
    if (clipPos.w != 0.0f) {
        clipPos.x /= clipPos.w;
        clipPos.y /= clipPos.w;
        clipPos.z /= clipPos.w;
    }

    // Convert NDC to screen coordinates
    // NDC: (-1, -1) at bottom-left, (+1, +1) at top-right
    // Screen: (0, 0) at top-left, (width, height) at bottom-right
    float screenX = viewportX + (clipPos.x + 1.0f) * 0.5f * viewportWidth;
    float screenY = viewportY + (1.0f - clipPos.y) * 0.5f * viewportHeight;  // Flip Y
    float screenZ = clipPos.z;  // Keep Z in NDC range for depth testing

    return ofVec3f(screenX, screenY, screenZ);
}

ofVec3f ofCamera::screenToWorld(const ofVec3f& screen) const {
    // Get viewport dimensions from Context (Phase 8.1)
    simd_float4 viewport = Context::instance().getViewport();
    float viewportX = viewport.x;
    float viewportY = viewport.y;
    float viewportWidth = viewport.z;
    float viewportHeight = viewport.w;

    // Convert screen coordinates to NDC (-1 to +1)
    // Screen: (0, 0) at top-left, (width, height) at bottom-right
    // NDC: (-1, -1) at bottom-left, (+1, +1) at top-right
    float ndcX = ((screen.x - viewportX) / viewportWidth) * 2.0f - 1.0f;
    float ndcY = 1.0f - ((screen.y - viewportY) / viewportHeight) * 2.0f;  // Flip Y
    float ndcZ = screen.z;  // Z is already in NDC range

    // Create clip space position (with w = 1 for inverse transform)
    ofVec4f clipPos(ndcX, ndcY, ndcZ, 1.0f);

    // Apply inverse MVP transform
    ofMatrix4x4 mvp = getModelViewProjectionMatrix();
    ofMatrix4x4 invMVP = mvp.getInverse();
    ofVec4f worldPos = invMVP * clipPos;

    // Perspective divide (if w != 1)
    if (worldPos.w != 0.0f && worldPos.w != 1.0f) {
        worldPos.x /= worldPos.w;
        worldPos.y /= worldPos.w;
        worldPos.z /= worldPos.w;
    }

    return ofVec3f(worldPos.x, worldPos.y, worldPos.z);
}

// ========================================================================
// Additional Getters
// ========================================================================

ofVec3f ofCamera::getForwardDir() const {
    // Forward is -Z in camera space, transform by orientation
    ofVec3f forward(0, 0, -1);
    return orientation_ * forward;
}

ofVec3f ofCamera::getUpDir() const {
    // Up is +Y in camera space, transform by orientation
    ofVec3f up(0, 1, 0);
    return orientation_ * up;
}

ofVec3f ofCamera::getRightDir() const {
    // Right is +X in camera space, transform by orientation
    ofVec3f right(1, 0, 0);
    return orientation_ * right;
}

// ========================================================================
// Internal Methods
// ========================================================================

void ofCamera::updateProjectionMatrix() const {
    if (!projectionDirty_) {
        return;
    }

    // Create perspective projection matrix
    // Convert FOV to radians
    float fovRadians = fov_ * M_PI / 180.0f;
    float halfFovTan = std::tan(fovRadians / 2.0f);

    float yScale = 1.0f / halfFovTan;
    float xScale = yScale / aspectRatio_;

    // Metal uses reverse-Z depth (1 at near, 0 at far) for better precision
    // Standard perspective projection formula
    float A = farClip_ / (nearClip_ - farClip_);
    float B = (nearClip_ * farClip_) / (nearClip_ - farClip_);

    // Column-major matrix
    projectionMatrix_.mat.columns[0] = simd_make_float4(xScale, 0, 0, 0);
    projectionMatrix_.mat.columns[1] = simd_make_float4(0, yScale, 0, 0);
    projectionMatrix_.mat.columns[2] = simd_make_float4(0, 0, A, -1);
    projectionMatrix_.mat.columns[3] = simd_make_float4(0, 0, B, 0);

    projectionDirty_ = false;
}

ofMatrix4x4 ofCamera::calculateViewMatrix() const {
    // Create view matrix from position and orientation
    // View matrix is the inverse of the camera's transform

    // Get rotation matrix from quaternion
    ofMatrix4x4 rotationMatrix = orientation_.getMatrix();

    // Invert rotation (transpose for orthogonal matrix)
    ofMatrix4x4 invRotation = rotationMatrix.getTransposed();

    // Create translation matrix (-position)
    ofMatrix4x4 translation = ofMatrix4x4::newTranslationMatrix(-position_);

    // View matrix = inverse(rotation) * inverse(translation)
    // Which is: transpose(rotation) * translate(-position)
    return invRotation * translation;
}

} // namespace oflike
