#include <Luch/Vulkan/VulkanDescriptorSet.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>
#include <Luch/Vulkan/VulkanDescriptorSetLayout.h>
#include <Luch/Vulkan/VulkanDescriptorSetWrites.h>

namespace Luch::Vulkan
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
