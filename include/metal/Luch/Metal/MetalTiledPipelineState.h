#pragma once

#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/TiledPipelineState.h>
#include <Luch/Graphics/TiledPipelineStateCreateInfo.h>
#include <Luch/Metal/MetalForwards.h>

namespace Luch::Metal
{
    using namespace Graphics;

    class MetalTiledPipelineState : public TiledPipelineState
    {
        friend class MetalGraphicsCommandList;
    public:
        MetalTiledPipelineState(
            MetalGraphicsDevice* device,
            const TiledPipelineStateCreateInfo& createInfo,
            mtlpp::RenderPipelineState pipelineState);

        const TiledPipelineStateCreateInfo& GetCreateInfo() const override { return createInfo; }
    private:
        TiledPipelineStateCreateInfo createInfo;
        mtlpp::RenderPipelineState pipelineState;
    };
}
