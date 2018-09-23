#include <Husky/Metal/MetalDescriptorSet.h>
#include <Husky/Metal/MetalDescriptorSetLayout.h>
#include <Husky/Metal/MetalGraphicsDevice.h>

namespace Husky::Metal
{
    using namespace Graphics;

    MetalDescriptorSet::MetalDescriptorSet(
        MetalGraphicsDevice* device,
        MetalDescriptorSetLayout* aDescriptorSetLayout)
        : DescriptorSet(device)
        , descriptorSetLayout(aDescriptorSetLayout)
    {
    }

    DescriptorSetLayout* MetalDescriptorSet::GetDescriptorSetLayout()
    {
        return descriptorSetLayout;
    }
}
