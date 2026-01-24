#pragma once

// ofxNeuralEngine - Machine Learning and Computer Vision with Core ML and Vision
//
// This addon provides easy-to-use interfaces for:
// - Image Classification (Phase 26.1) ✅
// - Pose Estimation (Phase 26.2) ✅
// - Depth Estimation (Phase 26.3) ✅
// - Style Transfer (Phase 26.4)
// - Person Segmentation (Phase 26.5)
// - Generic Core ML Models (Phase 26.6)
//
// All powered by Apple's Core ML and Vision frameworks with Neural Engine acceleration.
//
// Example usage:
//   #include "ofxNeuralEngine.h"
//
//   // Image classification
//   ofxNeuralEngine::ImageClassifier classifier;
//   classifier.load("MobileNetV3.mlmodelc");
//   auto results = classifier.classify(myImage);
//
//   // Pose estimation
//   ofxNeuralEngine::PoseEstimator poseEstimator;
//   poseEstimator.setup();
//   auto poses = poseEstimator.estimate(myImage);
//   poseEstimator.drawSkeletons(poses, width, height);
//
//   // Depth estimation
//   ofxNeuralEngine::DepthEstimator depthEstimator;
//   depthEstimator.load("DepthAnything.mlmodelc");
//   ofPixels depthMap = depthEstimator.estimate(myImage);

// Core ML Image Classification
#include "ImageClassifier.h"

// Vision Pose Estimation
#include "PoseEstimator.h"

// Core ML Depth Estimation
#include "DepthEstimator.h"

// Convenience namespace alias
namespace ofxNeuralEngine = NeuralEngine;
