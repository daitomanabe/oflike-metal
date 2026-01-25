#include "DepthEstimator.h"
#include "ofPixels.h"
#include "ofTexture.h"
#include <Foundation/Foundation.h>
#include <CoreML/CoreML.h>
#include <Vision/Vision.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <Metal/Metal.h>
#include <float.h>
#include <algorithm>

using namespace oflike;

namespace NeuralEngine {

// ============================================================================
// Implementation (pImpl pattern)
// ============================================================================

struct DepthEstimator::Impl {
    MLModel* model = nil;
    VNCoreMLModel* visionModel = nil;
    DepthEstimatorConfig config;
    DepthEstimatorInfo info;
    std::string lastError;

    ~Impl() {
        @autoreleasepool {
            model = nil;
            visionModel = nil;
        }
    }

    bool loadModel(const std::string& modelPath, const DepthEstimatorConfig& cfg) {
        @autoreleasepool {
            config = cfg;

            // Convert path to NSURL
            NSString* path = [NSString stringWithUTF8String:modelPath.c_str()];
            NSURL* modelURL = [NSURL fileURLWithPath:path];

            // Create model configuration
            MLModelConfiguration* configuration = [[MLModelConfiguration alloc] init];

            // Set compute units
            switch (config.computeUnits) {
                case DepthEstimatorConfig::ComputeUnits::CPUOnly:
                    configuration.computeUnits = MLComputeUnitsCPUOnly;
                    break;
                case DepthEstimatorConfig::ComputeUnits::CPUAndGPU:
                    configuration.computeUnits = MLComputeUnitsCPUAndGPU;
                    break;
                case DepthEstimatorConfig::ComputeUnits::CPUAndNeuralEngine:
                    configuration.computeUnits = MLComputeUnitsCPUAndNeuralEngine;
                    break;
                case DepthEstimatorConfig::ComputeUnits::All:
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

            // Get input image size
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

            // Get output size (typically same as input for depth estimation)
            for (MLFeatureDescription* output in description.outputDescriptionsByName.allValues) {
                if (output.type == MLFeatureTypeImage) {
                    MLImageConstraint* imageConstraint = output.imageConstraint;
                    if (imageConstraint) {
                        info.outputWidth = imageConstraint.pixelsWide;
                        info.outputHeight = imageConstraint.pixelsHigh;
                    }
                    break;
                } else if (output.type == MLFeatureTypeMultiArray) {
                    // Some models output as MultiArray (CHW format)
                    MLMultiArrayConstraint* arrayConstraint = output.multiArrayConstraint;
                    if (arrayConstraint && arrayConstraint.shape.count >= 3) {
                        // Assuming CHW format: [C, H, W]
                        info.outputHeight = [arrayConstraint.shape[1] unsignedIntegerValue];
                        info.outputWidth = [arrayConstraint.shape[2] unsignedIntegerValue];
                    }
                    break;
                }
            }

            // If output size not specified, assume same as input
            if (info.outputWidth == 0) info.outputWidth = info.inputWidth;
            if (info.outputHeight == 0) info.outputHeight = info.inputHeight;

            // Check if Neural Engine is supported
            if (@available(macOS 13.0, *)) {
                info.isNeuralEngineSupported = (configuration.computeUnits == MLComputeUnitsAll ||
                                                configuration.computeUnits == MLComputeUnitsCPUAndNeuralEngine);
            }

            return true;
        }
    }

    ofPixels estimateFromPixels(const ofPixels& pixels) {
        @autoreleasepool {
            if (!visionModel) {
                lastError = "Model not loaded";
                return ofPixels();
            }

            // Create CGImage from ofPixels
            size_t width = pixels.getWidth();
            size_t height = pixels.getHeight();
            size_t channels = pixels.getNumChannels();

            if (width == 0 || height == 0) {
                lastError = "Invalid input pixels";
                return ofPixels();
            }

            // Create color space
            CGColorSpaceRef colorSpace = nullptr;
            if (channels == 1) {
                colorSpace = CGColorSpaceCreateDeviceGray();
            } else {
                colorSpace = CGColorSpaceCreateDeviceRGB();
            }

            // Create bitmap context
            size_t bitsPerComponent = 8;
            size_t bytesPerRow = channels * width;
            CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;

            if (channels == 4) {
                bitmapInfo |= kCGImageAlphaPremultipliedLast;
            } else if (channels == 3) {
                bitmapInfo |= kCGImageAlphaNoneSkipLast;
            }

            CGContextRef context = CGBitmapContextCreate(
                (void*)pixels.getData(),
                width,
                height,
                bitsPerComponent,
                bytesPerRow,
                colorSpace,
                bitmapInfo
            );

            CGImageRef cgImage = CGBitmapContextCreateImage(context);

            // Perform inference
            ofPixels result = estimateFromCGImage(cgImage, width, height);

            // Cleanup
            CGImageRelease(cgImage);
            CGContextRelease(context);
            CGColorSpaceRelease(colorSpace);

            return result;
        }
    }

    ofPixels estimateFromCGImage(CGImageRef cgImage, size_t originalWidth, size_t originalHeight) {
        @autoreleasepool {
            // Create request
            VNCoreMLRequest* request = [[VNCoreMLRequest alloc] initWithModel:visionModel];
            request.imageCropAndScaleOption = VNImageCropAndScaleOptionScaleFit;

            // Create request handler
            VNImageRequestHandler* handler = [[VNImageRequestHandler alloc]
                initWithCGImage:cgImage
                options:@{}];

            // Perform request
            NSError* error = nil;
            if (![handler performRequests:@[request] error:&error]) {
                lastError = error ? [[error localizedDescription] UTF8String] : "Inference failed";
                return ofPixels();
            }

            // Get results
            if (request.results.count == 0) {
                lastError = "No results from model";
                return ofPixels();
            }

            VNObservation* observation = request.results[0];

            // Handle different output types
            if ([observation isKindOfClass:[VNPixelBufferObservation class]]) {
                VNPixelBufferObservation* pixelObservation = (VNPixelBufferObservation*)observation;
                return pixelBufferToOfPixels(pixelObservation.pixelBuffer);
            }
            else if ([observation isKindOfClass:[VNCoreMLFeatureValueObservation class]]) {
                VNCoreMLFeatureValueObservation* featureObservation = (VNCoreMLFeatureValueObservation*)observation;
                MLFeatureValue* featureValue = featureObservation.featureValue;

                if (featureValue.type == MLFeatureTypeImage) {
                    CVPixelBufferRef pixelBuffer = featureValue.imageBufferValue;
                    return pixelBufferToOfPixels(pixelBuffer);
                }
                else if (featureValue.type == MLFeatureTypeMultiArray) {
                    return multiArrayToOfPixels(featureValue.multiArrayValue);
                }
            }

            lastError = "Unsupported output type";
            return ofPixels();
        }
    }

    ofPixels pixelBufferToOfPixels(CVPixelBufferRef pixelBuffer) {
        @autoreleasepool {
            if (!pixelBuffer) {
                return ofPixels();
            }

            CVPixelBufferLockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);

            size_t width = CVPixelBufferGetWidth(pixelBuffer);
            size_t height = CVPixelBufferGetHeight(pixelBuffer);
            OSType pixelFormat = CVPixelBufferGetPixelFormatType(pixelBuffer);

            ofPixels result;

            // Handle different pixel formats
            if (pixelFormat == kCVPixelFormatType_OneComponent8) {
                // Grayscale
                result.allocate(width, height, 1);
                void* baseAddress = CVPixelBufferGetBaseAddress(pixelBuffer);
                size_t bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer);

                for (size_t y = 0; y < height; y++) {
                    unsigned char* src = (unsigned char*)baseAddress + y * bytesPerRow;
                    unsigned char* dst = result.getData() + y * width;
                    memcpy(dst, src, width);
                }
            }
            else if (pixelFormat == kCVPixelFormatType_OneComponent32Float) {
                // Float depth map - convert to grayscale
                result.allocate(width, height, 1);
                void* baseAddress = CVPixelBufferGetBaseAddress(pixelBuffer);
                size_t bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer);

                // Find min/max for normalization
                float minVal = FLT_MAX;
                float maxVal = -FLT_MAX;

                if (config.normalizeOutput) {
                    for (size_t y = 0; y < height; y++) {
                        float* src = (float*)((unsigned char*)baseAddress + y * bytesPerRow);
                        for (size_t x = 0; x < width; x++) {
                            float val = src[x];
                            if (val < minVal) minVal = val;
                            if (val > maxVal) maxVal = val;
                        }
                    }
                }

                float range = maxVal - minVal;
                if (range < 0.0001f) range = 1.0f;

                // Convert to 8-bit grayscale
                for (size_t y = 0; y < height; y++) {
                    float* src = (float*)((unsigned char*)baseAddress + y * bytesPerRow);
                    unsigned char* dst = result.getData() + y * width;
                    for (size_t x = 0; x < width; x++) {
                        float normalized = config.normalizeOutput ?
                            (src[x] - minVal) / range : src[x];
                        normalized = std::max(0.0f, std::min(1.0f, normalized));

                        if (config.invertDepth) {
                            normalized = 1.0f - normalized;
                        }

                        dst[x] = (unsigned char)(normalized * 255.0f);
                    }
                }
            }
            else {
                // Unsupported format - try to convert
                result.allocate(width, height, 1);
            }

            CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);

            return result;
        }
    }

    ofPixels multiArrayToOfPixels(MLMultiArray* array) {
        @autoreleasepool {
            if (!array) {
                return ofPixels();
            }

            // Get dimensions - assuming CHW format [C, H, W] or HW [H, W]
            NSArray<NSNumber*>* shape = array.shape;
            size_t width, height;

            if (shape.count == 2) {
                // HW format
                height = [shape[0] unsignedIntegerValue];
                width = [shape[1] unsignedIntegerValue];
            }
            else if (shape.count == 3) {
                // CHW format (C should be 1 for depth)
                height = [shape[1] unsignedIntegerValue];
                width = [shape[2] unsignedIntegerValue];
            }
            else {
                lastError = "Unsupported array shape";
                return ofPixels();
            }

            ofPixels result;
            result.allocate(width, height, 1);

            // Get data type
            if (array.dataType == MLMultiArrayDataTypeFloat32 ||
                array.dataType == MLMultiArrayDataTypeFloat64 ||
                array.dataType == MLMultiArrayDataTypeDouble) {

                // Find min/max for normalization
                float minVal = FLT_MAX;
                float maxVal = -FLT_MAX;

                if (config.normalizeOutput) {
                    for (size_t i = 0; i < width * height; i++) {
                        NSArray<NSNumber*>* index = @[@(i)];
                        float val = [array[index] floatValue];
                        if (val < minVal) minVal = val;
                        if (val > maxVal) maxVal = val;
                    }
                }

                float range = maxVal - minVal;
                if (range < 0.0001f) range = 1.0f;

                // Convert to 8-bit grayscale
                unsigned char* dst = result.getData();
                for (size_t i = 0; i < width * height; i++) {
                    NSArray<NSNumber*>* index = @[@(i)];
                    float val = [array[index] floatValue];
                    float normalized = config.normalizeOutput ?
                        (val - minVal) / range : val;
                    normalized = std::max(0.0f, std::min(1.0f, normalized));

                    if (config.invertDepth) {
                        normalized = 1.0f - normalized;
                    }

                    dst[i] = (unsigned char)(normalized * 255.0f);
                }
            }
            else {
                // Integer types - direct copy
                unsigned char* dst = result.getData();
                for (size_t i = 0; i < width * height; i++) {
                    NSArray<NSNumber*>* index = @[@(i)];
                    int val = [array[index] intValue];
                    if (config.invertDepth) {
                        val = 255 - val;
                    }
                    dst[i] = (unsigned char)std::max(0, std::min(255, val));
                }
            }

            return result;
        }
    }
};

// ============================================================================
// Constructor / Destructor
// ============================================================================

DepthEstimator::DepthEstimator()
    : impl_(std::make_unique<Impl>()) {
}

DepthEstimator::~DepthEstimator() = default;

DepthEstimator::DepthEstimator(DepthEstimator&& other) noexcept = default;
DepthEstimator& DepthEstimator::operator=(DepthEstimator&& other) noexcept = default;

// ============================================================================
// Model Loading
// ============================================================================

bool DepthEstimator::load(const std::string& modelPath) {
    DepthEstimatorConfig defaultConfig;
    return load(modelPath, defaultConfig);
}

bool DepthEstimator::load(const std::string& modelPath, const DepthEstimatorConfig& config) {
    return impl_->loadModel(modelPath, config);
}

void DepthEstimator::unload() {
    @autoreleasepool {
        impl_->model = nil;
        impl_->visionModel = nil;
        impl_->info = DepthEstimatorInfo();
    }
}

// ============================================================================
// Depth Estimation
// ============================================================================

ofPixels DepthEstimator::estimate(const ofPixels& pixels) {
    return impl_->estimateFromPixels(pixels);
}

ofPixels DepthEstimator::estimate(const ofTexture& texture) {
    // DEFERRED: Direct GPU path using MTLTexture
    //
    // Rationale:
    // - VNCoreMLRequest expects CVPixelBuffer or CGImage input
    // - Direct MTLTexture → CVPixelBuffer requires Metal compute pipeline
    // - Current CPU path via readToPixels() is sufficient for most use cases
    // - GPU optimization deferred until performance profiling shows bottleneck
    //
    // Workaround: Users should convert ofTexture to ofPixels manually:
    // Example:
    //   ofImage img;
    //   img.load("input.jpg");
    //   ofPixels depthMap = estimator.estimate(img.getPixels());

    impl_->lastError = "ofTexture input not yet implemented. Please convert to ofPixels first.";
    return ofPixels();
}

bool DepthEstimator::estimateToTexture(const ofTexture& input, ofTexture& output) {
    // Estimate depth
    ofPixels depthPixels = estimate(input);

    if (depthPixels.getWidth() == 0 || depthPixels.getHeight() == 0) {
        return false;
    }

    // Load into output texture (OF_IMAGE_GRAYSCALE for single channel)
    output.allocate(depthPixels.getWidth(), depthPixels.getHeight(), OF_IMAGE_GRAYSCALE);
    output.loadData(depthPixels);

    return true;
}

// ============================================================================
// Configuration
// ============================================================================

void DepthEstimator::setNormalizeOutput(bool normalize) {
    impl_->config.normalizeOutput = normalize;
}

bool DepthEstimator::getNormalizeOutput() const {
    return impl_->config.normalizeOutput;
}

void DepthEstimator::setInvertDepth(bool invert) {
    impl_->config.invertDepth = invert;
}

bool DepthEstimator::getInvertDepth() const {
    return impl_->config.invertDepth;
}

// ============================================================================
// Information
// ============================================================================

bool DepthEstimator::isLoaded() const {
    return impl_->info.isLoaded;
}

DepthEstimatorInfo DepthEstimator::getInfo() const {
    return impl_->info;
}

std::string DepthEstimator::getLastError() const {
    return impl_->lastError;
}

} // namespace NeuralEngine
