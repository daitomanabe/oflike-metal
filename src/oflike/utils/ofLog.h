#pragma once

// oflike-metal ofLog - openFrameworks API compatible logging
// Phase 15.2: Integrated with os_log for native macOS logging
//
// Usage:
//   ofLog() << "Message";                    // Notice level
//   ofLogVerbose() << "Debug info";          // Verbose/Debug
//   ofLogNotice() << "Info message";         // Notice/Info
//   ofLogWarning() << "Warning!";            // Warning
//   ofLogError() << "Error occurred";        // Error
//   ofLogFatalError() << "Fatal!";           // Fatal (calls abort())
//
// With module name:
//   ofLogError("Renderer") << "Failed to create texture";
//
// Log levels can be controlled:
//   ofSetLogLevel(OF_LOG_WARNING);  // Only show warnings and above
//
// Logs are sent to os_log and can be viewed in Console.app
// Log subsystem: com.oflike.metal

#include <string>
#include <sstream>

namespace oflike {

// Forward declaration for implementation
class ofLogStreamImpl;

// Log levels matching openFrameworks
enum ofLogLevel {
    OF_LOG_VERBOSE,      // Debug level in os_log (most verbose)
    OF_LOG_NOTICE,       // Info level in os_log
    OF_LOG_WARNING,      // Default level in os_log
    OF_LOG_ERROR,        // Error level in os_log
    OF_LOG_FATAL_ERROR,  // Fault level in os_log + abort()
    OF_LOG_SILENT        // No output
};

// Stream-based log class compatible with openFrameworks
class ofLogStream {
public:
    ofLogStream(ofLogLevel level, const char* module = "");
    ~ofLogStream();

    // Support for common types
    ofLogStream& operator<<(const std::string& value);
    ofLogStream& operator<<(const char* value);
    ofLogStream& operator<<(int value);
    ofLogStream& operator<<(unsigned int value);
    ofLogStream& operator<<(long value);
    ofLogStream& operator<<(unsigned long value);
    ofLogStream& operator<<(long long value);
    ofLogStream& operator<<(unsigned long long value);
    ofLogStream& operator<<(float value);
    ofLogStream& operator<<(double value);
    ofLogStream& operator<<(bool value);

    // Template for other types
    template<typename T>
    ofLogStream& operator<<(const T& value) {
        getStream() << value;
        return *this;
    }

private:
    ofLogStreamImpl* impl_;
    std::ostringstream& getStream();
};

// Logging functions (openFrameworks API compatible)

/// Log with verbose/debug level (os_log_debug)
/// @param module Optional module name for categorization
/// @return ofLogStream object for chaining with operator<<
inline ofLogStream ofLogVerbose(const std::string& module = "") {
    return ofLogStream(OF_LOG_VERBOSE, module.c_str());
}

/// Log with notice/info level (os_log_info)
/// @param module Optional module name for categorization
/// @return ofLogStream object for chaining with operator<<
inline ofLogStream ofLogNotice(const std::string& module = "") {
    return ofLogStream(OF_LOG_NOTICE, module.c_str());
}

/// Log with warning level (os_log default)
/// @param module Optional module name for categorization
/// @return ofLogStream object for chaining with operator<<
inline ofLogStream ofLogWarning(const std::string& module = "") {
    return ofLogStream(OF_LOG_WARNING, module.c_str());
}

/// Log with error level (os_log_error)
/// @param module Optional module name for categorization
/// @return ofLogStream object for chaining with operator<<
inline ofLogStream ofLogError(const std::string& module = "") {
    return ofLogStream(OF_LOG_ERROR, module.c_str());
}

/// Log with fatal error level (os_log_fault + abort)
/// This function will terminate the program after logging
/// @param module Optional module name for categorization
/// @return ofLogStream object for chaining with operator<<
inline ofLogStream ofLogFatalError(const std::string& module = "") {
    return ofLogStream(OF_LOG_FATAL_ERROR, module.c_str());
}

/// Log with custom level
/// @param level Log level to use
/// @param module Optional module name for categorization
/// @return ofLogStream object for chaining with operator<<
inline ofLogStream ofLog(ofLogLevel level = OF_LOG_NOTICE, const std::string& module = "") {
    return ofLogStream(level, module.c_str());
}

// Global log level control

/// Set minimum log level to display
/// Messages below this level will be filtered out
/// @param level Minimum log level (e.g., OF_LOG_WARNING)
void ofSetLogLevel(ofLogLevel level);

/// Get current log level
/// @return Current minimum log level
ofLogLevel ofGetLogLevel();

} // namespace oflike
