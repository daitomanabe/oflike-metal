#import <Foundation/Foundation.h>
#import <ImageIO/ImageIO.h>

#include "ofUtils.h"
#include "../image/ofPixels.h"
#include "../image/ofTexture.h"
#include "../../core/Context.h"
#include "ofLog.h"

using oflike::ofLogError;
using oflike::ofImageType;

// ============================================================================
// Image Loading - ofPixels variant (ImageIO)
// ============================================================================

bool ofLoadImage(ofPixels& pixels, const std::string& path) {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        NSURL* url = [NSURL fileURLWithPath:nsPath];

        if (![[NSFileManager defaultManager] fileExistsAtPath:nsPath]) {
            ofLogError("ofLoadImage") << "File not found: " << path;
            return false;
        }

        // Use ImageIO for loading
        CGImageSourceRef imageSource = CGImageSourceCreateWithURL((__bridge CFURLRef)url, NULL);
        if (!imageSource) {
            ofLogError("ofLoadImage") << "Failed to create image source: " << path;
            return false;
        }

        CGImageRef cgImage = CGImageSourceCreateImageAtIndex(imageSource, 0, NULL);
        CFRelease(imageSource);

        if (!cgImage) {
            ofLogError("ofLoadImage") << "Failed to decode image: " << path;
            return false;
        }

        // Get image dimensions
        const size_t width = CGImageGetWidth(cgImage);
        const size_t height = CGImageGetHeight(cgImage);

        // Determine channel count
        CGColorSpaceRef colorSpace = CGImageGetColorSpace(cgImage);
        CGColorSpaceModel colorSpaceModel = CGColorSpaceGetModel(colorSpace);

        size_t channels = 4;  // Default to RGBA
        if (colorSpaceModel == kCGColorSpaceModelMonochrome) {
            channels = 1;
        } else {
            CGImageAlphaInfo alphaInfo = CGImageGetAlphaInfo(cgImage);
            if (alphaInfo == kCGImageAlphaNone || alphaInfo == kCGImageAlphaNoneSkipLast || alphaInfo == kCGImageAlphaNoneSkipFirst) {
                channels = 3;
            }
        }

        // Determine ofImageType
        ofImageType type = ofImageType::OF_IMAGE_COLOR_ALPHA;
        if (channels == 1) {
            type = ofImageType::OF_IMAGE_GRAYSCALE;
        } else if (channels == 3) {
            type = ofImageType::OF_IMAGE_COLOR;
        }

        // Allocate pixel buffer
        pixels.allocate(width, height, type);

        // Create bitmap context
        size_t bytesPerRow = width * pixels.getBytesPerPixel();
        CGContextRef context = CGBitmapContextCreate(
            pixels.getData(),
            width,
            height,
            8,  // bits per component
            bytesPerRow,
            colorSpace,
            channels == 4 ? kCGImageAlphaPremultipliedLast : kCGImageAlphaNoneSkipLast
        );

        if (!context) {
            ofLogError("ofLoadImage") << "Failed to create bitmap context";
            CGImageRelease(cgImage);
            return false;
        }

        // Draw image to context (flips Y coordinate)
        CGContextDrawImage(context, CGRectMake(0, 0, width, height), cgImage);

        CGContextRelease(context);
        CGImageRelease(cgImage);

        return true;
    }
}

// 16-bit variant
bool ofLoadImage(ofShortPixels& pixels, const std::string& path) {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        NSURL* url = [NSURL fileURLWithPath:nsPath];

        if (![[NSFileManager defaultManager] fileExistsAtPath:nsPath]) {
            ofLogError("ofLoadImage") << "File not found: " << path;
            return false;
        }

        // Use ImageIO for loading
        CGImageSourceRef imageSource = CGImageSourceCreateWithURL((__bridge CFURLRef)url, NULL);
        if (!imageSource) {
            ofLogError("ofLoadImage") << "Failed to create image source: " << path;
            return false;
        }

        CGImageRef cgImage = CGImageSourceCreateImageAtIndex(imageSource, 0, NULL);
        CFRelease(imageSource);

        if (!cgImage) {
            ofLogError("ofLoadImage") << "Failed to decode image: " << path;
            return false;
        }

        // Get image properties
        const size_t width = CGImageGetWidth(cgImage);
        const size_t height = CGImageGetHeight(cgImage);
        const size_t bitsPerComponent = CGImageGetBitsPerComponent(cgImage);

        CGColorSpaceRef colorSpace = CGImageGetColorSpace(cgImage);
        CGColorSpaceModel colorSpaceModel = CGColorSpaceGetModel(colorSpace);

        // Determine channel count
        size_t channels = 4;  // Default to RGBA
        if (colorSpaceModel == kCGColorSpaceModelMonochrome) {
            channels = 1;
        } else {
            CGImageAlphaInfo alphaInfo = CGImageGetAlphaInfo(cgImage);
            if (alphaInfo == kCGImageAlphaNone || alphaInfo == kCGImageAlphaNoneSkipLast || alphaInfo == kCGImageAlphaNoneSkipFirst) {
                channels = 3;
            }
        }

        // Determine ofImageType
        ofImageType type = ofImageType::OF_IMAGE_COLOR_ALPHA;
        if (channels == 1) {
            type = ofImageType::OF_IMAGE_GRAYSCALE;
        } else if (channels == 3) {
            type = ofImageType::OF_IMAGE_COLOR;
        }

        // Allocate pixel buffer
        pixels.allocate(width, height, type);

        // Check if source is already 16-bit
        if (bitsPerComponent == 16) {
            // Native 16-bit loading
            size_t bytesPerRow = width * pixels.getBytesPerPixel();
            CGContextRef context = CGBitmapContextCreate(
                pixels.getData(),
                width,
                height,
                16,  // bits per component
                bytesPerRow,
                colorSpace,
                channels == 4 ? kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder16Host : kCGImageAlphaNoneSkipLast | kCGBitmapByteOrder16Host
            );

            if (!context) {
                ofLogError("ofLoadImage") << "Failed to create 16-bit bitmap context";
                CGImageRelease(cgImage);
                return false;
            }

            // Draw image to context
            CGContextDrawImage(context, CGRectMake(0, 0, width, height), cgImage);
            CGContextRelease(context);
            CGImageRelease(cgImage);
            return true;
        } else {
            // Convert from 8-bit to 16-bit
            CGImageRelease(cgImage);

            ofPixels temp;
            if (!ofLoadImage(temp, path)) {
                return false;
            }

            const unsigned char* src = temp.getData();
            unsigned short* dst = pixels.getData();
            size_t numValues = temp.getWidth() * temp.getHeight() * temp.getNumChannels();

            for (size_t i = 0; i < numValues; ++i) {
                dst[i] = static_cast<unsigned short>(src[i]) * 257;  // Scale 0-255 to 0-65535
            }

            return true;
        }
    }
}

// Float variant
bool ofLoadImage(ofFloatPixels& pixels, const std::string& path) {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        NSURL* url = [NSURL fileURLWithPath:nsPath];

        if (![[NSFileManager defaultManager] fileExistsAtPath:nsPath]) {
            ofLogError("ofLoadImage") << "File not found: " << path;
            return false;
        }

        // Use ImageIO for loading
        CGImageSourceRef imageSource = CGImageSourceCreateWithURL((__bridge CFURLRef)url, NULL);
        if (!imageSource) {
            ofLogError("ofLoadImage") << "Failed to create image source: " << path;
            return false;
        }

        CGImageRef cgImage = CGImageSourceCreateImageAtIndex(imageSource, 0, NULL);
        CFRelease(imageSource);

        if (!cgImage) {
            ofLogError("ofLoadImage") << "Failed to decode image: " << path;
            return false;
        }

        // Get image properties
        const size_t width = CGImageGetWidth(cgImage);
        const size_t height = CGImageGetHeight(cgImage);
        const size_t bitsPerComponent = CGImageGetBitsPerComponent(cgImage);

        CGColorSpaceRef colorSpace = CGImageGetColorSpace(cgImage);
        CGColorSpaceModel colorSpaceModel = CGColorSpaceGetModel(colorSpace);

        // Determine channel count
        size_t channels = 4;  // Default to RGBA
        if (colorSpaceModel == kCGColorSpaceModelMonochrome) {
            channels = 1;
        } else {
            CGImageAlphaInfo alphaInfo = CGImageGetAlphaInfo(cgImage);
            if (alphaInfo == kCGImageAlphaNone || alphaInfo == kCGImageAlphaNoneSkipLast || alphaInfo == kCGImageAlphaNoneSkipFirst) {
                channels = 3;
            }
        }

        // Determine ofImageType
        ofImageType type = ofImageType::OF_IMAGE_COLOR_ALPHA;
        if (channels == 1) {
            type = ofImageType::OF_IMAGE_GRAYSCALE;
        } else if (channels == 3) {
            type = ofImageType::OF_IMAGE_COLOR;
        }

        // Allocate pixel buffer
        pixels.allocate(width, height, type);

        // Check if source is float/HDR format
        if (bitsPerComponent == 32) {
            // Native float loading
            size_t bytesPerRow = width * pixels.getBytesPerPixel();
            CGContextRef context = CGBitmapContextCreate(
                pixels.getData(),
                width,
                height,
                32,  // bits per component
                bytesPerRow,
                colorSpace,
                channels == 4 ? kCGImageAlphaPremultipliedLast | kCGBitmapFloatComponents | kCGBitmapByteOrder32Host : kCGImageAlphaNoneSkipLast | kCGBitmapFloatComponents | kCGBitmapByteOrder32Host
            );

            if (!context) {
                ofLogError("ofLoadImage") << "Failed to create float bitmap context";
                CGImageRelease(cgImage);
                return false;
            }

            // Draw image to context
            CGContextDrawImage(context, CGRectMake(0, 0, width, height), cgImage);
            CGContextRelease(context);
            CGImageRelease(cgImage);
            return true;
        } else if (bitsPerComponent == 16) {
            // Convert from 16-bit to float
            CGImageRelease(cgImage);

            ofShortPixels temp;
            if (!ofLoadImage(temp, path)) {
                return false;
            }

            const unsigned short* src = temp.getData();
            float* dst = pixels.getData();
            size_t numValues = temp.getWidth() * temp.getHeight() * temp.getNumChannels();

            for (size_t i = 0; i < numValues; ++i) {
                dst[i] = static_cast<float>(src[i]) / 65535.0f;  // Scale 0-65535 to 0.0-1.0
            }

            return true;
        } else {
            // Convert from 8-bit to float
            CGImageRelease(cgImage);

            ofPixels temp;
            if (!ofLoadImage(temp, path)) {
                return false;
            }

            const unsigned char* src = temp.getData();
            float* dst = pixels.getData();
            size_t numValues = temp.getWidth() * temp.getHeight() * temp.getNumChannels();

            for (size_t i = 0; i < numValues; ++i) {
                dst[i] = static_cast<float>(src[i]) / 255.0f;  // Scale 0-255 to 0.0-1.0
            }

            return true;
        }
    }
}

// ============================================================================
// Image Loading - ofTexture variant (MTKTextureLoader)
// ============================================================================

bool ofLoadImage(ofTexture& texture, const std::string& path) {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        NSURL* url = [NSURL fileURLWithPath:nsPath];

        if (![[NSFileManager defaultManager] fileExistsAtPath:nsPath]) {
            ofLogError("ofLoadImage") << "File not found: " << path;
            return false;
        }

        // Get Metal device
        // TODO: MetalRenderer doesn't expose getDevice() method
        // For now, use fallback path
        // id<MTLDevice> device = (__bridge id<MTLDevice>)ctx().renderer()->getDevice();
        // if (!device) {
        //     ofLogError("ofLoadImage") << "No Metal device available";
        //     return false;
        // }

        // Fallback: load via ofPixels (MTKTextureLoader path requires device access)
        ofPixels pixels;
        if (!ofLoadImage(pixels, path)) {
            return false;
        }
        texture.loadData(pixels);
        return true;

        // TODO: USE MTKTextureLoader for direct GPU upload (when device access available)
        // MTKTextureLoader* loader = [[MTKTextureLoader alloc] initWithDevice:device];
        // ... (code removed, see git history)
    }
}

// ============================================================================
// Image Saving - ofPixels variant (CGImageDestination)
// ============================================================================

bool ofSaveImage(const ofPixels& pixels, const std::string& path, float quality) {
    @autoreleasepool {
        if (!pixels.isAllocated()) {
            ofLogError("ofSaveImage") << "Pixels not allocated";
            return false;
        }

        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        NSURL* url = [NSURL fileURLWithPath:nsPath];

        // Determine format from extension
        NSString* extension = [nsPath pathExtension].lowercaseString;
        CFStringRef imageType = kUTTypePNG;  // Default to PNG
        
        if ([extension isEqualToString:@"jpg"] || [extension isEqualToString:@"jpeg"]) {
            imageType = kUTTypeJPEG;
        } else if ([extension isEqualToString:@"tif"] || [extension isEqualToString:@"tiff"]) {
            imageType = kUTTypeTIFF;
        }

        // Create image destination
        CGImageDestinationRef destination = CGImageDestinationCreateWithURL(
            (__bridge CFURLRef)url,
            imageType,
            1,
            NULL
        );

        if (!destination) {
            ofLogError("ofSaveImage") << "Failed to create image destination: " << path;
            return false;
        }

        // Create color space
        size_t channels = pixels.getNumChannels();
        CGColorSpaceRef colorSpace;
        
        if (channels == 1) {
            colorSpace = CGColorSpaceCreateDeviceGray();
        } else {
            colorSpace = CGColorSpaceCreateDeviceRGB();
        }

        // Create CGImage from pixel data
        size_t bytesPerRow = pixels.getWidth() * pixels.getBytesPerPixel();
        CGContextRef context = CGBitmapContextCreate(
            const_cast<unsigned char*>(pixels.getData()),
            pixels.getWidth(),
            pixels.getHeight(),
            8,  // bits per component
            bytesPerRow,
            colorSpace,
            channels == 4 ? kCGImageAlphaPremultipliedLast : kCGImageAlphaNoneSkipLast
        );

        if (!context) {
            ofLogError("ofSaveImage") << "Failed to create bitmap context";
            CGColorSpaceRelease(colorSpace);
            CFRelease(destination);
            return false;
        }

        CGImageRef cgImage = CGBitmapContextCreateImage(context);
        CGContextRelease(context);

        if (!cgImage) {
            ofLogError("ofSaveImage") << "Failed to create CGImage";
            CGColorSpaceRelease(colorSpace);
            CFRelease(destination);
            return false;
        }

        // Set quality for JPEG
        NSDictionary* properties = nil;
        if (imageType == kUTTypeJPEG) {
            properties = @{
                (__bridge id)kCGImageDestinationLossyCompressionQuality: @(quality)
            };
        }

        // Add image to destination
        CGImageDestinationAddImage(destination, cgImage, (__bridge CFDictionaryRef)properties);

        // Finalize
        bool success = CGImageDestinationFinalize(destination);

        CGImageRelease(cgImage);
        CGColorSpaceRelease(colorSpace);
        CFRelease(destination);

        if (!success) {
            ofLogError("ofSaveImage") << "Failed to write image: " << path;
            return false;
        }

        return true;
    }
}

// 16-bit variant
bool ofSaveImage(const ofShortPixels& pixels, const std::string& path, float quality) {
    @autoreleasepool {
        if (!pixels.isAllocated()) {
            ofLogError("ofSaveImage") << "Pixels not allocated";
            return false;
        }

        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        NSURL* url = [NSURL fileURLWithPath:nsPath];

        // Determine format from extension
        NSString* extension = [nsPath pathExtension].lowercaseString;
        CFStringRef imageType = kUTTypeTIFF;  // Default to TIFF for 16-bit

        if ([extension isEqualToString:@"png"]) {
            imageType = kUTTypePNG;  // PNG also supports 16-bit
        } else if ([extension isEqualToString:@"tif"] || [extension isEqualToString:@"tiff"]) {
            imageType = kUTTypeTIFF;
        } else if ([extension isEqualToString:@"jpg"] || [extension isEqualToString:@"jpeg"]) {
            // JPEG doesn't support 16-bit, convert to 8-bit
            ofPixels temp;
            temp.allocate(pixels.getWidth(), pixels.getHeight(), pixels.getImageType());

            const unsigned short* src = pixels.getData();
            unsigned char* dst = temp.getData();
            size_t numValues = pixels.getWidth() * pixels.getHeight() * pixels.getNumChannels();

            for (size_t i = 0; i < numValues; ++i) {
                dst[i] = static_cast<unsigned char>(src[i] / 257);  // Scale 0-65535 to 0-255
            }

            return ofSaveImage(temp, path, quality);
        }

        // Create image destination
        CGImageDestinationRef destination = CGImageDestinationCreateWithURL(
            (__bridge CFURLRef)url,
            imageType,
            1,
            NULL
        );

        if (!destination) {
            ofLogError("ofSaveImage") << "Failed to create image destination: " << path;
            return false;
        }

        // Create color space
        size_t channels = pixels.getNumChannels();
        CGColorSpaceRef colorSpace;

        if (channels == 1) {
            colorSpace = CGColorSpaceCreateDeviceGray();
        } else {
            colorSpace = CGColorSpaceCreateDeviceRGB();
        }

        // Create CGImage from 16-bit pixel data
        size_t bytesPerRow = pixels.getWidth() * pixels.getBytesPerPixel();
        CGContextRef context = CGBitmapContextCreate(
            const_cast<unsigned short*>(pixels.getData()),
            pixels.getWidth(),
            pixels.getHeight(),
            16,  // bits per component
            bytesPerRow,
            colorSpace,
            channels == 4 ? kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder16Host : kCGImageAlphaNoneSkipLast | kCGBitmapByteOrder16Host
        );

        if (!context) {
            ofLogError("ofSaveImage") << "Failed to create 16-bit bitmap context";
            CGColorSpaceRelease(colorSpace);
            CFRelease(destination);
            return false;
        }

        CGImageRef cgImage = CGBitmapContextCreateImage(context);
        CGContextRelease(context);

        if (!cgImage) {
            ofLogError("ofSaveImage") << "Failed to create CGImage";
            CGColorSpaceRelease(colorSpace);
            CFRelease(destination);
            return false;
        }

        // No quality setting for TIFF/PNG (lossless)
        NSDictionary* properties = nil;

        // Add image to destination
        CGImageDestinationAddImage(destination, cgImage, (__bridge CFDictionaryRef)properties);

        // Finalize
        bool success = CGImageDestinationFinalize(destination);

        CGImageRelease(cgImage);
        CGColorSpaceRelease(colorSpace);
        CFRelease(destination);

        if (!success) {
            ofLogError("ofSaveImage") << "Failed to write image: " << path;
            return false;
        }

        return true;
    }
}

// Float variant
bool ofSaveImage(const ofFloatPixels& pixels, const std::string& path, float quality) {
    @autoreleasepool {
        if (!pixels.isAllocated()) {
            ofLogError("ofSaveImage") << "Pixels not allocated";
            return false;
        }

        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        NSURL* url = [NSURL fileURLWithPath:nsPath];

        // Determine format from extension
        NSString* extension = [nsPath pathExtension].lowercaseString;
        CFStringRef imageType = kUTTypeTIFF;  // Default to TIFF for float

        // Check if format supports float
        bool supportsFloat = false;
        if ([extension isEqualToString:@"tif"] || [extension isEqualToString:@"tiff"]) {
            imageType = kUTTypeTIFF;
            supportsFloat = true;
        } else if ([extension isEqualToString:@"exr"]) {
            // EXR requires external library (OpenEXR), not supported by ImageIO
            ofLogError("ofSaveImage") << "EXR format not supported yet (requires OpenEXR)";
            return false;
        } else if ([extension isEqualToString:@"hdr"]) {
            // HDR/Radiance format not directly supported by ImageIO
            ofLogError("ofSaveImage") << "HDR format not supported yet";
            return false;
        } else {
            // Other formats don't support float, convert to 8-bit
            ofPixels temp;
            temp.allocate(pixels.getWidth(), pixels.getHeight(), pixels.getImageType());

            const float* src = pixels.getData();
            unsigned char* dst = temp.getData();
            size_t numValues = pixels.getWidth() * pixels.getHeight() * pixels.getNumChannels();

            for (size_t i = 0; i < numValues; ++i) {
                float val = src[i] * 255.0f;
                val = std::max(0.0f, std::min(255.0f, val));  // Clamp
                dst[i] = static_cast<unsigned char>(val);
            }

            return ofSaveImage(temp, path, quality);
        }

        // Create image destination
        CGImageDestinationRef destination = CGImageDestinationCreateWithURL(
            (__bridge CFURLRef)url,
            imageType,
            1,
            NULL
        );

        if (!destination) {
            ofLogError("ofSaveImage") << "Failed to create image destination: " << path;
            return false;
        }

        // Create color space
        size_t channels = pixels.getNumChannels();
        CGColorSpaceRef colorSpace;

        if (channels == 1) {
            colorSpace = CGColorSpaceCreateDeviceGray();
        } else {
            colorSpace = CGColorSpaceCreateDeviceRGB();
        }

        // Create CGImage from float pixel data
        size_t bytesPerRow = pixels.getWidth() * pixels.getBytesPerPixel();
        CGContextRef context = CGBitmapContextCreate(
            const_cast<float*>(pixels.getData()),
            pixels.getWidth(),
            pixels.getHeight(),
            32,  // bits per component
            bytesPerRow,
            colorSpace,
            channels == 4 ? kCGImageAlphaPremultipliedLast | kCGBitmapFloatComponents | kCGBitmapByteOrder32Host : kCGImageAlphaNoneSkipLast | kCGBitmapFloatComponents | kCGBitmapByteOrder32Host
        );

        if (!context) {
            ofLogError("ofSaveImage") << "Failed to create float bitmap context";
            CGColorSpaceRelease(colorSpace);
            CFRelease(destination);
            return false;
        }

        CGImageRef cgImage = CGBitmapContextCreateImage(context);
        CGContextRelease(context);

        if (!cgImage) {
            ofLogError("ofSaveImage") << "Failed to create CGImage";
            CGColorSpaceRelease(colorSpace);
            CFRelease(destination);
            return false;
        }

        // No quality setting for TIFF (lossless)
        NSDictionary* properties = nil;

        // Add image to destination
        CGImageDestinationAddImage(destination, cgImage, (__bridge CFDictionaryRef)properties);

        // Finalize
        bool success = CGImageDestinationFinalize(destination);

        CGImageRelease(cgImage);
        CGColorSpaceRelease(colorSpace);
        CFRelease(destination);

        if (!success) {
            ofLogError("ofSaveImage") << "Failed to write image: " << path;
            return false;
        }

        return true;
    }
}
