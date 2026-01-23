#pragma once

// oflike-metal ofFilePath - openFrameworks API compatible file path utilities
// Phase 15.3: File system utilities using Foundation Framework (NSFileManager)
//
// Usage:
//   std::string abs = ofFilePath::getAbsolutePath("data/image.png");
//   std::string name = ofFilePath::getFileName("/path/to/file.txt");
//   std::string base = ofFilePath::getBaseName("/path/to/file.txt");  // "file"
//   std::string ext = ofFilePath::getFileExt("/path/to/file.txt");    // "txt"
//   std::string dir = ofFilePath::getEnclosingDirectory("/path/to/file.txt");
//   std::string path = ofFilePath::join("data", "images", "photo.jpg");

#include <string>
#include <vector>

namespace oflike {

/// File path utility functions (openFrameworks compatible)
namespace ofFilePath {

/// Convert relative path to absolute path
/// @param path Relative or absolute path
/// @return Absolute path
std::string getAbsolutePath(const std::string& path);

/// Get file name from path (including extension)
/// @param path File path
/// @return File name (e.g., "file.txt")
std::string getFileName(const std::string& path);

/// Get base name from path (without extension)
/// @param path File path
/// @return Base name without extension (e.g., "file")
std::string getBaseName(const std::string& path);

/// Get file extension from path
/// @param path File path
/// @return File extension without dot (e.g., "txt")
std::string getFileExt(const std::string& path);

/// Get enclosing directory from path
/// @param path File path
/// @return Directory path
std::string getEnclosingDirectory(const std::string& path);

/// Join path components with platform-specific separator
/// @param paths Variable number of path components
/// @return Joined path
std::string join(const std::string& path1, const std::string& path2);
std::string join(const std::string& path1, const std::string& path2, const std::string& path3);

/// Join vector of path components
/// @param paths Vector of path components
/// @return Joined path
std::string join(const std::vector<std::string>& paths);

/// Check if path is absolute
/// @param path Path to check
/// @return True if path is absolute
bool isAbsolute(const std::string& path);

/// Get current working directory
/// @return Current working directory path
std::string getCurrentWorkingDirectory();

/// Get user home directory
/// @return User home directory path
std::string getUserHomeDir();

} // namespace ofFilePath

} // namespace oflike
