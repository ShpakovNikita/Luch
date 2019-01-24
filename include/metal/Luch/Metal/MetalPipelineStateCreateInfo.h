#pragma once

#include <Luch/Graphics/GraphicsForwards.h>
#include <mtlpp.hpp>

namespace Luch::Metal
{
    using namespace Graphics;

    mtlpp::RenderPipelineDescriptor ToMetalGraphicsPipelineStateCreateInfo(const GraphicsPipelineStateCreateInfo& createInfo);
    mtlpp::DepthStencilDescriptor ToMetalDepthStencilDescriptor(const GraphicsPipelineStateCreateInfo& createInfo);
    mtlpp::ComputePipelineDescriptor ToMetalComputePipelineStateCreateInfo(const ComputePipelineStateCreateInfo& createInfo);
}
