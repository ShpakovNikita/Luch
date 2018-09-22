#include <Husky/Metal/MetalPipelineState.h>
#include <Husky/Metal/MetalGraphicsDevice.h>

namespace Husky::Metal
{
    using namespace Graphics;

    MetalPipelineState::MetalPipelineState(
        MetalGraphicsDevice* device,
        const PipelineStateCreateInfo& aCreateInfo,
        mtlpp::RenderPipelineState aState)
        : PipelineState(device)
        , createInfo(aCreateInfo)
        , state(aState)
    {
    }
}
