#include <Husky/Vulkan/VulkanDescriptorSet.h>
#include <Husky/Vulkan/VulkanGraphicsDevice.h>
#include <Husky/Vulkan/VulkanDescriptorSetLayout.h>
#include <Husky/Vulkan/VulkanDescriptorSetWrites.h>

namespace Husky::Vulkan
{
    VulkanDescriptorSet::VulkanDescriptorSet(
        VulkanGraphicsDevice* aDevice,
        vk::DescriptorSet aDescriptorSet)
        : device(aDevice)
        , descriptorSet(aDescriptorSet)
    {
    }

    inline vk::Result VulkanDescriptorSet::Free()
    {
        return device->GetDevice().freeDescriptorSets(descriptorPool, { descriptorSet });
    }

    void VulkanDescriptorSet::Update(const DescriptorSetWrites& writes)
    {
        if (writes.writes.empty())
        {
            return;
        }

        writes.device->device.updateDescriptorSets(writes.writes, {});
    }
}
