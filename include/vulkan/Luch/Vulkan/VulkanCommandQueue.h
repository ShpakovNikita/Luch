#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>
#include <Luch/Graphics/CommandQueue.h>
#include <Luch/Vulkan/VulkanQueueInfo.h>

using namespace Luch::Graphics;

namespace Luch::Vulkan
{
    class VulkanCommandQueue : public CommandQueue
    {
    public:
        VulkanCommandQueue(VulkanGraphicsDevice* aDevice, vk::Queue aSubmitQueue, vk::Queue aPresentQueue);
        ~VulkanCommandQueue() = default;

        GraphicsResultRefPtr<CommandPool> CreateCommandPool() override;

        // todo: inherit VulkanCommandPool::CommandPool
        GraphicsResultRefPtr<VulkanCommandPool> CreateCommandPool(
            QueueIndex queueIndex,
            bool transient = false,
            bool canReset = false);

        GraphicsResult Submit(
                GraphicsCommandList* commandList) override;

        GraphicsResult Submit(
            CopyCommandList* commandList) override;

        GraphicsResult Present(
            int32 imageIndex,
            Swapchain* swapchain) override;

    private:
        VulkanGraphicsDevice* device = nullptr;
        vk::Queue submitQueue;
        vk::Queue presentQueue;
    };
}
