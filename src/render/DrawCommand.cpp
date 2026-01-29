#include "DrawCommand.h"
#include <cstring>

namespace render {

// ============================================================================
// DrawCommand Implementation
// ============================================================================

DrawCommand::DrawCommand(const DrawCommand& other)
    : type(other.type) {
    // Copy union data based on type
    switch (type) {
        case CommandType::Draw2D:
            draw2D = other.draw2D;
            break;
        case CommandType::Draw3D:
            draw3D = other.draw3D;
            break;
        case CommandType::SetViewport:
            viewport = other.viewport;
            break;
        case CommandType::SetScissor:
            scissor = other.scissor;
            break;
        case CommandType::Clear:
            clear = other.clear;
            break;
        case CommandType::SetRenderTarget:
            renderTarget = other.renderTarget;
            break;
        case CommandType::SetCustomShader:
            customShader = other.customShader;
            break;
        default:
            break;
    }
}

DrawCommand& DrawCommand::operator=(const DrawCommand& other) {
    if (this != &other) {
        type = other.type;

        // Copy union data based on type
        switch (type) {
            case CommandType::Draw2D:
                draw2D = other.draw2D;
                break;
            case CommandType::Draw3D:
                draw3D = other.draw3D;
                break;
            case CommandType::SetViewport:
                viewport = other.viewport;
                break;
            case CommandType::SetScissor:
                scissor = other.scissor;
                break;
            case CommandType::Clear:
                clear = other.clear;
                break;
            case CommandType::SetRenderTarget:
                renderTarget = other.renderTarget;
                break;
            case CommandType::SetCustomShader:
                customShader = other.customShader;
                break;
            default:
                break;
        }
    }
    return *this;
}

DrawCommand::~DrawCommand() {
    // No dynamic memory to clean up in current implementation
    // Texture handles are managed externally
}

} // namespace render
