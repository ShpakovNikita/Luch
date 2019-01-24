#pragma once

#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/GraphicsPipelineState.h>
#include <Luch/Graphics/GraphicsPipelineStateCreateInfo.h>
#include <Luch/Metal/MetalForwards.h>

namespace Luch::Metal
{
    using namespace Graphics;

    class MetalGraphicsPipelineState : public GraphicsPipelineState
    {
        friend class MetalGraphicsCommandList;
    public:
        MetalGraphicsPipelineState(
            MetalGraphicsDevice* device,
            const GraphicsPipelineStateCreateInfo& createInfo,
            mtlpp::RenderPipelineState pipelineState,
            Optional<mtlpp::DepthStencilState> depthStencilState);

        const GraphicsPipelineStateCreateInfo& GetCreateInfo() const override { return createInfo; }
    private:
        GraphicsPipelineStateCreateInfo createInfo;
        mtlpp::RenderPipelineState pipelineState;
        Optional<mtlpp::DepthStencilState> depthStencilState;
    };
}
