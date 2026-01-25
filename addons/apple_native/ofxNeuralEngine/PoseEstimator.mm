#import "PoseEstimator.h"
#import <Vision/Vision.h>
#import <CoreImage/CoreImage.h>
#import "../../oflike/image/ofPixels.h"
#import "../../oflike/image/ofTexture.h"
#import "../../oflike/graphics/ofGraphics.h"
#import "../../oflike/types/ofColor.h"

namespace NeuralEngine {

// ============================================================================
// Joint Implementation
// ============================================================================

Joint::Joint()
    : name(JointName::Nose)
    , x(0.0f)
    , y(0.0f)
    , confidence(0.0f)
    , isDetected(false)
{
}

Joint::Joint(JointName n, float x_, float y_, float conf)
    : name(n)
    , x(x_)
    , y(y_)
    , confidence(conf)
    , isDetected(conf > 0.0f)
{
}

// ============================================================================
// HumanPose Implementation
// ============================================================================

HumanPose::HumanPose()
    : overallConfidence(0.0f)
    , personID(0)
{
}

const Joint* HumanPose::getJoint(JointName name) const {
    for (const auto& joint : joints) {
        if (joint.name == name) {
            return &joint;
        }
    }
    return nullptr;
}

Joint* HumanPose::getJoint(JointName name) {
    for (auto& joint : joints) {
        if (joint.name == name) {
            return &joint;
        }
    }
    return nullptr;
}

bool HumanPose::hasJoint(JointName name, float minConfidence) const {
    const Joint* joint = getJoint(name);
    return joint && joint->isDetected && joint->confidence >= minConfidence;
}

std::vector<Joint> HumanPose::getDetectedJoints(float minConfidence) const {
    std::vector<Joint> detected;
    for (const auto& joint : joints) {
        if (joint.isDetected && joint.confidence >= minConfidence) {
            detected.push_back(joint);
        }
    }
    return detected;
}

// ============================================================================
// VNHumanBodyPoseObservation Joint Name Mapping
// ============================================================================

static NSString* getVisionJointName(JointName name) {
    switch (name) {
        case JointName::Nose: return VNHumanBodyPoseObservationJointNameNose;
        case JointName::LeftEye: return VNHumanBodyPoseObservationJointNameLeftEye;
        case JointName::RightEye: return VNHumanBodyPoseObservationJointNameRightEye;
        case JointName::LeftEar: return VNHumanBodyPoseObservationJointNameLeftEar;
        case JointName::RightEar: return VNHumanBodyPoseObservationJointNameRightEar;
        case JointName::LeftShoulder: return VNHumanBodyPoseObservationJointNameLeftShoulder;
        case JointName::RightShoulder: return VNHumanBodyPoseObservationJointNameRightShoulder;
        case JointName::LeftElbow: return VNHumanBodyPoseObservationJointNameLeftElbow;
        case JointName::RightElbow: return VNHumanBodyPoseObservationJointNameRightElbow;
        case JointName::LeftWrist: return VNHumanBodyPoseObservationJointNameLeftWrist;
        case JointName::RightWrist: return VNHumanBodyPoseObservationJointNameRightWrist;
        case JointName::LeftHip: return VNHumanBodyPoseObservationJointNameLeftHip;
        case JointName::RightHip: return VNHumanBodyPoseObservationJointNameRightHip;
        case JointName::LeftKnee: return VNHumanBodyPoseObservationJointNameLeftKnee;
        case JointName::RightKnee: return VNHumanBodyPoseObservationJointNameRightKnee;
        case JointName::LeftAnkle: return VNHumanBodyPoseObservationJointNameLeftAnkle;
        case JointName::RightAnkle: return VNHumanBodyPoseObservationJointNameRightAnkle;
        case JointName::Neck: return VNHumanBodyPoseObservationJointNameNeck;
        case JointName::Root: return VNHumanBodyPoseObservationJointNameRoot;
        default: return nil;
    }
}

static std::vector<JointName> getAllJointNames() {
    return {
        JointName::Nose,
        JointName::LeftEye,
        JointName::RightEye,
        JointName::LeftEar,
        JointName::RightEar,
        JointName::LeftShoulder,
        JointName::RightShoulder,
        JointName::Neck,
        JointName::LeftElbow,
        JointName::RightElbow,
        JointName::LeftWrist,
        JointName::RightWrist,
        JointName::Root,
        JointName::LeftHip,
        JointName::RightHip,
        JointName::LeftKnee,
        JointName::RightKnee,
        JointName::LeftAnkle,
        JointName::RightAnkle
    };
}

// ============================================================================
// PoseEstimator::Impl
// ============================================================================

struct PoseEstimator::Impl {
    PoseEstimatorConfig config;
    std::string lastError;
    bool isReady = false;

    VNDetectHumanBodyPoseRequest* poseRequest = nil;

    Impl() = default;

    ~Impl() {
        shutdown();
    }

    bool setup(const PoseEstimatorConfig& cfg) {
        @autoreleasepool {
            config = cfg;
            lastError.clear();

            // Create pose detection request
            poseRequest = [[VNDetectHumanBodyPoseRequest alloc] init];
            if (!poseRequest) {
                lastError = "Failed to create VNDetectHumanBodyPoseRequest";
                return false;
            }

            isReady = true;
            return true;
        }
    }

    void shutdown() {
        @autoreleasepool {
            if (poseRequest) {
                poseRequest = nil;
            }
            isReady = false;
        }
    }

    std::vector<HumanPose> estimate(CGImageRef image) {
        @autoreleasepool {
            std::vector<HumanPose> results;

            if (!isReady || !image) {
                lastError = isReady ? "Invalid image" : "Estimator not ready";
                return results;
            }

            // Create request handler
            VNImageRequestHandler* handler = [[VNImageRequestHandler alloc]
                initWithCGImage:image
                options:@{}];

            // Perform request
            NSError* error = nil;
            if (![handler performRequests:@[poseRequest] error:&error]) {
                if (error) {
                    lastError = std::string("Vision error: ") + [error.localizedDescription UTF8String];
                }
                return results;
            }

            // Process results
            NSArray<VNHumanBodyPoseObservation*>* observations = poseRequest.results;
            if (!observations || observations.count == 0) {
                return results; // No people detected
            }

            // Apply max people limit
            NSUInteger maxCount = observations.count;
            if (config.maxPeople > 0 && maxCount > config.maxPeople) {
                maxCount = config.maxPeople;
            }

            // Extract poses
            for (NSUInteger i = 0; i < maxCount; i++) {
                VNHumanBodyPoseObservation* observation = observations[i];
                HumanPose pose = extractPose(observation);

                // Filter by overall confidence
                if (pose.overallConfidence >= config.minPoseConfidence) {
                    pose.personID = static_cast<int>(i);
                    results.push_back(pose);
                }
            }

            return results;
        }
    }

    HumanPose extractPose(VNHumanBodyPoseObservation* observation) {
        @autoreleasepool {
            HumanPose pose;
            float totalConfidence = 0.0f;
            int detectedCount = 0;

            // Extract all joints
            std::vector<JointName> allJoints = getAllJointNames();
            for (JointName jointName : allJoints) {
                NSString* visionName = getVisionJointName(jointName);
                if (!visionName) continue;

                NSError* error = nil;
                VNRecognizedPoint* point = [observation recognizedPointForJointName:visionName error:&error];

                if (point && !error) {
                    float confidence = point.confidence;

                    // Vision framework returns coordinates in normalized space (0,0 = bottom-left)
                    // We need to flip Y to match oflike coordinate system (0,0 = top-left)
                    float x = point.location.x;
                    float y = 1.0f - point.location.y;

                    Joint joint(jointName, x, y, confidence);

                    // Filter by confidence
                    if (confidence >= config.minJointConfidence) {
                        joint.isDetected = true;
                        totalConfidence += confidence;
                        detectedCount++;
                    }

                    pose.joints.push_back(joint);
                }
            }

            // Calculate overall confidence
            if (detectedCount > 0) {
                pose.overallConfidence = totalConfidence / detectedCount;
            }

            return pose;
        }
    }
};

// ============================================================================
// PoseEstimator Public Implementation
// ============================================================================

PoseEstimator::PoseEstimator()
    : impl_(std::make_unique<Impl>())
{
}

PoseEstimator::~PoseEstimator() = default;

PoseEstimator::PoseEstimator(PoseEstimator&& other) noexcept = default;
PoseEstimator& PoseEstimator::operator=(PoseEstimator&& other) noexcept = default;

bool PoseEstimator::setup() {
    return setup(PoseEstimatorConfig{});
}

bool PoseEstimator::setup(const PoseEstimatorConfig& config) {
    return impl_->setup(config);
}

void PoseEstimator::shutdown() {
    impl_->shutdown();
}

std::vector<HumanPose> PoseEstimator::estimate(const oflike::ofPixels& pixels) {
    @autoreleasepool {
        // Create CGImage from ofPixels
        size_t width = pixels.getWidth();
        size_t height = pixels.getHeight();
        size_t bitsPerComponent = 8;
        size_t bytesPerRow = pixels.getBytesPerRow();

        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        CGBitmapInfo bitmapInfo = kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big;

        if (pixels.getNumChannels() == 1) {
            colorSpace = CGColorSpaceCreateDeviceGray();
            bitmapInfo = kCGImageAlphaNone;
        } else if (pixels.getNumChannels() == 3) {
            bitmapInfo = kCGImageAlphaNoneSkipLast | kCGBitmapByteOrder32Big;
        }

        CGDataProviderRef provider = CGDataProviderCreateWithData(
            nullptr,
            pixels.getData(),
            height * bytesPerRow,
            nullptr
        );

        CGImageRef image = CGImageCreate(
            width, height,
            bitsPerComponent,
            bitsPerComponent * pixels.getNumChannels(),
            bytesPerRow,
            colorSpace,
            bitmapInfo,
            provider,
            nullptr, false,
            kCGRenderingIntentDefault
        );

        std::vector<HumanPose> results = impl_->estimate(image);

        CGImageRelease(image);
        CGDataProviderRelease(provider);
        CGColorSpaceRelease(colorSpace);

        return results;
    }
}

std::vector<HumanPose> PoseEstimator::estimate(const oflike::ofTexture& texture) {
    @autoreleasepool {
        // Convert texture to pixels
        oflike::ofPixels pixels;
        texture.readToPixels(pixels);
        return estimate(pixels);
    }
}

// ============================================================================
// Drawing Helpers
// ============================================================================

void PoseEstimator::drawSkeleton(const HumanPose& pose,
                                 float width,
                                 float height,
                                 float jointRadius,
                                 float boneThickness) {
    using namespace oflike;

    // Draw bones first (so joints appear on top)
    std::vector<SkeletonConnection> connections = getStandardConnections();

    ofSetLineWidth(boneThickness);
    ofSetColor(0, 255, 0, 200); // Green bones

    for (const auto& conn : connections) {
        const Joint* j1 = pose.getJoint(conn.joint1);
        const Joint* j2 = pose.getJoint(conn.joint2);

        if (j1 && j2 && j1->isDetected && j2->isDetected) {
            float x1 = j1->x * width;
            float y1 = j1->y * height;
            float x2 = j2->x * width;
            float y2 = j2->y * height;

            ofDrawLine(x1, y1, x2, y2);
        }
    }

    // Draw joints
    ofFill();
    for (const auto& joint : pose.joints) {
        if (joint.isDetected) {
            // Color based on confidence
            float conf = joint.confidence;
            ofSetColor(
                static_cast<int>((1.0f - conf) * 255),
                static_cast<int>(conf * 255),
                0,
                200
            );

            float x = joint.x * width;
            float y = joint.y * height;
            ofDrawCircle(x, y, jointRadius);
        }
    }
}

void PoseEstimator::drawSkeletons(const std::vector<HumanPose>& poses,
                                  float width,
                                  float height,
                                  float jointRadius,
                                  float boneThickness) {
    for (const auto& pose : poses) {
        drawSkeleton(pose, width, height, jointRadius, boneThickness);
    }
}

void PoseEstimator::drawJoint(const Joint& joint,
                              float width,
                              float height,
                              float radius) {
    if (!joint.isDetected) return;

    using namespace oflike;
    ofFill();
    ofSetColor(255, 255, 0, 200);

    float x = joint.x * width;
    float y = joint.y * height;
    ofDrawCircle(x, y, radius);
}

void PoseEstimator::drawConnection(const Joint& joint1,
                                   const Joint& joint2,
                                   float width,
                                   float height,
                                   float thickness) {
    if (!joint1.isDetected || !joint2.isDetected) return;

    using namespace oflike;
    ofSetLineWidth(thickness);
    ofSetColor(0, 255, 0, 200);

    float x1 = joint1.x * width;
    float y1 = joint1.y * height;
    float x2 = joint2.x * width;
    float y2 = joint2.y * height;

    ofDrawLine(x1, y1, x2, y2);
}

// ============================================================================
// Configuration
// ============================================================================

void PoseEstimator::setMinJointConfidence(float threshold) {
    impl_->config.minJointConfidence = std::max(0.0f, std::min(1.0f, threshold));
}

float PoseEstimator::getMinJointConfidence() const {
    return impl_->config.minJointConfidence;
}

void PoseEstimator::setMinPoseConfidence(float threshold) {
    impl_->config.minPoseConfidence = std::max(0.0f, std::min(1.0f, threshold));
}

float PoseEstimator::getMinPoseConfidence() const {
    return impl_->config.minPoseConfidence;
}

void PoseEstimator::setMaxPeople(size_t maxPeople) {
    impl_->config.maxPeople = maxPeople;
}

size_t PoseEstimator::getMaxPeople() const {
    return impl_->config.maxPeople;
}

// ============================================================================
// Information
// ============================================================================

bool PoseEstimator::isReady() const {
    return impl_->isReady;
}

std::vector<SkeletonConnection> PoseEstimator::getStandardConnections() {
    return {
        // Head
        {JointName::Nose, JointName::Neck},
        {JointName::Nose, JointName::LeftEye},
        {JointName::Nose, JointName::RightEye},
        {JointName::LeftEye, JointName::LeftEar},
        {JointName::RightEye, JointName::RightEar},

        // Torso
        {JointName::Neck, JointName::Root},
        {JointName::Neck, JointName::LeftShoulder},
        {JointName::Neck, JointName::RightShoulder},
        {JointName::Root, JointName::LeftHip},
        {JointName::Root, JointName::RightHip},

        // Left arm
        {JointName::LeftShoulder, JointName::LeftElbow},
        {JointName::LeftElbow, JointName::LeftWrist},

        // Right arm
        {JointName::RightShoulder, JointName::RightElbow},
        {JointName::RightElbow, JointName::RightWrist},

        // Left leg
        {JointName::LeftHip, JointName::LeftKnee},
        {JointName::LeftKnee, JointName::LeftAnkle},

        // Right leg
        {JointName::RightHip, JointName::RightKnee},
        {JointName::RightKnee, JointName::RightAnkle}
    };
}

std::string PoseEstimator::getJointName(JointName joint) {
    switch (joint) {
        case JointName::Nose: return "Nose";
        case JointName::LeftEye: return "LeftEye";
        case JointName::RightEye: return "RightEye";
        case JointName::LeftEar: return "LeftEar";
        case JointName::RightEar: return "RightEar";
        case JointName::LeftShoulder: return "LeftShoulder";
        case JointName::RightShoulder: return "RightShoulder";
        case JointName::LeftElbow: return "LeftElbow";
        case JointName::RightElbow: return "RightElbow";
        case JointName::LeftWrist: return "LeftWrist";
        case JointName::RightWrist: return "RightWrist";
        case JointName::LeftHip: return "LeftHip";
        case JointName::RightHip: return "RightHip";
        case JointName::LeftKnee: return "LeftKnee";
        case JointName::RightKnee: return "RightKnee";
        case JointName::LeftAnkle: return "LeftAnkle";
        case JointName::RightAnkle: return "RightAnkle";
        case JointName::Neck: return "Neck";
        case JointName::Root: return "Root";
        default: return "Unknown";
    }
}

std::string PoseEstimator::getLastError() const {
    return impl_->lastError;
}

} // namespace NeuralEngine
