#pragma once

#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/PipelineState.h>
#include <Husky/Graphics/PipelineStateCreateInfo.h>
#include <Husky/Metal/MetalForwards.h>

namespace Husky::Metal
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
            mtlpp::DepthStencilState depthStencilState);

        const PipelineStateCreateInfo& GetCreateInfo() const override { return createInfo; }
    private:
        PipelineStateCreateInfo createInfo;
        mtlpp::RenderPipelineState pipelineState;
        mtlpp::DepthStencilState depthStencilState;
    };
}
