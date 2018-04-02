#include <Husky/Vulkan/CommandPool.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    CommandPool::CommandPool(GraphicsDevice * aDevice, vk::CommandPool aCommandPool)
        : device(aDevice)
        , commandPool(aCommandPool)
    {
    }

    CommandPool::~CommandPool()
    {
        Destroy();
    }

    VulkanResultValue<RefPtrVector<CommandBuffer>> CommandPool::AllocateCommandBuffers(int32 count, CommandBufferLevel level)
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

        RefPtrVector<CommandBuffer> buffers;
        buffers.reserve(count);

        for (auto vulkanBuffer : allocatedBuffers)
        {
            buffers.emplace_back(MakeRef<CommandBuffer>(device, vulkanBuffer));
        }

        return { allocateResult, std::move(buffers) };
    }

    VulkanRefResultValue<CommandBuffer> CommandPool::AllocateCommandBuffer(CommandBufferLevel level)
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

        return { allocateResult, MakeRef<CommandBuffer>(device, allocatedBuffers[0]) };
    }

    vk::Result CommandPool::Reset(bool releaseResources)
    {
        vk::CommandPoolResetFlags flags;
        if (releaseResources)
        {
            flags |= vk::CommandPoolResetFlagBits::eReleaseResources;
        }

        return device->device.resetCommandPool(commandPool, flags);
    }

    void CommandPool::Destroy()
    {
        if (device)
        {
            device->DestroyCommandPool(this);
        }
    }
}
