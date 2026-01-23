#include "DrawList.h"
#include <algorithm>

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
    batchCount_ = 0;
    originalCommandCount_ = 0;
}

// ============================================================================
// Optimization (Phase 18.1)
// ============================================================================

bool DrawList::matricesEqual(const simd_float4x4& a, const simd_float4x4& b) const {
    // Compare each column vector
    for (int i = 0; i < 4; i++) {
        if (!simd_equal(a.columns[i], b.columns[i])) {
            return false;
        }
    }
    return true;
}

bool DrawList::matricesEqual3x3(const simd_float3x3& a, const simd_float3x3& b) const {
    // Compare each column vector
    for (int i = 0; i < 3; i++) {
        if (!simd_equal(a.columns[i], b.columns[i])) {
            return false;
        }
    }
    return true;
}

bool DrawList::canBatch2D(const DrawCommand2D& a, const DrawCommand2D& b) const {
    // Can only batch if all render state matches
    if (a.primitiveType != b.primitiveType) return false;
    if (a.blendMode != b.blendMode) return false;
    if (a.texture != b.texture) return false;
    if (!matricesEqual(a.transform, b.transform)) return false;

    // Both must use indices or both must not use indices
    bool aHasIndices = (a.indexCount > 0);
    bool bHasIndices = (b.indexCount > 0);
    if (aHasIndices != bHasIndices) return false;

    // Vertices must be consecutive in the buffer
    if (a.vertexOffset + a.vertexCount != b.vertexOffset) return false;

    // If using indices, they must be consecutive
    if (aHasIndices && (a.indexOffset + a.indexCount != b.indexOffset)) {
        return false;
    }

    return true;
}

bool DrawList::canBatch3D(const DrawCommand3D& a, const DrawCommand3D& b) const {
    // Can only batch if all render state matches
    if (a.primitiveType != b.primitiveType) return false;
    if (a.blendMode != b.blendMode) return false;
    if (a.texture != b.texture) return false;
    if (!matricesEqual(a.modelViewMatrix, b.modelViewMatrix)) return false;
    if (!matricesEqual(a.projectionMatrix, b.projectionMatrix)) return false;
    if (!matricesEqual3x3(a.normalMatrix, b.normalMatrix)) return false;

    // Depth and culling state must match
    if (a.depthTestEnabled != b.depthTestEnabled) return false;
    if (a.depthWriteEnabled != b.depthWriteEnabled) return false;
    if (a.cullBackFace != b.cullBackFace) return false;

    // Both must use indices or both must not use indices
    bool aHasIndices = (a.indexCount > 0);
    bool bHasIndices = (b.indexCount > 0);
    if (aHasIndices != bHasIndices) return false;

    // Vertices must be consecutive in the buffer
    if (a.vertexOffset + a.vertexCount != b.vertexOffset) return false;

    // If using indices, they must be consecutive
    if (aHasIndices && (a.indexOffset + a.indexCount != b.indexOffset)) {
        return false;
    }

    return true;
}

void DrawList::optimize() {
    if (commands_.empty()) {
        return;
    }

    originalCommandCount_ = commands_.size();
    batchCount_ = 0;

    std::vector<DrawCommand> optimized;
    optimized.reserve(commands_.size());

    size_t i = 0;
    while (i < commands_.size()) {
        DrawCommand cmd = commands_[i];

        // Try to batch consecutive draw commands
        if (cmd.type == CommandType::Draw2D) {
            // Look ahead and merge consecutive batchable 2D draws
            size_t j = i + 1;
            while (j < commands_.size() && commands_[j].type == CommandType::Draw2D) {
                if (canBatch2D(cmd.draw2D, commands_[j].draw2D)) {
                    // Merge command j into cmd
                    cmd.draw2D.vertexCount += commands_[j].draw2D.vertexCount;
                    if (cmd.draw2D.indexCount > 0) {
                        cmd.draw2D.indexCount += commands_[j].draw2D.indexCount;
                    }
                    batchCount_++;
                    j++;
                } else {
                    break;
                }
            }
            optimized.push_back(cmd);
            i = j;
        }
        else if (cmd.type == CommandType::Draw3D) {
            // Look ahead and merge consecutive batchable 3D draws
            size_t j = i + 1;
            while (j < commands_.size() && commands_[j].type == CommandType::Draw3D) {
                if (canBatch3D(cmd.draw3D, commands_[j].draw3D)) {
                    // Merge command j into cmd
                    cmd.draw3D.vertexCount += commands_[j].draw3D.vertexCount;
                    if (cmd.draw3D.indexCount > 0) {
                        cmd.draw3D.indexCount += commands_[j].draw3D.indexCount;
                    }
                    batchCount_++;
                    j++;
                } else {
                    break;
                }
            }
            optimized.push_back(cmd);
            i = j;
        }
        else {
            // Non-draw commands are not batched (viewport, scissor, clear, etc.)
            optimized.push_back(cmd);
            i++;
        }
    }

    // Replace commands with optimized version
    commands_ = std::move(optimized);
}

void DrawList::sortCommands() {
    // Sorting is more complex as it needs to preserve ordering of state commands
    // For Phase 18.1, we'll implement a simple sort that groups draw commands
    // between state-change boundaries

    if (commands_.empty()) {
        return;
    }

    // Find ranges between state commands and sort within each range
    size_t rangeStart = 0;

    for (size_t i = 0; i <= commands_.size(); i++) {
        // Check if we hit a state command or end of list
        bool isStateCmd = (i < commands_.size()) &&
                         (commands_[i].type != CommandType::Draw2D &&
                          commands_[i].type != CommandType::Draw3D);

        if (isStateCmd || i == commands_.size()) {
            // Sort the range [rangeStart, i) of draw commands
            if (i > rangeStart) {
                std::sort(commands_.begin() + rangeStart, commands_.begin() + i,
                         [](const DrawCommand& a, const DrawCommand& b) {
                    // Sort by type first (2D before 3D)
                    if (a.type != b.type) {
                        return a.type < b.type;
                    }

                    // Sort 2D commands
                    if (a.type == CommandType::Draw2D) {
                        // Sort by texture (nullptr < valid texture, then by pointer value)
                        if (a.draw2D.texture != b.draw2D.texture) {
                            return a.draw2D.texture < b.draw2D.texture;
                        }
                        // Then by blend mode
                        if (a.draw2D.blendMode != b.draw2D.blendMode) {
                            return a.draw2D.blendMode < b.draw2D.blendMode;
                        }
                        // Then by primitive type
                        return a.draw2D.primitiveType < b.draw2D.primitiveType;
                    }

                    // Sort 3D commands
                    if (a.type == CommandType::Draw3D) {
                        // Sort by texture
                        if (a.draw3D.texture != b.draw3D.texture) {
                            return a.draw3D.texture < b.draw3D.texture;
                        }
                        // Then by depth test (enabled first for early-z)
                        if (a.draw3D.depthTestEnabled != b.draw3D.depthTestEnabled) {
                            return a.draw3D.depthTestEnabled > b.draw3D.depthTestEnabled;
                        }
                        // Then by blend mode
                        if (a.draw3D.blendMode != b.draw3D.blendMode) {
                            return a.draw3D.blendMode < b.draw3D.blendMode;
                        }
                        // Then by primitive type
                        return a.draw3D.primitiveType < b.draw3D.primitiveType;
                    }

                    return false;
                });
            }

            // Move to next range (skip the state command)
            rangeStart = i + 1;
        }
    }
}

} // namespace render
