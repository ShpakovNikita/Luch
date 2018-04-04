#include <Husky/Vulkan/DescriptorPool.h>
#include <Husky/Vulkan/DescriptorSet.h>
#include <Husky/Vulkan/DescriptorSetLayout.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    DescriptorPool::DescriptorPool(GraphicsDevice* aDevice, vk::DescriptorPool aDescriptorPool)
        : device(aDevice)
        , descriptorPool(aDescriptorPool)
    {
    }

    DescriptorPool::~DescriptorPool()
    {
        Destroy();
    }

    VulkanResultValue<RefPtrVector<DescriptorSet>> DescriptorPool::AllocateDescriptorSets(const Vector<DescriptorSetLayout*>& layouts)
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

        RefPtrVector<DescriptorSet> sets;
        sets.reserve(count);

        for (auto vulkanSet : allocatedSets)
        {
            sets.emplace_back(MakeRef<DescriptorSet>(device, vulkanSet));
        }

        return { allocateResult, std::move(sets) };
    }

    VulkanRefResultValue<DescriptorSet> DescriptorPool::AllocateDescriptorSet(DescriptorSetLayout* layout)
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

        return { allocateResult, MakeRef<DescriptorSet>(device, allocatedSets[0]) };
    }

    vk::Result DescriptorPool::Reset()
    {
        return device->device.resetDescriptorPool(descriptorPool);
    }

    void DescriptorPool::Destroy()
    {
        if (device)
        {
            device->DestroyDescriptorPool(this);
        }
    }
}
