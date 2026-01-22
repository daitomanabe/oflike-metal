#pragma once

#include <string>
#include <sstream>
#include <iomanip>

// String conversion utilities
template<typename T>
inline std::string ofToString(const T& value) {
  std::ostringstream oss;
  oss << value;
  return oss.str();
}

template<typename T>
inline std::string ofToString(const T& value, int precision) {
  std::ostringstream oss;
  oss.precision(precision);
  oss << std::fixed << value;
  return oss.str();
}

template<typename T>
inline std::string ofToString(const T& value, int width, char fill) {
  std::ostringstream oss;
  oss << std::setw(width) << std::setfill(fill) << value;
  return oss.str();
}

// Parse string to number
inline int ofToInt(const std::string& str) {
  return std::stoi(str);
}

inline float ofToFloat(const std::string& str) {
  return std::stof(str);
}

inline double ofToDouble(const std::string& str) {
  return std::stod(str);
}
