#include "ofLog.h"
#import <os/log.h>
#include <cstdlib>

namespace oflike {

// Global log level state
static ofLogLevel g_currentLogLevel = OF_LOG_VERBOSE;

// Get os_log for the framework
static os_log_t getOfLog() {
    static os_log_t log = os_log_create("com.oflike.metal", "general");
    return log;
}

// Internal implementation class for os_log integration
class ofLogStreamImpl {
public:
    ofLogStreamImpl(ofLogLevel level, const char* module)
        : level_(level), module_(module) {}

    ~ofLogStreamImpl() {
        // Check if this log level should be displayed
        if (level_ < g_currentLogLevel || level_ >= OF_LOG_SILENT) {
            return;
        }

        std::string message = stream_.str();
        if (message.empty()) return;

        @autoreleasepool {
            os_log_t log = getOfLog();
            const char* msg_cstr = message.c_str();

            // Add module prefix if specified
            std::string fullMessage;
            if (module_ && module_[0] != '\0') {
                fullMessage = "[" + std::string(module_) + "] " + message;
                msg_cstr = fullMessage.c_str();
            }

            // Map ofLogLevel to os_log_type_t
            switch (level_) {
                case OF_LOG_VERBOSE:
                    os_log_debug(log, "%{public}s", msg_cstr);
                    break;

                case OF_LOG_NOTICE:
                    os_log_info(log, "%{public}s", msg_cstr);
                    break;

                case OF_LOG_WARNING:
                    os_log(log, "%{public}s", msg_cstr);
                    break;

                case OF_LOG_ERROR:
                    os_log_error(log, "%{public}s", msg_cstr);
                    break;

                case OF_LOG_FATAL_ERROR:
                    os_log_fault(log, "%{public}s", msg_cstr);
                    // Fatal errors should terminate the program
                    std::abort();
                    break;

                default:
                    os_log_info(log, "%{public}s", msg_cstr);
                    break;
            }
        }
    }

    template<typename T>
    ofLogStreamImpl& operator<<(const T& value) {
        stream_ << value;
        return *this;
    }

    // String stream getter for ofLogStream wrapper
    std::ostringstream& getStream() { return stream_; }

private:
    ofLogLevel level_;
    const char* module_;
    std::ostringstream stream_;
};

// ofLogStream implementation - wraps ofLogStreamImpl
ofLogStream::ofLogStream(ofLogLevel level, const char* module)
    : impl_(new ofLogStreamImpl(level, module)) {}

ofLogStream::~ofLogStream() {
    delete impl_;
}

std::ostringstream& ofLogStream::getStream() {
    return impl_->getStream();
}

ofLogStream& ofLogStream::operator<<(const std::string& value) {
    impl_->getStream() << value;
    return *this;
}

ofLogStream& ofLogStream::operator<<(const char* value) {
    impl_->getStream() << value;
    return *this;
}

ofLogStream& ofLogStream::operator<<(int value) {
    impl_->getStream() << value;
    return *this;
}

ofLogStream& ofLogStream::operator<<(unsigned int value) {
    impl_->getStream() << value;
    return *this;
}

ofLogStream& ofLogStream::operator<<(long value) {
    impl_->getStream() << value;
    return *this;
}

ofLogStream& ofLogStream::operator<<(unsigned long value) {
    impl_->getStream() << value;
    return *this;
}

ofLogStream& ofLogStream::operator<<(long long value) {
    impl_->getStream() << value;
    return *this;
}

ofLogStream& ofLogStream::operator<<(unsigned long long value) {
    impl_->getStream() << value;
    return *this;
}

ofLogStream& ofLogStream::operator<<(float value) {
    impl_->getStream() << value;
    return *this;
}

ofLogStream& ofLogStream::operator<<(double value) {
    impl_->getStream() << value;
    return *this;
}

ofLogStream& ofLogStream::operator<<(bool value) {
    impl_->getStream() << (value ? "true" : "false");
    return *this;
}

// Global log level control
void ofSetLogLevel(ofLogLevel level) {
    g_currentLogLevel = level;
}

ofLogLevel ofGetLogLevel() {
    return g_currentLogLevel;
}

} // namespace oflike
