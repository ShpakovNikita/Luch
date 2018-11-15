#pragma once

#include <Luch/Graphics/GraphicsForwards.h>
#include <mtlpp.hpp>

namespace Luch::Metal
{
    using namespace Graphics;

    mtlpp::RenderPipelineDescriptor ToMetalPipelineStateCreateInfo(const PipelineStateCreateInfo& createInfo);
    mtlpp::DepthStencilDescriptor ToMetalDepthStencilDescriptor(const PipelineStateCreateInfo& createInfo);
}
