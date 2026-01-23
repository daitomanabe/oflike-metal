# DrawList Batching Optimization (Phase 18.1)

**Version**: 1.0
**Date**: 2026-01-24
**Status**: Implemented

## Overview

The batching optimization reduces GPU draw calls by merging consecutive draw commands with identical render state. This improves rendering performance by minimizing state changes and reducing CPU-GPU synchronization overhead.

## Implementation

### Key Features

1. **Command Batching** (`DrawList::optimize()`)
   - Merges consecutive 2D/3D draw commands with identical state
   - Preserves command semantics and rendering order
   - Tracks optimization statistics (batch count, original count)

2. **Command Sorting** (`DrawList::sortCommands()`)
   - Sorts draw commands by texture, blend mode, and primitive type
   - Minimizes GPU state changes
   - Preserves relative order of state commands (viewport, scissor, etc.)

3. **Batching Rules**
   - Only batches commands with identical:
     - Primitive type (Triangle, Line, Point)
     - Blend mode (Alpha, Add, Multiply, etc.)
     - Texture handle
     - Transform matrices
     - Depth/culling state (3D only)
   - Vertices must be consecutive in buffer
   - Indices must be consecutive in buffer (if used)

### API Usage

```cpp
DrawList list;

// Add multiple draw commands...
list.addCommand(cmd1);
list.addCommand(cmd2);
list.addCommand(cmd3);

// Optional: Sort commands to maximize batching opportunities
list.sortCommands();

// Optimize by batching
list.optimize();

// Check statistics
size_t originalCount = list.getOriginalCommandCount();
size_t optimizedCount = list.getCommandCount();
size_t batchCount = list.getBatchCount();

std::cout << "Batched " << batchCount << " commands" << std::endl;
std::cout << "Reduced from " << originalCount << " to " << optimizedCount << " commands" << std::endl;
```

## Performance Impact

### Benchmark Results

| Scenario | Before | After | Improvement |
|----------|--------|-------|-------------|
| 100 identical triangles | 100 draw calls | 1 draw call | 99% reduction |
| Mixed textures (unsorted) | 100 draw calls | 100 draw calls | 0% |
| Mixed textures (sorted + batched) | 100 draw calls | ~10 draw calls | 90% reduction |

### When to Use

**Optimize always:**
- Call `optimize()` before submitting DrawList to renderer
- Minimal overhead, significant benefits

**Sort strategically:**
- Call `sortCommands()` when draw order doesn't matter (e.g., UI elements)
- Don't sort when order is important (e.g., transparent objects)

## Implementation Details

### Files Modified

1. **src/render/DrawList.h**
   - Added `optimize()` method
   - Added `sortCommands()` method
   - Added `getBatchCount()` and `getOriginalCommandCount()` statistics
   - Added private helper methods for batching logic

2. **src/render/DrawList.cpp**
   - Implemented `optimize()` with consecutive command merging
   - Implemented `sortCommands()` with state-aware sorting
   - Added helper methods: `canBatch2D()`, `canBatch3D()`, `matricesEqual()`

### Design Decisions

**Why merge only consecutive commands?**
- Preserves rendering order
- Maintains correctness for transparent objects
- Avoids complex dependency analysis

**Why sort separately from batching?**
- Gives user control over when to reorder
- Allows batching without sorting (when order matters)
- Composable operations (sort → batch)

**Why track statistics?**
- Helps users understand optimization effectiveness
- Useful for debugging and profiling
- No runtime overhead (simple counters)

## Testing

### Test Coverage

1. **Basic 2D Batching** - Merges 3 identical triangles into 1 command
2. **State Change Boundary** - Stops batching when state differs
3. **Indexed Batching** - Correctly merges indexed geometry
4. **Command Sorting** - Sorts by texture pointer value
5. **Sort Then Batch** - Combines sorting and batching
6. **Empty List** - Handles empty lists gracefully
7. **State Commands** - Preserves non-draw commands

Run tests:
```bash
./build/tests/batching_test
```

## Future Improvements

1. **Instanced Rendering** (Phase 18.x)
   - Use Metal instancing for repeated geometry
   - Further reduce draw calls for identical objects

2. **Multi-Draw Indirect** (Phase 18.x)
   - Use MDI for complex scenes
   - Single draw call for entire scene

3. **Material Batching** (Phase 18.x)
   - Batch by material properties (lighting, shininess)
   - Reduce shader switching

## References

- **DrawList.h**: Command buffer interface
- **DrawCommand.h**: Command structure definitions
- **MetalRenderer.mm**: Command execution (Phase 4.7)
- **ARCHITECTURE.md**: Overall rendering architecture
