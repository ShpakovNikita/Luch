#include <Husky/Metal/MetalPipelineLayout.h>
#include <Husky/Metal/MetalGraphicsDevice.h>

namespace Husky::Metal
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


