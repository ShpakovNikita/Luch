#include <Luch/Vulkan/VulkanCommandQueue.h>
#include <Luch/Vulkan/VulkanCommandPool.h>
#include <Luch/Vulkan/VulkanFence.h>
#include <Luch/Vulkan/VulkanSemaphore.h>
#include <Luch/Vulkan/VulkanSwapchain.h>

namespace Luch::Vulkan
{

VulkanCommandQueue::VulkanCommandQueue(VulkanGraphicsDevice* aDevice, vk::Queue aSubmitQueue, vk::Queue aPresentQueue)
    : CommandQueue(aDevice)
    , submitQueue(aSubmitQueue)
    , presentQueue(aPresentQueue)
{
}

GraphicsResultRefPtr<CommandPool> VulkanCommandQueue::CreateCommandPool()
{
    return GraphicsResult::InvalidValue;
}

GraphicsResultRefPtr<VulkanCommandPool> VulkanCommandQueue::CreateCommandPool(
    QueueIndex queueIndex,
    bool transient,
    bool canReset)
{
    vk::CommandPoolCreateInfo ci;

    if (transient)
    {
        ci.flags |= vk::CommandPoolCreateFlagBits::eTransient;
    }

    if (canReset)
    {
        ci.flags |= vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    }

    ci.setQueueFamilyIndex(queueIndex);

    auto [result, vulkanCommandPool] = device->GetDevice().createCommandPool(ci, device->GetAllocationCallbacks());

    if (result != vk::Result::eSuccess)
    {
        device->GetDevice().destroyCommandPool(vulkanCommandPool, device->GetAllocationCallbacks());
        return { result };
    }
    else
    {
        return { result, MakeRef<VulkanCommandPool>(device, vulkanCommandPool) };
    }
}

GraphicsResult VulkanCommandQueue::Submit(
        GraphicsCommandList* commandList) { return GraphicsResult::InvalidValue; }

GraphicsResult VulkanCommandQueue::Submit(
    CopyCommandList* commandList) { return GraphicsResult::InvalidValue; }

GraphicsResult VulkanCommandQueue::Present(
    int32 imageIndex,
    Swapchain* swapchain) { return GraphicsResult::InvalidValue; }

}
