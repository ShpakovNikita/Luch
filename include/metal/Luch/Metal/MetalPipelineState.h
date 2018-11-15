#pragma once

#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/PipelineState.h>
#include <Luch/Graphics/PipelineStateCreateInfo.h>
#include <Luch/Metal/MetalForwards.h>

namespace Luch::Metal
{
    using namespace Graphics;

    class MetalPipelineState : public PipelineState
    {
        friend class MetalGraphicsCommandList;
    public:
        MetalPipelineState(
            MetalGraphicsDevice* device,
            const PipelineStateCreateInfo& createInfo,
            mtlpp::RenderPipelineState pipelineState,
            Optional<mtlpp::DepthStencilState> depthStencilState);

        const PipelineStateCreateInfo& GetCreateInfo() const override { return createInfo; }
    private:
        PipelineStateCreateInfo createInfo;
        mtlpp::RenderPipelineState pipelineState;
        Optional<mtlpp::DepthStencilState> depthStencilState;
    };
}
