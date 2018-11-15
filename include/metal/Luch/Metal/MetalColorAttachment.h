#pragma once

#include <Luch/Graphics/BlendFactor.h>
#include <Luch/Graphics/BlendOperation.h>
#include <Luch/Graphics/ColorComponentFlags.h>
#include <Luch/Metal/MetalForwards.h>

namespace Luch::Metal
{
    using namespace Graphics;

    mtlpp::BlendFactor ToMetalBlendFactor(BlendFactor blendFactor);
    mtlpp::BlendOperation ToMetalBlendOperation(BlendOperation blendOperation);
    mtlpp::ColorWriteMask ToMetalColorWriteMask(ColorComponentFlags colorComponentFlags);
}
