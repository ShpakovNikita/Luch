#include <Husky/Metal/MetalDescriptorSetLayout.h>
#include <Husky/Metal/MetalGraphicsDevice.h>

namespace Husky::Metal
{
    using namespace Graphics;

    MetalDescriptorSetLayout::MetalDescriptorSetLayout(
        MetalGraphicsDevice* device,
        const DescriptorSetLayoutCreateInfo& aCreateInfo)
        : DescriptorSetLayout(device)
        , createInfo(aCreateInfo)
    {
    }
}
