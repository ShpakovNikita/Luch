#pragma once

#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/ComputePipelineState.h>
#include <Luch/Graphics/ComputePipelineStateCreateInfo.h>
#include <Luch/Metal/MetalForwards.h>

namespace Luch::Metal
{
    using namespace Graphics;

    class MetalComputePipelineState : public ComputePipelineState
    {
        friend class MetalComputeCommandList;
    public:
        MetalComputePipelineState(
            MetalGraphicsDevice* device,
            const ComputePipelineStateCreateInfo& createInfo,
            mtlpp::ComputePipelineState pipelineState);

        const ComputePipelineStateCreateInfo& GetCreateInfo() const override { return createInfo; }
    private:
        ComputePipelineStateCreateInfo createInfo;
        mtlpp::ComputePipelineState pipelineState;
    };
}
