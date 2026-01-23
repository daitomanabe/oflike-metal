#pragma once

#include <cmath>
#include <random>
#include <algorithm>

/// openFrameworks-compatible math utility functions
/// Provides random numbers, noise, mapping, clamping, interpolation, distance, and angle conversion
///
/// Phase 3.7: Math utility functions
/// - ofRandom() / ofRandomf() / ofRandomuf() - random number generation
/// - ofSeedRandom() - seed random generator
/// - ofNoise() / ofSignedNoise() - Perlin noise (simplified)
/// - ofMap() - map value from one range to another
/// - ofClamp() - constrain value to range
/// - ofLerp() - linear interpolation
/// - ofDist() / ofDistSquared() - distance calculations
/// - ofDegToRad() / ofRadToDeg() - angle conversions
/// - ofWrap() / ofWrapDegrees() / ofWrapRadians() - value wrapping

// MARK: - Constants

constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = PI * 2.0f;
constexpr float HALF_PI = PI * 0.5f;
constexpr float DEG_TO_RAD = PI / 180.0f;
constexpr float RAD_TO_DEG = 180.0f / PI;

// MARK: - Random Number Generation

/// Generate random float in range [0, max)
/// @param max Upper bound (exclusive)
/// @return Random float in [0, max)
inline float ofRandom(float max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, max);
    return dis(gen);
}

/// Generate random float in range [min, max)
/// @param min Lower bound (inclusive)
/// @param max Upper bound (exclusive)
/// @return Random float in [min, max)
inline float ofRandom(float min, float max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

/// Generate random float in range [0.0, 1.0)
/// @return Random float in [0.0, 1.0)
inline float ofRandomf() {
    return ofRandom(0.0f, 1.0f);
}

/// Generate random float in range [0.0, 1.0)
/// Alias for ofRandomf()
/// @return Random float in [0.0, 1.0)
inline float ofRandomuf() {
    return ofRandomf();
}

/// Seed random number generator
/// @param seed Seed value
inline void ofSeedRandom(int seed) {
    static std::mt19937 gen;
    gen.seed(seed);
}

// MARK: - Noise Functions

/// Simple Perlin noise implementation
/// Returns value in range [0, 1]
/// @param x Input coordinate
/// @return Noise value in [0, 1]
inline float ofNoise(float x) {
    // Simplified 1D Perlin noise approximation using sin
    // This is a basic implementation; for production use, consider a proper Perlin noise library
    float i = std::floor(x);
    float f = x - i;
    float u = f * f * (3.0f - 2.0f * f);

    auto hash = [](float n) -> float {
        return std::fmod(std::sin(n) * 43758.5453f, 1.0f);
    };

    float v1 = hash(i);
    float v2 = hash(i + 1.0f);
    return (v1 + (v2 - v1) * u) * 0.5f + 0.5f;
}

/// Simple 2D Perlin noise
/// Returns value in range [0, 1]
/// @param x Input x coordinate
/// @param y Input y coordinate
/// @return Noise value in [0, 1]
inline float ofNoise(float x, float y) {
    auto hash2 = [](float x, float y) -> float {
        return std::fmod(std::sin(x * 12.9898f + y * 78.233f) * 43758.5453f, 1.0f);
    };

    float ix = std::floor(x);
    float iy = std::floor(y);
    float fx = x - ix;
    float fy = y - iy;

    float ux = fx * fx * (3.0f - 2.0f * fx);
    float uy = fy * fy * (3.0f - 2.0f * fy);

    float a = hash2(ix, iy);
    float b = hash2(ix + 1.0f, iy);
    float c = hash2(ix, iy + 1.0f);
    float d = hash2(ix + 1.0f, iy + 1.0f);

    float v1 = a + (b - a) * ux;
    float v2 = c + (d - c) * ux;
    return (v1 + (v2 - v1) * uy) * 0.5f + 0.5f;
}

/// Simple 3D Perlin noise
/// Returns value in range [0, 1]
/// @param x Input x coordinate
/// @param y Input y coordinate
/// @param z Input z coordinate
/// @return Noise value in [0, 1]
inline float ofNoise(float x, float y, float z) {
    auto hash3 = [](float x, float y, float z) -> float {
        return std::fmod(std::sin(x * 12.9898f + y * 78.233f + z * 37.719f) * 43758.5453f, 1.0f);
    };

    float ix = std::floor(x);
    float iy = std::floor(y);
    float iz = std::floor(z);
    float fx = x - ix;
    float fy = y - iy;
    float fz = z - iz;

    float ux = fx * fx * (3.0f - 2.0f * fx);
    float uy = fy * fy * (3.0f - 2.0f * fy);
    float uz = fz * fz * (3.0f - 2.0f * fz);

    float a = hash3(ix, iy, iz);
    float b = hash3(ix + 1.0f, iy, iz);
    float c = hash3(ix, iy + 1.0f, iz);
    float d = hash3(ix + 1.0f, iy + 1.0f, iz);
    float e = hash3(ix, iy, iz + 1.0f);
    float f = hash3(ix + 1.0f, iy, iz + 1.0f);
    float g = hash3(ix, iy + 1.0f, iz + 1.0f);
    float h = hash3(ix + 1.0f, iy + 1.0f, iz + 1.0f);

    float k0 = a + (b - a) * ux;
    float k1 = c + (d - c) * ux;
    float k2 = e + (f - e) * ux;
    float k3 = g + (h - g) * ux;

    float k4 = k0 + (k1 - k0) * uy;
    float k5 = k2 + (k3 - k2) * uy;

    return (k4 + (k5 - k4) * uz) * 0.5f + 0.5f;
}

/// Signed noise - returns value in range [-1, 1]
/// @param x Input coordinate
/// @return Noise value in [-1, 1]
inline float ofSignedNoise(float x) {
    return ofNoise(x) * 2.0f - 1.0f;
}

/// Signed 2D noise - returns value in range [-1, 1]
/// @param x Input x coordinate
/// @param y Input y coordinate
/// @return Noise value in [-1, 1]
inline float ofSignedNoise(float x, float y) {
    return ofNoise(x, y) * 2.0f - 1.0f;
}

/// Signed 3D noise - returns value in range [-1, 1]
/// @param x Input x coordinate
/// @param y Input y coordinate
/// @param z Input z coordinate
/// @return Noise value in [-1, 1]
inline float ofSignedNoise(float x, float y, float z) {
    return ofNoise(x, y, z) * 2.0f - 1.0f;
}

// MARK: - Mapping and Clamping

/// Map value from one range to another
/// @param value Input value
/// @param inputMin Input range minimum
/// @param inputMax Input range maximum
/// @param outputMin Output range minimum
/// @param outputMax Output range maximum
/// @param clamp If true, clamp output to [outputMin, outputMax]
/// @return Mapped value
inline float ofMap(float value, float inputMin, float inputMax, float outputMin, float outputMax, bool clamp = false) {
    float outVal = ((value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin);

    if (clamp) {
        if (outputMax < outputMin) {
            if (outVal < outputMax) outVal = outputMax;
            else if (outVal > outputMin) outVal = outputMin;
        } else {
            if (outVal > outputMax) outVal = outputMax;
            else if (outVal < outputMin) outVal = outputMin;
        }
    }

    return outVal;
}

/// Clamp value to range [min, max]
/// @param value Input value
/// @param min Minimum value
/// @param max Maximum value
/// @return Clamped value
inline float ofClamp(float value, float min, float max) {
    return std::max(min, std::min(max, value));
}

/// Clamp integer value to range [min, max]
/// @param value Input value
/// @param min Minimum value
/// @param max Maximum value
/// @return Clamped value
inline int ofClamp(int value, int min, int max) {
    return std::max(min, std::min(max, value));
}

// MARK: - Interpolation

/// Linear interpolation between two values
/// @param start Start value (at t=0)
/// @param stop End value (at t=1)
/// @param amt Interpolation amount (0.0 to 1.0)
/// @return Interpolated value
inline float ofLerp(float start, float stop, float amt) {
    return start + (stop - start) * amt;
}

// MARK: - Distance

/// Calculate distance between two 2D points
/// @param x1 First point x coordinate
/// @param y1 First point y coordinate
/// @param x2 Second point x coordinate
/// @param y2 Second point y coordinate
/// @return Distance between points
inline float ofDist(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return std::sqrt(dx * dx + dy * dy);
}

/// Calculate distance between two 3D points
/// @param x1 First point x coordinate
/// @param y1 First point y coordinate
/// @param z1 First point z coordinate
/// @param x2 Second point x coordinate
/// @param y2 Second point y coordinate
/// @param z2 Second point z coordinate
/// @return Distance between points
inline float ofDist(float x1, float y1, float z1, float x2, float y2, float z2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

/// Calculate squared distance between two 2D points (faster, no sqrt)
/// @param x1 First point x coordinate
/// @param y1 First point y coordinate
/// @param x2 Second point x coordinate
/// @param y2 Second point y coordinate
/// @return Squared distance between points
inline float ofDistSquared(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return dx * dx + dy * dy;
}

/// Calculate squared distance between two 3D points (faster, no sqrt)
/// @param x1 First point x coordinate
/// @param y1 First point y coordinate
/// @param z1 First point z coordinate
/// @param x2 Second point x coordinate
/// @param y2 Second point y coordinate
/// @param z2 Second point z coordinate
/// @return Squared distance between points
inline float ofDistSquared(float x1, float y1, float z1, float x2, float y2, float z2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;
    return dx * dx + dy * dy + dz * dz;
}

// MARK: - Angle Conversion

/// Convert degrees to radians
/// @param degrees Angle in degrees
/// @return Angle in radians
inline float ofDegToRad(float degrees) {
    return degrees * DEG_TO_RAD;
}

/// Convert radians to degrees
/// @param radians Angle in radians
/// @return Angle in degrees
inline float ofRadToDeg(float radians) {
    return radians * RAD_TO_DEG;
}

// MARK: - Value Wrapping

/// Wrap value to range [from, to]
/// @param value Input value
/// @param from Range start
/// @param to Range end
/// @return Wrapped value
inline float ofWrap(float value, float from, float to) {
    float range = to - from;
    if (range == 0.0f) return from;

    float result = std::fmod(value - from, range);
    if (result < 0.0f) result += range;
    return result + from;
}

/// Wrap angle in degrees to range [0, 360]
/// @param degrees Angle in degrees
/// @return Wrapped angle in [0, 360]
inline float ofWrapDegrees(float degrees) {
    return ofWrap(degrees, 0.0f, 360.0f);
}

/// Wrap angle in radians to range [0, TWO_PI]
/// @param radians Angle in radians
/// @return Wrapped angle in [0, TWO_PI]
inline float ofWrapRadians(float radians) {
    return ofWrap(radians, 0.0f, TWO_PI);
}

// MARK: - Angle Difference

/// Calculate smallest difference between two angles in degrees
/// @param currentAngle Current angle in degrees
/// @param targetAngle Target angle in degrees
/// @return Angle difference in [-180, 180]
inline float ofAngleDifferenceDegrees(float currentAngle, float targetAngle) {
    float diff = targetAngle - currentAngle;
    while (diff < -180.0f) diff += 360.0f;
    while (diff > 180.0f) diff -= 360.0f;
    return diff;
}

/// Calculate smallest difference between two angles in radians
/// @param currentAngle Current angle in radians
/// @param targetAngle Target angle in radians
/// @return Angle difference in [-PI, PI]
inline float ofAngleDifferenceRadians(float currentAngle, float targetAngle) {
    float diff = targetAngle - currentAngle;
    while (diff < -PI) diff += TWO_PI;
    while (diff > PI) diff -= TWO_PI;
    return diff;
}

// MARK: - Sign Function

/// Get sign of value
/// @param value Input value
/// @return -1 if negative, 0 if zero, 1 if positive
inline int ofSign(float value) {
    if (value < 0.0f) return -1;
    if (value > 0.0f) return 1;
    return 0;
}
