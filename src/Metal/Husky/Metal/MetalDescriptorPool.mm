#include <Husky/Metal/MetalDescriptorPool.h>
#include <Husky/Metal/MetalGraphicsDevice.h>
#include <Husky/Metal/MetalDescriptorSet.h>
#include <Husky/Metal/MetalDescriptorSetLayout.h>

namespace Husky::Metal
{
    using namespace Graphics;

    MetalDescriptorPool::MetalDescriptorPool(
        MetalGraphicsDevice* device,
        const DescriptorPoolCreateInfo& aCreateInfo)
        : DescriptorPool(device)
        , createInfo(aCreateInfo)
    {
    }

    GraphicsResultRefPtr<DescriptorSet> MetalDescriptorPool::AllocateDescriptorSet(DescriptorSetLayout* layout)
    {
        auto mtlDevice = static_cast<MetalGraphicsDevice*>(GetGraphicsDevice());
        auto mtlDescriptorSetLayout = static_cast<MetalDescriptorSetLayout*>(layout);

        return { GraphicsResult::Success, MakeRef<MetalDescriptorSet>(mtlDevice, mtlDescriptorSetLayout) };
    }
}
