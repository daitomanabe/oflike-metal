#pragma once

// PoseEstimator - Vision framework human pose detection wrapper
//
// This class provides easy-to-use human body pose estimation using Apple's Vision framework.
// Leverages VNDetectHumanBodyPoseRequest for real-time pose detection on images and video.
//
// Features:
// - Full body skeleton detection (19 key points)
// - Confidence scores per joint
// - Multiple person detection
// - Real-time performance
// - Neural Engine acceleration
//
// Example usage:
//   PoseEstimator estimator;
//   estimator.setup();
//   auto poses = estimator.estimate(myImage);
//   for (const auto& pose : poses) {
//       estimator.drawSkeleton(pose);
//   }

#include <string>
#include <vector>
#include <memory>

// Forward declarations
namespace oflike {
    class ofPixels;
    class ofTexture;
    class ofVec2f;
}

namespace NeuralEngine {

// Human body joint names (VNHumanBodyPoseObservation joint names)
enum class JointName {
    Nose,
    LeftEye,
    RightEye,
    LeftEar,
    RightEar,
    LeftShoulder,
    RightShoulder,
    LeftElbow,
    RightElbow,
    LeftWrist,
    RightWrist,
    LeftHip,
    RightHip,
    LeftKnee,
    RightKnee,
    LeftAnkle,
    RightAnkle,
    Neck,
    Root
};

// Single body joint with position and confidence
struct Joint {
    JointName name;
    float x;                  // X coordinate (normalized 0.0 to 1.0)
    float y;                  // Y coordinate (normalized 0.0 to 1.0)
    float confidence;         // Confidence score 0.0 to 1.0
    bool isDetected;         // Whether joint was detected

    Joint();
    Joint(JointName n, float x, float y, float conf);
};

// Skeleton connection (bone) between two joints
struct SkeletonConnection {
    JointName joint1;
    JointName joint2;
};

// Single person's pose (full body skeleton)
struct HumanPose {
    std::vector<Joint> joints;     // All detected joints
    float overallConfidence;       // Average confidence of all joints
    int personID;                 // Person identifier (for tracking)

    HumanPose();

    // Get joint by name
    const Joint* getJoint(JointName name) const;
    Joint* getJoint(JointName name);

    // Check if joint exists and has sufficient confidence
    bool hasJoint(JointName name, float minConfidence = 0.3f) const;

    // Get all detected joints above confidence threshold
    std::vector<Joint> getDetectedJoints(float minConfidence = 0.3f) const;
};

// Configuration for pose estimator
struct PoseEstimatorConfig {
    // Minimum confidence threshold for joint detection (0.0 to 1.0)
    float minJointConfidence = 0.3f;

    // Minimum confidence threshold for overall pose detection
    float minPoseConfidence = 0.3f;

    // Maximum number of people to detect (0 = unlimited)
    size_t maxPeople = 0;

    // Whether to return normalized coordinates (0.0-1.0) or pixel coordinates
    bool normalizedCoordinates = true;
};

class PoseEstimator {
public:
    // ============================================================================
    // Constructors / Destructor
    // ============================================================================

    PoseEstimator();
    ~PoseEstimator();

    // Move semantics (disable copy due to Vision resources)
    PoseEstimator(PoseEstimator&& other) noexcept;
    PoseEstimator& operator=(PoseEstimator&& other) noexcept;

    // Disable copy
    PoseEstimator(const PoseEstimator&) = delete;
    PoseEstimator& operator=(const PoseEstimator&) = delete;

    // ============================================================================
    // Setup
    // ============================================================================

    // Setup pose estimator with default configuration
    bool setup();

    // Setup pose estimator with custom configuration
    bool setup(const PoseEstimatorConfig& config);

    // Shutdown and release resources
    void shutdown();

    // ============================================================================
    // Pose Estimation
    // ============================================================================

    // Estimate human poses from ofPixels
    // Returns vector of detected poses (may be empty if no people detected)
    std::vector<HumanPose> estimate(const oflike::ofPixels& pixels);

    // Estimate human poses from ofTexture
    // Returns vector of detected poses (may be empty if no people detected)
    std::vector<HumanPose> estimate(const oflike::ofTexture& texture);

    // ============================================================================
    // Drawing Helpers
    // ============================================================================

    // Draw skeleton for a single pose
    // Draws bones (connections) and joints (circles)
    // Parameters:
    //   pose - The pose to draw
    //   width - Image width (for denormalizing coordinates)
    //   height - Image height (for denormalizing coordinates)
    //   jointRadius - Radius of joint circles in pixels
    //   boneThickness - Thickness of bone lines in pixels
    void drawSkeleton(const HumanPose& pose,
                     float width,
                     float height,
                     float jointRadius = 5.0f,
                     float boneThickness = 2.0f);

    // Draw all skeletons from multiple poses
    void drawSkeletons(const std::vector<HumanPose>& poses,
                      float width,
                      float height,
                      float jointRadius = 5.0f,
                      float boneThickness = 2.0f);

    // Draw individual joint as circle
    void drawJoint(const Joint& joint,
                  float width,
                  float height,
                  float radius = 5.0f);

    // Draw connection (bone) between two joints
    void drawConnection(const Joint& joint1,
                       const Joint& joint2,
                       float width,
                       float height,
                       float thickness = 2.0f);

    // ============================================================================
    // Configuration
    // ============================================================================

    // Set minimum joint confidence threshold
    void setMinJointConfidence(float threshold);

    // Get current joint confidence threshold
    float getMinJointConfidence() const;

    // Set minimum pose confidence threshold
    void setMinPoseConfidence(float threshold);

    // Get current pose confidence threshold
    float getMinPoseConfidence() const;

    // Set maximum number of people to detect (0 = unlimited)
    void setMaxPeople(size_t maxPeople);

    // Get current max people setting
    size_t getMaxPeople() const;

    // ============================================================================
    // Information
    // ============================================================================

    // Check if pose estimator is ready
    bool isReady() const;

    // Get skeleton connections (standard human body bones)
    static std::vector<SkeletonConnection> getStandardConnections();

    // Get joint name as string
    static std::string getJointName(JointName joint);

    // Get last error message
    std::string getLastError() const;

private:
    // pImpl pattern - hide Objective-C++ implementation
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace NeuralEngine
