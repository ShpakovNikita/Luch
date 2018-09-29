#include <Husky/Metal/MetalColorAttachment.h>
#include <Husky/Types.h>
#include <Husky/Assert.h>

namespace Husky::Metal
{
    using namespace Graphics;

    mtlpp::BlendFactor ToMetalBlendFactor(BlendFactor blendFactor)
    {
        switch(blendFactor)
        {
        case BlendFactor::Zero:
            return mtlpp::BlendFactor::Zero;
        case BlendFactor::One:
            return mtlpp::BlendFactor::One;
        case BlendFactor::SourceColor:
            return mtlpp::BlendFactor::SourceColor;
        case BlendFactor::OneMinusSourceColor:
            return mtlpp::BlendFactor::OneMinusSourceColor;
        case BlendFactor::SourceAlpha:
            return mtlpp::BlendFactor::SourceAlpha;
        case BlendFactor::OneMinusSourceAlpha:
            return mtlpp::BlendFactor::OneMinusSourceAlpha;
        case BlendFactor::DestinationColor:
            return mtlpp::BlendFactor::DestinationColor;
        case BlendFactor::OneMinusDestinationColor:
            return mtlpp::BlendFactor::OneMinusDestinationColor;
        case BlendFactor::DestinationAlpha:
            return mtlpp::BlendFactor::DestinationAlpha;
        case BlendFactor::SourceAlphaSaturated:
            return mtlpp::BlendFactor::SourceAlphaSaturated;
        case BlendFactor::BlendColor:
            return mtlpp::BlendFactor::BlendColor;
        case BlendFactor::OneMinusBlendColor:
            return mtlpp::BlendFactor::OneMinusBlendColor;
        case BlendFactor::BlendAlpha:
            return mtlpp::BlendFactor::BlendAlpha;
        case BlendFactor::OneMinusBlendAlpha:
            return mtlpp::BlendFactor::OneMinusBlendAlpha;
        case BlendFactor::Source1Color:
            return mtlpp::BlendFactor::Source1Color;
        case BlendFactor::OneMinusSource1Color:
            return mtlpp::BlendFactor::OneMinusSource1Color;
        case BlendFactor::Source1Alpha:
            return mtlpp::BlendFactor::Source1Alpha;
        case BlendFactor::OneMinusSource1Alpha:
            return mtlpp::BlendFactor::OneMinusSource1Alpha;
        default:
            HUSKY_ASSERT_MSG(false, "Unknown blend factor");
            return mtlpp::BlendFactor::Zero;
        }
    }

    mtlpp::BlendOperation ToMetalBlendOperation(BlendOperation blendOperation)
    {
        switch(blendOperation)
        {
        case BlendOperation::Add:
            return mtlpp::BlendOperation::Add;
        case BlendOperation::Subtract:
            return mtlpp::BlendOperation::Subtract;
        case BlendOperation::ReverseSubtract:
            return mtlpp::BlendOperation::ReverseSubtract;
        case BlendOperation::Min:
            return mtlpp::BlendOperation::Min;
        case BlendOperation::Max:
            return mtlpp::BlendOperation::Max;
        default:
            HUSKY_ASSERT_MSG(false, "Unknown blend operation");
            return mtlpp::BlendOperation::Add;
        }
    }

    mtlpp::ColorWriteMask ToMetalColorWriteMask(ColorComponentFlags colorComponentFlags)
    {
        uint32 bits = 0;
        if((colorComponentFlags & ColorComponentFlags::Red) == ColorComponentFlags::Red)
        {
            bits |= (uint32)mtlpp::ColorWriteMask::Red;
        }
        if((colorComponentFlags & ColorComponentFlags::Green) == ColorComponentFlags::Green)
        {
            bits |= (uint32)mtlpp::ColorWriteMask::Green;
        }
        if((colorComponentFlags & ColorComponentFlags::Blue) == ColorComponentFlags::Blue)
        {
            bits |= (uint32)mtlpp::ColorWriteMask::Blue;
        }
        if((colorComponentFlags & ColorComponentFlags::Alpha) == ColorComponentFlags::Alpha)
        {
            bits |= (uint32)mtlpp::ColorWriteMask::Alpha;
        }
        mtlpp::ColorWriteMask colorWriteMask = static_cast<mtlpp::ColorWriteMask>(bits);
        return colorWriteMask;
    }
}
