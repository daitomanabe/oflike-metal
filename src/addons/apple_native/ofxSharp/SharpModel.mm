#import "SharpModel.h"
#import "SharpGaussian.h"
#import "ofPixels.h"
#import "ofTexture.h"
#import <CoreML/CoreML.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <Accelerate/Accelerate.h>
#import <simd/simd.h>
#import <dispatch/dispatch.h>
#include <chrono>
#include <vector>
#include <string>

namespace Sharp {

// ============================================================================
// Private Implementation
// ============================================================================

class SharpModel::Impl {
public:
    Impl()
        : model_(nil)
        , device_(nil)
        , config_()
        , modelInfo_()
        , lastStatus_(ModelStatus::Success)
        , lastError_("")
        , lastInferenceTime_(0.0)
        , totalInferenceTime_(0.0)
        , predictionCount_(0)
        , neuralEngineUsed_(false)
        , inferenceInProgress_(false)
        , minOpacity_(0.01f)
        , maxGaussians_(0)
        , postProcessingEnabled_(true) {
        @autoreleasepool {
            device_ = MTLCreateSystemDefaultDevice();
        }
    }

    ~Impl() {
        @autoreleasepool {
            unload();
            device_ = nil;
        }
    }

    // ========================================================================
    // Model Loading
    // ========================================================================

    bool load(const std::string& modelPath, const ModelConfig& config) {
        @autoreleasepool {
            unload();

            config_ = config;
            modelInfo_.modelPath = modelPath;

            // Convert path to NSURL
            NSString* nsPath = [NSString stringWithUTF8String:modelPath.c_str()];
            NSURL* modelURL = [NSURL fileURLWithPath:nsPath];

            // Check if file exists
            if (![[NSFileManager defaultManager] fileExistsAtPath:nsPath]) {
                lastStatus_ = ModelStatus::ErrorInvalidModelFormat;
                lastError_ = "Model file not found: " + modelPath;
                return false;
            }

            // Create model configuration
            MLModelConfiguration* mlConfig = [[MLModelConfiguration alloc] init];

            // Set compute units based on config
            switch (config.computeUnits) {
                case ModelConfig::ComputeUnits::All:
                    mlConfig.computeUnits = MLComputeUnitsAll;
                    break;
                case ModelConfig::ComputeUnits::CPUOnly:
                    mlConfig.computeUnits = MLComputeUnitsCPUOnly;
                    break;
                case ModelConfig::ComputeUnits::CPUAndGPU:
                    mlConfig.computeUnits = MLComputeUnitsCPUAndGPU;
                    break;
                case ModelConfig::ComputeUnits::CPUAndNeuralEngine:
                    mlConfig.computeUnits = MLComputeUnitsCPUAndNeuralEngine;
                    break;
            }

            // Enable low power mode if requested
            if (@available(macOS 13.0, *)) {
                // Note: MLModelConfiguration doesn't have a direct lowPowerMode property
                // This would need to be configured through MLPredictionOptions at inference time
            }

            // Load model
            NSError* error = nil;
            model_ = [MLModel modelWithContentsOfURL:modelURL
                                       configuration:mlConfig
                                               error:&error];

            if (error || !model_) {
                lastStatus_ = ModelStatus::ErrorInvalidModelFormat;
                lastError_ = error ? [[error localizedDescription] UTF8String] : "Unknown error loading model";
                model_ = nil;
                return false;
            }

            // Get model description
            MLModelDescription* description = model_.modelDescription;

            // Extract input dimensions (assuming image input)
            NSDictionary* inputDict = description.inputDescriptionsByName;
            if (inputDict.count > 0) {
                MLFeatureDescription* inputDesc = inputDict.allValues.firstObject;
                if (inputDesc.type == MLFeatureTypeImage) {
                    MLImageConstraint* imageConstraint = inputDesc.imageConstraint;
                    modelInfo_.inputWidth = imageConstraint.pixelsWide;
                    modelInfo_.inputHeight = imageConstraint.pixelsHigh;
                }
            }

            // Check Neural Engine support
            modelInfo_.isNeuralEngineSupported = [self isNeuralEngineSupported];
            modelInfo_.isLoaded = true;

            // Store configuration
            minOpacity_ = config.minOpacity;
            maxGaussians_ = config.maxGaussians;

            lastStatus_ = ModelStatus::Success;
            lastError_ = "";

            return true;
        }
    }

    void unload() {
        @autoreleasepool {
            if (model_) {
                model_ = nil;
                modelInfo_.isLoaded = false;
                modelInfo_.modelPath = "";
            }
        }
    }

    bool isLoaded() const {
        return model_ != nil && modelInfo_.isLoaded;
    }

    ModelInfo getModelInfo() const {
        return modelInfo_;
    }

    // ========================================================================
    // Synchronous Inference
    // ========================================================================

    GaussianCloud predict(const oflike::ofPixels& pixels) {
        if (!isLoaded()) {
            lastStatus_ = ModelStatus::ErrorModelNotLoaded;
            lastError_ = "Model not loaded";
            return GaussianCloud();
        }

        @autoreleasepool {
            auto startTime = std::chrono::high_resolution_clock::now();

            // Convert ofPixels to CVPixelBuffer
            CVPixelBufferRef pixelBuffer = nullptr;
            if (![self createPixelBufferFromPixels:pixels buffer:&pixelBuffer]) {
                lastStatus_ = ModelStatus::ErrorInvalidInput;
                lastError_ = "Failed to convert input pixels to CVPixelBuffer";
                return GaussianCloud();
            }

            // Perform inference
            GaussianCloud cloud = [self performInference:pixelBuffer];

            // Release pixel buffer
            CVPixelBufferRelease(pixelBuffer);

            // Record timing
            auto endTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = endTime - startTime;
            lastInferenceTime_ = duration.count();
            totalInferenceTime_ += lastInferenceTime_;
            predictionCount_++;

            return cloud;
        }
    }

    GaussianCloud predict(const oflike::ofTexture& texture) {
        if (!isLoaded()) {
            lastStatus_ = ModelStatus::ErrorModelNotLoaded;
            lastError_ = "Model not loaded";
            return GaussianCloud();
        }

        @autoreleasepool {
            auto startTime = std::chrono::high_resolution_clock::now();

            // Get Metal texture
            id<MTLTexture> mtlTexture = (__bridge id<MTLTexture>)texture.getNativeHandle();
            if (!mtlTexture) {
                lastStatus_ = ModelStatus::ErrorInvalidInput;
                lastError_ = "Invalid Metal texture";
                return GaussianCloud();
            }

            // Convert Metal texture to CVPixelBuffer
            CVPixelBufferRef pixelBuffer = nullptr;
            if (![self createPixelBufferFromTexture:mtlTexture buffer:&pixelBuffer]) {
                lastStatus_ = ModelStatus::ErrorInvalidInput;
                lastError_ = "Failed to convert Metal texture to CVPixelBuffer";
                return GaussianCloud();
            }

            // Perform inference
            GaussianCloud cloud = [self performInference:pixelBuffer];

            // Release pixel buffer
            CVPixelBufferRelease(pixelBuffer);

            // Record timing
            auto endTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = endTime - startTime;
            lastInferenceTime_ = duration.count();
            totalInferenceTime_ += lastInferenceTime_;
            predictionCount_++;

            return cloud;
        }
    }

    // ========================================================================
    // Asynchronous Inference
    // ========================================================================

    void predictAsync(const oflike::ofPixels& pixels, PredictCallback callback) {
        if (!isLoaded()) {
            callback(GaussianCloud(), ModelStatus::ErrorModelNotLoaded);
            return;
        }

        if (inferenceInProgress_) {
            callback(GaussianCloud(), ModelStatus::ErrorInferenceFailed);
            return;
        }

        inferenceInProgress_ = true;

        // Create a copy of pixels for async operation
        auto pixelsCopy = std::make_shared<oflike::ofPixels>(pixels);

        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
            GaussianCloud cloud = predict(*pixelsCopy);
            ModelStatus status = lastStatus_;

            inferenceInProgress_ = false;
            callback(std::move(cloud), status);
        });
    }

    void predictAsync(const oflike::ofTexture& texture, PredictCallback callback) {
        if (!isLoaded()) {
            callback(GaussianCloud(), ModelStatus::ErrorModelNotLoaded);
            return;
        }

        if (inferenceInProgress_) {
            callback(GaussianCloud(), ModelStatus::ErrorInferenceFailed);
            return;
        }

        inferenceInProgress_ = true;

        // Note: Texture needs to be copied for async operation
        // For now, perform sync on background thread
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
            GaussianCloud cloud = predict(texture);
            ModelStatus status = lastStatus_;

            inferenceInProgress_ = false;
            callback(std::move(cloud), status);
        });
    }

    bool isInferenceInProgress() const {
        return inferenceInProgress_;
    }

    void cancelInference() {
        // Core ML doesn't support cancellation mid-inference
        // This would require additional threading infrastructure
        inferenceInProgress_ = false;
    }

    // ========================================================================
    // Error Handling
    // ========================================================================

    ModelStatus getLastStatus() const {
        return lastStatus_;
    }

    std::string getLastError() const {
        return lastError_;
    }

    // ========================================================================
    // Performance Metrics
    // ========================================================================

    double getLastInferenceTime() const {
        return lastInferenceTime_;
    }

    double getAverageInferenceTime() const {
        if (predictionCount_ == 0) return 0.0;
        return totalInferenceTime_ / predictionCount_;
    }

    size_t getPredictionCount() const {
        return predictionCount_;
    }

    bool wasNeuralEngineUsed() const {
        return neuralEngineUsed_;
    }

    // ========================================================================
    // Configuration
    // ========================================================================

    void setMinOpacity(float threshold) {
        minOpacity_ = threshold;
    }

    void setMaxGaussians(size_t maxCount) {
        maxGaussians_ = maxCount;
    }

    void setPostProcessingEnabled(bool enabled) {
        postProcessingEnabled_ = enabled;
    }

private:
    // ========================================================================
    // Internal Helper Methods
    // ========================================================================

    GaussianCloud performInference(CVPixelBufferRef pixelBuffer) {
        @autoreleasepool {
            // Create MLFeatureValue from pixel buffer
            MLFeatureValue* inputValue = [MLFeatureValue featureValueWithPixelBuffer:pixelBuffer];
            if (!inputValue) {
                lastStatus_ = ModelStatus::ErrorInvalidInput;
                lastError_ = "Failed to create MLFeatureValue from pixel buffer";
                return GaussianCloud();
            }

            // Get input feature name
            NSString* inputName = model_.modelDescription.inputDescriptionsByName.allKeys.firstObject;
            if (!inputName) {
                lastStatus_ = ModelStatus::ErrorInferenceFailed;
                lastError_ = "Could not determine input feature name";
                return GaussianCloud();
            }

            // Create input feature provider
            NSError* error = nil;
            MLDictionaryFeatureProvider* inputProvider =
                [[MLDictionaryFeatureProvider alloc] initWithDictionary:@{inputName: inputValue}
                                                                   error:&error];
            if (error || !inputProvider) {
                lastStatus_ = ModelStatus::ErrorInvalidInput;
                lastError_ = error ? [[error localizedDescription] UTF8String] : "Failed to create input provider";
                return GaussianCloud();
            }

            // Create prediction options
            MLPredictionOptions* options = [[MLPredictionOptions alloc] init];

            // Perform prediction
            id<MLFeatureProvider> output = [model_ predictionFromFeatures:inputProvider
                                                                  options:options
                                                                    error:&error];
            if (error || !output) {
                lastStatus_ = ModelStatus::ErrorInferenceFailed;
                lastError_ = error ? [[error localizedDescription] UTF8String] : "Inference failed";
                return GaussianCloud();
            }

            // Parse output to GaussianCloud
            GaussianCloud cloud = [self parseModelOutput:output];

            // Apply post-processing if enabled
            if (postProcessingEnabled_) {
                [self applyPostProcessing:cloud];
            }

            lastStatus_ = ModelStatus::Success;
            lastError_ = "";

            return cloud;
        }
    }

    bool createPixelBufferFromPixels(const oflike::ofPixels& pixels, CVPixelBufferRef* outBuffer) {
        @autoreleasepool {
            size_t width = pixels.getWidth();
            size_t height = pixels.getHeight();
            size_t channels = pixels.getNumChannels();

            // Determine pixel format
            OSType pixelFormat;
            if (channels == 3) {
                pixelFormat = kCVPixelFormatType_24RGB;
            } else if (channels == 4) {
                pixelFormat = kCVPixelFormatType_32RGBA;
            } else if (channels == 1) {
                pixelFormat = kCVPixelFormatType_OneComponent8;
            } else {
                return false;
            }

            // Create pixel buffer
            NSDictionary* options = @{
                (id)kCVPixelBufferMetalCompatibilityKey: @YES,
                (id)kCVPixelBufferIOSurfacePropertiesKey: @{}
            };

            CVReturn status = CVPixelBufferCreate(
                kCFAllocatorDefault,
                width,
                height,
                pixelFormat,
                (__bridge CFDictionaryRef)options,
                outBuffer
            );

            if (status != kCVReturnSuccess || !outBuffer) {
                return false;
            }

            // Lock pixel buffer
            CVPixelBufferLockBaseAddress(*outBuffer, 0);

            // Copy pixel data
            void* baseAddress = CVPixelBufferGetBaseAddress(*outBuffer);
            size_t bytesPerRow = CVPixelBufferGetBytesPerRow(*outBuffer);
            const unsigned char* src = pixels.getData();

            for (size_t y = 0; y < height; y++) {
                memcpy((uint8_t*)baseAddress + y * bytesPerRow,
                       src + y * width * channels,
                       width * channels);
            }

            // Unlock pixel buffer
            CVPixelBufferUnlockBaseAddress(*outBuffer, 0);

            return true;
        }
    }

    bool createPixelBufferFromTexture(id<MTLTexture> texture, CVPixelBufferRef* outBuffer) {
        @autoreleasepool {
            size_t width = texture.width;
            size_t height = texture.height;

            // Create pixel buffer
            NSDictionary* options = @{
                (id)kCVPixelBufferMetalCompatibilityKey: @YES,
                (id)kCVPixelBufferIOSurfacePropertiesKey: @{}
            };

            CVReturn status = CVPixelBufferCreate(
                kCFAllocatorDefault,
                width,
                height,
                kCVPixelFormatType_32RGBA,
                (__bridge CFDictionaryRef)options,
                outBuffer
            );

            if (status != kCVReturnSuccess || !outBuffer) {
                return false;
            }

            // Lock pixel buffer
            CVPixelBufferLockBaseAddress(*outBuffer, 0);

            // Read texture data
            void* baseAddress = CVPixelBufferGetBaseAddress(*outBuffer);
            size_t bytesPerRow = CVPixelBufferGetBytesPerRow(*outBuffer);

            [texture getBytes:baseAddress
                  bytesPerRow:bytesPerRow
                   fromRegion:MTLRegionMake2D(0, 0, width, height)
                  mipmapLevel:0];

            // Unlock pixel buffer
            CVPixelBufferUnlockBaseAddress(*outBuffer, 0);

            return true;
        }
    }

    GaussianCloud parseModelOutput(id<MLFeatureProvider> output) {
        @autoreleasepool {
            GaussianCloud cloud;

            // Note: This is a placeholder implementation
            // The actual parsing logic depends on the SHARP model's output format
            // which typically includes:
            // - positions: [N, 3] array of xyz coordinates
            // - scales: [N, 3] array of scale factors
            // - rotations: [N, 4] array of quaternions
            // - opacities: [N, 1] array of alpha values
            // - sh_dc: [N, 3] array of base colors (RGB)
            // - sh_rest: [N, 15, 3] array of SH coefficients

            NSSet<NSString*>* outputNames = output.featureNames;

            // Try to extract Gaussian parameters from output
            // This is model-specific and would need to be adapted
            // for the actual SHARP model output format

            for (NSString* name in outputNames) {
                MLFeatureValue* value = [output featureValueForName:name];

                if (value.type == MLFeatureTypeMultiArray) {
                    MLMultiArray* array = value.multiArrayValue;

                    // Parse based on output name
                    if ([name isEqualToString:@"positions"]) {
                        [self parsePositions:array intoCloud:cloud];
                    } else if ([name isEqualToString:@"scales"]) {
                        [self parseScales:array intoCloud:cloud];
                    } else if ([name isEqualToString:@"rotations"]) {
                        [self parseRotations:array intoCloud:cloud];
                    } else if ([name isEqualToString:@"opacities"]) {
                        [self parseOpacities:array intoCloud:cloud];
                    } else if ([name isEqualToString:@"sh_dc"]) {
                        [self parseSHDC:array intoCloud:cloud];
                    }
                }
            }

            return cloud;
        }
    }

    void parsePositions(MLMultiArray* array, GaussianCloud& cloud) {
        // Placeholder - actual implementation depends on model output format
    }

    void parseScales(MLMultiArray* array, GaussianCloud& cloud) {
        // Placeholder - actual implementation depends on model output format
    }

    void parseRotations(MLMultiArray* array, GaussianCloud& cloud) {
        // Placeholder - actual implementation depends on model output format
    }

    void parseOpacities(MLMultiArray* array, GaussianCloud& cloud) {
        // Placeholder - actual implementation depends on model output format
    }

    void parseSHDC(MLMultiArray* array, GaussianCloud& cloud) {
        // Placeholder - actual implementation depends on model output format
    }

    void applyPostProcessing(GaussianCloud& cloud) {
        // Apply minimum opacity threshold
        if (minOpacity_ > 0.0f) {
            cloud.filterByOpacity(minOpacity_);
        }

        // Apply maximum Gaussian count limit
        if (maxGaussians_ > 0 && cloud.size() > maxGaussians_) {
            // Note: Would need to implement a method to truncate cloud
            // For now, just filter by opacity more aggressively
            cloud.removeInvisible(0.1f);
        }
    }

    bool isNeuralEngineSupported() {
        // Check if running on Apple Silicon
        #if TARGET_CPU_ARM64
            return true;
        #else
            return false;
        #endif
    }

    // ========================================================================
    // Member Variables
    // ========================================================================

    MLModel* model_;
    id<MTLDevice> device_;
    ModelConfig config_;
    ModelInfo modelInfo_;

    ModelStatus lastStatus_;
    std::string lastError_;

    double lastInferenceTime_;
    double totalInferenceTime_;
    size_t predictionCount_;
    bool neuralEngineUsed_;
    bool inferenceInProgress_;

    float minOpacity_;
    size_t maxGaussians_;
    bool postProcessingEnabled_;
};

// ============================================================================
// Public Interface Implementation
// ============================================================================

SharpModel::SharpModel()
    : impl_(std::make_unique<Impl>()) {
}

SharpModel::~SharpModel() = default;

SharpModel::SharpModel(SharpModel&& other) noexcept = default;
SharpModel& SharpModel::operator=(SharpModel&& other) noexcept = default;

// Model Loading
bool SharpModel::load(const std::string& modelPath) {
    ModelConfig config;
    return impl_->load(modelPath, config);
}

bool SharpModel::load(const std::string& modelPath, const ModelConfig& config) {
    return impl_->load(modelPath, config);
}

void SharpModel::unload() {
    impl_->unload();
}

bool SharpModel::isLoaded() const {
    return impl_->isLoaded();
}

ModelInfo SharpModel::getModelInfo() const {
    return impl_->getModelInfo();
}

// Synchronous Inference
GaussianCloud SharpModel::predict(const oflike::ofPixels& image) {
    return impl_->predict(image);
}

GaussianCloud SharpModel::predict(const oflike::ofTexture& texture) {
    return impl_->predict(texture);
}

// Asynchronous Inference
void SharpModel::predictAsync(const oflike::ofPixels& image, PredictCallback callback) {
    impl_->predictAsync(image, callback);
}

void SharpModel::predictAsync(const oflike::ofTexture& texture, PredictCallback callback) {
    impl_->predictAsync(texture, callback);
}

bool SharpModel::isInferenceInProgress() const {
    return impl_->isInferenceInProgress();
}

void SharpModel::cancelInference() {
    impl_->cancelInference();
}

// Error Handling
ModelStatus SharpModel::getLastStatus() const {
    return impl_->getLastStatus();
}

std::string SharpModel::getLastError() const {
    return impl_->getLastError();
}

std::string SharpModel::statusToString(ModelStatus status) {
    switch (status) {
        case ModelStatus::Success:
            return "Success";
        case ModelStatus::ErrorModelNotLoaded:
            return "Model not loaded";
        case ModelStatus::ErrorInvalidInput:
            return "Invalid input";
        case ModelStatus::ErrorInferenceFailed:
            return "Inference failed";
        case ModelStatus::ErrorNeuralEngineUnavailable:
            return "Neural Engine unavailable";
        case ModelStatus::ErrorInvalidModelFormat:
            return "Invalid model format";
        default:
            return "Unknown error";
    }
}

// Performance Metrics
double SharpModel::getLastInferenceTime() const {
    return impl_->getLastInferenceTime();
}

double SharpModel::getAverageInferenceTime() const {
    return impl_->getAverageInferenceTime();
}

size_t SharpModel::getPredictionCount() const {
    return impl_->getPredictionCount();
}

bool SharpModel::wasNeuralEngineUsed() const {
    return impl_->wasNeuralEngineUsed();
}

// Configuration
void SharpModel::setMinOpacity(float threshold) {
    impl_->setMinOpacity(threshold);
}

void SharpModel::setMaxGaussians(size_t maxCount) {
    impl_->setMaxGaussians(maxCount);
}

void SharpModel::setPostProcessingEnabled(bool enabled) {
    impl_->setPostProcessingEnabled(enabled);
}

} // namespace Sharp
