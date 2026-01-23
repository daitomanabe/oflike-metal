#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <ImageIO/ImageIO.h>
#import <Accelerate/Accelerate.h>

#include "ofImage.h"
#include "../../core/Context.h"
#include "../../render/metal/MetalRenderer.h"
#include "../utils/ofLog.h"
#include <algorithm>

namespace oflike {

// ============================================================================
// Implementation Structure
// ============================================================================

struct ofImage::Impl {
    ofPixels pixels;              // CPU pixel data
    ofTexture texture;            // GPU texture
    bool bUseTexture = true;      // Whether to use texture for drawing
    bool pixelsDirty = false;     // Pixels modified, need to sync to texture
    bool textureDirty = false;    // Texture modified, need to sync to pixels

    ~Impl() = default;
};

// ============================================================================
// Constructors & Destructor
// ============================================================================

ofImage::ofImage()
    : impl_(std::make_unique<Impl>()) {
}

ofImage::ofImage(const std::string& fileName)
    : impl_(std::make_unique<Impl>()) {
    load(fileName);
}

ofImage::~ofImage() = default;

ofImage::ofImage(ofImage&& other) noexcept = default;
ofImage& ofImage::operator=(ofImage&& other) noexcept = default;

// ============================================================================
// Internal Helpers
// ============================================================================

void ofImage::ensureImpl() {
    if (!impl_) {
        impl_ = std::make_unique<Impl>();
    }
}

void ofImage::syncPixelsFromTexture() const {
    // TODO: Implement GPU->CPU readback if needed in future
    // For now, we maintain pixels on CPU side
}

void ofImage::syncTextureFromPixels() {
    ensureImpl();
    if (!impl_->pixels.isAllocated()) return;

    // Allocate texture if needed
    if (!impl_->texture.isAllocated() ||
        impl_->texture.getWidth() != static_cast<int>(impl_->pixels.getWidth()) ||
        impl_->texture.getHeight() != static_cast<int>(impl_->pixels.getHeight())) {
        impl_->texture.allocate(impl_->pixels);
    }

    // Upload pixel data to texture
    impl_->texture.loadData(impl_->pixels);
    impl_->pixelsDirty = false;
}

// ============================================================================
// Loading & Saving
// ============================================================================

bool ofImage::load(const std::string& fileName) {
    @autoreleasepool {
        ensureImpl();

        NSString* nsPath = [NSString stringWithUTF8String:fileName.c_str()];
        NSURL* url = [NSURL fileURLWithPath:nsPath];

        if (![[NSFileManager defaultManager] fileExistsAtPath:nsPath]) {
            ofLogError("ofImage") << "File not found: " << fileName;
            return false;
        }

        // Strategy 1: Try MTKTextureLoader for direct GPU upload (fastest)
        // TODO: This requires accessing MetalRenderer's device directly
        // For now, use ImageIO path below
        // Future: Add MetalRenderer::getDevice() method or use renderer->loadTexture()

        // Strategy 2: Use ImageIO for CPU loading (always works)
        CGImageSourceRef imageSource = CGImageSourceCreateWithURL((__bridge CFURLRef)url, NULL);
        if (!imageSource) {
            ofLogError("ofImage") << "Failed to create image source: " << fileName;
            return false;
        }

        CGImageRef cgImage = CGImageSourceCreateImageAtIndex(imageSource, 0, NULL);
        CFRelease(imageSource);

        if (!cgImage) {
            ofLogError("ofImage") << "Failed to decode image: " << fileName;
            return false;
        }

        // Get image dimensions and properties
        const size_t width = CGImageGetWidth(cgImage);
        const size_t height = CGImageGetHeight(cgImage);
        const size_t bitsPerComponent = 8;

        // Determine channel count from color space
        CGColorSpaceRef colorSpace = CGImageGetColorSpace(cgImage);
        CGColorSpaceModel colorSpaceModel = CGColorSpaceGetModel(colorSpace);

        size_t channels = 4;  // Default to RGBA
        if (colorSpaceModel == kCGColorSpaceModelMonochrome) {
            channels = 1;  // Grayscale
        } else {
            CGImageAlphaInfo alphaInfo = CGImageGetAlphaInfo(cgImage);
            if (alphaInfo == kCGImageAlphaNone || alphaInfo == kCGImageAlphaNoneSkipLast || alphaInfo == kCGImageAlphaNoneSkipFirst) {
                channels = 3;  // RGB
            } else {
                channels = 4;  // RGBA
            }
        }

        // Allocate pixel buffer
        impl_->pixels.allocate(width, height, channels);

        // Create bitmap context for pixel extraction
        CGColorSpaceRef drawColorSpace = (channels == 1)
            ? CGColorSpaceCreateDeviceGray()
            : CGColorSpaceCreateDeviceRGB();

        CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
        if (channels == 4) {
            bitmapInfo |= kCGImageAlphaPremultipliedLast;
        } else if (channels == 3) {
            bitmapInfo |= kCGImageAlphaNoneSkipLast;
        }

        CGContextRef context = CGBitmapContextCreate(
            impl_->pixels.getData(),
            width,
            height,
            bitsPerComponent,
            channels * width,
            drawColorSpace,
            bitmapInfo
        );

        CGColorSpaceRelease(drawColorSpace);

        if (!context) {
            CGImageRelease(cgImage);
            ofLogError("ofImage") << "Failed to create bitmap context";
            return false;
        }

        // Draw image into bitmap context (flips Y to top-left origin)
        CGContextDrawImage(context, CGRectMake(0, 0, width, height), cgImage);
        CGContextRelease(context);
        CGImageRelease(cgImage);

        // Upload to GPU
        impl_->pixelsDirty = true;
        syncTextureFromPixels();

        ofLogVerbose("ofImage") << "Loaded image: " << fileName
            << " (" << width << "x" << height << ", " << channels << " channels)";

        return true;
    }
}

bool ofImage::save(const std::string& fileName, float quality) {
    @autoreleasepool {
        ensureImpl();

        if (!impl_->pixels.isAllocated()) {
            ofLogError("ofImage") << "Cannot save: image not allocated";
            return false;
        }

        // Determine format from file extension
        NSString* nsPath = [NSString stringWithUTF8String:fileName.c_str()];
        NSString* ext = [[nsPath pathExtension] lowercaseString];

        CFStringRef imageType = kUTTypePNG;  // Default to PNG
        NSDictionary* options = nil;

        if ([ext isEqualToString:@"jpg"] || [ext isEqualToString:@"jpeg"]) {
            imageType = kUTTypeJPEG;
            options = @{(__bridge id)kCGImageDestinationLossyCompressionQuality: @(quality)};
        } else if ([ext isEqualToString:@"tif"] || [ext isEqualToString:@"tiff"]) {
            imageType = kUTTypeTIFF;
        } else if ([ext isEqualToString:@"png"]) {
            imageType = kUTTypePNG;
        } else {
            ofLogWarning("ofImage") << "Unknown extension '" << [ext UTF8String]
                << "', saving as PNG";
        }

        // Create CGImage from pixels
        const size_t width = impl_->pixels.getWidth();
        const size_t height = impl_->pixels.getHeight();
        const size_t channels = impl_->pixels.getNumChannels();
        const size_t bitsPerComponent = 8;
        const size_t bytesPerRow = channels * width;

        CGColorSpaceRef colorSpace = (channels == 1)
            ? CGColorSpaceCreateDeviceGray()
            : CGColorSpaceCreateDeviceRGB();

        CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
        if (channels == 4) {
            bitmapInfo |= kCGImageAlphaPremultipliedLast;
        } else if (channels == 3) {
            bitmapInfo |= kCGImageAlphaNoneSkipLast;
        }

        CGDataProviderRef dataProvider = CGDataProviderCreateWithData(
            NULL,
            impl_->pixels.getData(),
            impl_->pixels.getTotalBytes(),
            NULL
        );

        CGImageRef cgImage = CGImageCreate(
            width,
            height,
            bitsPerComponent,
            bitsPerComponent * channels,
            bytesPerRow,
            colorSpace,
            bitmapInfo,
            dataProvider,
            NULL,
            false,
            kCGRenderingIntentDefault
        );

        CGColorSpaceRelease(colorSpace);
        CGDataProviderRelease(dataProvider);

        if (!cgImage) {
            ofLogError("ofImage") << "Failed to create CGImage for saving";
            return false;
        }

        // Create destination and write image
        NSURL* url = [NSURL fileURLWithPath:nsPath];
        CGImageDestinationRef destination = CGImageDestinationCreateWithURL(
            (__bridge CFURLRef)url,
            imageType,
            1,
            NULL
        );

        if (!destination) {
            CGImageRelease(cgImage);
            ofLogError("ofImage") << "Failed to create image destination: " << fileName;
            return false;
        }

        CGImageDestinationAddImage(destination, cgImage, (__bridge CFDictionaryRef)options);
        bool success = CGImageDestinationFinalize(destination);

        CFRelease(destination);
        CGImageRelease(cgImage);

        if (success) {
            ofLogVerbose("ofImage") << "Saved image: " << fileName;
        } else {
            ofLogError("ofImage") << "Failed to save image: " << fileName;
        }

        return success;
    }
}

// ============================================================================
// Drawing
// ============================================================================

void ofImage::draw(float x, float y) const {
    if (!isAllocated()) return;

    if (impl_->bUseTexture && impl_->texture.isAllocated()) {
        impl_->texture.draw(x, y);
    }
}

void ofImage::draw(float x, float y, float w, float h) const {
    if (!isAllocated()) return;

    if (impl_->bUseTexture && impl_->texture.isAllocated()) {
        impl_->texture.draw(x, y, w, h);
    }
}

void ofImage::draw(float x, float y, float z) const {
    if (!isAllocated()) return;

    if (impl_->bUseTexture && impl_->texture.isAllocated()) {
        impl_->texture.draw(x, y, z);
    }
}

void ofImage::draw(float x, float y, float z, float w, float h) const {
    if (!isAllocated()) return;

    if (impl_->bUseTexture && impl_->texture.isAllocated()) {
        impl_->texture.draw(x, y, z, w, h);
    }
}

// ============================================================================
// Data Access
// ============================================================================

ofPixels& ofImage::getPixels() {
    ensureImpl();
    return impl_->pixels;
}

const ofPixels& ofImage::getPixels() const {
    return impl_->pixels;
}

ofTexture& ofImage::getTexture() {
    ensureImpl();
    if (impl_->pixelsDirty) {
        const_cast<ofImage*>(this)->syncTextureFromPixels();
    }
    return impl_->texture;
}

const ofTexture& ofImage::getTexture() const {
    return impl_->texture;
}

// ============================================================================
// Pixel Manipulation
// ============================================================================

void ofImage::setFromPixels(const ofPixels& pixels) {
    ensureImpl();
    impl_->pixels = pixels;
    impl_->pixelsDirty = true;
    syncTextureFromPixels();
}

void ofImage::setFromPixels(const ofShortPixels& pixels) {
    ensureImpl();
    // Convert to 8-bit
    impl_->pixels.allocate(pixels.getWidth(), pixels.getHeight(), pixels.getNumChannels());
    const size_t totalElements = pixels.getWidth() * pixels.getHeight() * pixels.getNumChannels();
    for (size_t i = 0; i < totalElements; ++i) {
        impl_->pixels[i] = static_cast<uint8_t>(pixels[i] >> 8);  // Convert 16-bit to 8-bit
    }
    impl_->pixelsDirty = true;
    syncTextureFromPixels();
}

void ofImage::setFromPixels(const ofFloatPixels& pixels) {
    ensureImpl();
    // Convert to 8-bit
    impl_->pixels.allocate(pixels.getWidth(), pixels.getHeight(), pixels.getNumChannels());
    const size_t totalElements = pixels.getWidth() * pixels.getHeight() * pixels.getNumChannels();
    for (size_t i = 0; i < totalElements; ++i) {
        impl_->pixels[i] = static_cast<uint8_t>(pixels[i] * 255.0f);  // Convert float to 8-bit
    }
    impl_->pixelsDirty = true;
    syncTextureFromPixels();
}

void ofImage::setFromPixels(const unsigned char* pixels, int w, int h, ofImageType type) {
    ensureImpl();
    impl_->pixels.setFromPixels(pixels, w, h, type);
    impl_->pixelsDirty = true;
    syncTextureFromPixels();
}

void ofImage::update() {
    if (impl_->pixelsDirty) {
        syncTextureFromPixels();
    }
}

// ============================================================================
// Image Manipulation
// ============================================================================

void ofImage::resize(int newWidth, int newHeight) {
    @autoreleasepool {
        ensureImpl();

        if (!impl_->pixels.isAllocated()) {
            ofLogWarning("ofImage") << "Cannot resize: image not allocated";
            return;
        }

        if (newWidth <= 0 || newHeight <= 0) {
            ofLogWarning("ofImage") << "Invalid resize dimensions";
            return;
        }

        const size_t oldWidth = impl_->pixels.getWidth();
        const size_t oldHeight = impl_->pixels.getHeight();
        const size_t channels = impl_->pixels.getNumChannels();

        if (oldWidth == static_cast<size_t>(newWidth) && oldHeight == static_cast<size_t>(newHeight)) {
            return;  // Already at target size
        }

        // Use vImage for high-quality Lanczos resampling
        vImage_Buffer srcBuffer = {
            .data = impl_->pixels.getData(),
            .height = static_cast<vImagePixelCount>(oldHeight),
            .width = static_cast<vImagePixelCount>(oldWidth),
            .rowBytes = oldWidth * channels
        };

        ofPixels newPixels;
        newPixels.allocate(newWidth, newHeight, channels);

        vImage_Buffer dstBuffer = {
            .data = newPixels.getData(),
            .height = static_cast<vImagePixelCount>(newHeight),
            .width = static_cast<vImagePixelCount>(newWidth),
            .rowBytes = static_cast<size_t>(newWidth) * channels
        };

        vImage_Error error = kvImageNoError;

        if (channels == 1) {
            error = vImageScale_Planar8(&srcBuffer, &dstBuffer, NULL, kvImageHighQualityResampling);
        } else if (channels == 3 || channels == 4) {
            error = vImageScale_ARGB8888(&srcBuffer, &dstBuffer, NULL, kvImageHighQualityResampling);
        }

        if (error != kvImageNoError) {
            ofLogError("ofImage") << "vImage resize failed with error: " << error;
            return;
        }

        impl_->pixels = std::move(newPixels);
        impl_->pixelsDirty = true;
        syncTextureFromPixels();
    }
}

void ofImage::crop(int x, int y, int w, int h) {
    ensureImpl();

    if (!impl_->pixels.isAllocated()) {
        ofLogWarning("ofImage") << "Cannot crop: image not allocated";
        return;
    }

    const int srcWidth = static_cast<int>(impl_->pixels.getWidth());
    const int srcHeight = static_cast<int>(impl_->pixels.getHeight());
    const size_t channels = impl_->pixels.getNumChannels();

    // Clamp crop rectangle to image bounds
    x = std::max(0, std::min(x, srcWidth - 1));
    y = std::max(0, std::min(y, srcHeight - 1));
    w = std::max(1, std::min(w, srcWidth - x));
    h = std::max(1, std::min(h, srcHeight - y));

    ofPixels croppedPixels;
    croppedPixels.allocate(w, h, channels);

    // Copy pixels row by row
    for (int row = 0; row < h; ++row) {
        const size_t srcIdx = impl_->pixels.getPixelIndex(x, y + row);
        const size_t dstIdx = croppedPixels.getPixelIndex(0, row);
        std::memcpy(
            croppedPixels.getData() + dstIdx,
            impl_->pixels.getData() + srcIdx,
            w * channels
        );
    }

    impl_->pixels = std::move(croppedPixels);
    impl_->pixelsDirty = true;
    syncTextureFromPixels();
}

void ofImage::rotate90(int nClockwiseTurns) {
    ensureImpl();

    if (!impl_->pixels.isAllocated()) {
        ofLogWarning("ofImage") << "Cannot rotate: image not allocated";
        return;
    }

    // Normalize turns to 0-3 range
    nClockwiseTurns = ((nClockwiseTurns % 4) + 4) % 4;

    if (nClockwiseTurns == 0) {
        return;  // No rotation needed
    }

    const size_t width = impl_->pixels.getWidth();
    const size_t height = impl_->pixels.getHeight();
    const size_t channels = impl_->pixels.getNumChannels();

    for (int turn = 0; turn < nClockwiseTurns; ++turn) {
        const size_t oldWidth = impl_->pixels.getWidth();
        const size_t oldHeight = impl_->pixels.getHeight();

        ofPixels rotatedPixels;
        rotatedPixels.allocate(oldHeight, oldWidth, channels);  // Swap dimensions

        // Rotate 90 degrees clockwise
        for (size_t y = 0; y < oldHeight; ++y) {
            for (size_t x = 0; x < oldWidth; ++x) {
                const size_t srcIdx = impl_->pixels.getPixelIndex(x, y);
                const size_t dstIdx = rotatedPixels.getPixelIndex(oldHeight - 1 - y, x);

                for (size_t c = 0; c < channels; ++c) {
                    rotatedPixels[dstIdx + c] = impl_->pixels[srcIdx + c];
                }
            }
        }

        impl_->pixels = std::move(rotatedPixels);
    }

    impl_->pixelsDirty = true;
    syncTextureFromPixels();
}

void ofImage::mirror(bool vertical) {
    ensureImpl();

    if (!impl_->pixels.isAllocated()) {
        ofLogWarning("ofImage") << "Cannot mirror: image not allocated";
        return;
    }

    const size_t width = impl_->pixels.getWidth();
    const size_t height = impl_->pixels.getHeight();
    const size_t channels = impl_->pixels.getNumChannels();

    if (vertical) {
        // Mirror vertically (flip top-bottom)
        for (size_t y = 0; y < height / 2; ++y) {
            const size_t topIdx = impl_->pixels.getPixelIndex(0, y);
            const size_t bottomIdx = impl_->pixels.getPixelIndex(0, height - 1 - y);

            for (size_t x = 0; x < width * channels; ++x) {
                std::swap(impl_->pixels[topIdx + x], impl_->pixels[bottomIdx + x]);
            }
        }
    } else {
        // Mirror horizontally (flip left-right)
        for (size_t y = 0; y < height; ++y) {
            for (size_t x = 0; x < width / 2; ++x) {
                const size_t leftIdx = impl_->pixels.getPixelIndex(x, y);
                const size_t rightIdx = impl_->pixels.getPixelIndex(width - 1 - x, y);

                for (size_t c = 0; c < channels; ++c) {
                    std::swap(impl_->pixels[leftIdx + c], impl_->pixels[rightIdx + c]);
                }
            }
        }
    }

    impl_->pixelsDirty = true;
    syncTextureFromPixels();
}

// ============================================================================
// Format Conversion
// ============================================================================

void ofImage::setImageType(ofImageType newType) {
    ensureImpl();

    if (!impl_->pixels.isAllocated()) {
        ofLogWarning("ofImage") << "Cannot set image type: image not allocated";
        return;
    }

    impl_->pixels.setImageType(newType);
    impl_->pixelsDirty = true;
    syncTextureFromPixels();
}

ofImageType ofImage::getImageType() const {
    if (!impl_ || !impl_->pixels.isAllocated()) {
        return OF_IMAGE_COLOR_ALPHA;
    }
    return impl_->pixels.getImageType();
}

// ============================================================================
// Properties
// ============================================================================

float ofImage::getWidth() const {
    if (!impl_ || !impl_->pixels.isAllocated()) {
        return 0.0f;
    }
    return static_cast<float>(impl_->pixels.getWidth());
}

float ofImage::getHeight() const {
    if (!impl_ || !impl_->pixels.isAllocated()) {
        return 0.0f;
    }
    return static_cast<float>(impl_->pixels.getHeight());
}

bool ofImage::isAllocated() const {
    return impl_ && impl_->pixels.isAllocated();
}

bool ofImage::isLoaded() const {
    return isAllocated();
}

void ofImage::clear() {
    if (impl_) {
        impl_->pixels.clear();
        impl_->texture.clear();
        impl_->pixelsDirty = false;
        impl_->textureDirty = false;
    }
}

} // namespace oflike
