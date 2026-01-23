#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <type_traits>
#include <algorithm>
#include <cctype>

/// Template implementation for ofToString functions
/// This file provides the implementation of ofToString templates
/// Phase 15.1: String utility functions

// MARK: - Basic ofToString (single parameter)

/// Convert value to string using default formatting
template<typename T>
std::string ofToString(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

// MARK: - ofToString with precision control

/// Convert value to string with precision (for floating-point types)
template<typename T>
std::string ofToString(const T& value, int precision) {
    std::ostringstream oss;

    // Only apply precision for floating-point types
    if constexpr (std::is_floating_point_v<T>) {
        oss << std::fixed << std::setprecision(precision);
    }

    oss << value;
    return oss.str();
}

// MARK: - ofToString with width and fill

/// Convert value to string with width and fill character
template<typename T>
std::string ofToString(const T& value, int width, char fill) {
    std::ostringstream oss;
    oss << std::setw(width) << std::setfill(fill) << value;
    return oss.str();
}

// MARK: - Specialized overloads for common types

/// Specialized version for bool
template<>
inline std::string ofToString(const bool& value) {
    return value ? "true" : "false";
}

/// Specialized version for bool with precision (ignores precision)
template<>
inline std::string ofToString(const bool& value, int precision) {
    (void)precision; // Unused
    return value ? "true" : "false";
}

/// Specialized version for std::string (pass-through)
template<>
inline std::string ofToString(const std::string& value) {
    return value;
}

/// Specialized version for std::string with precision (ignores precision)
template<>
inline std::string ofToString(const std::string& value, int precision) {
    (void)precision; // Unused
    return value;
}

/// Specialized version for const char*
template<>
inline std::string ofToString(const char* const& value) {
    return std::string(value);
}

/// Specialized version for const char* with precision
template<>
inline std::string ofToString(const char* const& value, int precision) {
    (void)precision; // Unused
    return std::string(value);
}

// MARK: - String to Type Conversion Functions

/// Convert string to integer
inline int ofToInt(const std::string& str) {
    try {
        // std::stoi handles whitespace and throws on invalid input
        return std::stoi(str);
    } catch (const std::invalid_argument&) {
        // Return 0 for invalid input (oF behavior)
        return 0;
    } catch (const std::out_of_range&) {
        // Return 0 for out-of-range values (oF behavior)
        return 0;
    }
}

/// Convert string to float
inline float ofToFloat(const std::string& str) {
    try {
        // std::stof handles whitespace and throws on invalid input
        return std::stof(str);
    } catch (const std::invalid_argument&) {
        // Return 0.0f for invalid input (oF behavior)
        return 0.0f;
    } catch (const std::out_of_range&) {
        // Return 0.0f for out-of-range values (oF behavior)
        return 0.0f;
    }
}

/// Convert string to boolean
inline bool ofToBool(const std::string& str) {
    // Empty string is false
    if (str.empty()) {
        return false;
    }

    // Convert to lowercase for case-insensitive comparison
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    // Check for true values
    if (lower == "true" || lower == "1" || lower == "yes") {
        return true;
    }

    // All other values are false (including "false", "0", "no", etc.)
    return false;
}
