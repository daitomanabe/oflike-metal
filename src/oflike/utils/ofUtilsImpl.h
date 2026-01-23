#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <type_traits>

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
