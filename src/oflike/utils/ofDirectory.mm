#include "ofDirectory.h"
#include "ofFile.h"
#include "ofFilePath.h"
#import <Foundation/Foundation.h>
#include <algorithm>

namespace oflike {

struct ofDirectory::Impl {
    std::string path;
    std::vector<std::string> entries;
    std::vector<std::string> allowedExtensions;

    Impl() = default;
    explicit Impl(const std::string& p) : path(p) {}
};

ofDirectory::ofDirectory() : impl_(std::make_unique<Impl>()) {}

ofDirectory::ofDirectory(const std::string& path)
    : impl_(std::make_unique<Impl>(path)) {}

ofDirectory::~ofDirectory() = default;

ofDirectory::ofDirectory(ofDirectory&& other) noexcept = default;
ofDirectory& ofDirectory::operator=(ofDirectory&& other) noexcept = default;

void ofDirectory::open(const std::string& path) {
    impl_->path = path;
    impl_->entries.clear();
}

void ofDirectory::close() {
    impl_->path.clear();
    impl_->entries.clear();
}

bool ofDirectory::exists() const {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:impl_->path.c_str()];
        BOOL isDir = NO;
        BOOL exists = [[NSFileManager defaultManager] fileExistsAtPath:nsPath isDirectory:&isDir];
        return exists && isDir;
    }
}

std::string ofDirectory::getPath() const {
    return impl_->path;
}

std::string ofDirectory::getAbsolutePath() const {
    return ofFilePath::getAbsolutePath(impl_->path);
}

bool ofDirectory::create(bool recursive) {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:impl_->path.c_str()];
        NSError* error = nil;

        BOOL success = [[NSFileManager defaultManager]
            createDirectoryAtPath:nsPath
            withIntermediateDirectories:recursive
            attributes:nil
            error:&error];

        if (!success && error) {
            NSLog(@"ofDirectory::create failed: %@", error.localizedDescription);
            return false;
        }

        return success;
    }
}

bool ofDirectory::remove(bool recursive) {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:impl_->path.c_str()];
        NSError* error = nil;

        // If not recursive, check if directory is empty
        if (!recursive) {
            NSArray* contents = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:nsPath error:nil];
            if (contents && [contents count] > 0) {
                NSLog(@"ofDirectory::remove failed: Directory not empty (use recursive=true)");
                return false;
            }
        }

        BOOL success = [[NSFileManager defaultManager] removeItemAtPath:nsPath error:&error];

        if (!success && error) {
            NSLog(@"ofDirectory::remove failed: %@", error.localizedDescription);
            return false;
        }

        return success;
    }
}

bool ofDirectory::listDir() {
    impl_->entries.clear();

    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:impl_->path.c_str()];
        NSError* error = nil;

        NSArray* contents = [[NSFileManager defaultManager]
            contentsOfDirectoryAtPath:nsPath
            error:&error];

        if (error || !contents) {
            NSLog(@"ofDirectory::listDir failed: %@", error.localizedDescription);
            return false;
        }

        for (NSString* item in contents) {
            std::string itemName = std::string([item UTF8String]);
            std::string fullPath = ofFilePath::join(impl_->path, itemName);

            // Apply extension filter if set
            if (!impl_->allowedExtensions.empty()) {
                std::string ext = ofFilePath::getFileExt(fullPath);
                bool allowed = false;

                for (const auto& allowedExt : impl_->allowedExtensions) {
                    if (ext == allowedExt) {
                        allowed = true;
                        break;
                    }
                }

                if (!allowed) continue;
            }

            impl_->entries.push_back(fullPath);
        }

        return true;
    }
}

std::size_t ofDirectory::size() const {
    return impl_->entries.size();
}

std::string ofDirectory::getPath(std::size_t index) const {
    if (index >= impl_->entries.size()) return "";
    return impl_->entries[index];
}

std::string ofDirectory::getName(std::size_t index) const {
    if (index >= impl_->entries.size()) return "";
    return ofFilePath::getFileName(impl_->entries[index]);
}

std::vector<std::string> ofDirectory::getPaths() const {
    return impl_->entries;
}

std::vector<std::string> ofDirectory::getNames() const {
    std::vector<std::string> names;
    names.reserve(impl_->entries.size());

    for (const auto& path : impl_->entries) {
        names.push_back(ofFilePath::getFileName(path));
    }

    return names;
}

bool ofDirectory::isDirectory(std::size_t index) const {
    if (index >= impl_->entries.size()) return false;

    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:impl_->entries[index].c_str()];
        BOOL isDir = NO;
        BOOL exists = [[NSFileManager defaultManager] fileExistsAtPath:nsPath isDirectory:&isDir];
        return exists && isDir;
    }
}

bool ofDirectory::isFile(std::size_t index) const {
    if (index >= impl_->entries.size()) return false;

    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:impl_->entries[index].c_str()];
        BOOL isDir = NO;
        BOOL exists = [[NSFileManager defaultManager] fileExistsAtPath:nsPath isDirectory:&isDir];
        return exists && !isDir;
    }
}

ofFile ofDirectory::getFile(std::size_t index) const {
    if (index >= impl_->entries.size()) return ofFile();
    return ofFile(impl_->entries[index]);
}

void ofDirectory::sort() {
    std::sort(impl_->entries.begin(), impl_->entries.end());
}

void ofDirectory::allowExt(const std::string& extension) {
    impl_->allowedExtensions.push_back(extension);
}

void ofDirectory::clearExtFilters() {
    impl_->allowedExtensions.clear();
}

// Global helper functions

bool ofDirectoryExists(const std::string& path) {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        BOOL isDir = NO;
        BOOL exists = [[NSFileManager defaultManager] fileExistsAtPath:nsPath isDirectory:&isDir];
        return exists && isDir;
    }
}

bool ofDirectoryCreate(const std::string& path, bool recursive) {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        NSError* error = nil;

        BOOL success = [[NSFileManager defaultManager]
            createDirectoryAtPath:nsPath
            withIntermediateDirectories:recursive
            attributes:nil
            error:&error];

        return success;
    }
}

} // namespace oflike
