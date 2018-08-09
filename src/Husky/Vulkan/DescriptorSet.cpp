#include <Husky/Vulkan/DescriptorSet.h>
#include <Husky/Vulkan/DeviceBuffer.h>
#include <Husky/Vulkan/GraphicsDevice.h>
#include <Husky/Vulkan/DescriptorSetLayout.h>
#include <Husky/Vulkan/DescriptorSetWrites.h>

namespace Husky::Vulkan
{
    DescriptorSet::DescriptorSet(GraphicsDevice* aDevice, vk::DescriptorSet aDescriptorSet)
        : device(aDevice)
        , descriptorSet(aDescriptorSet)
    {
    }

    inline vk::Result DescriptorSet::Free()
    {
        return device->GetDevice().freeDescriptorSets(descriptorPool, { descriptorSet });
    }

    void DescriptorSet::Update(const DescriptorSetWrites& writes)
    {
        if (writes.writes.empty())
        {
            return;
        }

        writes.device->device.updateDescriptorSets(writes.writes, {});
    }
}
