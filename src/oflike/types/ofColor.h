#pragma once

// oflike-metal ofColor - openFrameworks API compatible color class
// Uses 8-bit RGBA values (0-255) like openFrameworks

#include <cstdint>
#include <algorithm>
#include <cmath>

namespace oflike {

/// \brief Template color class for RGBA colors
/// \details ofColor_ provides openFrameworks-compatible color operations
/// using a template for different data types (uint8_t, uint16_t, float)
template<typename PixelType>
class ofColor_ {
public:
    // ========================================================================
    // Member Variables
    // ========================================================================

    PixelType r;  ///< Red channel
    PixelType g;  ///< Green channel
    PixelType b;  ///< Blue channel
    PixelType a;  ///< Alpha channel

    // ========================================================================
    // Static Constants
    // ========================================================================

    /// \brief Get the maximum value for this pixel type
    static constexpr float limit() {
        if constexpr (std::is_floating_point<PixelType>::value) {
            return 1.0f;
        } else if constexpr (sizeof(PixelType) == 1) {
            return 255.0f;
        } else if constexpr (sizeof(PixelType) == 2) {
            return 65535.0f;
        } else {
            return 255.0f;
        }
    }

    // ========================================================================
    // Constructors
    // ========================================================================

    /// \brief Default constructor, initializes to opaque white
    ofColor_() : r(limit()), g(limit()), b(limit()), a(limit()) {}

    /// \brief Construct from gray value (with optional alpha)
    explicit ofColor_(PixelType _gray, PixelType _a = limit())
        : r(_gray), g(_gray), b(_gray), a(_a) {}

    /// \brief Construct from RGB (with optional alpha)
    ofColor_(PixelType _r, PixelType _g, PixelType _b, PixelType _a = limit())
        : r(_r), g(_g), b(_b), a(_a) {}

    /// \brief Copy constructor
    ofColor_(const ofColor_& color)
        : r(color.r), g(color.g), b(color.b), a(color.a) {}

    /// \brief Construct from different pixel type color
    template<typename SrcType>
    ofColor_(const ofColor_<SrcType>& color) {
        const float srcMax = ofColor_<SrcType>::limit();
        const float dstMax = limit();
        const float scale = dstMax / srcMax;
        r = static_cast<PixelType>(color.r * scale);
        g = static_cast<PixelType>(color.g * scale);
        b = static_cast<PixelType>(color.b * scale);
        a = static_cast<PixelType>(color.a * scale);
    }

    // ========================================================================
    // Setters
    // ========================================================================

    /// \brief Set RGB components
    void set(PixelType _r, PixelType _g, PixelType _b) {
        r = _r;
        g = _g;
        b = _b;
    }

    /// \brief Set RGBA components
    void set(PixelType _r, PixelType _g, PixelType _b, PixelType _a) {
        r = _r;
        g = _g;
        b = _b;
        a = _a;
    }

    /// \brief Set from gray value
    void set(PixelType _gray) {
        r = g = b = _gray;
    }

    /// \brief Set from hex value (0xRRGGBB or 0xAARRGGBB)
    void setHex(uint32_t hexColor, float alpha = limit()) {
        const float scale = limit() / 255.0f;

        if (hexColor > 0xFFFFFF) {
            // 0xAARRGGBB format
            a = static_cast<PixelType>(((hexColor >> 24) & 0xFF) * scale);
            r = static_cast<PixelType>(((hexColor >> 16) & 0xFF) * scale);
            g = static_cast<PixelType>(((hexColor >> 8) & 0xFF) * scale);
            b = static_cast<PixelType>((hexColor & 0xFF) * scale);
        } else {
            // 0xRRGGBB format
            r = static_cast<PixelType>(((hexColor >> 16) & 0xFF) * scale);
            g = static_cast<PixelType>(((hexColor >> 8) & 0xFF) * scale);
            b = static_cast<PixelType>((hexColor & 0xFF) * scale);
            a = static_cast<PixelType>(alpha);
        }
    }

    /// \brief Get hex value (0xRRGGBB format, alpha ignored)
    uint32_t getHex() const {
        const float scale = 255.0f / limit();
        uint32_t red = static_cast<uint32_t>(r * scale) & 0xFF;
        uint32_t green = static_cast<uint32_t>(g * scale) & 0xFF;
        uint32_t blue = static_cast<uint32_t>(b * scale) & 0xFF;
        return (red << 16) | (green << 8) | blue;
    }

    // ========================================================================
    // HSB Conversion
    // ========================================================================

    /// \brief Set color from HSB values
    /// \param hue Hue (0-limit())
    /// \param saturation Saturation (0-limit())
    /// \param brightness Brightness (0-limit())
    /// \param alpha Alpha (0-limit())
    void setHsb(float hue, float saturation, float brightness, float alpha = limit()) {
        const float maxVal = limit();
        hue = std::fmod(hue, maxVal) / maxVal;
        saturation = std::clamp(saturation / maxVal, 0.0f, 1.0f);
        brightness = std::clamp(brightness / maxVal, 0.0f, 1.0f);

        if (saturation == 0.0f) {
            // Achromatic (gray)
            r = g = b = static_cast<PixelType>(brightness * maxVal);
        } else {
            float h = hue * 6.0f;
            int i = static_cast<int>(std::floor(h));
            float f = h - i;
            float p = brightness * (1.0f - saturation);
            float q = brightness * (1.0f - saturation * f);
            float t = brightness * (1.0f - saturation * (1.0f - f));

            switch (i % 6) {
                case 0: r = static_cast<PixelType>(brightness * maxVal);
                        g = static_cast<PixelType>(t * maxVal);
                        b = static_cast<PixelType>(p * maxVal); break;
                case 1: r = static_cast<PixelType>(q * maxVal);
                        g = static_cast<PixelType>(brightness * maxVal);
                        b = static_cast<PixelType>(p * maxVal); break;
                case 2: r = static_cast<PixelType>(p * maxVal);
                        g = static_cast<PixelType>(brightness * maxVal);
                        b = static_cast<PixelType>(t * maxVal); break;
                case 3: r = static_cast<PixelType>(p * maxVal);
                        g = static_cast<PixelType>(q * maxVal);
                        b = static_cast<PixelType>(brightness * maxVal); break;
                case 4: r = static_cast<PixelType>(t * maxVal);
                        g = static_cast<PixelType>(p * maxVal);
                        b = static_cast<PixelType>(brightness * maxVal); break;
                case 5: r = static_cast<PixelType>(brightness * maxVal);
                        g = static_cast<PixelType>(p * maxVal);
                        b = static_cast<PixelType>(q * maxVal); break;
            }
        }
        a = static_cast<PixelType>(alpha);
    }

    /// \brief Get hue component (0-limit())
    float getHue() const {
        const float maxVal = limit();
        float rf = r / maxVal;
        float gf = g / maxVal;
        float bf = b / maxVal;

        float max = std::max({rf, gf, bf});
        float min = std::min({rf, gf, bf});
        float delta = max - min;

        if (delta < 0.00001f) {
            return 0.0f;
        }

        float hue;
        if (max == rf) {
            hue = (gf - bf) / delta + (gf < bf ? 6.0f : 0.0f);
        } else if (max == gf) {
            hue = (bf - rf) / delta + 2.0f;
        } else {
            hue = (rf - gf) / delta + 4.0f;
        }

        return (hue / 6.0f) * maxVal;
    }

    /// \brief Get saturation component (0-limit())
    float getSaturation() const {
        const float maxVal = limit();
        float rf = r / maxVal;
        float gf = g / maxVal;
        float bf = b / maxVal;

        float max = std::max({rf, gf, bf});
        float min = std::min({rf, gf, bf});

        if (max < 0.00001f) {
            return 0.0f;
        }

        return ((max - min) / max) * maxVal;
    }

    /// \brief Get brightness component (0-limit())
    float getBrightness() const {
        const float maxVal = limit();
        float rf = r / maxVal;
        float gf = g / maxVal;
        float bf = b / maxVal;

        return std::max({rf, gf, bf}) * maxVal;
    }

    /// \brief Get lightness component (0-limit())
    float getLightness() const {
        const float maxVal = limit();
        float rf = r / maxVal;
        float gf = g / maxVal;
        float bf = b / maxVal;

        float max = std::max({rf, gf, bf});
        float min = std::min({rf, gf, bf});

        return ((max + min) / 2.0f) * maxVal;
    }

    // ========================================================================
    // Color Operations
    // ========================================================================

    /// \brief Linear interpolation between this color and another
    ofColor_ lerp(const ofColor_& target, float amount) const {
        amount = std::clamp(amount, 0.0f, 1.0f);
        return ofColor_(
            static_cast<PixelType>(r + (target.r - r) * amount),
            static_cast<PixelType>(g + (target.g - g) * amount),
            static_cast<PixelType>(b + (target.b - b) * amount),
            static_cast<PixelType>(a + (target.a - a) * amount)
        );
    }

    /// \brief Get clamped color (all components within valid range)
    ofColor_ getClamped() const {
        const PixelType maxVal = static_cast<PixelType>(limit());
        const PixelType minVal = 0;
        return ofColor_(
            std::clamp(r, minVal, maxVal),
            std::clamp(g, minVal, maxVal),
            std::clamp(b, minVal, maxVal),
            std::clamp(a, minVal, maxVal)
        );
    }

    /// \brief Clamp this color in place
    ofColor_& clamp() {
        *this = getClamped();
        return *this;
    }

    /// \brief Get inverted color (1 - component)
    ofColor_ getInverted() const {
        const PixelType maxVal = static_cast<PixelType>(limit());
        return ofColor_(
            maxVal - r,
            maxVal - g,
            maxVal - b,
            a  // Alpha is not inverted
        );
    }

    /// \brief Invert this color in place
    ofColor_& invert() {
        *this = getInverted();
        return *this;
    }

    /// \brief Get normalized color (mapped to 0-1 range)
    ofColor_ getNormalized() const {
        const float maxVal = limit();
        if constexpr (std::is_floating_point<PixelType>::value) {
            return *this;
        } else {
            return ofColor_(
                static_cast<PixelType>((r / maxVal) * limit()),
                static_cast<PixelType>((g / maxVal) * limit()),
                static_cast<PixelType>((b / maxVal) * limit()),
                static_cast<PixelType>((a / maxVal) * limit())
            );
        }
    }

    /// \brief Normalize this color in place
    ofColor_& normalize() {
        *this = getNormalized();
        return *this;
    }

    // ========================================================================
    // Operators
    // ========================================================================

    /// \brief Component-wise addition
    ofColor_ operator+(const ofColor_& color) const {
        return ofColor_(r + color.r, g + color.g, b + color.b, a + color.a);
    }

    /// \brief Component-wise addition with scalar
    ofColor_ operator+(float val) const {
        PixelType v = static_cast<PixelType>(val);
        return ofColor_(r + v, g + v, b + v, a);
    }

    /// \brief In-place addition
    ofColor_& operator+=(const ofColor_& color) {
        r += color.r;
        g += color.g;
        b += color.b;
        a += color.a;
        return *this;
    }

    /// \brief Component-wise subtraction
    ofColor_ operator-(const ofColor_& color) const {
        return ofColor_(r - color.r, g - color.g, b - color.b, a - color.a);
    }

    /// \brief Component-wise subtraction with scalar
    ofColor_ operator-(float val) const {
        PixelType v = static_cast<PixelType>(val);
        return ofColor_(r - v, g - v, b - v, a);
    }

    /// \brief In-place subtraction
    ofColor_& operator-=(const ofColor_& color) {
        r -= color.r;
        g -= color.g;
        b -= color.b;
        a -= color.a;
        return *this;
    }

    /// \brief Component-wise multiplication
    ofColor_ operator*(const ofColor_& color) const {
        const float maxVal = limit();
        return ofColor_(
            static_cast<PixelType>((r / maxVal) * (color.r / maxVal) * maxVal),
            static_cast<PixelType>((g / maxVal) * (color.g / maxVal) * maxVal),
            static_cast<PixelType>((b / maxVal) * (color.b / maxVal) * maxVal),
            static_cast<PixelType>((a / maxVal) * (color.a / maxVal) * maxVal)
        );
    }

    /// \brief Scalar multiplication
    ofColor_ operator*(float val) const {
        return ofColor_(
            static_cast<PixelType>(r * val),
            static_cast<PixelType>(g * val),
            static_cast<PixelType>(b * val),
            static_cast<PixelType>(a * val)
        );
    }

    /// \brief In-place multiplication
    ofColor_& operator*=(const ofColor_& color) {
        *this = *this * color;
        return *this;
    }

    /// \brief In-place scalar multiplication
    ofColor_& operator*=(float val) {
        r = static_cast<PixelType>(r * val);
        g = static_cast<PixelType>(g * val);
        b = static_cast<PixelType>(b * val);
        a = static_cast<PixelType>(a * val);
        return *this;
    }

    /// \brief Component-wise division
    ofColor_ operator/(const ofColor_& color) const {
        const float maxVal = limit();
        return ofColor_(
            color.r != 0 ? static_cast<PixelType>((r / maxVal) / (color.r / maxVal) * maxVal) : 0,
            color.g != 0 ? static_cast<PixelType>((g / maxVal) / (color.g / maxVal) * maxVal) : 0,
            color.b != 0 ? static_cast<PixelType>((b / maxVal) / (color.b / maxVal) * maxVal) : 0,
            color.a != 0 ? static_cast<PixelType>((a / maxVal) / (color.a / maxVal) * maxVal) : 0
        );
    }

    /// \brief Scalar division
    ofColor_ operator/(float val) const {
        if (val == 0.0f) return *this;
        return ofColor_(
            static_cast<PixelType>(r / val),
            static_cast<PixelType>(g / val),
            static_cast<PixelType>(b / val),
            static_cast<PixelType>(a / val)
        );
    }

    /// \brief In-place division
    ofColor_& operator/=(const ofColor_& color) {
        *this = *this / color;
        return *this;
    }

    /// \brief In-place scalar division
    ofColor_& operator/=(float val) {
        if (val != 0.0f) {
            r = static_cast<PixelType>(r / val);
            g = static_cast<PixelType>(g / val);
            b = static_cast<PixelType>(b / val);
            a = static_cast<PixelType>(a / val);
        }
        return *this;
    }

    /// \brief Equality comparison
    bool operator==(const ofColor_& color) const {
        return r == color.r && g == color.g && b == color.b && a == color.a;
    }

    /// \brief Inequality comparison
    bool operator!=(const ofColor_& color) const {
        return !(*this == color);
    }

    /// \brief Assignment operator
    ofColor_& operator=(const ofColor_& color) {
        r = color.r;
        g = color.g;
        b = color.b;
        a = color.a;
        return *this;
    }

    /// \brief Array access operator
    PixelType& operator[](int n) {
        switch (n) {
            case 0: return r;
            case 1: return g;
            case 2: return b;
            case 3: return a;
            default: return r;
        }
    }

    /// \brief Const array access operator
    const PixelType& operator[](int n) const {
        switch (n) {
            case 0: return r;
            case 1: return g;
            case 2: return b;
            case 3: return a;
            default: return r;
        }
    }

    // ========================================================================
    // Static Factory Methods
    // ========================================================================

    /// \brief Create color from hex value
    static ofColor_ fromHex(uint32_t hexColor, float alpha = limit()) {
        ofColor_ color;
        color.setHex(hexColor, alpha);
        return color;
    }

    /// \brief Create color from HSB values
    static ofColor_ fromHsb(float hue, float saturation, float brightness, float alpha = limit()) {
        ofColor_ color;
        color.setHsb(hue, saturation, brightness, alpha);
        return color;
    }

    // ========================================================================
    // Predefined Colors (static constants will be defined in cpp)
    // ========================================================================

    static const ofColor_ white;
    static const ofColor_ gray;
    static const ofColor_ black;
    static const ofColor_ red;
    static const ofColor_ green;
    static const ofColor_ blue;
    static const ofColor_ cyan;
    static const ofColor_ magenta;
    static const ofColor_ yellow;
};

// ========================================================================
// Type Definitions
// ========================================================================

using ofColor = ofColor_<uint8_t>;           ///< 8-bit RGBA color (0-255)
using ofFloatColor = ofColor_<float>;        ///< Float RGBA color (0.0-1.0)
using ofShortColor = ofColor_<uint16_t>;     ///< 16-bit RGBA color (0-65535)

// ========================================================================
// Static Color Constants (Definition)
// ========================================================================

// Static color definitions
template<typename PixelType>
const ofColor_<PixelType> ofColor_<PixelType>::white(
    static_cast<PixelType>(ofColor_<PixelType>::limit()),
    static_cast<PixelType>(ofColor_<PixelType>::limit()),
    static_cast<PixelType>(ofColor_<PixelType>::limit())
);

template<typename PixelType>
const ofColor_<PixelType> ofColor_<PixelType>::gray(
    static_cast<PixelType>(ofColor_<PixelType>::limit() / 2),
    static_cast<PixelType>(ofColor_<PixelType>::limit() / 2),
    static_cast<PixelType>(ofColor_<PixelType>::limit() / 2)
);

template<typename PixelType>
const ofColor_<PixelType> ofColor_<PixelType>::black(0, 0, 0);

template<typename PixelType>
const ofColor_<PixelType> ofColor_<PixelType>::red(
    static_cast<PixelType>(ofColor_<PixelType>::limit()), 0, 0
);

template<typename PixelType>
const ofColor_<PixelType> ofColor_<PixelType>::green(
    0, static_cast<PixelType>(ofColor_<PixelType>::limit()), 0
);

template<typename PixelType>
const ofColor_<PixelType> ofColor_<PixelType>::blue(
    0, 0, static_cast<PixelType>(ofColor_<PixelType>::limit())
);

template<typename PixelType>
const ofColor_<PixelType> ofColor_<PixelType>::cyan(
    0,
    static_cast<PixelType>(ofColor_<PixelType>::limit()),
    static_cast<PixelType>(ofColor_<PixelType>::limit())
);

template<typename PixelType>
const ofColor_<PixelType> ofColor_<PixelType>::magenta(
    static_cast<PixelType>(ofColor_<PixelType>::limit()),
    0,
    static_cast<PixelType>(ofColor_<PixelType>::limit())
);

template<typename PixelType>
const ofColor_<PixelType> ofColor_<PixelType>::yellow(
    static_cast<PixelType>(ofColor_<PixelType>::limit()),
    static_cast<PixelType>(ofColor_<PixelType>::limit()),
    0
);

} // namespace oflike
