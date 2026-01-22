#pragma once

#include <algorithm>
#include "../math/ofVec2f.h"

class ofRectangle {
public:
    float x = 0;
    float y = 0;
    float width = 0;
    float height = 0;

    ofRectangle() = default;

    ofRectangle(float px, float py, float w, float h)
        : x(px), y(py), width(w), height(h) {}

    ofRectangle(const ofVec2f& pos, float w, float h)
        : x(pos.x), y(pos.y), width(w), height(h) {}

    void set(float px, float py, float w, float h) {
        x = px;
        y = py;
        width = w;
        height = h;
    }

    void set(const ofVec2f& pos, float w, float h) {
        x = pos.x;
        y = pos.y;
        width = w;
        height = h;
    }

    void setFromCenter(float cx, float cy, float w, float h) {
        x = cx - w * 0.5f;
        y = cy - h * 0.5f;
        width = w;
        height = h;
    }

    void setFromCenter(const ofVec2f& center, float w, float h) {
        setFromCenter(center.x, center.y, w, h);
    }

    float getX() const { return x; }
    float getY() const { return y; }
    float getWidth() const { return width; }
    float getHeight() const { return height; }

    float getMinX() const { return std::min(x, x + width); }
    float getMaxX() const { return std::max(x, x + width); }
    float getMinY() const { return std::min(y, y + height); }
    float getMaxY() const { return std::max(y, y + height); }

    float getLeft() const { return getMinX(); }
    float getRight() const { return getMaxX(); }
    float getTop() const { return getMinY(); }
    float getBottom() const { return getMaxY(); }

    ofVec2f getPosition() const { return ofVec2f(x, y); }

    ofVec2f getCenter() const {
        return ofVec2f(x + width * 0.5f, y + height * 0.5f);
    }

    ofVec2f getTopLeft() const { return ofVec2f(getLeft(), getTop()); }
    ofVec2f getTopRight() const { return ofVec2f(getRight(), getTop()); }
    ofVec2f getBottomLeft() const { return ofVec2f(getLeft(), getBottom()); }
    ofVec2f getBottomRight() const { return ofVec2f(getRight(), getBottom()); }

    float getArea() const { return std::abs(width) * std::abs(height); }
    float getPerimeter() const { return 2.0f * std::abs(width) + 2.0f * std::abs(height); }
    float getAspectRatio() const { return std::abs(width) / std::abs(height); }

    bool isEmpty() const { return width == 0 || height == 0; }

    bool inside(float px, float py) const {
        return px >= getMinX() && px <= getMaxX() &&
               py >= getMinY() && py <= getMaxY();
    }

    bool inside(const ofVec2f& p) const {
        return inside(p.x, p.y);
    }

    bool inside(const ofRectangle& r) const {
        return inside(r.getMinX(), r.getMinY()) &&
               inside(r.getMaxX(), r.getMaxY());
    }

    bool intersects(const ofRectangle& r) const {
        return getMinX() < r.getMaxX() && getMaxX() > r.getMinX() &&
               getMinY() < r.getMaxY() && getMaxY() > r.getMinY();
    }

    ofRectangle getIntersection(const ofRectangle& r) const {
        float minX = std::max(getMinX(), r.getMinX());
        float maxX = std::min(getMaxX(), r.getMaxX());
        float minY = std::max(getMinY(), r.getMinY());
        float maxY = std::min(getMaxY(), r.getMaxY());

        if (minX > maxX || minY > maxY) {
            return ofRectangle();
        }
        return ofRectangle(minX, minY, maxX - minX, maxY - minY);
    }

    ofRectangle getUnion(const ofRectangle& r) const {
        float minX = std::min(getMinX(), r.getMinX());
        float maxX = std::max(getMaxX(), r.getMaxX());
        float minY = std::min(getMinY(), r.getMinY());
        float maxY = std::max(getMaxY(), r.getMaxY());
        return ofRectangle(minX, minY, maxX - minX, maxY - minY);
    }

    void standardize() {
        if (width < 0) {
            x += width;
            width = -width;
        }
        if (height < 0) {
            y += height;
            height = -height;
        }
    }

    ofRectangle getStandardized() const {
        ofRectangle r = *this;
        r.standardize();
        return r;
    }

    void translate(float dx, float dy) {
        x += dx;
        y += dy;
    }

    void translate(const ofVec2f& d) {
        translate(d.x, d.y);
    }

    void scale(float s) {
        width *= s;
        height *= s;
    }

    void scale(float sx, float sy) {
        width *= sx;
        height *= sy;
    }

    void scaleFromCenter(float s) {
        scaleFromCenter(s, s);
    }

    void scaleFromCenter(float sx, float sy) {
        float cx = x + width * 0.5f;
        float cy = y + height * 0.5f;
        width *= sx;
        height *= sy;
        x = cx - width * 0.5f;
        y = cy - height * 0.5f;
    }

    void growToInclude(float px, float py) {
        float minX = std::min(x, px);
        float minY = std::min(y, py);
        float maxX = std::max(x + width, px);
        float maxY = std::max(y + height, py);
        x = minX;
        y = minY;
        width = maxX - minX;
        height = maxY - minY;
    }

    void growToInclude(const ofVec2f& p) {
        growToInclude(p.x, p.y);
    }

    void growToInclude(const ofRectangle& r) {
        float minX = std::min(getMinX(), r.getMinX());
        float minY = std::min(getMinY(), r.getMinY());
        float maxX = std::max(getMaxX(), r.getMaxX());
        float maxY = std::max(getMaxY(), r.getMaxY());
        x = minX;
        y = minY;
        width = maxX - minX;
        height = maxY - minY;
    }

    bool operator==(const ofRectangle& r) const {
        return x == r.x && y == r.y && width == r.width && height == r.height;
    }

    bool operator!=(const ofRectangle& r) const {
        return !(*this == r);
    }

    ofRectangle operator+(const ofVec2f& p) const {
        return ofRectangle(x + p.x, y + p.y, width, height);
    }

    ofRectangle operator-(const ofVec2f& p) const {
        return ofRectangle(x - p.x, y - p.y, width, height);
    }

    ofRectangle& operator+=(const ofVec2f& p) {
        x += p.x;
        y += p.y;
        return *this;
    }

    ofRectangle& operator-=(const ofVec2f& p) {
        x -= p.x;
        y -= p.y;
        return *this;
    }
};
