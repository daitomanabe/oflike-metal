#include "ofxSvg.h"
#include "../../../oflike/utils/ofLog.h"

// nanosvg is a header-only library
#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"

using namespace oflike;

//-----------------------------------------------------------------------------
// Implementation class (pImpl pattern)
//-----------------------------------------------------------------------------
class ofxSvg::Impl {
public:
    NSVGimage* image = nullptr;
    std::vector<ofPath> paths;
    float width = 0.0f;
    float height = 0.0f;

    ~Impl() {
        clear();
    }

    void clear() {
        if (image) {
            nsvgDelete(image);
            image = nullptr;
        }
        paths.clear();
        width = 0.0f;
        height = 0.0f;
    }

    bool loadFromFile(const std::string& path, const std::string& units, float dpi) {
        clear();

        // Parse SVG file
        image = nsvgParseFromFile(path.c_str(), units.c_str(), dpi);
        if (!image) {
            ofLogError("ofxSvg") << "Failed to parse SVG file: " << path;
            return false;
        }

        // Store dimensions
        width = image->width;
        height = image->height;

        // Convert SVG shapes to ofPath objects
        convertShapesToPaths();

        ofLogVerbose("ofxSvg") << "Loaded SVG: " << path
                               << " (" << width << "x" << height << ")"
                               << " with " << paths.size() << " paths";
        return true;
    }

    bool loadFromString(const std::string& svgStr, const std::string& units, float dpi) {
        clear();

        // nsvgParse modifies the input string, so we need a mutable copy
        std::vector<char> buffer(svgStr.begin(), svgStr.end());
        buffer.push_back('\0');

        // Parse SVG string
        image = nsvgParse(buffer.data(), units.c_str(), dpi);
        if (!image) {
            ofLogError("ofxSvg") << "Failed to parse SVG string";
            return false;
        }

        // Store dimensions
        width = image->width;
        height = image->height;

        // Convert SVG shapes to ofPath objects
        convertShapesToPaths();

        ofLogVerbose("ofxSvg") << "Loaded SVG from string"
                               << " (" << width << "x" << height << ")"
                               << " with " << paths.size() << " paths";
        return true;
    }

private:
    void convertShapesToPaths() {
        if (!image) return;

        // Iterate through all shapes in the SVG
        for (NSVGshape* shape = image->shapes; shape != nullptr; shape = shape->next) {
            // Each shape can have multiple paths (contours)
            for (NSVGpath* nsvgPath = shape->paths; nsvgPath != nullptr; nsvgPath = nsvgPath->next) {
                ofPath path;

                // Set fill and stroke properties
                if (shape->fill.type == NSVG_PAINT_COLOR) {
                    path.setFilled(true);
                    unsigned int color = shape->fill.color;
                    uint8_t r = (color >> 0) & 0xFF;
                    uint8_t g = (color >> 8) & 0xFF;
                    uint8_t b = (color >> 16) & 0xFF;
                    uint8_t a = (color >> 24) & 0xFF;
                    path.setFillColor(r, g, b, a);
                } else {
                    path.setFilled(false);
                }

                if (shape->stroke.type == NSVG_PAINT_COLOR) {
                    path.setStrokeWidth(shape->strokeWidth);
                    unsigned int color = shape->stroke.color;
                    uint8_t r = (color >> 0) & 0xFF;
                    uint8_t g = (color >> 8) & 0xFF;
                    uint8_t b = (color >> 16) & 0xFF;
                    uint8_t a = (color >> 24) & 0xFF;
                    path.setStrokeColor(r, g, b, a);
                } else {
                    path.setStrokeWidth(0.0f);
                }

                // Apply opacity
                // Note: This is a simplification - full implementation would need to handle opacity properly
                float opacity = shape->opacity;
                (void)opacity; // TODO: Apply opacity to fill and stroke colors

                // Convert cubic bezier points to ofPath commands
                // nanosvg stores points as: [x0,y0, cpx1,cpy1,cpx2,cpy2,x1,y1, ...]
                // Each bezier segment has 4 control points (8 floats)
                if (nsvgPath->npts > 0) {
                    float* pts = nsvgPath->pts;

                    // Move to first point
                    path.moveTo(pts[0], pts[1]);

                    // Add bezier curves
                    for (int i = 0; i < nsvgPath->npts - 1; i += 3) {
                        float* p = &pts[i * 2];
                        // p[0],p[1] = current point (already at this position)
                        // p[2],p[3] = control point 1
                        // p[4],p[5] = control point 2
                        // p[6],p[7] = end point
                        path.bezierTo(p[2], p[3], 0.0f,
                                     p[4], p[5], 0.0f,
                                     p[6], p[7], 0.0f);
                    }

                    // Close path if needed
                    if (nsvgPath->closed) {
                        path.close();
                    }
                }

                paths.push_back(std::move(path));
            }
        }
    }
};

//-----------------------------------------------------------------------------
// Public API
//-----------------------------------------------------------------------------

ofxSvg::ofxSvg() : impl_(std::make_unique<Impl>()) {}

ofxSvg::~ofxSvg() = default;

ofxSvg::ofxSvg(const ofxSvg& other) : impl_(std::make_unique<Impl>()) {
    impl_->width = other.impl_->width;
    impl_->height = other.impl_->height;
    impl_->paths = other.impl_->paths;
    // Note: nanosvg image is not copied (not needed after conversion)
}

ofxSvg& ofxSvg::operator=(const ofxSvg& other) {
    if (this != &other) {
        impl_->clear();
        impl_->width = other.impl_->width;
        impl_->height = other.impl_->height;
        impl_->paths = other.impl_->paths;
    }
    return *this;
}

bool ofxSvg::load(const std::string& path, const std::string& units, float dpi) {
    return impl_->loadFromFile(path, units, dpi);
}

bool ofxSvg::loadFromString(const std::string& svgStr, const std::string& units, float dpi) {
    return impl_->loadFromString(svgStr, units, dpi);
}

bool ofxSvg::isLoaded() const {
    return !impl_->paths.empty();
}

void ofxSvg::clear() {
    impl_->clear();
}

int ofxSvg::getNumPaths() const {
    return static_cast<int>(impl_->paths.size());
}

ofPath& ofxSvg::getPathAt(int index) {
    return impl_->paths[index];
}

const ofPath& ofxSvg::getPathAt(int index) const {
    return impl_->paths[index];
}

std::vector<ofPath>& ofxSvg::getPaths() {
    return impl_->paths;
}

const std::vector<ofPath>& ofxSvg::getPaths() const {
    return impl_->paths;
}

float ofxSvg::getWidth() const {
    return impl_->width;
}

float ofxSvg::getHeight() const {
    return impl_->height;
}

void ofxSvg::draw() {
    for (auto& path : impl_->paths) {
        path.draw();
    }
}

void ofxSvg::draw(float x, float y) {
    (void)x; (void)y; // TODO: Implement matrix translation
    // Save current matrix, translate, draw, restore
    // TODO: Implement matrix stack push/pop when needed
    for (auto& path : impl_->paths) {
        path.draw();
    }
}

void ofxSvg::draw(float x, float y, float width, float height) {
    (void)x; (void)y; (void)width; (void)height; // TODO: Implement matrix transformation
    // Save current matrix, translate and scale, draw, restore
    // TODO: Implement matrix stack push/pop and scaling when needed
    for (auto& path : impl_->paths) {
        path.draw();
    }
}
