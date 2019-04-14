#include <Luch/Vulkan/DescriptorSet/VulkanDescriptorPool.h>
#include <Luch/Vulkan/DescriptorSet/VulkanDescriptorSet.h>
#include <Luch/Vulkan/DescriptorSet/VulkanDescriptorSetLayout.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>

namespace Luch::Vulkan
{
    VulkanDescriptorPool::VulkanDescriptorPool(
        VulkanGraphicsDevice* aDevice,
        vk::DescriptorPool aDescriptorPool)
        : DescriptorPool (aDevice)
        , device(aDevice)
        , descriptorPool(aDescriptorPool)
    {
    }

    VulkanDescriptorPool::~VulkanDescriptorPool()
    {
        Destroy();
    }

    GraphicsResultRefPtr<DescriptorSet> VulkanDescriptorPool::AllocateDescriptorSet(
        DescriptorSetLayout* layout)
    {
        VulkanDescriptorSetLayout* vkDSL = static_cast<VulkanDescriptorSetLayout*>(layout);
        // const DescriptorSetLayoutCreateInfo& ci = layout->GetCreateInfo();
        // DescriptorSetType dsType = ci.type;
        // const Vector<DescriptorSetBinding>& dsBindings = ci.bindings;

        vk::DescriptorSetAllocateInfo allocateInfo;
        allocateInfo.setDescriptorPool(descriptorPool);
        allocateInfo.setDescriptorSetCount(1);
        allocateInfo.setPSetLayouts(&vkDSL->GetDescriptorSetLayout());

        auto[allocateResult, allocatedSets] = device->device.allocateDescriptorSets(allocateInfo);
        if (allocateResult != vk::Result::eSuccess)
        {
            return { allocateResult };
        }

        return { allocateResult, MakeRef<VulkanDescriptorSet>(device, allocatedSets[0]) };
    }

    vk::Result VulkanDescriptorPool::Reset()
    {
        return device->device.resetDescriptorPool(descriptorPool);
    }

    void VulkanDescriptorPool::Destroy()
    {
        if (device)
        {
            device->DestroyDescriptorPool(this);
        }
    }
}
