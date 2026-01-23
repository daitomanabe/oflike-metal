#include "ofBuffer.h"
#import <Foundation/Foundation.h>
#include <cstring>
#include <algorithm>
#include <sstream>

namespace oflike {

struct ofBuffer::Impl {
    std::vector<char> data;

    Impl() = default;

    Impl(const char* ptr, std::size_t size) {
        if (ptr && size > 0) {
            data.assign(ptr, ptr + size);
        }
    }

    Impl(const std::string& text) {
        data.assign(text.begin(), text.end());
    }
};

ofBuffer::ofBuffer() : impl_(std::make_unique<Impl>()) {}

ofBuffer::ofBuffer(const char* data, std::size_t size)
    : impl_(std::make_unique<Impl>(data, size)) {}

ofBuffer::ofBuffer(const std::string& text)
    : impl_(std::make_unique<Impl>(text)) {}

ofBuffer::~ofBuffer() = default;

ofBuffer::ofBuffer(ofBuffer&& other) noexcept = default;
ofBuffer& ofBuffer::operator=(ofBuffer&& other) noexcept = default;

ofBuffer::ofBuffer(const ofBuffer& other)
    : impl_(std::make_unique<Impl>(*other.impl_)) {}

ofBuffer& ofBuffer::operator=(const ofBuffer& other) {
    if (this != &other) {
        impl_ = std::make_unique<Impl>(*other.impl_);
    }
    return *this;
}

void ofBuffer::set(const char* data, std::size_t size) {
    impl_->data.clear();
    if (data && size > 0) {
        impl_->data.assign(data, data + size);
    }
}

void ofBuffer::set(const std::string& text) {
    impl_->data.clear();
    impl_->data.assign(text.begin(), text.end());
}

void ofBuffer::append(const char* data, std::size_t size) {
    if (data && size > 0) {
        impl_->data.insert(impl_->data.end(), data, data + size);
    }
}

void ofBuffer::append(const std::string& text) {
    impl_->data.insert(impl_->data.end(), text.begin(), text.end());
}

void ofBuffer::clear() {
    impl_->data.clear();
}

void ofBuffer::allocate(std::size_t size) {
    impl_->data.resize(size);
}

const char* ofBuffer::getData() const {
    return impl_->data.empty() ? nullptr : impl_->data.data();
}

char* ofBuffer::getData() {
    return impl_->data.empty() ? nullptr : impl_->data.data();
}

std::size_t ofBuffer::size() const {
    return impl_->data.size();
}

std::string ofBuffer::getText() const {
    if (impl_->data.empty()) return "";
    return std::string(impl_->data.begin(), impl_->data.end());
}

std::vector<std::string> ofBuffer::getLines() const {
    std::vector<std::string> lines;
    if (impl_->data.empty()) return lines;

    std::string text = getText();
    std::istringstream stream(text);
    std::string line;

    while (std::getline(stream, line)) {
        // Remove trailing \r if present (Windows line endings)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }

    return lines;
}

bool ofBuffer::writeTo(const std::string& path) const {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        NSData* nsData = [NSData dataWithBytes:getData() length:size()];

        NSError* error = nil;
        BOOL success = [nsData writeToFile:nsPath options:NSDataWritingAtomic error:&error];

        if (!success && error) {
            NSLog(@"ofBuffer::writeTo failed: %@", error.localizedDescription);
            return false;
        }

        return success;
    }
}

bool ofBuffer::isAllocated() const {
    return !impl_->data.empty();
}

ofBuffer ofBufferFromFile(const std::string& path) {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        NSError* error = nil;
        NSData* nsData = [NSData dataWithContentsOfFile:nsPath options:0 error:&error];

        if (!nsData || error) {
            NSLog(@"ofBufferFromFile failed: %@", error.localizedDescription);
            return ofBuffer();
        }

        const char* bytes = static_cast<const char*>([nsData bytes]);
        std::size_t length = [nsData length];

        return ofBuffer(bytes, length);
    }
}

} // namespace oflike
