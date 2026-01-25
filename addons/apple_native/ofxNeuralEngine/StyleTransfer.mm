#include "StyleTransfer.h"
#include "ofPixels.h"
#include "ofTexture.h"
#include <Foundation/Foundation.h>
#include <CoreML/CoreML.h>
#include <Vision/Vision.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreImage/CoreImage.h>
#include <ImageIO/ImageIO.h>
#include <Metal/Metal.h>
#include <algorithm>

using namespace oflike;

namespace NeuralEngine {

// ============================================================================
// Implementation (pImpl pattern)
// ============================================================================

struct StyleTransfer::Impl {
    MLModel* model = nil;
    VNCoreMLModel* visionModel = nil;
    StyleTransferConfig config;
    StyleTransferInfo info;
    std::string lastError;
    CIContext* ciContext = nil;

    ~Impl() {
        @autoreleasepool {
            model = nil;
            visionModel = nil;
            ciContext = nil;
        }
    }

    bool loadModel(const std::string& modelPath, const StyleTransferConfig& cfg) {
        @autoreleasepool {
            config = cfg;

            // Convert path to NSURL
            NSString* path = [NSString stringWithUTF8String:modelPath.c_str()];
            NSURL* modelURL = [NSURL fileURLWithPath:path];

            // Create model configuration
            MLModelConfiguration* configuration = [[MLModelConfiguration alloc] init];

            // Set compute units
            switch (config.computeUnits) {
                case StyleTransferConfig::ComputeUnits::CPUOnly:
                    configuration.computeUnits = MLComputeUnitsCPUOnly;
                    break;
                case StyleTransferConfig::ComputeUnits::CPUAndGPU:
                    configuration.computeUnits = MLComputeUnitsCPUAndGPU;
                    break;
                case StyleTransferConfig::ComputeUnits::CPUAndNeuralEngine:
                    configuration.computeUnits = MLComputeUnitsCPUAndNeuralEngine;
                    break;
                case StyleTransferConfig::ComputeUnits::All:
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

            // Create Core Image context for GPU processing
            id<MTLDevice> device = MTLCreateSystemDefaultDevice();
            if (device) {
                ciContext = [CIContext contextWithMTLDevice:device];
            } else {
                ciContext = [CIContext context];
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

            // Get output size
            for (MLFeatureDescription* output in description.outputDescriptionsByName.allValues) {
                if (output.type == MLFeatureTypeImage) {
                    MLImageConstraint* imageConstraint = output.imageConstraint;
                    if (imageConstraint) {
                        info.outputWidth = imageConstraint.pixelsWide;
                        info.outputHeight = imageConstraint.pixelsHigh;
                    }
                    break;
                }
            }

            // If output size not specified, assume same as input
            if (info.outputWidth == 0) info.outputWidth = info.inputWidth;
            if (info.outputHeight == 0) info.outputHeight = info.inputHeight;

            // Check if style intensity is supported
            for (MLFeatureDescription* input in description.inputDescriptionsByName.allValues) {
                NSString* name = input.name;
                if ([name containsString:@"style"] || [name containsString:@"intensity"] ||
                    [name containsString:@"alpha"]) {
                    info.supportsStyleIntensity = true;
                    break;
                }
            }

            // Check if Neural Engine is supported
            if (@available(macOS 13.0, *)) {
                info.isNeuralEngineSupported = (configuration.computeUnits == MLComputeUnitsAll ||
                                                configuration.computeUnits == MLComputeUnitsCPUAndNeuralEngine);
            }

            return true;
        }
    }

    ofPixels transferFromPixels(const ofPixels& pixels) {
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
            CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

            // Create bitmap context
            size_t bitsPerComponent = 8;
            size_t bytesPerRow = 4 * width; // Always use RGBA for style transfer
            CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault | kCGImageAlphaPremultipliedLast;

            // Convert to RGBA if needed
            const unsigned char* sourceData = pixels.getData();
            unsigned char* rgbaData = nullptr;
            bool needsFree = false;

            if (channels == 4) {
                // Already RGBA
                sourceData = pixels.getData();
            } else if (channels == 3) {
                // RGB -> RGBA
                rgbaData = new unsigned char[width * height * 4];
                for (size_t i = 0; i < width * height; i++) {
                    rgbaData[i * 4 + 0] = sourceData[i * 3 + 0]; // R
                    rgbaData[i * 4 + 1] = sourceData[i * 3 + 1]; // G
                    rgbaData[i * 4 + 2] = sourceData[i * 3 + 2]; // B
                    rgbaData[i * 4 + 3] = 255;                   // A
                }
                sourceData = rgbaData;
                needsFree = true;
            } else {
                // Grayscale -> RGBA
                rgbaData = new unsigned char[width * height * 4];
                for (size_t i = 0; i < width * height; i++) {
                    unsigned char gray = sourceData[i];
                    rgbaData[i * 4 + 0] = gray;
                    rgbaData[i * 4 + 1] = gray;
                    rgbaData[i * 4 + 2] = gray;
                    rgbaData[i * 4 + 3] = 255;
                }
                sourceData = rgbaData;
                needsFree = true;
            }

            CGContextRef context = CGBitmapContextCreate(
                (void*)sourceData,
                width,
                height,
                bitsPerComponent,
                bytesPerRow,
                colorSpace,
                bitmapInfo
            );

            CGImageRef cgImage = CGBitmapContextCreateImage(context);

            // Perform inference
            ofPixels result = transferFromCGImage(cgImage, width, height);

            // Cleanup
            CGImageRelease(cgImage);
            CGContextRelease(context);
            CGColorSpaceRelease(colorSpace);

            if (needsFree) {
                delete[] rgbaData;
            }

            return result;
        }
    }

    ofPixels transferFromCGImage(CGImageRef cgImage, size_t originalWidth, size_t originalHeight) {
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
                lastError = error ? [[error localizedDescription] UTF8String] : "Style transfer failed";
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
            if (pixelFormat == kCVPixelFormatType_32BGRA || pixelFormat == kCVPixelFormatType_32ARGB) {
                // BGRA or ARGB -> RGB
                result.allocate(width, height, 3);
                void* baseAddress = CVPixelBufferGetBaseAddress(pixelBuffer);
                size_t bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer);

                for (size_t y = 0; y < height; y++) {
                    unsigned char* src = (unsigned char*)baseAddress + y * bytesPerRow;
                    unsigned char* dst = result.getData() + y * width * 3;

                    for (size_t x = 0; x < width; x++) {
                        if (pixelFormat == kCVPixelFormatType_32BGRA) {
                            // BGRA -> RGB
                            dst[x * 3 + 0] = src[x * 4 + 2]; // R
                            dst[x * 3 + 1] = src[x * 4 + 1]; // G
                            dst[x * 3 + 2] = src[x * 4 + 0]; // B
                        } else {
                            // ARGB -> RGB
                            dst[x * 3 + 0] = src[x * 4 + 1]; // R
                            dst[x * 3 + 1] = src[x * 4 + 2]; // G
                            dst[x * 3 + 2] = src[x * 4 + 3]; // B
                        }
                    }
                }

                // Apply color preservation if enabled
                if (config.preserveColors) {
                    // This is a simple implementation - could be improved with luminance transfer
                    // For now, we just return the stylized result
                }
            }
            else if (pixelFormat == kCVPixelFormatType_24RGB) {
                // RGB -> RGB (direct copy)
                result.allocate(width, height, 3);
                void* baseAddress = CVPixelBufferGetBaseAddress(pixelBuffer);
                size_t bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer);

                for (size_t y = 0; y < height; y++) {
                    unsigned char* src = (unsigned char*)baseAddress + y * bytesPerRow;
                    unsigned char* dst = result.getData() + y * width * 3;
                    memcpy(dst, src, width * 3);
                }
            }
            else {
                // Try to convert using Core Image
                CIImage* ciImage = [CIImage imageWithCVPixelBuffer:pixelBuffer];
                if (ciImage && ciContext) {
                    CGImageRef cgImage = [ciContext createCGImage:ciImage fromRect:ciImage.extent];
                    if (cgImage) {
                        result = cgImageToOfPixels(cgImage);
                        CGImageRelease(cgImage);
                    }
                }
            }

            CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);

            return result;
        }
    }

    ofPixels cgImageToOfPixels(CGImageRef cgImage) {
        @autoreleasepool {
            size_t width = CGImageGetWidth(cgImage);
            size_t height = CGImageGetHeight(cgImage);

            ofPixels result;
            result.allocate(width, height, 3);

            CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
            CGContextRef context = CGBitmapContextCreate(
                result.getData(),
                width,
                height,
                8,
                width * 3,
                colorSpace,
                kCGBitmapByteOrderDefault | kCGImageAlphaNoneSkipLast
            );

            CGContextDrawImage(context, CGRectMake(0, 0, width, height), cgImage);

            CGContextRelease(context);
            CGColorSpaceRelease(colorSpace);

            return result;
        }
    }

    std::vector<ofPixels> transferBatch(const std::vector<ofPixels>& images) {
        std::vector<ofPixels> results;
        results.reserve(images.size());

        for (const auto& image : images) {
            results.push_back(transferFromPixels(image));
        }

        return results;
    }
};

// ============================================================================
// Constructor / Destructor
// ============================================================================

StyleTransfer::StyleTransfer()
    : impl_(std::make_unique<Impl>()) {
}

StyleTransfer::~StyleTransfer() = default;

StyleTransfer::StyleTransfer(StyleTransfer&& other) noexcept = default;
StyleTransfer& StyleTransfer::operator=(StyleTransfer&& other) noexcept = default;

// ============================================================================
// Model Loading
// ============================================================================

bool StyleTransfer::load(const std::string& modelPath) {
    StyleTransferConfig defaultConfig;
    return load(modelPath, defaultConfig);
}

bool StyleTransfer::load(const std::string& modelPath, const StyleTransferConfig& config) {
    return impl_->loadModel(modelPath, config);
}

void StyleTransfer::unload() {
    @autoreleasepool {
        impl_->model = nil;
        impl_->visionModel = nil;
        impl_->ciContext = nil;
        impl_->info = StyleTransferInfo();
    }
}

// ============================================================================
// Style Transfer
// ============================================================================

ofPixels StyleTransfer::transfer(const ofPixels& pixels) {
    return impl_->transferFromPixels(pixels);
}

ofPixels StyleTransfer::transfer(const ofTexture& texture) {
    // DEFERRED: Direct GPU path using MTLTexture and Core Image
    //
    // Rationale:
    // - VNCoreMLRequest expects CVPixelBuffer or CGImage input
    // - Direct MTLTexture → CVPixelBuffer requires IOSurface + CVPixelBufferPool
    // - Core Image path (CIImage → MTLTexture) adds complexity
    // - CPU path sufficient for style transfer (not real-time bottleneck)
    //
    // Workaround: Users should convert ofTexture to ofPixels manually:
    // Example:
    //   ofPixels pixels;
    //   texture.readToPixels(pixels);
    //   ofPixels stylized = transfer.transfer(pixels);

    impl_->lastError = "ofTexture input not yet implemented. Please convert to ofPixels first.";
    return ofPixels();
}

bool StyleTransfer::transferToTexture(const ofTexture& input, ofTexture& output) {
    // Transfer style
    ofPixels stylizedPixels = transfer(input);

    if (stylizedPixels.getWidth() == 0 || stylizedPixels.getHeight() == 0) {
        return false;
    }

    // Load into output texture
    output.allocate(stylizedPixels.getWidth(), stylizedPixels.getHeight(),
                    stylizedPixels.getNumChannels() == 1 ? OF_IMAGE_GRAYSCALE : OF_IMAGE_COLOR);
    output.loadData(stylizedPixels);

    return true;
}

std::vector<ofPixels> StyleTransfer::transferBatch(const std::vector<ofPixels>& images) {
    return impl_->transferBatch(images);
}

// ============================================================================
// Configuration
// ============================================================================

void StyleTransfer::setStyleIntensity(float intensity) {
    impl_->config.styleIntensity = std::max(0.0f, std::min(1.0f, intensity));
}

float StyleTransfer::getStyleIntensity() const {
    return impl_->config.styleIntensity;
}

void StyleTransfer::setPreserveColors(bool preserve) {
    impl_->config.preserveColors = preserve;
}

bool StyleTransfer::getPreserveColors() const {
    return impl_->config.preserveColors;
}

// ============================================================================
// Information
// ============================================================================

bool StyleTransfer::isLoaded() const {
    return impl_->info.isLoaded;
}

StyleTransferInfo StyleTransfer::getInfo() const {
    return impl_->info;
}

std::string StyleTransfer::getLastError() const {
    return impl_->lastError;
}

} // namespace NeuralEngine
