#include "ofFile.h"
#include "ofBuffer.h"
#include "ofFilePath.h"
#import <Foundation/Foundation.h>

namespace oflike {

struct ofFile::Impl {
    std::string path;

    Impl() = default;
    explicit Impl(const std::string& p) : path(p) {}
};

ofFile::ofFile() : impl_(std::make_unique<Impl>()) {}

ofFile::ofFile(const std::string& path)
    : impl_(std::make_unique<Impl>(path)) {}

ofFile::~ofFile() = default;

ofFile::ofFile(ofFile&& other) noexcept = default;
ofFile& ofFile::operator=(ofFile&& other) noexcept = default;

bool ofFile::open(const std::string& path) {
    impl_->path = path;
    return exists();
}

void ofFile::close() {
    impl_->path.clear();
}

bool ofFile::exists() const {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:impl_->path.c_str()];
        return [[NSFileManager defaultManager] fileExistsAtPath:nsPath];
    }
}

std::string ofFile::getFilePath() const {
    return impl_->path;
}

std::string ofFile::getFileName() const {
    return ofFilePath::getFileName(impl_->path);
}

std::string ofFile::getBaseName() const {
    return ofFilePath::getBaseName(impl_->path);
}

std::string ofFile::getExtension() const {
    return ofFilePath::getFileExt(impl_->path);
}

std::string ofFile::getAbsolutePath() const {
    return ofFilePath::getAbsolutePath(impl_->path);
}

std::string ofFile::getEnclosingDirectory() const {
    return ofFilePath::getEnclosingDirectory(impl_->path);
}

uint64_t ofFile::getSize() const {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:impl_->path.c_str()];
        NSError* error = nil;
        NSDictionary* attrs = [[NSFileManager defaultManager] attributesOfItemAtPath:nsPath error:&error];

        if (error || !attrs) {
            return 0;
        }

        return [attrs fileSize];
    }
}

bool ofFile::isDirectory() const {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:impl_->path.c_str()];
        BOOL isDir = NO;
        BOOL exists = [[NSFileManager defaultManager] fileExistsAtPath:nsPath isDirectory:&isDir];
        return exists && isDir;
    }
}

bool ofFile::isFile() const {
    return exists() && !isDirectory();
}

bool ofFile::create() {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:impl_->path.c_str()];
        NSData* emptyData = [NSData data];
        return [[NSFileManager defaultManager] createFileAtPath:nsPath
                                                       contents:emptyData
                                                     attributes:nil];
    }
}

bool ofFile::copyTo(const std::string& destination, bool overwrite) {
    @autoreleasepool {
        NSString* srcPath = [NSString stringWithUTF8String:impl_->path.c_str()];
        NSString* dstPath = [NSString stringWithUTF8String:destination.c_str()];
        NSFileManager* fm = [NSFileManager defaultManager];

        // Check if destination exists
        if ([fm fileExistsAtPath:dstPath]) {
            if (!overwrite) {
                return false;
            }
            // Remove existing file
            NSError* removeError = nil;
            [fm removeItemAtPath:dstPath error:&removeError];
            if (removeError) {
                return false;
            }
        }

        NSError* error = nil;
        BOOL success = [fm copyItemAtPath:srcPath toPath:dstPath error:&error];

        if (!success && error) {
            NSLog(@"ofFile::copyTo failed: %@", error.localizedDescription);
            return false;
        }

        return success;
    }
}

bool ofFile::moveTo(const std::string& destination, bool overwrite) {
    @autoreleasepool {
        NSString* srcPath = [NSString stringWithUTF8String:impl_->path.c_str()];
        NSString* dstPath = [NSString stringWithUTF8String:destination.c_str()];
        NSFileManager* fm = [NSFileManager defaultManager];

        // Check if destination exists
        if ([fm fileExistsAtPath:dstPath]) {
            if (!overwrite) {
                return false;
            }
            // Remove existing file
            NSError* removeError = nil;
            [fm removeItemAtPath:dstPath error:&removeError];
            if (removeError) {
                return false;
            }
        }

        NSError* error = nil;
        BOOL success = [fm moveItemAtPath:srcPath toPath:dstPath error:&error];

        if (!success && error) {
            NSLog(@"ofFile::moveTo failed: %@", error.localizedDescription);
            return false;
        }

        // Update path after successful move
        if (success) {
            impl_->path = destination;
        }

        return success;
    }
}

bool ofFile::remove() {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:impl_->path.c_str()];
        NSError* error = nil;
        BOOL success = [[NSFileManager defaultManager] removeItemAtPath:nsPath error:&error];

        if (!success && error) {
            NSLog(@"ofFile::remove failed: %@", error.localizedDescription);
            return false;
        }

        return success;
    }
}

ofBuffer ofFile::readToBuffer() const {
    return ofBufferFromFile(impl_->path);
}

bool ofFile::writeFromBuffer(const ofBuffer& buffer) {
    return buffer.writeTo(impl_->path);
}

// Global helper functions

bool ofFileExists(const std::string& path) {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        return [[NSFileManager defaultManager] fileExistsAtPath:nsPath];
    }
}

bool ofFileRemove(const std::string& path) {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        NSError* error = nil;
        BOOL success = [[NSFileManager defaultManager] removeItemAtPath:nsPath error:&error];
        return success;
    }
}

} // namespace oflike
