#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <type_traits>
#include <algorithm>
#include <cctype>
#include <vector>

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

// MARK: - Hexadecimal Conversion Functions

/// Convert integer to hexadecimal string (lowercase)
inline std::string ofToHex(int value) {
    std::ostringstream oss;
    oss << std::hex << std::noshowbase << value;
    return oss.str();
}

/// Convert string to hexadecimal string (byte-by-byte)
inline std::string ofToHex(const std::string& value) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');

    for (unsigned char c : value) {
        // Each byte becomes 2 hex characters (lowercase)
        oss << std::setw(2) << static_cast<int>(c);
    }

    return oss.str();
}

/// Convert hexadecimal string to integer
inline int ofHexToInt(const std::string& hexString) {
    if (hexString.empty()) {
        return 0;
    }

    try {
        // std::stoi with base 16 handles "0x" prefix automatically
        // It also handles both uppercase and lowercase hex digits
        return std::stoi(hexString, nullptr, 16);
    } catch (const std::invalid_argument&) {
        // Return 0 for invalid input (oF behavior)
        return 0;
    } catch (const std::out_of_range&) {
        // Return 0 for out-of-range values (oF behavior)
        return 0;
    }
}

// MARK: - String Splitting and Joining Functions

/// Helper function to trim whitespace from both ends of a string
inline std::string trimString(const std::string& str) {
    if (str.empty()) {
        return str;
    }

    // Find first non-whitespace character
    size_t start = 0;
    while (start < str.length() && std::isspace(static_cast<unsigned char>(str[start]))) {
        ++start;
    }

    // Find last non-whitespace character
    size_t end = str.length();
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
        --end;
    }

    return str.substr(start, end - start);
}

/// Split string by delimiter into vector of strings
inline std::vector<std::string> ofSplitString(const std::string& source,
                                                const std::string& delimiter,
                                                bool ignoreEmpty,
                                                bool trim) {
    std::vector<std::string> result;

    // Handle empty delimiter case (split every character)
    if (delimiter.empty()) {
        for (char c : source) {
            result.push_back(std::string(1, c));
        }
        return result;
    }

    size_t start = 0;
    size_t end = source.find(delimiter);

    while (end != std::string::npos) {
        std::string token = source.substr(start, end - start);

        // Apply trim if requested
        if (trim) {
            token = trimString(token);
        }

        // Add to result unless it's empty and we're ignoring empty strings
        if (!ignoreEmpty || !token.empty()) {
            result.push_back(token);
        }

        start = end + delimiter.length();
        end = source.find(delimiter, start);
    }

    // Add the last token
    std::string token = source.substr(start);

    // Apply trim if requested
    if (trim) {
        token = trimString(token);
    }

    // Add to result unless it's empty and we're ignoring empty strings
    if (!ignoreEmpty || !token.empty()) {
        result.push_back(token);
    }

    return result;
}

/// Join vector of strings with delimiter
inline std::string ofJoinString(const std::vector<std::string>& stringElements,
                                 const std::string& delimiter) {
    if (stringElements.empty()) {
        return "";
    }

    std::ostringstream oss;

    // Add first element
    oss << stringElements[0];

    // Add remaining elements with delimiter
    for (size_t i = 1; i < stringElements.size(); ++i) {
        oss << delimiter << stringElements[i];
    }

    return oss.str();
}

// MARK: - Case Conversion Functions

/// Convert string to lowercase
inline std::string ofToLower(const std::string& src) {
    std::string result = src;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

/// Convert string to uppercase
inline std::string ofToUpper(const std::string& src) {
    std::string result = src;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

/// Trim whitespace from both ends of a string
inline std::string ofTrim(const std::string& src) {
    return trimString(src);
}
