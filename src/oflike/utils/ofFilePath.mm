#include "ofFilePath.h"
#import <Foundation/Foundation.h>
#include <vector>

namespace oflike {
namespace ofFilePath {

std::string getAbsolutePath(const std::string& path) {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        NSString* absPath = [nsPath stringByStandardizingPath];

        // If not absolute, resolve relative to current directory
        if (![absPath isAbsolutePath]) {
            NSString* currentDir = [[NSFileManager defaultManager] currentDirectoryPath];
            absPath = [[currentDir stringByAppendingPathComponent:nsPath] stringByStandardizingPath];
        }

        return std::string([absPath UTF8String]);
    }
}

std::string getFileName(const std::string& path) {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        NSString* fileName = [nsPath lastPathComponent];
        return std::string([fileName UTF8String]);
    }
}

std::string getBaseName(const std::string& path) {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        NSString* fileName = [nsPath lastPathComponent];
        NSString* baseName = [fileName stringByDeletingPathExtension];
        return std::string([baseName UTF8String]);
    }
}

std::string getFileExt(const std::string& path) {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        NSString* ext = [nsPath pathExtension];
        return std::string([ext UTF8String]);
    }
}

std::string getEnclosingDirectory(const std::string& path) {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        NSString* dir = [nsPath stringByDeletingLastPathComponent];
        return std::string([dir UTF8String]);
    }
}

std::string join(const std::string& path1, const std::string& path2) {
    @autoreleasepool {
        NSString* nsPath1 = [NSString stringWithUTF8String:path1.c_str()];
        NSString* nsPath2 = [NSString stringWithUTF8String:path2.c_str()];
        NSString* joined = [nsPath1 stringByAppendingPathComponent:nsPath2];
        return std::string([joined UTF8String]);
    }
}

std::string join(const std::string& path1, const std::string& path2, const std::string& path3) {
    return join(join(path1, path2), path3);
}

std::string join(const std::vector<std::string>& paths) {
    if (paths.empty()) return "";

    std::string result = paths[0];
    for (size_t i = 1; i < paths.size(); ++i) {
        result = join(result, paths[i]);
    }
    return result;
}

bool isAbsolute(const std::string& path) {
    @autoreleasepool {
        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        return [nsPath isAbsolutePath];
    }
}

std::string getCurrentWorkingDirectory() {
    @autoreleasepool {
        NSString* cwd = [[NSFileManager defaultManager] currentDirectoryPath];
        return std::string([cwd UTF8String]);
    }
}

std::string getUserHomeDir() {
    @autoreleasepool {
        NSString* homeDir = NSHomeDirectory();
        return std::string([homeDir UTF8String]);
    }
}

} // namespace ofFilePath
} // namespace oflike
