#include <Luch/Metal/MetalTiledPipelineState.h>
#include <Luch/Metal/MetalGraphicsDevice.h>

namespace Luch::Metal
{
    using namespace Graphics;

    MetalTiledPipelineState::MetalTiledPipelineState(
        MetalGraphicsDevice* device,
        const TiledPipelineStateCreateInfo& aCreateInfo,
        mtlpp::RenderPipelineState aPipelineState)
        : TiledPipelineState(device)
        , createInfo(aCreateInfo)
        , pipelineState(aPipelineState)
    {
    }
}
