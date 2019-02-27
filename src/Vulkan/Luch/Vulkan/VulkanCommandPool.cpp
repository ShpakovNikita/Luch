#include <Luch/Vulkan/VulkanCommandPool.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>
#include <Luch/Vulkan/VulkanCopyCommandList.h>
#include <Luch/Vulkan/VulkanGraphicsCommandList.h>

namespace Luch::Vulkan
{
    VulkanCommandPool::VulkanCommandPool(
        VulkanGraphicsDevice* aDevice,
        vk::CommandPool aCommandPool)
        : CommandPool (aDevice)
        , device(aDevice)
        , commandPool(aCommandPool)
    {
    }

    VulkanCommandPool::~VulkanCommandPool()
    {
        Destroy();
    }

    GraphicsResultRefPtr<GraphicsCommandList> VulkanCommandPool::AllocateGraphicsCommandList()
    {
        vk::CommandBufferAllocateInfo allocateInfo;
        allocateInfo.setCommandBufferCount(1);
        allocateInfo.setCommandPool(commandPool);
        allocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);

        auto [allocateResult, allocatedBuffers] = device->device.allocateCommandBuffers(allocateInfo);
        if (allocateResult != vk::Result::eSuccess)
        {
            return { allocateResult };
        }
        auto copyCommandList = MakeRef<VulkanGraphicsCommandList>(device, allocatedBuffers.front());
        return {GraphicsResult::Success, copyCommandList};
    }

    GraphicsResultRefPtr<CopyCommandList> VulkanCommandPool::AllocateCopyCommandList()
    {
        vk::CommandBufferAllocateInfo allocateInfo;
        allocateInfo.setCommandBufferCount(1);
        allocateInfo.setCommandPool(commandPool);
        allocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);

        auto [allocateResult, allocatedBuffers] = device->device.allocateCommandBuffers(allocateInfo);
        if (allocateResult != vk::Result::eSuccess)
        {
            return { allocateResult };
        }
        auto copyCommandList = MakeRef<VulkanCopyCommandList>(device, allocatedBuffers.front());
        return {GraphicsResult::Success, copyCommandList};
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
