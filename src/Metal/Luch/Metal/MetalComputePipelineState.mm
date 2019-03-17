#include <Luch/Metal/MetalComputePipelineState.h>
#include <Luch/Metal/MetalGraphicsDevice.h>

namespace Luch::Metal
{
    using namespace Graphics;

    MetalComputePipelineState::MetalComputePipelineState(
        MetalGraphicsDevice* device,
        const ComputePipelineStateCreateInfo& aCreateInfo,
        mtlpp::ComputePipelineState aPipelineState)
        : ComputePipelineState(device)
        , createInfo(aCreateInfo)
        , pipelineState(aPipelineState)
    {
    }
}
