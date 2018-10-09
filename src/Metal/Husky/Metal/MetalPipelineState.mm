#include <Husky/Metal/MetalPipelineState.h>
#include <Husky/Metal/MetalGraphicsDevice.h>

namespace Husky::Metal
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
