#pragma once

// oflike-metal ofDirectory - openFrameworks API compatible directory class
// Phase 15.3: Directory operations using Foundation Framework (NSFileManager)
//
// Usage:
//   ofDirectory dir("data");
//   if (dir.exists()) {
//       dir.listDir();
//       for (size_t i = 0; i < dir.size(); i++) {
//           std::string path = dir.getPath(i);
//       }
//   }
//
//   dir.create();  // Create directory

#include <string>
#include <vector>
#include <memory>
#include <cstddef>

namespace oflike {

class ofFile;

/// Directory operations class (openFrameworks compatible)
class ofDirectory {
public:
    ofDirectory();
    explicit ofDirectory(const std::string& path);
    ~ofDirectory();

    // Move semantics
    ofDirectory(ofDirectory&& other) noexcept;
    ofDirectory& operator=(ofDirectory&& other) noexcept;

    // No copy
    ofDirectory(const ofDirectory&) = delete;
    ofDirectory& operator=(const ofDirectory&) = delete;

    /// Open directory at path
    /// @param path Directory path
    void open(const std::string& path);

    /// Close directory
    void close();

    /// Check if directory exists
    /// @return True if directory exists
    bool exists() const;

    /// Get directory path
    /// @return Directory path
    std::string getPath() const;

    /// Get absolute path
    /// @return Absolute directory path
    std::string getAbsolutePath() const;

    /// Create directory (including parent directories)
    /// @param recursive Create parent directories if needed
    /// @return True if successful
    bool create(bool recursive = true);

    /// Remove/delete directory
    /// @param recursive Remove contents recursively
    /// @return True if successful
    bool remove(bool recursive = false);

    /// List directory contents
    /// @return True if successful
    bool listDir();

    /// Get number of entries
    /// @return Number of files/directories
    std::size_t size() const;

    /// Get path of entry at index
    /// @param index Entry index
    /// @return File/directory path
    std::string getPath(std::size_t index) const;

    /// Get name of entry at index
    /// @param index Entry index
    /// @return File/directory name
    std::string getName(std::size_t index) const;

    /// Get all paths
    /// @return Vector of all paths
    std::vector<std::string> getPaths() const;

    /// Get all names
    /// @return Vector of all names
    std::vector<std::string> getNames() const;

    /// Check if entry at index is directory
    /// @param index Entry index
    /// @return True if directory
    bool isDirectory(std::size_t index) const;

    /// Check if entry at index is file
    /// @param index Entry index
    /// @return True if file
    bool isFile(std::size_t index) const;

    /// Get ofFile for entry at index
    /// @param index Entry index
    /// @return ofFile object
    ofFile getFile(std::size_t index) const;

    /// Sort entries alphabetically
    void sort();

    /// Allow filtering by extension
    /// @param extension File extension (without dot, e.g., "png")
    void allowExt(const std::string& extension);

    /// Clear extension filters
    void clearExtFilters();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

/// Check if directory exists
/// @param path Directory path
/// @return True if directory exists
bool ofDirectoryExists(const std::string& path);

/// Create directory
/// @param path Directory path
/// @param recursive Create parent directories
/// @return True if successful
bool ofDirectoryCreate(const std::string& path, bool recursive = true);

} // namespace oflike
