#pragma once

#include <Husky/Graphics/GraphicsForwards.h>
#include <mtlpp.hpp>

namespace Husky::Metal
{
    using namespace Graphics;

    mtlpp::RenderPipelineDescriptor ToMetalPipelineStateCreateInfo(const PipelineStateCreateInfo& createInfo);
    mtlpp::DepthStencilDescriptor ToMetalDepthStencilDescriptor(const PipelineStateCreateInfo& createInfo);
}
