#include <Luch/Vulkan/VulkanDescriptorPool.h>
#include <Luch/Vulkan/VulkanDescriptorSet.h>
#include <Luch/Vulkan/VulkanDescriptorSetLayout.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>

namespace Luch::Vulkan
{
    VulkanDescriptorPool::VulkanDescriptorPool(
        VulkanGraphicsDevice* aDevice,
        vk::DescriptorPool aDescriptorPool)
        : device(aDevice)
        , descriptorPool(aDescriptorPool)
    {
    }

    VulkanDescriptorPool::~VulkanDescriptorPool()
    {
        Destroy();
    }

    GraphicsResultValue<RefPtrVector<VulkanDescriptorSet>> VulkanDescriptorPool::AllocateDescriptorSets(
        const Vector<VulkanDescriptorSetLayout*>& layouts)
    {
        auto count = layouts.size();

        Vector<vk::DescriptorSetLayout> vulkanLayouts;
        vulkanLayouts.reserve(count);

        for (auto& layout : layouts)
        {
            vulkanLayouts.push_back(layout->GetDescriptorSetLayout());
        }

        vk::DescriptorSetAllocateInfo allocateInfo;
        allocateInfo.setDescriptorPool(descriptorPool);
        allocateInfo.setDescriptorSetCount(count);
        allocateInfo.setPSetLayouts(vulkanLayouts.data());

        auto [allocateResult, allocatedSets] = device->device.allocateDescriptorSets(allocateInfo);
        if (allocateResult != vk::Result::eSuccess)
        {
            return { allocateResult };
        }

        RefPtrVector<VulkanDescriptorSet> sets;
        sets.reserve(count);

        for (auto vulkanSet : allocatedSets)
        {
            sets.emplace_back(MakeRef<VulkanDescriptorSet>(device, vulkanSet));
        }

        return { allocateResult, std::move(sets) };
    }

    GraphicsResultRefPtr<VulkanDescriptorSet> VulkanDescriptorPool::AllocateDescriptorSet(
        VulkanDescriptorSetLayout* layout)
    {
        vk::DescriptorSetLayout vulkanLayout = layout->GetDescriptorSetLayout();

        vk::DescriptorSetAllocateInfo allocateInfo;
        allocateInfo.setDescriptorPool(descriptorPool);
        allocateInfo.setDescriptorSetCount(1);
        allocateInfo.setPSetLayouts(&vulkanLayout);

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
