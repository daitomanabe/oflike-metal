#include "ImageClassifier.h"
#include "ofPixels.h"
#include "ofTexture.h"
#include <Foundation/Foundation.h>
#include <CoreML/CoreML.h>
#include <Vision/Vision.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>

using namespace oflike;

namespace NeuralEngine {

// ============================================================================
// Implementation (pImpl pattern)
// ============================================================================

struct ImageClassifier::Impl {
    MLModel* model = nil;
    VNCoreMLModel* visionModel = nil;
    ClassifierConfig config;
    ClassifierInfo info;
    std::string lastError;

    // Class labels (if available in model metadata)
    std::vector<std::string> classLabels;

    ~Impl() {
        @autoreleasepool {
            model = nil;
            visionModel = nil;
        }
    }

    bool loadModel(const std::string& modelPath, const ClassifierConfig& cfg) {
        @autoreleasepool {
            config = cfg;

            // Convert path to NSURL
            NSString* path = [NSString stringWithUTF8String:modelPath.c_str()];
            NSURL* modelURL = [NSURL fileURLWithPath:path];

            // Create model configuration
            MLModelConfiguration* configuration = [[MLModelConfiguration alloc] init];

            // Set compute units
            switch (config.computeUnits) {
                case ClassifierConfig::ComputeUnits::CPUOnly:
                    configuration.computeUnits = MLComputeUnitsCPUOnly;
                    break;
                case ClassifierConfig::ComputeUnits::CPUAndGPU:
                    configuration.computeUnits = MLComputeUnitsCPUAndGPU;
                    break;
                case ClassifierConfig::ComputeUnits::CPUAndNeuralEngine:
                    configuration.computeUnits = MLComputeUnitsCPUAndNeuralEngine;
                    break;
                case ClassifierConfig::ComputeUnits::All:
                default:
                    configuration.computeUnits = MLComputeUnitsAll;
                    break;
            }

            // Load model
            NSError* error = nil;
            model = [MLModel modelWithContentsOfURL:modelURL configuration:configuration error:&error];

            if (error || !model) {
                lastError = error ? [[error localizedDescription] UTF8String] : "Failed to load model";
                return false;
            }

            // Create Vision model wrapper
            visionModel = [VNCoreMLModel modelForMLModel:model error:&error];

            if (error || !visionModel) {
                lastError = error ? [[error localizedDescription] UTF8String] : "Failed to create Vision model";
                return false;
            }

            // Get model info
            info.modelPath = modelPath;
            info.isLoaded = true;

            // Try to get model description
            MLModelDescription* description = model.modelDescription;

            // Get input image size (look for image input)
            for (MLFeatureDescription* input in description.inputDescriptionsByName.allValues) {
                if (input.type == MLFeatureTypeImage) {
                    MLImageConstraint* imageConstraint = input.imageConstraint;
                    if (imageConstraint) {
                        info.inputWidth = imageConstraint.pixelsWide;
                        info.inputHeight = imageConstraint.pixelsHigh;
                    }
                    break;
                }
            }

            // Check if Neural Engine is supported
            if (@available(macOS 13.0, *)) {
                info.isNeuralEngineSupported = (configuration.computeUnits == MLComputeUnitsAll ||
                                                configuration.computeUnits == MLComputeUnitsCPUAndNeuralEngine);
            }

            // Try to extract class labels from model metadata
            NSDictionary* metadata = description.metadata[MLModelDescriptionKey];
            if (metadata && [metadata isKindOfClass:[NSDictionary class]]) {
                // Common keys for class labels
                NSArray* labels = metadata[@"classes"];
                if (!labels) labels = metadata[@"classLabels"];
                if (!labels) labels = metadata[@"MLModelDescriptionKey"][@"classes"];

                if (labels && [labels isKindOfClass:[NSArray class]]) {
                    classLabels.clear();
                    for (id label in labels) {
                        if ([label isKindOfClass:[NSString class]]) {
                            classLabels.push_back([(NSString*)label UTF8String]);
                        }
                    }
                    info.numClasses = classLabels.size();
                }
            }

            // If we couldn't get labels from metadata, try to infer from output
            if (classLabels.empty()) {
                // Look for classifier output (typically a dictionary or array)
                for (MLFeatureDescription* output in description.outputDescriptionsByName.allValues) {
                    if (output.type == MLFeatureTypeDictionary) {
                        // This is likely a classification output
                        // We can't get the labels without running inference
                        // Just set a placeholder count
                        info.numClasses = 1000; // Common for ImageNet models
                        break;
                    }
                }
            }

            return true;
        }
    }

    std::vector<ClassificationResult> classifyPixels(const ofPixels& pixels) {
        @autoreleasepool {
            if (!visionModel) {
                lastError = "Model not loaded";
                return {};
            }

            // Create CGImage from ofPixels
            size_t width = pixels.getWidth();
            size_t height = pixels.getHeight();
            size_t channels = pixels.getNumChannels();

            if (channels != 3 && channels != 4) {
                lastError = "Only RGB and RGBA images are supported";
                return {};
            }

            // Create color space
            CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

            // Create bitmap context
            CGBitmapInfo bitmapInfo = channels == 4 ? kCGImageAlphaPremultipliedLast : kCGImageAlphaNoneSkipLast;
            size_t bytesPerRow = width * 4; // Always use 4 channels for CG

            // Copy pixels to RGBA format
            std::vector<unsigned char> rgbaData(width * height * 4);
            const unsigned char* src = pixels.getData();

            for (size_t i = 0; i < width * height; ++i) {
                rgbaData[i * 4 + 0] = src[i * channels + 0]; // R
                rgbaData[i * 4 + 1] = src[i * channels + 1]; // G
                rgbaData[i * 4 + 2] = src[i * channels + 2]; // B
                rgbaData[i * 4 + 3] = channels == 4 ? src[i * channels + 3] : 255; // A
            }

            CGContextRef context = CGBitmapContextCreate(rgbaData.data(), width, height, 8,
                                                         bytesPerRow, colorSpace,
                                                         bitmapInfo);

            if (!context) {
                CGColorSpaceRelease(colorSpace);
                lastError = "Failed to create bitmap context";
                return {};
            }

            CGImageRef cgImage = CGBitmapContextCreateImage(context);
            CGContextRelease(context);
            CGColorSpaceRelease(colorSpace);

            if (!cgImage) {
                lastError = "Failed to create CGImage";
                return {};
            }

            // Create Vision request
            VNImageRequestHandler* handler = [[VNImageRequestHandler alloc] initWithCGImage:cgImage
                                                                                    options:@{}];
            CGImageRelease(cgImage);

            // Create classification request
            VNCoreMLRequest* request = [[VNCoreMLRequest alloc] initWithModel:visionModel];

            // Perform inference
            NSError* error = nil;
            BOOL success = [handler performRequests:@[request] error:&error];

            if (!success || error) {
                lastError = error ? [[error localizedDescription] UTF8String] : "Classification failed";
                return {};
            }

            // Extract results
            std::vector<ClassificationResult> results;

            NSArray<VNObservation*>* observations = request.results;
            for (VNObservation* observation in observations) {
                if ([observation isKindOfClass:[VNClassificationObservation class]]) {
                    VNClassificationObservation* classification = (VNClassificationObservation*)observation;

                    // Filter by confidence threshold
                    if (classification.confidence >= config.minConfidence) {
                        ClassificationResult result;
                        result.label = [classification.identifier UTF8String];
                        result.confidence = classification.confidence;

                        // Try to find class index if we have labels
                        result.classIndex = -1;
                        if (!classLabels.empty()) {
                            auto it = std::find(classLabels.begin(), classLabels.end(), result.label);
                            if (it != classLabels.end()) {
                                result.classIndex = static_cast<int>(std::distance(classLabels.begin(), it));
                            }
                        }

                        results.push_back(result);
                    }
                }
            }

            // Sort by confidence (highest first)
            std::sort(results.begin(), results.end(),
                     [](const ClassificationResult& a, const ClassificationResult& b) {
                         return a.confidence > b.confidence;
                     });

            // Limit to top-K
            if (results.size() > config.topK) {
                results.resize(config.topK);
            }

            return results;
        }
    }
};

// ============================================================================
// ImageClassifier Implementation
// ============================================================================

ImageClassifier::ImageClassifier()
    : impl_(std::make_unique<Impl>()) {
}

ImageClassifier::~ImageClassifier() = default;

ImageClassifier::ImageClassifier(ImageClassifier&& other) noexcept = default;
ImageClassifier& ImageClassifier::operator=(ImageClassifier&& other) noexcept = default;

bool ImageClassifier::load(const std::string& modelPath) {
    ClassifierConfig defaultConfig;
    return load(modelPath, defaultConfig);
}

bool ImageClassifier::load(const std::string& modelPath, const ClassifierConfig& config) {
    return impl_->loadModel(modelPath, config);
}

void ImageClassifier::unload() {
    @autoreleasepool {
        impl_->model = nil;
        impl_->visionModel = nil;
        impl_->info = ClassifierInfo();
        impl_->classLabels.clear();
    }
}

std::vector<ClassificationResult> ImageClassifier::classify(const ofPixels& pixels) {
    return impl_->classifyPixels(pixels);
}

std::vector<ClassificationResult> ImageClassifier::classify(const ofTexture& texture) {
    // Download texture pixels and classify
    ofPixels pixels;
    texture.readToPixels(pixels);
    return classify(pixels);
}

void ImageClassifier::setTopK(size_t k) {
    impl_->config.topK = k;
}

size_t ImageClassifier::getTopK() const {
    return impl_->config.topK;
}

void ImageClassifier::setMinConfidence(float threshold) {
    impl_->config.minConfidence = std::max(0.0f, std::min(1.0f, threshold));
}

float ImageClassifier::getMinConfidence() const {
    return impl_->config.minConfidence;
}

bool ImageClassifier::isLoaded() const {
    return impl_->info.isLoaded;
}

ClassifierInfo ImageClassifier::getInfo() const {
    return impl_->info;
}

std::vector<std::string> ImageClassifier::getClassLabels() const {
    return impl_->classLabels;
}

std::string ImageClassifier::getLastError() const {
    return impl_->lastError;
}

} // namespace NeuralEngine
