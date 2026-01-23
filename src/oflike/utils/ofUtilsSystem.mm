#include "ofUtils.h"
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#include <ctime>
#include <sstream>

namespace oflike {

std::string ofSystem(const std::string& command) {
    @autoreleasepool {
        // Create NSTask (NSTask is the macOS way to execute shell commands)
        NSTask* task = [[NSTask alloc] init];
        [task setLaunchPath:@"/bin/sh"];
        [task setArguments:@[@"-c", [NSString stringWithUTF8String:command.c_str()]]];

        // Create pipe to capture output
        NSPipe* pipe = [NSPipe pipe];
        [task setStandardOutput:pipe];
        [task setStandardError:pipe];

        // Launch task
        NSError* error = nil;
        [task launchAndReturnError:&error];

        if (error) {
            NSLog(@"ofSystem failed to launch: %@", error.localizedDescription);
            return "";
        }

        // Read output
        NSFileHandle* file = [pipe fileHandleForReading];
        NSData* data = [file readDataToEndOfFile];
        [task waitUntilExit];

        // Convert to string
        NSString* output = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        if (!output) {
            return "";
        }

        return std::string([output UTF8String]);
    }
}

bool ofLaunchBrowser(const std::string& url) {
    @autoreleasepool {
        NSString* urlString = [NSString stringWithUTF8String:url.c_str()];
        NSURL* nsUrl = [NSURL URLWithString:urlString];

        if (!nsUrl) {
            NSLog(@"ofLaunchBrowser: Invalid URL: %@", urlString);
            return false;
        }

        // Use NSWorkspace to open URL in default browser
        BOOL success = [[NSWorkspace sharedWorkspace] openURL:nsUrl];

        if (!success) {
            NSLog(@"ofLaunchBrowser: Failed to open URL: %@", urlString);
            return false;
        }

        return success;
    }
}

std::string ofGetTimestampString(const std::string& format) {
    @autoreleasepool {
        NSDate* now = [NSDate date];
        NSDateFormatter* formatter = [[NSDateFormatter alloc] init];

        if (format.empty()) {
            // Default format: YYYY-MM-DD-HH-MM-SS
            [formatter setDateFormat:@"yyyy-MM-dd-HH-mm-ss"];
        } else {
            // Convert strftime format to NSDateFormatter format
            // Common conversions:
            // %Y -> yyyy, %m -> MM, %d -> dd
            // %H -> HH, %M -> mm, %S -> ss
            std::string nsFormat = format;

            // Simple conversion for common patterns
            size_t pos = 0;
            while ((pos = nsFormat.find("%Y", pos)) != std::string::npos) {
                nsFormat.replace(pos, 2, "yyyy");
                pos += 4;
            }
            pos = 0;
            while ((pos = nsFormat.find("%m", pos)) != std::string::npos) {
                nsFormat.replace(pos, 2, "MM");
                pos += 2;
            }
            pos = 0;
            while ((pos = nsFormat.find("%d", pos)) != std::string::npos) {
                nsFormat.replace(pos, 2, "dd");
                pos += 2;
            }
            pos = 0;
            while ((pos = nsFormat.find("%H", pos)) != std::string::npos) {
                nsFormat.replace(pos, 2, "HH");
                pos += 2;
            }
            pos = 0;
            while ((pos = nsFormat.find("%M", pos)) != std::string::npos) {
                nsFormat.replace(pos, 2, "mm");
                pos += 2;
            }
            pos = 0;
            while ((pos = nsFormat.find("%S", pos)) != std::string::npos) {
                nsFormat.replace(pos, 2, "ss");
                pos += 2;
            }

            [formatter setDateFormat:[NSString stringWithUTF8String:nsFormat.c_str()]];
        }

        NSString* timestamp = [formatter stringFromDate:now];
        return std::string([timestamp UTF8String]);
    }
}

} // namespace oflike
