#include <Luch/Metal/MetalGraphicsPipelineState.h>
#include <Luch/Metal/MetalGraphicsDevice.h>

namespace Luch::Metal
{
    using namespace Graphics;

    MetalGraphicsPipelineState::MetalGraphicsPipelineState(
        MetalGraphicsDevice* device,
        const GraphicsPipelineStateCreateInfo& aCreateInfo,
        mtlpp::RenderPipelineState aPipelineState,
        Optional<mtlpp::DepthStencilState> aDepthStencilState)
        : GraphicsPipelineState(device)
        , createInfo(aCreateInfo)
        , pipelineState(aPipelineState)
        , depthStencilState(aDepthStencilState)
    {
    }
}
