// Test for DrawList batching optimization (Phase 18.1)

#include "render/DrawList.h"
#include "render/RenderTypes.h"
#include <iostream>
#include <cassert>

using namespace render;

void printTestResult(const char* testName, bool passed) {
    std::cout << "[" << (passed ? "PASS" : "FAIL") << "] " << testName << std::endl;
}

// Helper to create a simple 2D transform matrix
simd_float4x4 makeTransform2D(float x, float y) {
    simd_float4x4 m = matrix_identity_float4x4;
    m.columns[3].x = x;
    m.columns[3].y = y;
    return m;
}

// Test 1: Basic batching of consecutive similar 2D draws
void testBasic2DBatching() {
    DrawList list;

    // Add 3 consecutive triangles with the same state
    simd_float4x4 transform = makeTransform2D(0, 0);

    for (int i = 0; i < 3; i++) {
        // Add 3 vertices for each triangle
        Vertex2D v1, v2, v3;
        v1.position = simd_make_float2(0, 0);
        v2.position = simd_make_float2(1, 0);
        v3.position = simd_make_float2(0.5f, 1);

        uint32_t offset = list.addVertex2D(v1);
        list.addVertex2D(v2);
        list.addVertex2D(v3);

        DrawCommand2D cmd;
        cmd.vertexOffset = offset;
        cmd.vertexCount = 3;
        cmd.primitiveType = PrimitiveType::Triangle;
        cmd.blendMode = BlendMode::Alpha;
        cmd.texture = nullptr;
        cmd.transform = transform;

        list.addCommand(cmd);
    }

    size_t originalCount = list.getCommandCount();

    // Optimize - should batch all 3 into 1
    list.optimize();

    size_t optimizedCount = list.getCommandCount();
    size_t batchCount = list.getBatchCount();

    bool passed = (originalCount == 3) && (optimizedCount == 1) && (batchCount == 2);
    printTestResult("Basic 2D Batching", passed);

    if (passed) {
        const auto& commands = list.getCommands();
        const DrawCommand2D& merged = commands[0].draw2D;
        assert(merged.vertexCount == 9); // 3 triangles * 3 vertices
        std::cout << "  - Batched 3 commands into 1 (9 vertices total)" << std::endl;
    }
}

// Test 2: Batching stops when state changes
void testStateChangeBoundary() {
    DrawList list;

    simd_float4x4 transform = makeTransform2D(0, 0);

    // Add 2 triangles with same state
    for (int i = 0; i < 2; i++) {
        Vertex2D v1, v2, v3;
        v1.position = simd_make_float2(0, 0);
        v2.position = simd_make_float2(1, 0);
        v3.position = simd_make_float2(0.5f, 1);

        uint32_t offset = list.addVertex2D(v1);
        list.addVertex2D(v2);
        list.addVertex2D(v3);

        DrawCommand2D cmd;
        cmd.vertexOffset = offset;
        cmd.vertexCount = 3;
        cmd.primitiveType = PrimitiveType::Triangle;
        cmd.blendMode = BlendMode::Alpha;
        cmd.texture = nullptr;
        cmd.transform = transform;

        list.addCommand(cmd);
    }

    // Add a triangle with different blend mode
    {
        Vertex2D v1, v2, v3;
        v1.position = simd_make_float2(0, 0);
        v2.position = simd_make_float2(1, 0);
        v3.position = simd_make_float2(0.5f, 1);

        uint32_t offset = list.addVertex2D(v1);
        list.addVertex2D(v2);
        list.addVertex2D(v3);

        DrawCommand2D cmd;
        cmd.vertexOffset = offset;
        cmd.vertexCount = 3;
        cmd.primitiveType = PrimitiveType::Triangle;
        cmd.blendMode = BlendMode::Add; // Different blend mode!
        cmd.texture = nullptr;
        cmd.transform = transform;

        list.addCommand(cmd);
    }

    size_t originalCount = list.getCommandCount();
    list.optimize();
    size_t optimizedCount = list.getCommandCount();

    // Should batch first 2, but not the 3rd
    bool passed = (originalCount == 3) && (optimizedCount == 2);
    printTestResult("State Change Boundary", passed);

    if (passed) {
        std::cout << "  - Batched 2 commands, kept 1 separate (different blend mode)" << std::endl;
    }
}

// Test 3: Batching with indices
void testIndexedBatching() {
    DrawList list;

    simd_float4x4 transform = makeTransform2D(0, 0);

    // Add 2 indexed quads
    for (int i = 0; i < 2; i++) {
        // Add 4 vertices for quad
        Vertex2D v1, v2, v3, v4;
        v1.position = simd_make_float2(0, 0);
        v2.position = simd_make_float2(1, 0);
        v3.position = simd_make_float2(1, 1);
        v4.position = simd_make_float2(0, 1);

        uint32_t vtxOffset = list.addVertex2D(v1);
        list.addVertex2D(v2);
        list.addVertex2D(v3);
        list.addVertex2D(v4);

        // Add 6 indices for 2 triangles
        uint32_t idxOffset = list.addIndex(vtxOffset + 0);
        list.addIndex(vtxOffset + 1);
        list.addIndex(vtxOffset + 2);
        list.addIndex(vtxOffset + 0);
        list.addIndex(vtxOffset + 2);
        list.addIndex(vtxOffset + 3);

        DrawCommand2D cmd;
        cmd.vertexOffset = vtxOffset;
        cmd.vertexCount = 4;
        cmd.indexOffset = idxOffset;
        cmd.indexCount = 6;
        cmd.primitiveType = PrimitiveType::Triangle;
        cmd.blendMode = BlendMode::Alpha;
        cmd.texture = nullptr;
        cmd.transform = transform;

        list.addCommand(cmd);
    }

    size_t originalCount = list.getCommandCount();
    list.optimize();
    size_t optimizedCount = list.getCommandCount();

    bool passed = (originalCount == 2) && (optimizedCount == 1);
    printTestResult("Indexed Batching", passed);

    if (passed) {
        const auto& commands = list.getCommands();
        const DrawCommand2D& merged = commands[0].draw2D;
        assert(merged.vertexCount == 8); // 2 quads * 4 vertices
        assert(merged.indexCount == 12); // 2 quads * 6 indices
        std::cout << "  - Batched 2 indexed quads into 1 (8 vertices, 12 indices)" << std::endl;
    }
}

// Test 4: Command sorting
void testCommandSorting() {
    DrawList list;

    simd_float4x4 transform = makeTransform2D(0, 0);

    // Create fake texture pointers
    void* texture1 = (void*)0x1000;
    void* texture2 = (void*)0x2000;
    void* texture3 = nullptr;

    // Add commands with textures in mixed order
    for (int i = 0; i < 3; i++) {
        void* tex = (i == 0) ? texture2 : (i == 1) ? texture1 : texture3;

        Vertex2D v;
        v.position = simd_make_float2(0, 0);
        uint32_t offset = list.addVertex2D(v);

        DrawCommand2D cmd;
        cmd.vertexOffset = offset;
        cmd.vertexCount = 1;
        cmd.primitiveType = PrimitiveType::Triangle;
        cmd.blendMode = BlendMode::Alpha;
        cmd.texture = tex;
        cmd.transform = transform;

        list.addCommand(cmd);
    }

    // Sort commands
    list.sortCommands();

    const auto& commands = list.getCommands();

    // After sorting, should be ordered: nullptr < 0x1000 < 0x2000
    bool passed = (commands[0].draw2D.texture == nullptr) &&
                  (commands[1].draw2D.texture == texture1) &&
                  (commands[2].draw2D.texture == texture2);

    printTestResult("Command Sorting", passed);

    if (passed) {
        std::cout << "  - Commands sorted by texture (nullptr, 0x1000, 0x2000)" << std::endl;
    }
}

// Test 5: Sort then batch
void testSortThenBatch() {
    DrawList list;

    simd_float4x4 transform = makeTransform2D(0, 0);

    void* texture1 = (void*)0x1000;
    void* texture2 = (void*)0x2000;

    // Add commands with same texture consecutively (can be batched before sort)
    // Add 2x tex1, then 2x tex2
    // This demonstrates that consecutive commands with same state batch well

    for (int i = 0; i < 4; i++) {
        void* tex = (i < 2) ? texture1 : texture2;

        Vertex2D v;
        v.position = simd_make_float2(0, 0);
        uint32_t offset = list.addVertex2D(v);

        DrawCommand2D cmd;
        cmd.vertexOffset = offset;
        cmd.vertexCount = 1;
        cmd.primitiveType = PrimitiveType::Triangle;
        cmd.blendMode = BlendMode::Alpha;
        cmd.texture = tex;
        cmd.transform = transform;

        list.addCommand(cmd);
    }

    size_t originalCount = list.getCommandCount();

    // Batch (should merge tex1+tex1 and tex2+tex2)
    list.optimize();

    size_t optimizedCount = list.getCommandCount();

    // Note: Sorting alone doesn't enable batching unless vertices are consecutive.
    // This test shows that consecutive commands with matching state batch well.
    bool passed = (originalCount == 4) && (optimizedCount == 2);
    printTestResult("Consecutive State Batching", passed);

    if (passed) {
        std::cout << "  - Batched 4 commands (2 tex1, 2 tex2) into 2" << std::endl;
    }
}

// Test 6: Empty list
void testEmptyList() {
    DrawList list;

    list.optimize();
    list.sortCommands();

    bool passed = (list.getCommandCount() == 0) && (list.getBatchCount() == 0);
    printTestResult("Empty List", passed);
}

// Test 7: Non-batchable commands (state commands)
void testStateCommands() {
    DrawList list;

    // Add viewport command
    SetViewportCommand vp;
    vp.viewport = Rect(0, 0, 800, 600);
    list.addCommand(vp);

    // Add draw command
    Vertex2D v;
    v.position = simd_make_float2(0, 0);
    uint32_t offset = list.addVertex2D(v);

    DrawCommand2D cmd;
    cmd.vertexOffset = offset;
    cmd.vertexCount = 1;
    cmd.primitiveType = PrimitiveType::Triangle;
    cmd.transform = makeTransform2D(0, 0);
    list.addCommand(cmd);

    size_t originalCount = list.getCommandCount();
    list.optimize();
    size_t optimizedCount = list.getCommandCount();

    bool passed = (originalCount == 2) && (optimizedCount == 2);
    printTestResult("State Commands", passed);

    if (passed) {
        std::cout << "  - State commands preserved (viewport + draw)" << std::endl;
    }
}

int main() {
    std::cout << "\n=== DrawList Batching Optimization Tests (Phase 18.1) ===\n" << std::endl;

    testBasic2DBatching();
    testStateChangeBoundary();
    testIndexedBatching();
    testCommandSorting();
    testSortThenBatch();
    testEmptyList();
    testStateCommands();

    std::cout << "\n=== All tests completed ===\n" << std::endl;

    return 0;
}
