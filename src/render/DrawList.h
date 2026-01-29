#pragma once

#include "DrawCommand.h"
#include "RenderTypes.h"
#include <vector>
#include <cstdint>

namespace render {

// ============================================================================
// DrawList - Command and Vertex Buffer Manager
// ============================================================================

/**
 * DrawList manages rendering commands, vertices, and indices for a frame.
 *
 * Features:
 * - Command buffer storage and management
 * - Vertex buffer allocation and management (2D and 3D)
 * - Index buffer allocation and management
 * - Frame-based lifecycle (reset after frame submission)
 *
 * Usage:
 *   DrawList list;
 *   uint32_t vtxOffset = list.addVertex2D(vertices);
 *   DrawCommand2D cmd;
 *   cmd.vertexOffset = vtxOffset;
 *   cmd.vertexCount = vertices.size();
 *   list.addCommand(cmd);
 *   // ... submit to renderer ...
 *   list.reset();  // Reset for next frame
 */
class DrawList {
public:
    DrawList();
    ~DrawList();

    // ========================================================================
    // Command Management
    // ========================================================================

    /**
     * Add a 2D draw command to the list.
     * @param cmd The 2D draw command to add
     */
    void addCommand(const DrawCommand2D& cmd);

    /**
     * Add a 3D draw command to the list.
     * @param cmd The 3D draw command to add
     */
    void addCommand(const DrawCommand3D& cmd);

    /**
     * Add a viewport command to the list.
     * @param cmd The viewport command to add
     */
    void addCommand(const SetViewportCommand& cmd);

    /**
     * Add a scissor command to the list.
     * @param cmd The scissor command to add
     */
    void addCommand(const SetScissorCommand& cmd);

    /**
     * Add a clear command to the list.
     * @param cmd The clear command to add
     */
    void addCommand(const SetClearCommand& cmd);

    /**
     * Add a render target command to the list.
     * @param cmd The render target command to add
     */
    void addCommand(const SetRenderTargetCommand& cmd);

    /**
     * Add a custom shader command to the list.
     * @param cmd The custom shader command to add
     */
    void addCommand(const SetCustomShaderCommand& cmd);

    /**
     * Get all commands in the list.
     * @return Const reference to the command vector
     */
    const std::vector<DrawCommand>& getCommands() const { return commands_; }

    /**
     * Get the number of commands in the list.
     * @return Number of commands
     */
    size_t getCommandCount() const { return commands_.size(); }

    // ========================================================================
    // Vertex Management (2D)
    // ========================================================================

    /**
     * Add a single 2D vertex to the buffer.
     * @param vertex The vertex to add
     * @return Offset of the added vertex
     */
    uint32_t addVertex2D(const Vertex2D& vertex);

    /**
     * Add multiple 2D vertices to the buffer.
     * @param vertices Pointer to vertex array
     * @param count Number of vertices to add
     * @return Offset of the first added vertex
     */
    uint32_t addVertices2D(const Vertex2D* vertices, size_t count);

    /**
     * Add multiple 2D vertices to the buffer.
     * @param vertices Vector of vertices to add
     * @return Offset of the first added vertex
     */
    uint32_t addVertices2D(const std::vector<Vertex2D>& vertices);

    /**
     * Get all 2D vertices in the buffer.
     * @return Const reference to the 2D vertex vector
     */
    const std::vector<Vertex2D>& getVertices2D() const { return vertices2D_; }

    /**
     * Get the number of 2D vertices in the buffer.
     * @return Number of 2D vertices
     */
    size_t getVertex2DCount() const { return vertices2D_.size(); }

    /**
     * Get raw pointer to 2D vertex data (for GPU upload).
     * @return Pointer to vertex data, or nullptr if empty
     */
    const Vertex2D* getVertex2DData() const {
        return vertices2D_.empty() ? nullptr : vertices2D_.data();
    }

    /**
     * Get size of 2D vertex data in bytes.
     * @return Size in bytes
     */
    size_t getVertex2DDataSize() const {
        return vertices2D_.size() * sizeof(Vertex2D);
    }

    // ========================================================================
    // Vertex Management (3D)
    // ========================================================================

    /**
     * Add a single 3D vertex to the buffer.
     * @param vertex The vertex to add
     * @return Offset of the added vertex
     */
    uint32_t addVertex3D(const Vertex3D& vertex);

    /**
     * Add multiple 3D vertices to the buffer.
     * @param vertices Pointer to vertex array
     * @param count Number of vertices to add
     * @return Offset of the first added vertex
     */
    uint32_t addVertices3D(const Vertex3D* vertices, size_t count);

    /**
     * Add multiple 3D vertices to the buffer.
     * @param vertices Vector of vertices to add
     * @return Offset of the first added vertex
     */
    uint32_t addVertices3D(const std::vector<Vertex3D>& vertices);

    /**
     * Get all 3D vertices in the buffer.
     * @return Const reference to the 3D vertex vector
     */
    const std::vector<Vertex3D>& getVertices3D() const { return vertices3D_; }

    /**
     * Get the number of 3D vertices in the buffer.
     * @return Number of 3D vertices
     */
    size_t getVertex3DCount() const { return vertices3D_.size(); }

    /**
     * Get raw pointer to 3D vertex data (for GPU upload).
     * @return Pointer to vertex data, or nullptr if empty
     */
    const Vertex3D* getVertex3DData() const {
        return vertices3D_.empty() ? nullptr : vertices3D_.data();
    }

    /**
     * Get size of 3D vertex data in bytes.
     * @return Size in bytes
     */
    size_t getVertex3DDataSize() const {
        return vertices3D_.size() * sizeof(Vertex3D);
    }

    // ========================================================================
    // Index Management
    // ========================================================================

    /**
     * Add a single index to the buffer.
     * @param index The index to add
     * @return Offset of the added index
     */
    uint32_t addIndex(uint32_t index);

    /**
     * Add multiple indices to the buffer.
     * @param indices Pointer to index array
     * @param count Number of indices to add
     * @return Offset of the first added index
     */
    uint32_t addIndices(const uint32_t* indices, size_t count);

    /**
     * Add multiple indices to the buffer.
     * @param indices Vector of indices to add
     * @return Offset of the first added index
     */
    uint32_t addIndices(const std::vector<uint32_t>& indices);

    /**
     * Get all indices in the buffer.
     * @return Const reference to the index vector
     */
    const std::vector<uint32_t>& getIndices() const { return indices_; }

    /**
     * Get the number of indices in the buffer.
     * @return Number of indices
     */
    size_t getIndexCount() const { return indices_.size(); }

    /**
     * Get raw pointer to index data (for GPU upload).
     * @return Pointer to index data, or nullptr if empty
     */
    const uint32_t* getIndexData() const {
        return indices_.empty() ? nullptr : indices_.data();
    }

    /**
     * Get size of index data in bytes.
     * @return Size in bytes
     */
    size_t getIndexDataSize() const {
        return indices_.size() * sizeof(uint32_t);
    }

    // ========================================================================
    // Lifecycle Management
    // ========================================================================

    /**
     * Reset the draw list for a new frame.
     * Clears all commands, vertices, and indices.
     * Call this after submitting the frame to the renderer.
     */
    void reset();

    /**
     * Check if the draw list is empty.
     * @return True if no commands, vertices, or indices are present
     */
    bool isEmpty() const {
        return commands_.empty() && vertices2D_.empty() &&
               vertices3D_.empty() && indices_.empty();
    }

    /**
     * Reserve space for commands to avoid reallocation.
     * @param count Expected number of commands
     */
    void reserveCommands(size_t count) {
        commands_.reserve(count);
    }

    /**
     * Reserve space for 2D vertices to avoid reallocation.
     * @param count Expected number of 2D vertices
     */
    void reserveVertices2D(size_t count) {
        vertices2D_.reserve(count);
    }

    /**
     * Reserve space for 3D vertices to avoid reallocation.
     * @param count Expected number of 3D vertices
     */
    void reserveVertices3D(size_t count) {
        vertices3D_.reserve(count);
    }

    /**
     * Reserve space for indices to avoid reallocation.
     * @param count Expected number of indices
     */
    void reserveIndices(size_t count) {
        indices_.reserve(count);
    }

    // ========================================================================
    // Optimization (Phase 18.1)
    // ========================================================================

    /**
     * Optimize the command list by batching consecutive similar commands.
     * This reduces draw calls by merging commands with identical render state.
     *
     * Should be called before submitting to the renderer.
     * Note: This modifies the command list in-place.
     */
    void optimize();

    /**
     * Sort commands to minimize state changes.
     * Groups commands by texture, blend mode, and other state to reduce
     * GPU pipeline stalls.
     *
     * Should be called before submitting to the renderer.
     * Note: This may reorder commands (preserves relative order of state commands).
     */
    void sortCommands();

    /**
     * Get statistics about batching effectiveness.
     * @return Number of commands that were batched together
     */
    size_t getBatchCount() const { return batchCount_; }

    /**
     * Get the number of commands before optimization.
     * @return Original command count
     */
    size_t getOriginalCommandCount() const { return originalCommandCount_; }

private:
    // Command buffer
    std::vector<DrawCommand> commands_;

    // Vertex buffers
    std::vector<Vertex2D> vertices2D_;
    std::vector<Vertex3D> vertices3D_;

    // Index buffer (shared between 2D and 3D)
    std::vector<uint32_t> indices_;

    // Optimization statistics
    size_t batchCount_ = 0;
    size_t originalCommandCount_ = 0;

    // Helper methods for optimization
    bool canBatch2D(const DrawCommand2D& a, const DrawCommand2D& b) const;
    bool canBatch3D(const DrawCommand3D& a, const DrawCommand3D& b) const;
    bool matricesEqual(const simd_float4x4& a, const simd_float4x4& b) const;
    bool matricesEqual3x3(const simd_float3x3& a, const simd_float3x3& b) const;
};

} // namespace render
