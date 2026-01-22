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

  static ofColor white() { return ofColor(255, 255, 255); }
  static ofColor black() { return ofColor(0, 0, 0); }
  static ofColor red() { return ofColor(255, 0, 0); }
  static ofColor green() { return ofColor(0, 255, 0); }
  static ofColor blue() { return ofColor(0, 0, 255); }
  static ofColor yellow() { return ofColor(255, 255, 0); }
  static ofColor cyan() { return ofColor(0, 255, 255); }
  static ofColor magenta() { return ofColor(255, 0, 255); }
  static ofColor gray() { return ofColor(128, 128, 128); }
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
};
