#pragma once

// oflike-metal ofFile - openFrameworks API compatible file class
// Phase 15.3: File operations using Foundation Framework (NSFileManager)
//
// Usage:
//   ofFile file("data/image.png");
//   if (file.exists()) {
//       uint64_t size = file.getSize();
//       std::string name = file.getFileName();
//   }
//
//   file.copyTo("backup/image.png");

#include <string>
#include <memory>
#include <cstdint>

namespace oflike {

class ofBuffer;

/// File operations class (openFrameworks compatible)
class ofFile {
public:
    ofFile();
    explicit ofFile(const std::string& path);
    ~ofFile();

    // Move semantics
    ofFile(ofFile&& other) noexcept;
    ofFile& operator=(ofFile&& other) noexcept;

    // No copy (file handles shouldn't be copied)
    ofFile(const ofFile&) = delete;
    ofFile& operator=(const ofFile&) = delete;

    /// Open file at path
    /// @param path File path
    /// @return True if file exists
    bool open(const std::string& path);

    /// Close file
    void close();

    /// Check if file exists
    /// @return True if file exists
    bool exists() const;

    /// Get file path
    /// @return File path
    std::string getFilePath() const;

    /// Get file name (including extension)
    /// @return File name
    std::string getFileName() const;

    /// Get base name (without extension)
    /// @return Base name
    std::string getBaseName() const;

    /// Get file extension
    /// @return File extension
    std::string getExtension() const;

    /// Get absolute path
    /// @return Absolute file path
    std::string getAbsolutePath() const;

    /// Get enclosing directory
    /// @return Directory path
    std::string getEnclosingDirectory() const;

    /// Get file size in bytes
    /// @return File size (0 if file doesn't exist)
    uint64_t getSize() const;

    /// Check if path is a directory
    /// @return True if directory
    bool isDirectory() const;

    /// Check if path is a file (not directory)
    /// @return True if file
    bool isFile() const;

    /// Create file (empty)
    /// @return True if successful
    bool create();

    /// Copy file to destination
    /// @param destination Destination path
    /// @param overwrite Overwrite existing file
    /// @return True if successful
    bool copyTo(const std::string& destination, bool overwrite = false);

    /// Move file to destination
    /// @param destination Destination path
    /// @param overwrite Overwrite existing file
    /// @return True if successful
    bool moveTo(const std::string& destination, bool overwrite = false);

    /// Remove/delete file
    /// @return True if successful
    bool remove();

    /// Read entire file as buffer
    /// @return Buffer containing file data
    ofBuffer readToBuffer() const;

    /// Write buffer to file
    /// @param buffer Buffer to write
    /// @return True if successful
    bool writeFromBuffer(const ofBuffer& buffer);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

/// Check if file exists
/// @param path File path
/// @return True if file exists
bool ofFileExists(const std::string& path);

/// Remove/delete file
/// @param path File path
/// @return True if successful
bool ofFileRemove(const std::string& path);

} // namespace oflike
