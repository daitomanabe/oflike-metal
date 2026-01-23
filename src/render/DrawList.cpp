#include "DrawList.h"

namespace render {

// ============================================================================
// Constructor / Destructor
// ============================================================================

DrawList::DrawList() {
    // Reserve reasonable default capacity to avoid frequent reallocations
    commands_.reserve(128);
    vertices2D_.reserve(1024);
    vertices3D_.reserve(512);
    indices_.reserve(2048);
}

DrawList::~DrawList() {
    // Default destructor is sufficient (std::vector cleans up automatically)
}

// ============================================================================
// Command Management
// ============================================================================

void DrawList::addCommand(const DrawCommand2D& cmd) {
    commands_.emplace_back(cmd);
}

void DrawList::addCommand(const DrawCommand3D& cmd) {
    commands_.emplace_back(cmd);
}

void DrawList::addCommand(const SetViewportCommand& cmd) {
    commands_.emplace_back(cmd);
}

void DrawList::addCommand(const SetScissorCommand& cmd) {
    commands_.emplace_back(cmd);
}

void DrawList::addCommand(const SetClearCommand& cmd) {
    commands_.emplace_back(cmd);
}

void DrawList::addCommand(const SetRenderTargetCommand& cmd) {
    commands_.emplace_back(cmd);
}

// ============================================================================
// Vertex Management (2D)
// ============================================================================

uint32_t DrawList::addVertex2D(const Vertex2D& vertex) {
    uint32_t offset = static_cast<uint32_t>(vertices2D_.size());
    vertices2D_.push_back(vertex);
    return offset;
}

uint32_t DrawList::addVertices2D(const Vertex2D* vertices, size_t count) {
    if (count == 0 || vertices == nullptr) {
        return static_cast<uint32_t>(vertices2D_.size());
    }

    uint32_t offset = static_cast<uint32_t>(vertices2D_.size());
    vertices2D_.insert(vertices2D_.end(), vertices, vertices + count);
    return offset;
}

uint32_t DrawList::addVertices2D(const std::vector<Vertex2D>& vertices) {
    if (vertices.empty()) {
        return static_cast<uint32_t>(vertices2D_.size());
    }

    uint32_t offset = static_cast<uint32_t>(vertices2D_.size());
    vertices2D_.insert(vertices2D_.end(), vertices.begin(), vertices.end());
    return offset;
}

// ============================================================================
// Vertex Management (3D)
// ============================================================================

uint32_t DrawList::addVertex3D(const Vertex3D& vertex) {
    uint32_t offset = static_cast<uint32_t>(vertices3D_.size());
    vertices3D_.push_back(vertex);
    return offset;
}

uint32_t DrawList::addVertices3D(const Vertex3D* vertices, size_t count) {
    if (count == 0 || vertices == nullptr) {
        return static_cast<uint32_t>(vertices3D_.size());
    }

    uint32_t offset = static_cast<uint32_t>(vertices3D_.size());
    vertices3D_.insert(vertices3D_.end(), vertices, vertices + count);
    return offset;
}

uint32_t DrawList::addVertices3D(const std::vector<Vertex3D>& vertices) {
    if (vertices.empty()) {
        return static_cast<uint32_t>(vertices3D_.size());
    }

    uint32_t offset = static_cast<uint32_t>(vertices3D_.size());
    vertices3D_.insert(vertices3D_.end(), vertices.begin(), vertices.end());
    return offset;
}

// ============================================================================
// Index Management
// ============================================================================

uint32_t DrawList::addIndex(uint32_t index) {
    uint32_t offset = static_cast<uint32_t>(indices_.size());
    indices_.push_back(index);
    return offset;
}

uint32_t DrawList::addIndices(const uint32_t* indices, size_t count) {
    if (count == 0 || indices == nullptr) {
        return static_cast<uint32_t>(indices_.size());
    }

    uint32_t offset = static_cast<uint32_t>(indices_.size());
    indices_.insert(indices_.end(), indices, indices + count);
    return offset;
}

uint32_t DrawList::addIndices(const std::vector<uint32_t>& indices) {
    if (indices.empty()) {
        return static_cast<uint32_t>(indices_.size());
    }

    uint32_t offset = static_cast<uint32_t>(indices_.size());
    indices_.insert(indices_.end(), indices.begin(), indices.end());
    return offset;
}

// ============================================================================
// Lifecycle Management
// ============================================================================

void DrawList::reset() {
    commands_.clear();
    vertices2D_.clear();
    vertices3D_.clear();
    indices_.clear();
}

} // namespace render
