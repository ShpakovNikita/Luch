#include <Luch/Vulkan/VulkanCommandPool.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>

namespace Luch::Vulkan
{
    VulkanCommandPool::VulkanCommandPool(
        VulkanGraphicsDevice* aDevice,
        vk::CommandPool aCommandPool)
        : device(aDevice)
        , commandPool(aCommandPool)
    {
    }

    VulkanCommandPool::~VulkanCommandPool()
    {
        Destroy();
    }

    VulkanResultValue<RefPtrVector<VulkanCommandBuffer>> VulkanCommandPool::AllocateCommandBuffers(
        int32 count,
        CommandBufferLevel level)
    {
        vk::CommandBufferAllocateInfo allocateInfo;
        allocateInfo.setCommandBufferCount(count);
        allocateInfo.setCommandPool(commandPool);

        switch (level)
        {
        case CommandBufferLevel::Primary:
            allocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);
            break;
        case CommandBufferLevel::Secondary:
            allocateInfo.setLevel(vk::CommandBufferLevel::eSecondary);
            break;
        }

        auto [allocateResult, allocatedBuffers] = device->device.allocateCommandBuffers(allocateInfo);
        if (allocateResult != vk::Result::eSuccess)
        {
            return { allocateResult };
        }

        RefPtrVector<VulkanCommandBuffer> buffers;
        buffers.reserve(count);

        for (auto vulkanBuffer : allocatedBuffers)
        {
            buffers.emplace_back(MakeRef<VulkanCommandBuffer>(device, vulkanBuffer));
        }

        return { allocateResult, std::move(buffers) };
    }

    VulkanRefResultValue<VulkanCommandBuffer> VulkanCommandPool::AllocateCommandBuffer(CommandBufferLevel level)
    {
        vk::CommandBufferAllocateInfo allocateInfo;
        allocateInfo.setCommandBufferCount(1);
        allocateInfo.setCommandPool(commandPool);

        switch (level)
        {
        case CommandBufferLevel::Primary:
            allocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);
            break;
        case CommandBufferLevel::Secondary:
            allocateInfo.setLevel(vk::CommandBufferLevel::eSecondary);
            break;
        }

        auto[allocateResult, allocatedBuffers] = device->device.allocateCommandBuffers(allocateInfo);
        if (allocateResult != vk::Result::eSuccess)
        {
            return { allocateResult };
        }

        return { allocateResult, MakeRef<VulkanCommandBuffer>(device, allocatedBuffers[0]) };
    }

    vk::Result VulkanCommandPool::Reset(bool releaseResources)
    {
        vk::CommandPoolResetFlags flags;
        if (releaseResources)
        {
            flags |= vk::CommandPoolResetFlagBits::eReleaseResources;
        }

        return device->device.resetCommandPool(commandPool, flags);
    }

    void VulkanCommandPool::Destroy()
    {
        if (device)
        {
            device->DestroyCommandPool(this);
        }
    }
}
