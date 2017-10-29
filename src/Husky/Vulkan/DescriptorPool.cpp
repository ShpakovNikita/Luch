#include <Husky/Vulkan/DescriptorPool.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    DescriptorPool::DescriptorPool(GraphicsDevice * aDevice, vk::DescriptorPool aDescriptorPool)
        : device(aDevice)
        , descriptorPool(aDescriptorPool)
    {
    }

    DescriptorPool::DescriptorPool(DescriptorPool&& other)
        : device(other.device)
        , descriptorPool(other.descriptorPool)
    {
        other.device = nullptr;
        other.descriptorPool = nullptr;
    }

    DescriptorPool& DescriptorPool::operator=(DescriptorPool&& other)
    {
        Destroy();

        device = other.device;
        descriptorPool = other.descriptorPool;

        other.device = nullptr;
        other.descriptorPool = nullptr;

        return *this;
    }

    DescriptorPool::~DescriptorPool()
    {
        Destroy();
    }

    VulkanResultValue<Vector<DescriptorSet>> DescriptorPool::AllocateDescriptorSets(Vector<DescriptorSetLayout*> layouts)
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

        Vector<DescriptorSet> sets;
        sets.reserve(count);

        for (auto vulkanSet : allocatedSets)
        {
            sets.emplace_back(device, vulkanSet);
        }

        return { allocateResult, std::move(sets) };
    }

    VulkanResultValue<DescriptorSet> DescriptorPool::AllocateDescriptorSet(DescriptorSetLayout* layout)
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

        return { allocateResult, DescriptorSet{ device, allocatedSets[0]} };
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
