#include "SharpCameraPath.h"
#include "3d/ofCamera.h"
#include "math/ofVec3f.h"
#include <simd/simd.h>
#include <cmath>
#include <algorithm>

namespace Sharp {

// ============================================================================
// Internal Helper Functions
// ============================================================================

namespace {

// Linear interpolation
inline oflike::float3 lerp(const oflike::float3& a, const oflike::float3& b, float t) {
    return simd_mix(a, b, t);
}

// Catmull-Rom spline interpolation
// Given 4 control points p0, p1, p2, p3 and t in [0,1], interpolate between p1 and p2
oflike::float3 catmullRom(const oflike::float3& p0,
                          const oflike::float3& p1,
                          const oflike::float3& p2,
                          const oflike::float3& p3,
                          float t) {
    float t2 = t * t;
    float t3 = t2 * t;

    oflike::float3 result;
    result.x = 0.5f * ((2.0f * p1.x) +
                       (-p0.x + p2.x) * t +
                       (2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * t2 +
                       (-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * t3);

    result.y = 0.5f * ((2.0f * p1.y) +
                       (-p0.y + p2.y) * t +
                       (2.0f * p0.y - 5.0f * p1.y + 4.0f * p2.y - p3.y) * t2 +
                       (-p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y) * t3);

    result.z = 0.5f * ((2.0f * p1.z) +
                       (-p0.z + p2.z) * t +
                       (2.0f * p0.z - 5.0f * p1.z + 4.0f * p2.z - p3.z) * t2 +
                       (-p0.z + 3.0f * p1.z - 3.0f * p2.z + p3.z) * t3);

    return result;
}

// Cubic Bezier interpolation
oflike::float3 bezier(const oflike::float3& p0,
                      const oflike::float3& p1,
                      const oflike::float3& p2,
                      const oflike::float3& p3,
                      float t) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    oflike::float3 result = p0 * uuu;
    result = result + p1 * (3.0f * uu * t);
    result = result + p2 * (3.0f * u * tt);
    result = result + p3 * ttt;

    return result;
}

// Quaternion slerp
oflike::quatf slerp(const oflike::quatf& q0, const oflike::quatf& q1, float t) {
    return simd_slerp(q0, q1, t);
}

} // anonymous namespace

// ============================================================================
// Implementation
// ============================================================================

struct CameraPath::Impl {
    // Path configuration
    PathType pathType = PathType::Keyframe;
    InterpolationMode interpolationMode = InterpolationMode::CatmullRom;
    PlaybackMode playbackMode = PlaybackMode::Loop;

    // Playback state
    bool playing = false;
    float currentTime = 0.0f;
    float duration = 0.0f;
    float speed = 1.0f;
    bool pingPongReverse = false;

    // LookAt settings
    oflike::float3 lookAtTarget{0.0f, 0.0f, 0.0f};
    bool lookAtEnabled = true;

    // Keyframes
    std::vector<CameraKeyframe> keyframes;
    bool keyframesSorted = true;

    // Orbit path settings
    oflike::float3 orbitCenter{0.0f, 0.0f, 0.0f};
    float orbitRadius = 1.0f;
    oflike::float3 orbitAxis{0.0f, 1.0f, 0.0f};
    float orbitStartAngle = 0.0f;

    // Dolly path settings
    std::vector<oflike::float3> dollyPoints;
    bool dollyClosed = false;

    // Spiral path settings
    oflike::float3 spiralCenter{0.0f, 0.0f, 0.0f};
    float spiralRadius = 1.0f;
    float spiralHeight = 0.0f;
    float spiralRevolutions = 1.0f;
    oflike::float3 spiralAxis{0.0f, 1.0f, 0.0f};

    // Current state cache
    mutable oflike::float3 cachedPosition{0.0f, 0.0f, 0.0f};
    mutable oflike::float3 cachedTarget{0.0f, 0.0f, -1.0f};
    mutable float cachedFov = 60.0f;
    mutable bool cacheDirty = true;

    // Methods
    void updateCache() const;
    void computeOrbitPosition() const;
    void computeDollyPosition() const;
    void computeSpiralPosition() const;
    void computeKeyframePosition() const;
};

// ============================================================================
// Constructor / Destructor
// ============================================================================

CameraPath::CameraPath()
    : impl_(std::make_unique<Impl>())
{
}

CameraPath::~CameraPath() = default;

CameraPath::CameraPath(CameraPath&& other) noexcept = default;
CameraPath& CameraPath::operator=(CameraPath&& other) noexcept = default;

// ============================================================================
// Path Configuration
// ============================================================================

void CameraPath::setOrbitPath(const oflike::float3& center,
                               float radius,
                               float duration,
                               const oflike::float3& axis,
                               float startAngle) {
    impl_->pathType = PathType::Orbit;
    impl_->orbitCenter = center;
    impl_->orbitRadius = radius;
    impl_->duration = duration;
    impl_->orbitAxis = simd_normalize(axis);
    impl_->orbitStartAngle = startAngle;
    impl_->lookAtTarget = center;
    impl_->lookAtEnabled = true;
    impl_->cacheDirty = true;
}

void CameraPath::setDollyPath(const std::vector<oflike::float3>& controlPoints,
                               float duration,
                               bool closed) {
    impl_->pathType = PathType::Dolly;
    impl_->dollyPoints = controlPoints;
    impl_->duration = duration;
    impl_->dollyClosed = closed;
    impl_->cacheDirty = true;
}

void CameraPath::setSpiralPath(const oflike::float3& center,
                                float radius,
                                float height,
                                float revolutions,
                                float duration,
                                const oflike::float3& axis) {
    impl_->pathType = PathType::Spiral;
    impl_->spiralCenter = center;
    impl_->spiralRadius = radius;
    impl_->spiralHeight = height;
    impl_->spiralRevolutions = revolutions;
    impl_->duration = duration;
    impl_->spiralAxis = simd_normalize(axis);
    impl_->lookAtTarget = center;
    impl_->lookAtEnabled = true;
    impl_->cacheDirty = true;
}

// ============================================================================
// Keyframe Animation
// ============================================================================

void CameraPath::addKeyframe(float time,
                              const oflike::float3& position,
                              const oflike::float3& target,
                              float fov) {
    impl_->pathType = PathType::Keyframe;
    impl_->keyframes.emplace_back(time, position, target, fov);
    impl_->keyframesSorted = false;
    impl_->cacheDirty = true;

    // Update duration if needed
    if (time > impl_->duration) {
        impl_->duration = time;
    }
}

void CameraPath::addKeyframe(const CameraKeyframe& keyframe) {
    impl_->pathType = PathType::Keyframe;
    impl_->keyframes.push_back(keyframe);
    impl_->keyframesSorted = false;
    impl_->cacheDirty = true;

    // Update duration if needed
    if (keyframe.time > impl_->duration) {
        impl_->duration = keyframe.time;
    }
}

void CameraPath::clearKeyframes() {
    impl_->keyframes.clear();
    impl_->keyframesSorted = true;
    impl_->duration = 0.0f;
    impl_->cacheDirty = true;
}

size_t CameraPath::getKeyframeCount() const {
    return impl_->keyframes.size();
}

const CameraKeyframe& CameraPath::getKeyframe(size_t index) const {
    return impl_->keyframes.at(index);
}

void CameraPath::setKeyframe(size_t index, const CameraKeyframe& keyframe) {
    impl_->keyframes.at(index) = keyframe;
    impl_->keyframesSorted = false;
    impl_->cacheDirty = true;

    // Update duration
    float maxTime = 0.0f;
    for (const auto& kf : impl_->keyframes) {
        maxTime = std::max(maxTime, kf.time);
    }
    impl_->duration = maxTime;
}

void CameraPath::removeKeyframe(size_t index) {
    if (index < impl_->keyframes.size()) {
        impl_->keyframes.erase(impl_->keyframes.begin() + index);
        impl_->cacheDirty = true;

        // Update duration
        float maxTime = 0.0f;
        for (const auto& kf : impl_->keyframes) {
            maxTime = std::max(maxTime, kf.time);
        }
        impl_->duration = maxTime;
    }
}

void CameraPath::sortKeyframes() {
    std::sort(impl_->keyframes.begin(), impl_->keyframes.end(),
              [](const CameraKeyframe& a, const CameraKeyframe& b) {
                  return a.time < b.time;
              });
    impl_->keyframesSorted = true;
}

// ============================================================================
// Interpolation
// ============================================================================

void CameraPath::setInterpolationMode(InterpolationMode mode) {
    impl_->interpolationMode = mode;
    impl_->cacheDirty = true;
}

InterpolationMode CameraPath::getInterpolationMode() const {
    return impl_->interpolationMode;
}

// ============================================================================
// LookAt Target
// ============================================================================

void CameraPath::setLookAtTarget(const oflike::float3& target) {
    impl_->lookAtTarget = target;
    impl_->cacheDirty = true;
}

oflike::float3 CameraPath::getLookAtTarget() const {
    return impl_->lookAtTarget;
}

void CameraPath::setLookAtEnabled(bool enabled) {
    impl_->lookAtEnabled = enabled;
    impl_->cacheDirty = true;
}

bool CameraPath::isLookAtEnabled() const {
    return impl_->lookAtEnabled;
}

// ============================================================================
// Playback Control
// ============================================================================

void CameraPath::play() {
    impl_->playing = true;
}

void CameraPath::pause() {
    impl_->playing = false;
}

void CameraPath::stop() {
    impl_->playing = false;
    impl_->currentTime = 0.0f;
    impl_->pingPongReverse = false;
    impl_->cacheDirty = true;
}

bool CameraPath::isPlaying() const {
    return impl_->playing;
}

void CameraPath::setPlaybackMode(PlaybackMode mode) {
    impl_->playbackMode = mode;
}

PlaybackMode CameraPath::getPlaybackMode() const {
    return impl_->playbackMode;
}

void CameraPath::setSpeed(float speed) {
    impl_->speed = speed;
}

float CameraPath::getSpeed() const {
    return impl_->speed;
}

// ============================================================================
// Time Control
// ============================================================================

void CameraPath::update(float deltaTime) {
    if (!impl_->playing || impl_->duration <= 0.0f) {
        return;
    }

    float dt = deltaTime * impl_->speed;

    switch (impl_->playbackMode) {
        case PlaybackMode::Once:
            impl_->currentTime += dt;
            if (impl_->currentTime >= impl_->duration) {
                impl_->currentTime = impl_->duration;
                impl_->playing = false;
            }
            break;

        case PlaybackMode::Loop:
            impl_->currentTime += dt;
            if (impl_->currentTime >= impl_->duration) {
                impl_->currentTime = std::fmod(impl_->currentTime, impl_->duration);
            }
            break;

        case PlaybackMode::PingPong:
            if (impl_->pingPongReverse) {
                impl_->currentTime -= dt;
                if (impl_->currentTime <= 0.0f) {
                    impl_->currentTime = 0.0f;
                    impl_->pingPongReverse = false;
                }
            } else {
                impl_->currentTime += dt;
                if (impl_->currentTime >= impl_->duration) {
                    impl_->currentTime = impl_->duration;
                    impl_->pingPongReverse = true;
                }
            }
            break;
    }

    impl_->cacheDirty = true;
}

void CameraPath::setTime(float time) {
    impl_->currentTime = std::max(0.0f, std::min(time, impl_->duration));
    impl_->cacheDirty = true;
}

float CameraPath::getTime() const {
    return impl_->currentTime;
}

float CameraPath::getDuration() const {
    return impl_->duration;
}

float CameraPath::getNormalizedTime() const {
    if (impl_->duration <= 0.0f) return 0.0f;
    return impl_->currentTime / impl_->duration;
}

void CameraPath::setNormalizedTime(float t) {
    setTime(t * impl_->duration);
}

// ============================================================================
// Camera Application
// ============================================================================

void CameraPath::applyToCamera(oflike::ofCamera& camera) const {
    impl_->updateCache();

    camera.setPosition(oflike::ofVec3f(impl_->cachedPosition.x,
                                       impl_->cachedPosition.y,
                                       impl_->cachedPosition.z));

    if (impl_->lookAtEnabled || impl_->pathType == PathType::Keyframe) {
        camera.lookAt(oflike::ofVec3f(impl_->cachedTarget.x,
                                      impl_->cachedTarget.y,
                                      impl_->cachedTarget.z));
    }

    camera.setFov(impl_->cachedFov);
}

oflike::float3 CameraPath::getPosition() const {
    impl_->updateCache();
    return impl_->cachedPosition;
}

oflike::float3 CameraPath::getTarget() const {
    impl_->updateCache();
    return impl_->cachedTarget;
}

float CameraPath::getFov() const {
    impl_->updateCache();
    return impl_->cachedFov;
}

// ============================================================================
// Path Info
// ============================================================================

PathType CameraPath::getPathType() const {
    return impl_->pathType;
}

bool CameraPath::isConfigured() const {
    switch (impl_->pathType) {
        case PathType::Orbit:
        case PathType::Spiral:
            return impl_->duration > 0.0f;
        case PathType::Dolly:
            return !impl_->dollyPoints.empty() && impl_->duration > 0.0f;
        case PathType::Keyframe:
            return impl_->keyframes.size() >= 2;
    }
    return false;
}

// ============================================================================
// Internal Implementation
// ============================================================================

void CameraPath::Impl::updateCache() const {
    if (!cacheDirty) return;

    switch (pathType) {
        case PathType::Orbit:
            computeOrbitPosition();
            break;
        case PathType::Dolly:
            computeDollyPosition();
            break;
        case PathType::Spiral:
            computeSpiralPosition();
            break;
        case PathType::Keyframe:
            computeKeyframePosition();
            break;
    }

    cacheDirty = false;
}

void CameraPath::Impl::computeOrbitPosition() const {
    if (duration <= 0.0f) {
        cachedPosition = orbitCenter;
        cachedTarget = orbitCenter;
        cachedFov = 60.0f;
        return;
    }

    float t = currentTime / duration;
    float angle = orbitStartAngle + t * 2.0f * M_PI;

    // Create rotation quaternion
    oflike::quatf rot = simd_quaternion(angle, orbitAxis);

    // Initial position (on X axis)
    oflike::float3 offset = {orbitRadius, 0.0f, 0.0f};

    // Rotate offset
    oflike::float3 rotatedOffset = simd_act(rot, offset);

    cachedPosition = orbitCenter + rotatedOffset;
    cachedTarget = lookAtTarget;
    cachedFov = 60.0f;
}

void CameraPath::Impl::computeDollyPosition() const {
    if (dollyPoints.empty() || duration <= 0.0f) {
        cachedPosition = {0.0f, 0.0f, 0.0f};
        cachedTarget = {0.0f, 0.0f, -1.0f};
        cachedFov = 60.0f;
        return;
    }

    if (dollyPoints.size() == 1) {
        cachedPosition = dollyPoints[0];
        cachedTarget = lookAtTarget;
        cachedFov = 60.0f;
        return;
    }

    float t = currentTime / duration;
    int numSegments = dollyClosed ? dollyPoints.size() : (dollyPoints.size() - 1);
    float segmentLength = 1.0f / numSegments;

    int segment = static_cast<int>(t / segmentLength);
    segment = std::min(segment, numSegments - 1);

    float localT = (t - segment * segmentLength) / segmentLength;

    int i0 = segment;
    int i1 = (segment + 1) % dollyPoints.size();

    // Linear interpolation for now (could use spline later)
    cachedPosition = lerp(dollyPoints[i0], dollyPoints[i1], localT);
    cachedTarget = lookAtTarget;
    cachedFov = 60.0f;
}

void CameraPath::Impl::computeSpiralPosition() const {
    if (duration <= 0.0f) {
        cachedPosition = spiralCenter;
        cachedTarget = spiralCenter;
        cachedFov = 60.0f;
        return;
    }

    float t = currentTime / duration;
    float angle = t * spiralRevolutions * 2.0f * M_PI;
    float heightOffset = t * spiralHeight;

    // Create rotation quaternion
    oflike::quatf rot = simd_quaternion(angle, spiralAxis);

    // Initial position (on X axis)
    oflike::float3 offset = {spiralRadius, 0.0f, 0.0f};

    // Rotate offset
    oflike::float3 rotatedOffset = simd_act(rot, offset);

    // Add height along spiral axis
    oflike::float3 heightVec = spiralAxis * heightOffset;

    cachedPosition = spiralCenter + rotatedOffset + heightVec;
    cachedTarget = lookAtTarget;
    cachedFov = 60.0f;
}

void CameraPath::Impl::computeKeyframePosition() const {
    if (keyframes.empty()) {
        cachedPosition = {0.0f, 0.0f, 5.0f};
        cachedTarget = {0.0f, 0.0f, 0.0f};
        cachedFov = 60.0f;
        return;
    }

    if (keyframes.size() == 1) {
        cachedPosition = keyframes[0].position;
        cachedTarget = keyframes[0].target;
        cachedFov = keyframes[0].fov;
        return;
    }

    // Ensure keyframes are sorted
    if (!keyframesSorted) {
        std::sort(const_cast<std::vector<CameraKeyframe>&>(keyframes).begin(),
                  const_cast<std::vector<CameraKeyframe>&>(keyframes).end(),
                  [](const CameraKeyframe& a, const CameraKeyframe& b) {
                      return a.time < b.time;
                  });
        const_cast<Impl*>(this)->keyframesSorted = true;
    }

    // Find keyframe segment
    size_t k1 = 0;
    for (size_t i = 0; i < keyframes.size() - 1; ++i) {
        if (currentTime >= keyframes[i].time && currentTime <= keyframes[i + 1].time) {
            k1 = i;
            break;
        }
    }

    // Clamp to last keyframe if beyond
    if (currentTime >= keyframes.back().time) {
        cachedPosition = keyframes.back().position;
        cachedTarget = keyframes.back().target;
        cachedFov = keyframes.back().fov;
        return;
    }

    size_t k2 = k1 + 1;

    // Calculate local t
    float t1 = keyframes[k1].time;
    float t2 = keyframes[k2].time;
    float localT = (currentTime - t1) / (t2 - t1);

    // Interpolate based on mode
    switch (interpolationMode) {
        case InterpolationMode::Linear:
            cachedPosition = lerp(keyframes[k1].position, keyframes[k2].position, localT);
            cachedTarget = lerp(keyframes[k1].target, keyframes[k2].target, localT);
            cachedFov = keyframes[k1].fov + (keyframes[k2].fov - keyframes[k1].fov) * localT;
            break;

        case InterpolationMode::CatmullRom: {
            // Get 4 control points
            size_t k0 = (k1 > 0) ? k1 - 1 : k1;
            size_t k3 = (k2 + 1 < keyframes.size()) ? k2 + 1 : k2;

            cachedPosition = catmullRom(keyframes[k0].position,
                                        keyframes[k1].position,
                                        keyframes[k2].position,
                                        keyframes[k3].position,
                                        localT);

            cachedTarget = catmullRom(keyframes[k0].target,
                                      keyframes[k1].target,
                                      keyframes[k2].target,
                                      keyframes[k3].target,
                                      localT);

            cachedFov = keyframes[k1].fov + (keyframes[k2].fov - keyframes[k1].fov) * localT;
            break;
        }

        case InterpolationMode::Bezier: {
            // For Bezier, we generate control points from keyframes
            // Simple tangent estimation
            oflike::float3 tan1 = (keyframes[k2].position - keyframes[k1].position) * 0.333f;
            oflike::float3 tan2 = tan1;

            oflike::float3 p0 = keyframes[k1].position;
            oflike::float3 p1 = p0 + tan1;
            oflike::float3 p3 = keyframes[k2].position;
            oflike::float3 p2 = p3 - tan2;

            cachedPosition = bezier(p0, p1, p2, p3, localT);

            // Same for target
            tan1 = (keyframes[k2].target - keyframes[k1].target) * 0.333f;
            tan2 = tan1;

            p0 = keyframes[k1].target;
            p1 = p0 + tan1;
            p3 = keyframes[k2].target;
            p2 = p3 - tan2;

            cachedTarget = bezier(p0, p1, p2, p3, localT);

            cachedFov = keyframes[k1].fov + (keyframes[k2].fov - keyframes[k1].fov) * localT;
            break;
        }
    }
}

} // namespace Sharp
