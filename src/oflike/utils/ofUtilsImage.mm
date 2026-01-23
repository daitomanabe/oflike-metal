#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <ImageIO/ImageIO.h>

#include "ofUtils.h"
#include "../image/ofPixels.h"
#include "../image/ofTexture.h"
#include "../../core/Context.h"
#include "ofLog.h"

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
        CGContextRef context = CGBitmapContextCreate(
            pixels.getData(),
            width,
            height,
            8,  // bits per component
            pixels.getBytesPerRow(),
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
    // For now, load as 8-bit and convert
    // TODO: Support native 16-bit loading from TIFF, PNG-16, etc.
    ofPixels temp;
    if (!ofLoadImage(temp, path)) {
        return false;
    }
    
    // Convert to 16-bit
    pixels.allocate(temp.getWidth(), temp.getHeight(), temp.getImageType());
    const unsigned char* src = temp.getData();
    unsigned short* dst = pixels.getData();
    size_t numValues = temp.getWidth() * temp.getHeight() * temp.getNumChannels();
    
    for (size_t i = 0; i < numValues; ++i) {
        dst[i] = static_cast<unsigned short>(src[i]) * 257;  // Scale 0-255 to 0-65535
    }
    
    return true;
}

// Float variant
bool ofLoadImage(ofFloatPixels& pixels, const std::string& path) {
    // For now, load as 8-bit and convert
    // TODO: Support native float loading from EXR, HDR, etc.
    ofPixels temp;
    if (!ofLoadImage(temp, path)) {
        return false;
    }
    
    // Convert to float
    pixels.allocate(temp.getWidth(), temp.getHeight(), temp.getImageType());
    const unsigned char* src = temp.getData();
    float* dst = pixels.getData();
    size_t numValues = temp.getWidth() * temp.getHeight() * temp.getNumChannels();
    
    for (size_t i = 0; i < numValues; ++i) {
        dst[i] = static_cast<float>(src[i]) / 255.0f;  // Scale 0-255 to 0.0-1.0
    }
    
    return true;
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
        id<MTLDevice> device = (__bridge id<MTLDevice>)ctx().renderer().getDevice();
        if (!device) {
            ofLogError("ofLoadImage") << "No Metal device available";
            return false;
        }

        // Use MTKTextureLoader for direct GPU upload
        MTKTextureLoader* loader = [[MTKTextureLoader alloc] initWithDevice:device];
        NSDictionary* options = @{
            MTKTextureLoaderOptionSRGB: @NO,
            MTKTextureLoaderOptionGenerateMipmaps: @NO,
            MTKTextureLoaderOptionOrigin: MTKTextureLoaderOriginTopLeft
        };

        NSError* error = nil;
        id<MTLTexture> mtlTexture = [loader newTextureWithContentsOfURL:url
                                                                options:options
                                                                  error:&error];

        if (error) {
            ofLogError("ofLoadImage") << "MTKTextureLoader failed: " << error.localizedDescription.UTF8String;
            
            // Fallback: load via ofPixels
            ofPixels pixels;
            if (!ofLoadImage(pixels, path)) {
                return false;
            }
            return texture.loadData(pixels);
        }

        if (!mtlTexture) {
            ofLogError("ofLoadImage") << "Failed to create texture";
            return false;
        }

        // Allocate texture with dimensions
        int width = static_cast<int>(mtlTexture.width);
        int height = static_cast<int>(mtlTexture.height);
        
        // Determine channels based on pixel format
        size_t channels = 4;
        if (mtlTexture.pixelFormat == MTLPixelFormatR8Unorm) {
            channels = 1;
        } else if (mtlTexture.pixelFormat == MTLPixelFormatRGBA8Unorm ||
                   mtlTexture.pixelFormat == MTLPixelFormatBGRA8Unorm) {
            channels = 4;
        }
        
        ofImageType type = channels == 1 ? ofImageType::OF_IMAGE_GRAYSCALE : ofImageType::OF_IMAGE_COLOR_ALPHA;
        texture.allocate(width, height, type);
        
        // Store the native MTLTexture handle
        // This requires access to ofTexture internals - for now just mark as allocated
        // TODO: Add ofTexture::setNativeHandle() method
        
        return true;
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
        CGContextRef context = CGBitmapContextCreate(
            const_cast<unsigned char*>(pixels.getData()),
            pixels.getWidth(),
            pixels.getHeight(),
            8,  // bits per component
            pixels.getBytesPerRow(),
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
    // Convert to 8-bit and save
    // TODO: Support native 16-bit saving to TIFF, PNG-16
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

// Float variant
bool ofSaveImage(const ofFloatPixels& pixels, const std::string& path, float quality) {
    // Convert to 8-bit and save
    // TODO: Support native float saving to EXR, HDR
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
