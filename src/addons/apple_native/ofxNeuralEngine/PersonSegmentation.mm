#import "PersonSegmentation.h"
#import "../../oflike/image/ofPixels.h"
#import "../../oflike/image/ofTexture.h"
#import <Vision/Vision.h>
#import <Metal/Metal.h>
#import <CoreImage/CoreImage.h>
#import <Accelerate/Accelerate.h>

using namespace oflike;

namespace NeuralEngine {

// ============================================================================
// SegmentationResult Implementation
// ============================================================================

SegmentationResult::SegmentationResult() {
    info.isValid = false;
}

ofPixels SegmentationResult::applyMask(const ofPixels& input) const {
    if (!info.isValid || !mask.isAllocated() || !input.isAllocated()) {
        return ofPixels();
    }

    // Ensure mask and input have same dimensions
    if (mask.getWidth() != input.getWidth() || mask.getHeight() != input.getHeight()) {
        return ofPixels();
    }

    ofPixels result;
    result.allocate(input.getWidth(), input.getHeight(), 4); // RGBA

    const uint8_t* maskData = mask.getData();
    const uint8_t* inputData = input.getData();
    uint8_t* resultData = result.getData();

    size_t numPixels = input.getWidth() * input.getHeight();
    size_t inputChannels = input.getNumChannels();

    for (size_t i = 0; i < numPixels; i++) {
        uint8_t alpha = maskData[i]; // Grayscale mask

        if (inputChannels >= 3) {
            resultData[i * 4 + 0] = inputData[i * inputChannels + 0]; // R
            resultData[i * 4 + 1] = inputData[i * inputChannels + 1]; // G
            resultData[i * 4 + 2] = inputData[i * inputChannels + 2]; // B
        } else {
            // Grayscale input
            uint8_t gray = inputData[i * inputChannels];
            resultData[i * 4 + 0] = gray;
            resultData[i * 4 + 1] = gray;
            resultData[i * 4 + 2] = gray;
        }
        resultData[i * 4 + 3] = alpha; // A
    }

    return result;
}

ofPixels SegmentationResult::getInvertedMask() const {
    if (!info.isValid || !mask.isAllocated()) {
        return ofPixels();
    }

    ofPixels inverted;
    inverted.allocate(mask.getWidth(), mask.getHeight(), 1);

    const uint8_t* maskData = mask.getData();
    uint8_t* invertedData = inverted.getData();

    size_t numPixels = mask.getWidth() * mask.getHeight();
    for (size_t i = 0; i < numPixels; i++) {
        invertedData[i] = 255 - maskData[i];
    }

    return inverted;
}

ofPixels SegmentationResult::getFeatheredMask(float radius) const {
    if (!info.isValid || !mask.isAllocated() || radius <= 0.0f) {
        return mask;
    }

    // Use vImage for efficient Gaussian blur
    vImage_Buffer src;
    src.data = (void*)mask.getData();
    src.height = mask.getHeight();
    src.width = mask.getWidth();
    src.rowBytes = mask.getWidth();

    ofPixels feathered;
    feathered.allocate(mask.getWidth(), mask.getHeight(), 1);

    vImage_Buffer dest;
    dest.data = feathered.getData();
    dest.height = feathered.getHeight();
    dest.width = feathered.getWidth();
    dest.rowBytes = feathered.getWidth();

    // Calculate kernel size from radius
    uint32_t kernelSize = (uint32_t)(radius * 2.0f + 1.0f);
    if (kernelSize % 2 == 0) kernelSize++; // Must be odd

    vImage_Error error = vImageBoxConvolve_Planar8(
        &src, &dest,
        nullptr,
        0, 0,
        kernelSize, kernelSize,
        nullptr,
        kvImageEdgeExtend
    );

    if (error != kvImageNoError) {
        return mask; // Return original on error
    }

    return feathered;
}

// ============================================================================
// PersonSegmentation::Impl
// ============================================================================

struct PersonSegmentation::Impl {
    VNGeneratePersonSegmentationRequest* request;
    PersonSegmentationConfig config;
    std::string lastError;
    bool isSetup;
    bool isProcessing;
    dispatch_queue_t processingQueue;

    Impl() : request(nil), isSetup(false), isProcessing(false) {
        @autoreleasepool {
            processingQueue = dispatch_queue_create("com.oflike.metal.person_segmentation", DISPATCH_QUEUE_SERIAL);
        }
    }

    ~Impl() {
        shutdown();
    }

    void shutdown() {
        @autoreleasepool {
            if (request) {
                request = nil;
            }
            isSetup = false;
        }
    }

    bool setupRequest() {
        @autoreleasepool {
            // Create person segmentation request
            request = [[VNGeneratePersonSegmentationRequest alloc] init];
            if (!request) {
                lastError = "Failed to create VNGeneratePersonSegmentationRequest";
                return false;
            }

            // Configure quality level
            switch (config.quality) {
                case SegmentationQuality::Fast:
                    request.qualityLevel = VNGeneratePersonSegmentationRequestQualityLevelFast;
                    break;
                case SegmentationQuality::Balanced:
                    request.qualityLevel = VNGeneratePersonSegmentationRequestQualityLevelBalanced;
                    break;
                case SegmentationQuality::Accurate:
                    request.qualityLevel = VNGeneratePersonSegmentationRequestQualityLevelAccurate;
                    break;
            }

            // Configure output scale
            request.outputPixelFormat = kCVPixelFormatType_OneComponent8;

            return true;
        }
    }

    SegmentationResult performSegmentation(const ofPixels& pixels) {
        @autoreleasepool {
            SegmentationResult result;

            if (!pixels.isAllocated() || pixels.getWidth() == 0 || pixels.getHeight() == 0) {
                result.errorMessage = "Invalid input pixels";
                return result;
            }

            // Convert ofPixels to CGImage
            size_t width = pixels.getWidth();
            size_t height = pixels.getHeight();
            size_t channels = pixels.getNumChannels();
            size_t bitsPerComponent = 8;
            size_t bytesPerRow = width * channels;

            CGColorSpaceRef colorSpace = nullptr;
            CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;

            if (channels == 4) {
                colorSpace = CGColorSpaceCreateDeviceRGB();
                bitmapInfo |= kCGImageAlphaPremultipliedLast;
            } else if (channels == 3) {
                colorSpace = CGColorSpaceCreateDeviceRGB();
                bitmapInfo |= kCGImageAlphaNoneSkipLast;
            } else if (channels == 1) {
                colorSpace = CGColorSpaceCreateDeviceGray();
            } else {
                result.errorMessage = "Unsupported pixel format";
                return result;
            }

            CGDataProviderRef provider = CGDataProviderCreateWithData(
                nullptr,
                pixels.getData(),
                pixels.size(),
                nullptr
            );

            CGImageRef cgImage = CGImageCreate(
                width, height,
                bitsPerComponent,
                bitsPerComponent * channels,
                bytesPerRow,
                colorSpace,
                bitmapInfo,
                provider,
                nullptr,
                false,
                kCGRenderingIntentDefault
            );

            CGColorSpaceRelease(colorSpace);
            CGDataProviderRelease(provider);

            if (!cgImage) {
                result.errorMessage = "Failed to create CGImage";
                return result;
            }

            // Perform segmentation
            VNImageRequestHandler* handler = [[VNImageRequestHandler alloc]
                initWithCGImage:cgImage
                options:@{}];

            NSError* error = nil;
            BOOL success = [handler performRequests:@[request] error:&error];

            CGImageRelease(cgImage);

            if (!success || error) {
                result.errorMessage = error ? error.localizedDescription.UTF8String : "Segmentation failed";
                return result;
            }

            // Extract results
            NSArray<VNPixelBufferObservation*>* observations = request.results;
            if (!observations || observations.count == 0) {
                result.errorMessage = "No person detected in image";
                return result;
            }

            // Get first (best) result
            VNPixelBufferObservation* observation = observations[0];
            CVPixelBufferRef pixelBuffer = observation.pixelBuffer;

            if (!pixelBuffer) {
                result.errorMessage = "Failed to get pixel buffer from observation";
                return result;
            }

            // Convert CVPixelBuffer to ofPixels
            CVPixelBufferLockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);

            size_t maskWidth = CVPixelBufferGetWidth(pixelBuffer);
            size_t maskHeight = CVPixelBufferGetHeight(pixelBuffer);
            uint8_t* baseAddress = (uint8_t*)CVPixelBufferGetBaseAddress(pixelBuffer);
            size_t maskBytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer);

            result.mask.allocate(maskWidth, maskHeight, 1);
            uint8_t* maskData = result.mask.getData();

            // Copy mask data
            for (size_t y = 0; y < maskHeight; y++) {
                memcpy(
                    maskData + y * maskWidth,
                    baseAddress + y * maskBytesPerRow,
                    maskWidth
                );
            }

            CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);

            // Apply output scale if needed
            if (config.outputScale != 1.0f && config.outputScale > 0.0f) {
                size_t scaledWidth = (size_t)(maskWidth * config.outputScale);
                size_t scaledHeight = (size_t)(maskHeight * config.outputScale);

                // Use vImage for high-quality scaling
                vImage_Buffer src;
                src.data = result.mask.getData();
                src.height = maskHeight;
                src.width = maskWidth;
                src.rowBytes = maskWidth;

                ofPixels scaledMask;
                scaledMask.allocate(scaledWidth, scaledHeight, 1);

                vImage_Buffer dest;
                dest.data = scaledMask.getData();
                dest.height = scaledHeight;
                dest.width = scaledWidth;
                dest.rowBytes = scaledWidth;

                vImage_Error vError = vImageScale_Planar8(&src, &dest, nullptr, kvImageHighQualityResampling);
                if (vError == kvImageNoError) {
                    result.mask = scaledMask;
                    maskWidth = scaledWidth;
                    maskHeight = scaledHeight;
                }
            }

            // Apply edge smoothing if enabled
            if (config.smoothEdges) {
                result.mask = result.getFeatheredMask(1.5f);
            }

            // Fill result info
            result.info.inputWidth = width;
            result.info.inputHeight = height;
            result.info.maskWidth = maskWidth;
            result.info.maskHeight = maskHeight;
            result.info.numPeopleDetected = observations.count;
            result.info.averageConfidence = observation.confidence;
            result.info.isValid = true;

            return result;
        }
    }
};

// ============================================================================
// PersonSegmentation Public API
// ============================================================================

PersonSegmentation::PersonSegmentation()
    : pImpl(std::make_unique<Impl>()) {
}

PersonSegmentation::~PersonSegmentation() {
    shutdown();
}

PersonSegmentation::PersonSegmentation(PersonSegmentation&& other) noexcept
    : pImpl(std::move(other.pImpl)) {
}

PersonSegmentation& PersonSegmentation::operator=(PersonSegmentation&& other) noexcept {
    if (this != &other) {
        pImpl = std::move(other.pImpl);
    }
    return *this;
}

bool PersonSegmentation::setup() {
    return setup(PersonSegmentationConfig());
}

bool PersonSegmentation::setup(const PersonSegmentationConfig& config) {
    @autoreleasepool {
        if (pImpl->isSetup) {
            shutdown();
        }

        pImpl->config = config;

        if (!pImpl->setupRequest()) {
            return false;
        }

        pImpl->isSetup = true;
        return true;
    }
}

void PersonSegmentation::shutdown() {
    pImpl->shutdown();
}

bool PersonSegmentation::isSetup() const {
    return pImpl->isSetup;
}

SegmentationResult PersonSegmentation::segment(const ofPixels& pixels) {
    if (!pImpl->isSetup) {
        SegmentationResult result;
        result.errorMessage = "PersonSegmentation not setup";
        return result;
    }

    return pImpl->performSegmentation(pixels);
}

SegmentationResult PersonSegmentation::segment(ofTexture& texture) {
    if (!pImpl->isSetup) {
        SegmentationResult result;
        result.errorMessage = "PersonSegmentation not setup";
        return result;
    }

    // Read texture to pixels (TODO: optimize with direct Metal processing)
    ofPixels pixels;
    texture.readToPixels(pixels);

    return pImpl->performSegmentation(pixels);
}

void PersonSegmentation::segmentAsync(const ofPixels& pixels, SegmentationCallback callback) {
    if (!pImpl->isSetup) {
        SegmentationResult result;
        result.errorMessage = "PersonSegmentation not setup";
        callback(result);
        return;
    }

    pImpl->isProcessing = true;

    // Copy pixels for async processing
    ofPixels pixelsCopy = pixels;

    dispatch_async(pImpl->processingQueue, ^{
        @autoreleasepool {
            SegmentationResult result = pImpl->performSegmentation(pixelsCopy);
            pImpl->isProcessing = false;

            if (callback) {
                callback(result);
            }
        }
    });
}

void PersonSegmentation::segmentAsync(ofTexture& texture, SegmentationCallback callback) {
    if (!pImpl->isSetup) {
        SegmentationResult result;
        result.errorMessage = "PersonSegmentation not setup";
        callback(result);
        return;
    }

    // Read texture to pixels
    ofPixels pixels;
    texture.readToPixels(pixels);

    segmentAsync(pixels, callback);
}

void PersonSegmentation::cancelAsync() {
    // Note: VNRequest doesn't support cancellation
    // This is a placeholder for future implementation
    pImpl->isProcessing = false;
}

bool PersonSegmentation::isProcessing() const {
    return pImpl->isProcessing;
}

bool PersonSegmentation::setConfig(const PersonSegmentationConfig& config) {
    if (!pImpl->isSetup) {
        pImpl->lastError = "PersonSegmentation not setup";
        return false;
    }

    pImpl->config = config;
    return pImpl->setupRequest();
}

PersonSegmentationConfig PersonSegmentation::getConfig() const {
    return pImpl->config;
}

void PersonSegmentation::setQuality(SegmentationQuality quality) {
    pImpl->config.quality = quality;
    if (pImpl->isSetup) {
        pImpl->setupRequest();
    }
}

void PersonSegmentation::setOutputScale(float scale) {
    pImpl->config.outputScale = scale;
}

void PersonSegmentation::setMinConfidence(float confidence) {
    pImpl->config.minConfidence = confidence;
}

void PersonSegmentation::setMultiPerson(bool enabled) {
    pImpl->config.multiPerson = enabled;
}

void PersonSegmentation::setSmoothEdges(bool enabled) {
    pImpl->config.smoothEdges = enabled;
}

bool PersonSegmentation::isNeuralEngineAvailable() {
    // Neural Engine is available on Apple Silicon M1+
    @autoreleasepool {
        return VNIsAppleNeuralEngineAvailable();
    }
}

bool PersonSegmentation::isSupported() {
    // Person segmentation requires macOS 12.0+ (VNGeneratePersonSegmentationRequest)
    if (@available(macOS 12.0, *)) {
        return true;
    }
    return false;
}

std::string PersonSegmentation::getLastError() const {
    return pImpl->lastError;
}

// ============================================================================
// Utility Functions
// ============================================================================

ofPixels PersonSegmentation::composite(
    const ofPixels& foreground,
    const ofPixels& background,
    const ofPixels& mask
) {
    if (!foreground.isAllocated() || !background.isAllocated() || !mask.isAllocated()) {
        return ofPixels();
    }

    size_t width = foreground.getWidth();
    size_t height = foreground.getHeight();

    if (background.getWidth() != width || background.getHeight() != height ||
        mask.getWidth() != width || mask.getHeight() != height) {
        return ofPixels();
    }

    ofPixels result;
    result.allocate(width, height, 3); // RGB

    const uint8_t* fgData = foreground.getData();
    const uint8_t* bgData = background.getData();
    const uint8_t* maskData = mask.getData();
    uint8_t* resultData = result.getData();

    size_t fgChannels = foreground.getNumChannels();
    size_t bgChannels = background.getNumChannels();
    size_t numPixels = width * height;

    for (size_t i = 0; i < numPixels; i++) {
        float alpha = maskData[i] / 255.0f;
        float invAlpha = 1.0f - alpha;

        for (size_t c = 0; c < 3; c++) {
            uint8_t fg = (c < fgChannels) ? fgData[i * fgChannels + c] : 0;
            uint8_t bg = (c < bgChannels) ? bgData[i * bgChannels + c] : 0;
            resultData[i * 3 + c] = (uint8_t)(fg * alpha + bg * invAlpha);
        }
    }

    return result;
}

ofPixels PersonSegmentation::extractPerson(
    const ofPixels& image,
    const ofPixels& mask
) {
    if (!image.isAllocated() || !mask.isAllocated()) {
        return ofPixels();
    }

    if (image.getWidth() != mask.getWidth() || image.getHeight() != mask.getHeight()) {
        return ofPixels();
    }

    ofPixels result;
    result.allocate(image.getWidth(), image.getHeight(), 4); // RGBA

    const uint8_t* imgData = image.getData();
    const uint8_t* maskData = mask.getData();
    uint8_t* resultData = result.getData();

    size_t channels = image.getNumChannels();
    size_t numPixels = image.getWidth() * image.getHeight();

    for (size_t i = 0; i < numPixels; i++) {
        for (size_t c = 0; c < 3; c++) {
            resultData[i * 4 + c] = (c < channels) ? imgData[i * channels + c] : 0;
        }
        resultData[i * 4 + 3] = maskData[i]; // Alpha from mask
    }

    return result;
}

ofPixels PersonSegmentation::blurBackground(
    const ofPixels& image,
    const ofPixels& mask,
    float blurRadius
) {
    if (!image.isAllocated() || !mask.isAllocated() || blurRadius <= 0.0f) {
        return image;
    }

    if (image.getWidth() != mask.getWidth() || image.getHeight() != mask.getHeight()) {
        return image;
    }

    // Create blurred version of entire image
    ofPixels blurred;
    blurred.allocate(image.getWidth(), image.getHeight(), image.getNumChannels());

    // Use vImage for Gaussian blur per channel
    uint32_t kernelSize = (uint32_t)(blurRadius * 2.0f + 1.0f);
    if (kernelSize % 2 == 0) kernelSize++;

    for (size_t c = 0; c < image.getNumChannels(); c++) {
        vImage_Buffer src, dest;
        src.height = dest.height = image.getHeight();
        src.width = dest.width = image.getWidth();
        src.rowBytes = dest.rowBytes = image.getWidth();

        // Extract channel
        std::vector<uint8_t> srcChannel(image.getWidth() * image.getHeight());
        std::vector<uint8_t> destChannel(image.getWidth() * image.getHeight());

        const uint8_t* imgData = image.getData();
        for (size_t i = 0; i < srcChannel.size(); i++) {
            srcChannel[i] = imgData[i * image.getNumChannels() + c];
        }

        src.data = srcChannel.data();
        dest.data = destChannel.data();

        vImageBoxConvolve_Planar8(&src, &dest, nullptr, 0, 0, kernelSize, kernelSize, nullptr, kvImageEdgeExtend);

        // Copy back
        uint8_t* blurredData = blurred.getData();
        for (size_t i = 0; i < destChannel.size(); i++) {
            blurredData[i * blurred.getNumChannels() + c] = destChannel[i];
        }
    }

    // Composite original (person) and blurred (background) using mask
    ofPixels result;
    result.allocate(image.getWidth(), image.getHeight(), image.getNumChannels());

    const uint8_t* imgData = image.getData();
    const uint8_t* blurredData = blurred.getData();
    const uint8_t* maskData = mask.getData();
    uint8_t* resultData = result.getData();

    size_t numPixels = image.getWidth() * image.getHeight();
    size_t channels = image.getNumChannels();

    for (size_t i = 0; i < numPixels; i++) {
        float alpha = maskData[i] / 255.0f; // Person alpha
        float invAlpha = 1.0f - alpha;      // Background alpha

        for (size_t c = 0; c < channels; c++) {
            uint8_t original = imgData[i * channels + c];
            uint8_t blurredVal = blurredData[i * channels + c];
            resultData[i * channels + c] = (uint8_t)(original * alpha + blurredVal * invAlpha);
        }
    }

    return result;
}

} // namespace NeuralEngine
