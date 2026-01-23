#pragma once

// oflike-metal ofLog - openFrameworks API compatible logging
// Minimal implementation for Phase 15.2

#include <iostream>
#include <string>
#include <sstream>

namespace oflike {

// Log levels
enum ofLogLevel {
    OF_LOG_VERBOSE,
    OF_LOG_NOTICE,
    OF_LOG_WARNING,
    OF_LOG_ERROR,
    OF_LOG_FATAL_ERROR,
    OF_LOG_SILENT
};

// Internal log stream class
class ofLogStream {
public:
    ofLogStream(ofLogLevel level, const char* module = "")
        : level_(level), module_(module) {}

    ~ofLogStream() {
        if (level_ >= OF_LOG_SILENT) return;

        std::string prefix;
        switch (level_) {
            case OF_LOG_VERBOSE:   prefix = "[verbose] "; break;
            case OF_LOG_NOTICE:    prefix = "[notice] "; break;
            case OF_LOG_WARNING:   prefix = "[warning] "; break;
            case OF_LOG_ERROR:     prefix = "[error] "; break;
            case OF_LOG_FATAL_ERROR: prefix = "[fatal] "; break;
            default: break;
        }

        if (module_ && module_[0] != '\0') {
            std::cerr << prefix << "[" << module_ << "] " << stream_.str() << std::endl;
        } else {
            std::cerr << prefix << stream_.str() << std::endl;
        }
    }

    template<typename T>
    ofLogStream& operator<<(const T& value) {
        stream_ << value;
        return *this;
    }

private:
    ofLogLevel level_;
    const char* module_;
    std::ostringstream stream_;
};

// Logging functions
inline ofLogStream ofLogVerbose(const std::string& module = "") {
    return ofLogStream(OF_LOG_VERBOSE, module.c_str());
}

inline ofLogStream ofLogNotice(const std::string& module = "") {
    return ofLogStream(OF_LOG_NOTICE, module.c_str());
}

inline ofLogStream ofLogWarning(const std::string& module = "") {
    return ofLogStream(OF_LOG_WARNING, module.c_str());
}

inline ofLogStream ofLogError(const std::string& module = "") {
    return ofLogStream(OF_LOG_ERROR, module.c_str());
}

inline ofLogStream ofLogFatalError(const std::string& module = "") {
    return ofLogStream(OF_LOG_FATAL_ERROR, module.c_str());
}

inline ofLogStream ofLog(ofLogLevel level = OF_LOG_NOTICE, const std::string& module = "") {
    return ofLogStream(level, module.c_str());
}

// Set global log level (TODO: implement in Phase 15.2)
inline void ofSetLogLevel(ofLogLevel level) {
    // Placeholder
}

} // namespace oflike
