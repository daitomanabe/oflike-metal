#include "RenderTypes.h"
#include <Metal/Metal.h>

namespace render {

BlendConfig BlendConfig::forMode(BlendMode mode) {
    BlendConfig config{};

    switch (mode) {
        case BlendMode::Disabled:
            config.blendingEnabled = false;
            config.sourceRGBBlendFactor = MTLBlendFactorOne;
            config.destinationRGBBlendFactor = MTLBlendFactorZero;
            config.rgbBlendOperation = MTLBlendOperationAdd;
            config.sourceAlphaBlendFactor = MTLBlendFactorOne;
            config.destinationAlphaBlendFactor = MTLBlendFactorZero;
            config.alphaBlendOperation = MTLBlendOperationAdd;
            break;

        case BlendMode::Alpha:
            // src.rgb * src.a + dst.rgb * (1 - src.a)
            config.blendingEnabled = true;
            config.sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
            config.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
            config.rgbBlendOperation = MTLBlendOperationAdd;
            config.sourceAlphaBlendFactor = MTLBlendFactorOne;
            config.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
            config.alphaBlendOperation = MTLBlendOperationAdd;
            break;

        case BlendMode::Add:
            // src.rgb + dst.rgb
            config.blendingEnabled = true;
            config.sourceRGBBlendFactor = MTLBlendFactorOne;
            config.destinationRGBBlendFactor = MTLBlendFactorOne;
            config.rgbBlendOperation = MTLBlendOperationAdd;
            config.sourceAlphaBlendFactor = MTLBlendFactorOne;
            config.destinationAlphaBlendFactor = MTLBlendFactorOne;
            config.alphaBlendOperation = MTLBlendOperationAdd;
            break;

        case BlendMode::Subtract:
            // dst.rgb - src.rgb
            config.blendingEnabled = true;
            config.sourceRGBBlendFactor = MTLBlendFactorOne;
            config.destinationRGBBlendFactor = MTLBlendFactorOne;
            config.rgbBlendOperation = MTLBlendOperationReverseSubtract;
            config.sourceAlphaBlendFactor = MTLBlendFactorOne;
            config.destinationAlphaBlendFactor = MTLBlendFactorOne;
            config.alphaBlendOperation = MTLBlendOperationReverseSubtract;
            break;

        case BlendMode::Multiply:
            // src.rgb * dst.rgb
            config.blendingEnabled = true;
            config.sourceRGBBlendFactor = MTLBlendFactorDestinationColor;
            config.destinationRGBBlendFactor = MTLBlendFactorZero;
            config.rgbBlendOperation = MTLBlendOperationAdd;
            config.sourceAlphaBlendFactor = MTLBlendFactorDestinationAlpha;
            config.destinationAlphaBlendFactor = MTLBlendFactorZero;
            config.alphaBlendOperation = MTLBlendOperationAdd;
            break;

        case BlendMode::Screen:
            // 1 - (1 - src.rgb) * (1 - dst.rgb) = src + dst - src * dst
            config.blendingEnabled = true;
            config.sourceRGBBlendFactor = MTLBlendFactorOne;
            config.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceColor;
            config.rgbBlendOperation = MTLBlendOperationAdd;
            config.sourceAlphaBlendFactor = MTLBlendFactorOne;
            config.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
            config.alphaBlendOperation = MTLBlendOperationAdd;
            break;

        case BlendMode::PremultipliedAlpha:
            // src.rgb + dst.rgb * (1 - src.a)
            config.blendingEnabled = true;
            config.sourceRGBBlendFactor = MTLBlendFactorOne;
            config.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
            config.rgbBlendOperation = MTLBlendOperationAdd;
            config.sourceAlphaBlendFactor = MTLBlendFactorOne;
            config.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
            config.alphaBlendOperation = MTLBlendOperationAdd;
            break;
    }

    return config;
}

} // namespace render
