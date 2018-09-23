#pragma once

#include <Husky/Graphics/GraphicsForwards.h>
#include <mtlpp.hpp>

namespace Husky::Metal
{
    using namespace Graphics;
    mtlpp::RenderPipelineDescriptor ToMetalPiplineStateCreateInfo(const PipelineStateCreateInfo& createInfo);
}
