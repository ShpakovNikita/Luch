#pragma once

#include <Husky/Graphics/BlendFactor.h>
#include <Husky/Graphics/BlendOperation.h>
#include <Husky/Graphics/ColorComponentFlags.h>
#include <Husky/Metal/MetalForwards.h>

namespace Husky::Metal
{
    using namespace Graphics;

    mtlpp::BlendFactor ToMetalBlendFactor(BlendFactor blendFactor);
    mtlpp::BlendOperation ToMetalBlendOperation(BlendOperation blendOperation);
    mtlpp::ColorWriteMask ToMetalColorWriteMask(ColorComponentFlags colorComponentFlags);
}
