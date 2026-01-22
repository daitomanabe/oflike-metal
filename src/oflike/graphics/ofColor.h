#pragma once

#include <cmath>
#include <algorithm>

struct ofColor {
  unsigned char r{255}, g{255}, b{255}, a{255};

  ofColor() = default;
  ofColor(unsigned char gray) : r(gray), g(gray), b(gray), a(255) {}
  ofColor(unsigned char r_, unsigned char g_, unsigned char b_, unsigned char a_ = 255)
    : r(r_), g(g_), b(b_), a(a_) {}

  void set(unsigned char r_, unsigned char g_, unsigned char b_, unsigned char a_ = 255) {
    r = r_; g = g_; b = b_; a = a_;
  }

  void setHsb(float hue, float saturation, float brightness, float alpha = 255.f) {
    float h = std::fmod(hue, 256.f) / 256.f;
    float s = saturation / 255.f;
    float v = brightness / 255.f;

    if (s == 0) {
      r = g = b = static_cast<unsigned char>(v * 255);
    } else {
      float hh = h * 6.f;
      int i = static_cast<int>(hh);
      float f = hh - i;
      float p = v * (1 - s);
      float q = v * (1 - s * f);
      float t = v * (1 - s * (1 - f));

      float rf, gf, bf;
      switch (i % 6) {
        case 0: rf = v; gf = t; bf = p; break;
        case 1: rf = q; gf = v; bf = p; break;
        case 2: rf = p; gf = v; bf = t; break;
        case 3: rf = p; gf = q; bf = v; break;
        case 4: rf = t; gf = p; bf = v; break;
        default: rf = v; gf = p; bf = q; break;
      }
      r = static_cast<unsigned char>(rf * 255);
      g = static_cast<unsigned char>(gf * 255);
      b = static_cast<unsigned char>(bf * 255);
    }
    a = static_cast<unsigned char>(alpha);
  }

  float getHue() const {
    float rf = r / 255.f;
    float gf = g / 255.f;
    float bf = b / 255.f;
    float maxVal = std::max({rf, gf, bf});
    float minVal = std::min({rf, gf, bf});
    float delta = maxVal - minVal;

    if (delta < 0.00001f) return 0;

    float hue = 0;
    if (maxVal == rf) {
      hue = std::fmod((gf - bf) / delta, 6.f);
    } else if (maxVal == gf) {
      hue = (bf - rf) / delta + 2.f;
    } else {
      hue = (rf - gf) / delta + 4.f;
    }

    hue *= 42.5f;
    if (hue < 0) hue += 255;
    return hue;
  }

  float getSaturation() const {
    float maxVal = std::max({r, g, b}) / 255.f;
    float minVal = std::min({r, g, b}) / 255.f;
    if (maxVal < 0.00001f) return 0;
    return ((maxVal - minVal) / maxVal) * 255.f;
  }

  float getBrightness() const {
    return static_cast<float>(std::max({r, g, b}));
  }

  static ofColor fromHsb(float hue, float saturation, float brightness, float alpha = 255.f) {
    ofColor c;
    c.setHsb(hue, saturation, brightness, alpha);
    return c;
  }

  // Linear interpolation between this color and another
  ofColor getLerped(const ofColor& target, float t) const {
    t = std::max(0.f, std::min(1.f, t));
    return ofColor(
      static_cast<unsigned char>(r + (target.r - r) * t),
      static_cast<unsigned char>(g + (target.g - g) * t),
      static_cast<unsigned char>(b + (target.b - b) * t),
      static_cast<unsigned char>(a + (target.a - a) * t)
    );
  }

  // In-place lerp
  void lerp(const ofColor& target, float t) {
    *this = getLerped(target, t);
  }

  // Hex color
  void setHex(int hexColor, int alpha = 255) {
    r = (hexColor >> 16) & 0xFF;
    g = (hexColor >> 8) & 0xFF;
    b = hexColor & 0xFF;
    a = alpha;
  }

  int getHex() const {
    return (r << 16) | (g << 8) | b;
  }

  static ofColor fromHex(int hexColor, int alpha = 255) {
    ofColor c;
    c.setHex(hexColor, alpha);
    return c;
  }

  // Basic colors
  static ofColor white() { return ofColor(255, 255, 255); }
  static ofColor black() { return ofColor(0, 0, 0); }
  static ofColor red() { return ofColor(255, 0, 0); }
  static ofColor green() { return ofColor(0, 255, 0); }
  static ofColor blue() { return ofColor(0, 0, 255); }
  static ofColor yellow() { return ofColor(255, 255, 0); }
  static ofColor cyan() { return ofColor(0, 255, 255); }
  static ofColor magenta() { return ofColor(255, 0, 255); }
  static ofColor gray() { return ofColor(128, 128, 128); }

  // Extended colors (used in examples)
  static ofColor lightGray() { return ofColor(192, 192, 192); }
  static ofColor darkGray() { return ofColor(64, 64, 64); }
  static ofColor lightBlue() { return ofColor(173, 216, 230); }
  static ofColor blueSteel() { return ofColor(70, 130, 180); }
  static ofColor seaGreen() { return ofColor(46, 139, 87); }
  static ofColor orange() { return ofColor(255, 165, 0); }
  static ofColor pink() { return ofColor(255, 192, 203); }
  static ofColor purple() { return ofColor(128, 0, 128); }
  static ofColor brown() { return ofColor(139, 69, 19); }
  static ofColor olive() { return ofColor(128, 128, 0); }
  static ofColor navy() { return ofColor(0, 0, 128); }
  static ofColor teal() { return ofColor(0, 128, 128); }
  static ofColor maroon() { return ofColor(128, 0, 0); }
};

struct ofFloatColor {
  float r{1.f}, g{1.f}, b{1.f}, a{1.f};

  ofFloatColor() = default;
  ofFloatColor(float gray) : r(gray), g(gray), b(gray), a(1.f) {}
  ofFloatColor(float r_, float g_, float b_, float a_ = 1.f) : r(r_), g(g_), b(b_), a(a_) {}

  ofFloatColor(const ofColor& c)
    : r(c.r / 255.f), g(c.g / 255.f), b(c.b / 255.f), a(c.a / 255.f) {}

  ofColor toOfColor() const {
    return ofColor(
      static_cast<unsigned char>(r * 255),
      static_cast<unsigned char>(g * 255),
      static_cast<unsigned char>(b * 255),
      static_cast<unsigned char>(a * 255)
    );
  }

  // Linear interpolation between this color and another
  ofFloatColor getLerped(const ofFloatColor& target, float t) const {
    t = std::max(0.f, std::min(1.f, t));
    return ofFloatColor(
      r + (target.r - r) * t,
      g + (target.g - g) * t,
      b + (target.b - b) * t,
      a + (target.a - a) * t
    );
  }

  // In-place lerp
  void lerp(const ofFloatColor& target, float t) {
    *this = getLerped(target, t);
  }

  // Hex color
  void setHex(int hexColor, float alpha = 1.f) {
    r = ((hexColor >> 16) & 0xFF) / 255.f;
    g = ((hexColor >> 8) & 0xFF) / 255.f;
    b = (hexColor & 0xFF) / 255.f;
    a = alpha;
  }

  int getHex() const {
    return (static_cast<int>(r * 255) << 16) |
           (static_cast<int>(g * 255) << 8) |
           static_cast<int>(b * 255);
  }

  static ofFloatColor fromHex(int hexColor, float alpha = 1.f) {
    ofFloatColor c;
    c.setHex(hexColor, alpha);
    return c;
  }

  // HSB support
  void setHsb(float hue, float saturation, float brightness, float alpha = 1.f) {
    ofColor c;
    c.setHsb(hue * 255.f, saturation * 255.f, brightness * 255.f, alpha * 255.f);
    r = c.r / 255.f;
    g = c.g / 255.f;
    b = c.b / 255.f;
    a = c.a / 255.f;
  }

  static ofFloatColor fromHsb(float hue, float saturation, float brightness, float alpha = 1.f) {
    ofFloatColor c;
    c.setHsb(hue, saturation, brightness, alpha);
    return c;
  }

  // Named colors
  static ofFloatColor white() { return ofFloatColor(1.f, 1.f, 1.f); }
  static ofFloatColor black() { return ofFloatColor(0.f, 0.f, 0.f); }
  static ofFloatColor red() { return ofFloatColor(1.f, 0.f, 0.f); }
  static ofFloatColor green() { return ofFloatColor(0.f, 1.f, 0.f); }
  static ofFloatColor blue() { return ofFloatColor(0.f, 0.f, 1.f); }
  static ofFloatColor gray() { return ofFloatColor(0.5f, 0.5f, 0.5f); }
  static ofFloatColor lightGray() { return ofFloatColor(0.75f, 0.75f, 0.75f); }
};
