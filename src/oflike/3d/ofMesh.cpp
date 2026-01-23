#include "ofMesh.h"
#include "../graphics/ofGraphics.h"
#include "../../render/DrawList.h"
#include "../../core/Context.h"
#include <cmath>
#include <unordered_map>
#include <algorithm>

namespace oflike {

// ============================================================================
// Construction / Destruction
// ============================================================================

ofMesh::ofMesh()
    : mode_(OF_PRIMITIVE_TRIANGLES) {
}

ofMesh::ofMesh(ofPrimitiveMode mode)
    : mode_(mode) {
}

ofMesh::~ofMesh() = default;

ofMesh::ofMesh(ofMesh&& other) noexcept
    : mode_(other.mode_)
    , vertices_(std::move(other.vertices_))
    , normals_(std::move(other.normals_))
    , texCoords_(std::move(other.texCoords_))
    , colors_(std::move(other.colors_))
    , indices_(std::move(other.indices_)) {
}

ofMesh& ofMesh::operator=(ofMesh&& other) noexcept {
    if (this != &other) {
        mode_ = other.mode_;
        vertices_ = std::move(other.vertices_);
        normals_ = std::move(other.normals_);
        texCoords_ = std::move(other.texCoords_);
        colors_ = std::move(other.colors_);
        indices_ = std::move(other.indices_);
    }
    return *this;
}

// ============================================================================
// Vertex Data Management
// ============================================================================

void ofMesh::addVertex(const ofVec3f& v) {
    vertices_.push_back(v);
}

void ofMesh::addVertices(const std::vector<ofVec3f>& verts) {
    vertices_.insert(vertices_.end(), verts.begin(), verts.end());
}

void ofMesh::addVertices(const ofVec3f* verts, size_t count) {
    vertices_.reserve(vertices_.size() + count);
    for (size_t i = 0; i < count; ++i) {
        vertices_.push_back(verts[i]);
    }
}

void ofMesh::addNormal(const ofVec3f& n) {
    normals_.push_back(n);
}

void ofMesh::addNormals(const std::vector<ofVec3f>& norms) {
    normals_.insert(normals_.end(), norms.begin(), norms.end());
}

void ofMesh::addNormals(const ofVec3f* norms, size_t count) {
    normals_.reserve(normals_.size() + count);
    for (size_t i = 0; i < count; ++i) {
        normals_.push_back(norms[i]);
    }
}

void ofMesh::addTexCoord(const ofVec2f& t) {
    texCoords_.push_back(t);
}

void ofMesh::addTexCoords(const std::vector<ofVec2f>& tcs) {
    texCoords_.insert(texCoords_.end(), tcs.begin(), tcs.end());
}

void ofMesh::addTexCoords(const ofVec2f* tcs, size_t count) {
    texCoords_.reserve(texCoords_.size() + count);
    for (size_t i = 0; i < count; ++i) {
        texCoords_.push_back(tcs[i]);
    }
}

void ofMesh::addColor(const ofColor& c) {
    colors_.push_back(c);
}

void ofMesh::addColors(const std::vector<ofColor>& cols) {
    colors_.insert(colors_.end(), cols.begin(), cols.end());
}

void ofMesh::addColors(const ofColor* cols, size_t count) {
    colors_.reserve(colors_.size() + count);
    for (size_t i = 0; i < count; ++i) {
        colors_.push_back(cols[i]);
    }
}

// ============================================================================
// Index Management
// ============================================================================

void ofMesh::addIndex(uint32_t i) {
    indices_.push_back(i);
}

void ofMesh::addIndices(const std::vector<uint32_t>& inds) {
    indices_.insert(indices_.end(), inds.begin(), inds.end());
}

void ofMesh::addIndices(const uint32_t* inds, size_t count) {
    indices_.reserve(indices_.size() + count);
    for (size_t i = 0; i < count; ++i) {
        indices_.push_back(inds[i]);
    }
}

void ofMesh::addTriangle(uint32_t i1, uint32_t i2, uint32_t i3) {
    indices_.push_back(i1);
    indices_.push_back(i2);
    indices_.push_back(i3);
}

// ============================================================================
// Primitive Mode
// ============================================================================

void ofMesh::setMode(ofPrimitiveMode mode) {
    mode_ = mode;
}

ofPrimitiveMode ofMesh::getMode() const {
    return mode_;
}

// ============================================================================
// Drawing
// ============================================================================

void ofMesh::draw() const {
    if (vertices_.empty()) {
        return;
    }

    // Get current rendering context
    auto& drawList = core::ctx().drawList();
    auto& state = core::ctx().renderState();

    // Map ofPrimitiveMode to render::PrimitiveType
    render::PrimitiveType primType;
    switch (mode_) {
        case OF_PRIMITIVE_TRIANGLES:
            primType = render::PrimitiveType::Triangle;
            break;
        case OF_PRIMITIVE_TRIANGLE_STRIP:
            primType = render::PrimitiveType::TriangleStrip;
            break;
        case OF_PRIMITIVE_LINES:
            primType = render::PrimitiveType::Line;
            break;
        case OF_PRIMITIVE_LINE_STRIP:
            primType = render::PrimitiveType::LineStrip;
            break;
        case OF_PRIMITIVE_POINTS:
            primType = render::PrimitiveType::Point;
            break;
        default:
            primType = render::PrimitiveType::Triangle;
            break;
    }

    // Build vertex data
    const size_t numVerts = vertices_.size();
    const bool hasNorms = normals_.size() == numVerts;
    const bool hasTexCoords = texCoords_.size() == numVerts;
    const bool hasColors = colors_.size() == numVerts;

    std::vector<render::Vertex3D> renderVerts;
    renderVerts.reserve(numVerts);

    // Default values
    const ofVec3f defaultNormal(0, 0, 1);
    const ofVec2f defaultTexCoord(0, 0);
    const ofColor defaultColor = state.color;

    for (size_t i = 0; i < numVerts; ++i) {
        const ofVec3f& v = vertices_[i];
        const ofVec3f& n = hasNorms ? normals_[i] : defaultNormal;
        const ofVec2f& tc = hasTexCoords ? texCoords_[i] : defaultTexCoord;
        const ofColor& c = hasColors ? colors_[i] : defaultColor;

        renderVerts.emplace_back(
            simd_make_float3(v.x, v.y, v.z),
            simd_make_float3(n.x, n.y, n.z),
            simd_make_float2(tc.x, tc.y),
            simd_make_float4(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f)
        );
    }

    // Draw with or without indices
    if (!indices_.empty()) {
        drawList.addMesh3D(renderVerts.data(), renderVerts.size(),
                          indices_.data(), indices_.size(),
                          primType, state.matrix, nullptr);
    } else {
        drawList.addMesh3D(renderVerts.data(), renderVerts.size(),
                          nullptr, 0,
                          primType, state.matrix, nullptr);
    }
}

void ofMesh::drawWireframe() const {
    // Save current mode
    ofPrimitiveMode savedMode = mode_;

    // Temporarily set to lines mode
    const_cast<ofMesh*>(this)->mode_ = OF_PRIMITIVE_LINES;

    // Draw
    draw();

    // Restore mode
    const_cast<ofMesh*>(this)->mode_ = savedMode;
}

void ofMesh::drawVertices() const {
    // Save current mode
    ofPrimitiveMode savedMode = mode_;

    // Temporarily set to points mode
    const_cast<ofMesh*>(this)->mode_ = OF_PRIMITIVE_POINTS;

    // Draw
    draw();

    // Restore mode
    const_cast<ofMesh*>(this)->mode_ = savedMode;
}

void ofMesh::drawFaces() const {
    // Save current mode
    ofPrimitiveMode savedMode = mode_;

    // Temporarily set to triangles mode
    const_cast<ofMesh*>(this)->mode_ = OF_PRIMITIVE_TRIANGLES;

    // Draw
    draw();

    // Restore mode
    const_cast<ofMesh*>(this)->mode_ = savedMode;
}

// ============================================================================
// Data Access
// ============================================================================

std::vector<ofVec3f>& ofMesh::getVertices() {
    return vertices_;
}

const std::vector<ofVec3f>& ofMesh::getVertices() const {
    return vertices_;
}

std::vector<ofVec3f>& ofMesh::getNormals() {
    return normals_;
}

const std::vector<ofVec3f>& ofMesh::getNormals() const {
    return normals_;
}

std::vector<ofVec2f>& ofMesh::getTexCoords() {
    return texCoords_;
}

const std::vector<ofVec2f>& ofMesh::getTexCoords() const {
    return texCoords_;
}

std::vector<ofColor>& ofMesh::getColors() {
    return colors_;
}

const std::vector<ofColor>& ofMesh::getColors() const {
    return colors_;
}

std::vector<uint32_t>& ofMesh::getIndices() {
    return indices_;
}

const std::vector<uint32_t>& ofMesh::getIndices() const {
    return indices_;
}

// ============================================================================
// Utility
// ============================================================================

void ofMesh::clear() {
    vertices_.clear();
    normals_.clear();
    texCoords_.clear();
    colors_.clear();
    indices_.clear();
}

size_t ofMesh::getNumVertices() const {
    return vertices_.size();
}

size_t ofMesh::getNumNormals() const {
    return normals_.size();
}

size_t ofMesh::getNumTexCoords() const {
    return texCoords_.size();
}

size_t ofMesh::getNumColors() const {
    return colors_.size();
}

size_t ofMesh::getNumIndices() const {
    return indices_.size();
}

bool ofMesh::hasIndices() const {
    return !indices_.empty();
}

bool ofMesh::hasNormals() const {
    return !normals_.empty();
}

bool ofMesh::hasTexCoords() const {
    return !texCoords_.empty();
}

bool ofMesh::hasColors() const {
    return !colors_.empty();
}

// ============================================================================
// Mesh Operations
// ============================================================================

void ofMesh::mergeDuplicateVertices() {
    if (vertices_.empty()) {
        return;
    }

    // Hash function for ofVec3f (position comparison)
    struct Vec3Hash {
        size_t operator()(const ofVec3f& v) const {
            // Simple hash combining x, y, z
            size_t h1 = std::hash<float>{}(v.x);
            size_t h2 = std::hash<float>{}(v.y);
            size_t h3 = std::hash<float>{}(v.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };

    struct Vec3Equal {
        bool operator()(const ofVec3f& a, const ofVec3f& b) const {
            const float epsilon = 1e-6f;
            return std::abs(a.x - b.x) < epsilon &&
                   std::abs(a.y - b.y) < epsilon &&
                   std::abs(a.z - b.z) < epsilon;
        }
    };

    std::unordered_map<ofVec3f, uint32_t, Vec3Hash, Vec3Equal> uniqueVertices;
    std::vector<uint32_t> remapTable(vertices_.size());

    std::vector<ofVec3f> newVertices;
    std::vector<ofVec3f> newNormals;
    std::vector<ofVec2f> newTexCoords;
    std::vector<ofColor> newColors;

    const bool hasNorms = normals_.size() == vertices_.size();
    const bool hasTexCoords = texCoords_.size() == vertices_.size();
    const bool hasColors = colors_.size() == vertices_.size();

    for (size_t i = 0; i < vertices_.size(); ++i) {
        const ofVec3f& v = vertices_[i];

        auto it = uniqueVertices.find(v);
        if (it != uniqueVertices.end()) {
            // Vertex already exists
            remapTable[i] = it->second;
        } else {
            // New unique vertex
            uint32_t newIndex = static_cast<uint32_t>(newVertices.size());
            uniqueVertices[v] = newIndex;
            remapTable[i] = newIndex;

            newVertices.push_back(v);
            if (hasNorms) newNormals.push_back(normals_[i]);
            if (hasTexCoords) newTexCoords.push_back(texCoords_[i]);
            if (hasColors) newColors.push_back(colors_[i]);
        }
    }

    // Update indices
    if (!indices_.empty()) {
        for (uint32_t& idx : indices_) {
            if (idx < remapTable.size()) {
                idx = remapTable[idx];
            }
        }
    } else {
        // If no indices existed, create them now
        indices_.clear();
        indices_.reserve(vertices_.size());
        for (size_t i = 0; i < vertices_.size(); ++i) {
            indices_.push_back(remapTable[i]);
        }
    }

    // Replace data
    vertices_ = std::move(newVertices);
    if (hasNorms) normals_ = std::move(newNormals);
    if (hasTexCoords) texCoords_ = std::move(newTexCoords);
    if (hasColors) colors_ = std::move(newColors);
}

void ofMesh::setupIndicesAuto() {
    indices_.clear();
    indices_.reserve(vertices_.size());
    for (uint32_t i = 0; i < static_cast<uint32_t>(vertices_.size()); ++i) {
        indices_.push_back(i);
    }
}

void ofMesh::smoothNormals() {
    if (vertices_.empty()) {
        return;
    }

    // Initialize normals to zero
    normals_.clear();
    normals_.resize(vertices_.size(), ofVec3f(0, 0, 0));

    // Accumulate face normals
    if (!indices_.empty()) {
        // Indexed mesh
        for (size_t i = 0; i + 2 < indices_.size(); i += 3) {
            uint32_t i0 = indices_[i];
            uint32_t i1 = indices_[i + 1];
            uint32_t i2 = indices_[i + 2];

            if (i0 >= vertices_.size() || i1 >= vertices_.size() || i2 >= vertices_.size()) {
                continue;
            }

            const ofVec3f& v0 = vertices_[i0];
            const ofVec3f& v1 = vertices_[i1];
            const ofVec3f& v2 = vertices_[i2];

            ofVec3f edge1 = v1 - v0;
            ofVec3f edge2 = v2 - v0;
            ofVec3f faceNormal = edge1.cross(edge2);

            normals_[i0] += faceNormal;
            normals_[i1] += faceNormal;
            normals_[i2] += faceNormal;
        }
    } else {
        // Non-indexed mesh
        for (size_t i = 0; i + 2 < vertices_.size(); i += 3) {
            const ofVec3f& v0 = vertices_[i];
            const ofVec3f& v1 = vertices_[i + 1];
            const ofVec3f& v2 = vertices_[i + 2];

            ofVec3f edge1 = v1 - v0;
            ofVec3f edge2 = v2 - v0;
            ofVec3f faceNormal = edge1.cross(edge2);

            normals_[i] += faceNormal;
            normals_[i + 1] += faceNormal;
            normals_[i + 2] += faceNormal;
        }
    }

    // Normalize all normals
    for (ofVec3f& n : normals_) {
        n.normalize();
    }
}

void ofMesh::flatNormals() {
    if (vertices_.empty()) {
        return;
    }

    std::vector<ofVec3f> newVertices;
    std::vector<ofVec3f> newNormals;
    std::vector<ofVec2f> newTexCoords;
    std::vector<ofColor> newColors;

    const bool hasTexCoords = texCoords_.size() == vertices_.size();
    const bool hasColors = colors_.size() == vertices_.size();

    if (!indices_.empty()) {
        // Indexed mesh - create unique vertices per triangle
        for (size_t i = 0; i + 2 < indices_.size(); i += 3) {
            uint32_t i0 = indices_[i];
            uint32_t i1 = indices_[i + 1];
            uint32_t i2 = indices_[i + 2];

            if (i0 >= vertices_.size() || i1 >= vertices_.size() || i2 >= vertices_.size()) {
                continue;
            }

            const ofVec3f& v0 = vertices_[i0];
            const ofVec3f& v1 = vertices_[i1];
            const ofVec3f& v2 = vertices_[i2];

            ofVec3f edge1 = v1 - v0;
            ofVec3f edge2 = v2 - v0;
            ofVec3f faceNormal = edge1.cross(edge2).normalize();

            newVertices.push_back(v0);
            newVertices.push_back(v1);
            newVertices.push_back(v2);

            newNormals.push_back(faceNormal);
            newNormals.push_back(faceNormal);
            newNormals.push_back(faceNormal);

            if (hasTexCoords) {
                newTexCoords.push_back(texCoords_[i0]);
                newTexCoords.push_back(texCoords_[i1]);
                newTexCoords.push_back(texCoords_[i2]);
            }

            if (hasColors) {
                newColors.push_back(colors_[i0]);
                newColors.push_back(colors_[i1]);
                newColors.push_back(colors_[i2]);
            }
        }

        // Clear indices (now using direct vertex order)
        indices_.clear();
    } else {
        // Non-indexed mesh
        for (size_t i = 0; i + 2 < vertices_.size(); i += 3) {
            const ofVec3f& v0 = vertices_[i];
            const ofVec3f& v1 = vertices_[i + 1];
            const ofVec3f& v2 = vertices_[i + 2];

            ofVec3f edge1 = v1 - v0;
            ofVec3f edge2 = v2 - v0;
            ofVec3f faceNormal = edge1.cross(edge2).normalize();

            newVertices.push_back(v0);
            newVertices.push_back(v1);
            newVertices.push_back(v2);

            newNormals.push_back(faceNormal);
            newNormals.push_back(faceNormal);
            newNormals.push_back(faceNormal);

            if (hasTexCoords) {
                newTexCoords.push_back(texCoords_[i]);
                newTexCoords.push_back(texCoords_[i + 1]);
                newTexCoords.push_back(texCoords_[i + 2]);
            }

            if (hasColors) {
                newColors.push_back(colors_[i]);
                newColors.push_back(colors_[i + 1]);
                newColors.push_back(colors_[i + 2]);
            }
        }
    }

    vertices_ = std::move(newVertices);
    normals_ = std::move(newNormals);
    if (hasTexCoords) texCoords_ = std::move(newTexCoords);
    if (hasColors) colors_ = std::move(newColors);
}

void ofMesh::append(const ofMesh& mesh) {
    const uint32_t vertexOffset = static_cast<uint32_t>(vertices_.size());

    // Append vertices
    vertices_.insert(vertices_.end(), mesh.vertices_.begin(), mesh.vertices_.end());

    // Append normals
    if (!mesh.normals_.empty()) {
        normals_.insert(normals_.end(), mesh.normals_.begin(), mesh.normals_.end());
    }

    // Append texcoords
    if (!mesh.texCoords_.empty()) {
        texCoords_.insert(texCoords_.end(), mesh.texCoords_.begin(), mesh.texCoords_.end());
    }

    // Append colors
    if (!mesh.colors_.empty()) {
        colors_.insert(colors_.end(), mesh.colors_.begin(), mesh.colors_.end());
    }

    // Append indices (with offset)
    if (!mesh.indices_.empty()) {
        indices_.reserve(indices_.size() + mesh.indices_.size());
        for (uint32_t idx : mesh.indices_) {
            indices_.push_back(idx + vertexOffset);
        }
    }
}

// ============================================================================
// Static Mesh Generators
// ============================================================================

ofMesh ofMesh::plane(float width, float height, int columns, int rows) {
    ofMesh mesh(OF_PRIMITIVE_TRIANGLES);

    const float halfW = width * 0.5f;
    const float halfH = height * 0.5f;

    // Generate vertices
    for (int y = 0; y <= rows; ++y) {
        for (int x = 0; x <= columns; ++x) {
            float u = static_cast<float>(x) / columns;
            float v = static_cast<float>(y) / rows;

            float px = -halfW + u * width;
            float py = -halfH + v * height;

            mesh.addVertex(ofVec3f(px, py, 0));
            mesh.addNormal(ofVec3f(0, 0, 1));
            mesh.addTexCoord(ofVec2f(u, v));
        }
    }

    // Generate indices
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < columns; ++x) {
            uint32_t i0 = y * (columns + 1) + x;
            uint32_t i1 = i0 + 1;
            uint32_t i2 = i0 + (columns + 1);
            uint32_t i3 = i2 + 1;

            mesh.addTriangle(i0, i2, i1);
            mesh.addTriangle(i1, i2, i3);
        }
    }

    return mesh;
}

ofMesh ofMesh::sphere(float radius, int resolution) {
    ofMesh mesh(OF_PRIMITIVE_TRIANGLES);

    const int latSegments = resolution;
    const int lonSegments = resolution * 2;

    // Generate vertices
    for (int lat = 0; lat <= latSegments; ++lat) {
        float theta = static_cast<float>(lat) * M_PI / latSegments;
        float sinTheta = std::sin(theta);
        float cosTheta = std::cos(theta);

        for (int lon = 0; lon <= lonSegments; ++lon) {
            float phi = static_cast<float>(lon) * 2.0f * M_PI / lonSegments;
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            float x = cosPhi * sinTheta;
            float y = cosTheta;
            float z = sinPhi * sinTheta;

            mesh.addVertex(ofVec3f(x * radius, y * radius, z * radius));
            mesh.addNormal(ofVec3f(x, y, z));
            mesh.addTexCoord(ofVec2f(static_cast<float>(lon) / lonSegments,
                                     static_cast<float>(lat) / latSegments));
        }
    }

    // Generate indices
    for (int lat = 0; lat < latSegments; ++lat) {
        for (int lon = 0; lon < lonSegments; ++lon) {
            uint32_t i0 = lat * (lonSegments + 1) + lon;
            uint32_t i1 = i0 + lonSegments + 1;

            mesh.addTriangle(i0, i1, i0 + 1);
            mesh.addTriangle(i0 + 1, i1, i1 + 1);
        }
    }

    return mesh;
}

ofMesh ofMesh::box(float width, float height, float depth) {
    ofMesh mesh(OF_PRIMITIVE_TRIANGLES);

    const float halfW = width * 0.5f;
    const float halfH = height * 0.5f;
    const float halfD = depth * 0.5f;

    // 6 faces, 4 vertices each = 24 vertices total
    // Front face (+Z)
    mesh.addVertex(ofVec3f(-halfW, -halfH,  halfD));
    mesh.addVertex(ofVec3f( halfW, -halfH,  halfD));
    mesh.addVertex(ofVec3f( halfW,  halfH,  halfD));
    mesh.addVertex(ofVec3f(-halfW,  halfH,  halfD));
    for (int i = 0; i < 4; ++i) mesh.addNormal(ofVec3f(0, 0, 1));
    mesh.addTexCoord(ofVec2f(0, 1));
    mesh.addTexCoord(ofVec2f(1, 1));
    mesh.addTexCoord(ofVec2f(1, 0));
    mesh.addTexCoord(ofVec2f(0, 0));

    // Back face (-Z)
    mesh.addVertex(ofVec3f( halfW, -halfH, -halfD));
    mesh.addVertex(ofVec3f(-halfW, -halfH, -halfD));
    mesh.addVertex(ofVec3f(-halfW,  halfH, -halfD));
    mesh.addVertex(ofVec3f( halfW,  halfH, -halfD));
    for (int i = 0; i < 4; ++i) mesh.addNormal(ofVec3f(0, 0, -1));
    mesh.addTexCoord(ofVec2f(0, 1));
    mesh.addTexCoord(ofVec2f(1, 1));
    mesh.addTexCoord(ofVec2f(1, 0));
    mesh.addTexCoord(ofVec2f(0, 0));

    // Right face (+X)
    mesh.addVertex(ofVec3f( halfW, -halfH,  halfD));
    mesh.addVertex(ofVec3f( halfW, -halfH, -halfD));
    mesh.addVertex(ofVec3f( halfW,  halfH, -halfD));
    mesh.addVertex(ofVec3f( halfW,  halfH,  halfD));
    for (int i = 0; i < 4; ++i) mesh.addNormal(ofVec3f(1, 0, 0));
    mesh.addTexCoord(ofVec2f(0, 1));
    mesh.addTexCoord(ofVec2f(1, 1));
    mesh.addTexCoord(ofVec2f(1, 0));
    mesh.addTexCoord(ofVec2f(0, 0));

    // Left face (-X)
    mesh.addVertex(ofVec3f(-halfW, -halfH, -halfD));
    mesh.addVertex(ofVec3f(-halfW, -halfH,  halfD));
    mesh.addVertex(ofVec3f(-halfW,  halfH,  halfD));
    mesh.addVertex(ofVec3f(-halfW,  halfH, -halfD));
    for (int i = 0; i < 4; ++i) mesh.addNormal(ofVec3f(-1, 0, 0));
    mesh.addTexCoord(ofVec2f(0, 1));
    mesh.addTexCoord(ofVec2f(1, 1));
    mesh.addTexCoord(ofVec2f(1, 0));
    mesh.addTexCoord(ofVec2f(0, 0));

    // Top face (+Y)
    mesh.addVertex(ofVec3f(-halfW,  halfH,  halfD));
    mesh.addVertex(ofVec3f( halfW,  halfH,  halfD));
    mesh.addVertex(ofVec3f( halfW,  halfH, -halfD));
    mesh.addVertex(ofVec3f(-halfW,  halfH, -halfD));
    for (int i = 0; i < 4; ++i) mesh.addNormal(ofVec3f(0, 1, 0));
    mesh.addTexCoord(ofVec2f(0, 1));
    mesh.addTexCoord(ofVec2f(1, 1));
    mesh.addTexCoord(ofVec2f(1, 0));
    mesh.addTexCoord(ofVec2f(0, 0));

    // Bottom face (-Y)
    mesh.addVertex(ofVec3f(-halfW, -halfH, -halfD));
    mesh.addVertex(ofVec3f( halfW, -halfH, -halfD));
    mesh.addVertex(ofVec3f( halfW, -halfH,  halfD));
    mesh.addVertex(ofVec3f(-halfW, -halfH,  halfD));
    for (int i = 0; i < 4; ++i) mesh.addNormal(ofVec3f(0, -1, 0));
    mesh.addTexCoord(ofVec2f(0, 1));
    mesh.addTexCoord(ofVec2f(1, 1));
    mesh.addTexCoord(ofVec2f(1, 0));
    mesh.addTexCoord(ofVec2f(0, 0));

    // Generate indices (6 faces * 2 triangles * 3 indices = 36 indices)
    for (uint32_t face = 0; face < 6; ++face) {
        uint32_t offset = face * 4;
        mesh.addTriangle(offset + 0, offset + 1, offset + 2);
        mesh.addTriangle(offset + 0, offset + 2, offset + 3);
    }

    return mesh;
}

ofMesh ofMesh::cone(float radius, float height, int radiusSegments, int heightSegments) {
    ofMesh mesh(OF_PRIMITIVE_TRIANGLES);

    const float halfH = height * 0.5f;

    // Top vertex (apex)
    mesh.addVertex(ofVec3f(0, halfH, 0));
    mesh.addNormal(ofVec3f(0, 1, 0));
    mesh.addTexCoord(ofVec2f(0.5f, 0));

    // Side vertices
    for (int h = 0; h <= heightSegments; ++h) {
        float t = static_cast<float>(h) / heightSegments;
        float y = halfH - t * height;
        float r = (1.0f - t) * radius;

        for (int seg = 0; seg <= radiusSegments; ++seg) {
            float angle = static_cast<float>(seg) * 2.0f * M_PI / radiusSegments;
            float x = std::cos(angle) * r;
            float z = std::sin(angle) * r;

            mesh.addVertex(ofVec3f(x, y, z));

            // Normal for cone side
            float nx = std::cos(angle);
            float ny = radius / height;
            float nz = std::sin(angle);
            ofVec3f normal(nx, ny, nz);
            normal.normalize();
            mesh.addNormal(normal);

            mesh.addTexCoord(ofVec2f(static_cast<float>(seg) / radiusSegments, t));
        }
    }

    // Generate side indices
    uint32_t topVertex = 0;
    for (int h = 0; h < heightSegments; ++h) {
        for (int seg = 0; seg < radiusSegments; ++seg) {
            uint32_t i0 = 1 + h * (radiusSegments + 1) + seg;
            uint32_t i1 = i0 + 1;
            uint32_t i2 = i0 + (radiusSegments + 1);
            uint32_t i3 = i2 + 1;

            mesh.addTriangle(i0, i2, i1);
            mesh.addTriangle(i1, i2, i3);
        }
    }

    // Base cap
    uint32_t baseCenter = static_cast<uint32_t>(mesh.getNumVertices());
    mesh.addVertex(ofVec3f(0, -halfH, 0));
    mesh.addNormal(ofVec3f(0, -1, 0));
    mesh.addTexCoord(ofVec2f(0.5f, 1.0f));

    for (int seg = 0; seg <= radiusSegments; ++seg) {
        float angle = static_cast<float>(seg) * 2.0f * M_PI / radiusSegments;
        float x = std::cos(angle) * radius;
        float z = std::sin(angle) * radius;

        mesh.addVertex(ofVec3f(x, -halfH, z));
        mesh.addNormal(ofVec3f(0, -1, 0));
        mesh.addTexCoord(ofVec2f(0.5f + 0.5f * std::cos(angle),
                                 0.5f + 0.5f * std::sin(angle)));
    }

    for (int seg = 0; seg < radiusSegments; ++seg) {
        mesh.addTriangle(baseCenter, baseCenter + seg + 1, baseCenter + seg + 2);
    }

    return mesh;
}

ofMesh ofMesh::cylinder(float radius, float height, int radiusSegments, int heightSegments) {
    ofMesh mesh(OF_PRIMITIVE_TRIANGLES);

    const float halfH = height * 0.5f;

    // Side vertices
    for (int h = 0; h <= heightSegments; ++h) {
        float t = static_cast<float>(h) / heightSegments;
        float y = halfH - t * height;

        for (int seg = 0; seg <= radiusSegments; ++seg) {
            float angle = static_cast<float>(seg) * 2.0f * M_PI / radiusSegments;
            float x = std::cos(angle) * radius;
            float z = std::sin(angle) * radius;

            mesh.addVertex(ofVec3f(x, y, z));

            ofVec3f normal(x, 0, z);
            normal.normalize();
            mesh.addNormal(normal);

            mesh.addTexCoord(ofVec2f(static_cast<float>(seg) / radiusSegments, t));
        }
    }

    // Generate side indices
    for (int h = 0; h < heightSegments; ++h) {
        for (int seg = 0; seg < radiusSegments; ++seg) {
            uint32_t i0 = h * (radiusSegments + 1) + seg;
            uint32_t i1 = i0 + 1;
            uint32_t i2 = i0 + (radiusSegments + 1);
            uint32_t i3 = i2 + 1;

            mesh.addTriangle(i0, i2, i1);
            mesh.addTriangle(i1, i2, i3);
        }
    }

    // Top cap
    uint32_t topCenter = static_cast<uint32_t>(mesh.getNumVertices());
    mesh.addVertex(ofVec3f(0, halfH, 0));
    mesh.addNormal(ofVec3f(0, 1, 0));
    mesh.addTexCoord(ofVec2f(0.5f, 0.5f));

    for (int seg = 0; seg <= radiusSegments; ++seg) {
        float angle = static_cast<float>(seg) * 2.0f * M_PI / radiusSegments;
        float x = std::cos(angle) * radius;
        float z = std::sin(angle) * radius;

        mesh.addVertex(ofVec3f(x, halfH, z));
        mesh.addNormal(ofVec3f(0, 1, 0));
        mesh.addTexCoord(ofVec2f(0.5f + 0.5f * std::cos(angle),
                                 0.5f + 0.5f * std::sin(angle)));
    }

    for (int seg = 0; seg < radiusSegments; ++seg) {
        mesh.addTriangle(topCenter, topCenter + seg + 1, topCenter + seg + 2);
    }

    // Bottom cap
    uint32_t bottomCenter = static_cast<uint32_t>(mesh.getNumVertices());
    mesh.addVertex(ofVec3f(0, -halfH, 0));
    mesh.addNormal(ofVec3f(0, -1, 0));
    mesh.addTexCoord(ofVec2f(0.5f, 0.5f));

    for (int seg = 0; seg <= radiusSegments; ++seg) {
        float angle = static_cast<float>(seg) * 2.0f * M_PI / radiusSegments;
        float x = std::cos(angle) * radius;
        float z = std::sin(angle) * radius;

        mesh.addVertex(ofVec3f(x, -halfH, z));
        mesh.addNormal(ofVec3f(0, -1, 0));
        mesh.addTexCoord(ofVec2f(0.5f + 0.5f * std::cos(angle),
                                 0.5f + 0.5f * std::sin(angle)));
    }

    for (int seg = 0; seg < radiusSegments; ++seg) {
        mesh.addTriangle(bottomCenter, bottomCenter + seg + 2, bottomCenter + seg + 1);
    }

    return mesh;
}

ofMesh ofMesh::icosphere(float radius, int subdivisions) {
    // Start with an icosahedron (20-sided polyhedron)
    ofMesh mesh(OF_PRIMITIVE_TRIANGLES);

    const float t = (1.0f + std::sqrt(5.0f)) / 2.0f;

    // 12 vertices of icosahedron
    std::vector<ofVec3f> verts = {
        ofVec3f(-1,  t,  0), ofVec3f( 1,  t,  0), ofVec3f(-1, -t,  0), ofVec3f( 1, -t,  0),
        ofVec3f( 0, -1,  t), ofVec3f( 0,  1,  t), ofVec3f( 0, -1, -t), ofVec3f( 0,  1, -t),
        ofVec3f( t,  0, -1), ofVec3f( t,  0,  1), ofVec3f(-t,  0, -1), ofVec3f(-t,  0,  1)
    };

    // Normalize to unit sphere
    for (auto& v : verts) {
        v.normalize();
        v *= radius;
    }

    // 20 triangular faces of icosahedron
    std::vector<std::array<uint32_t, 3>> faces = {
        {0, 11, 5}, {0, 5, 1}, {0, 1, 7}, {0, 7, 10}, {0, 10, 11},
        {1, 5, 9}, {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
        {3, 9, 4}, {3, 4, 2}, {3, 2, 6}, {3, 6, 8}, {3, 8, 9},
        {4, 9, 5}, {2, 4, 11}, {6, 2, 10}, {8, 6, 7}, {9, 8, 1}
    };

    // Subdivide
    for (int iter = 0; iter < subdivisions; ++iter) {
        std::vector<std::array<uint32_t, 3>> newFaces;
        std::unordered_map<uint64_t, uint32_t> midpointCache;

        auto getMidpoint = [&](uint32_t i1, uint32_t i2) -> uint32_t {
            uint64_t key = (static_cast<uint64_t>(std::min(i1, i2)) << 32) | std::max(i1, i2);
            auto it = midpointCache.find(key);
            if (it != midpointCache.end()) {
                return it->second;
            }

            ofVec3f mid = (verts[i1] + verts[i2]) * 0.5f;
            mid.normalize();
            mid *= radius;

            uint32_t newIdx = static_cast<uint32_t>(verts.size());
            verts.push_back(mid);
            midpointCache[key] = newIdx;
            return newIdx;
        };

        for (const auto& face : faces) {
            uint32_t v0 = face[0];
            uint32_t v1 = face[1];
            uint32_t v2 = face[2];

            uint32_t m01 = getMidpoint(v0, v1);
            uint32_t m12 = getMidpoint(v1, v2);
            uint32_t m20 = getMidpoint(v2, v0);

            newFaces.push_back({v0, m01, m20});
            newFaces.push_back({v1, m12, m01});
            newFaces.push_back({v2, m20, m12});
            newFaces.push_back({m01, m12, m20});
        }

        faces = std::move(newFaces);
    }

    // Build mesh
    for (const auto& v : verts) {
        mesh.addVertex(v);
        ofVec3f normal = v;
        normal.normalize();
        mesh.addNormal(normal);

        // Spherical UV mapping
        float u = 0.5f + std::atan2(normal.z, normal.x) / (2.0f * M_PI);
        float v = 0.5f - std::asin(normal.y) / M_PI;
        mesh.addTexCoord(ofVec2f(u, v));
    }

    for (const auto& face : faces) {
        mesh.addTriangle(face[0], face[1], face[2]);
    }

    return mesh;
}

// ============================================================================
// File I/O (PLY / OBJ) - Stub for now
// ============================================================================

bool ofMesh::load(const std::string& filename) {
    // TODO: Implement PLY/OBJ loading
    return false;
}

bool ofMesh::save(const std::string& filename) const {
    // TODO: Implement PLY/OBJ saving
    return false;
}

} // namespace oflike
