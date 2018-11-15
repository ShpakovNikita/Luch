#include <Luch/Metal/MetalPipelineLayout.h>
#include <Luch/Metal/MetalGraphicsDevice.h>

namespace Luch::Metal
{
    using namespace Graphics;

    MetalPipelineLayout::MetalPipelineLayout(
        MetalGraphicsDevice* device,
        const PipelineLayoutCreateInfo& aCreateInfo)
        : PipelineLayout(device)
        , createInfo(aCreateInfo)
    {
    }


}


