#include <Luch/Metal/MetalPipelineState.h>
#include <Luch/Metal/MetalGraphicsDevice.h>

namespace Luch::Metal
{
    using namespace Graphics;

    MetalPipelineState::MetalPipelineState(
        MetalGraphicsDevice* device,
        const PipelineStateCreateInfo& aCreateInfo,
        mtlpp::RenderPipelineState aPipelineState,
        Optional<mtlpp::DepthStencilState> aDepthStencilState)
        : PipelineState(device)
        , createInfo(aCreateInfo)
        , pipelineState(aPipelineState)
        , depthStencilState(aDepthStencilState)
    {
    }
}
