#include <Luch/Metal/MetalDescriptorSetLayout.h>
#include <Luch/Metal/MetalGraphicsDevice.h>

namespace Luch::Metal
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
