#include <Luch/Metal/MetalDescriptorPool.h>
#include <Luch/Metal/MetalGraphicsDevice.h>
#include <Luch/Metal/MetalDescriptorSet.h>
#include <Luch/Metal/MetalDescriptorSetLayout.h>

namespace Luch::Metal
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
