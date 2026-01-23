#pragma once

// oflike-metal ofBuffer - openFrameworks API compatible buffer class
// Phase 15.3: Binary data buffer with file I/O support
//
// Usage:
//   ofBuffer buffer;
//   buffer.set("Hello, World!", 13);
//   buffer.writeTo("output.txt");
//
//   ofBuffer loaded = ofBufferFromFile("input.txt");
//   std::string text = loaded.getText();

#include <string>
#include <vector>
#include <cstddef>
#include <memory>

namespace oflike {

/// Binary buffer class for file I/O (openFrameworks compatible)
class ofBuffer {
public:
    ofBuffer();
    ofBuffer(const char* data, std::size_t size);
    ofBuffer(const std::string& text);
    ~ofBuffer();

    // Move semantics
    ofBuffer(ofBuffer&& other) noexcept;
    ofBuffer& operator=(ofBuffer&& other) noexcept;

    // Copy semantics
    ofBuffer(const ofBuffer& other);
    ofBuffer& operator=(const ofBuffer& other);

    /// Set buffer data
    /// @param data Pointer to data
    /// @param size Size in bytes
    void set(const char* data, std::size_t size);

    /// Set buffer from string
    /// @param text Text string
    void set(const std::string& text);

    /// Append data to buffer
    /// @param data Pointer to data
    /// @param size Size in bytes
    void append(const char* data, std::size_t size);

    /// Append string to buffer
    /// @param text Text string
    void append(const std::string& text);

    /// Clear buffer
    void clear();

    /// Allocate buffer with specified size
    /// @param size Size in bytes
    void allocate(std::size_t size);

    /// Get buffer data pointer (read-only)
    /// @return Pointer to data
    const char* getData() const;

    /// Get buffer data pointer (mutable)
    /// @return Pointer to data
    char* getData();

    /// Get buffer size
    /// @return Size in bytes
    std::size_t size() const;

    /// Get buffer as text string
    /// @return Text string (assumes UTF-8)
    std::string getText() const;

    /// Get buffer as lines
    /// @return Vector of lines
    std::vector<std::string> getLines() const;

    /// Write buffer to file
    /// @param path File path
    /// @return True if successful
    bool writeTo(const std::string& path) const;

    /// Check if buffer is allocated
    /// @return True if buffer has data
    bool isAllocated() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

/// Load buffer from file
/// @param path File path
/// @return Buffer containing file data
ofBuffer ofBufferFromFile(const std::string& path);

} // namespace oflike
