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
        VulkanCommandQueue(VulkanGraphicsDevice* aDevice, VulkanQueueInfo aQueueInfo);
        ~VulkanCommandQueue() = default;

        GraphicsResultRefPtr<CommandPool> CreateCommandPool() override;

        GraphicsResult Submit(
                GraphicsCommandList* commandList) override;

        GraphicsResult Submit(
            CopyCommandList* commandList) override;

        GraphicsResult Present(
            int32 imageIndex,
            Swapchain* swapchain) override;

    private:
        GraphicsResultRefPtr<VulkanCommandPool> CreateVulkanCommandPool(
            QueueIndex queueIndex,
            bool transient = false,
            bool canReset = false);

        VulkanGraphicsDevice* device = nullptr;
        VulkanQueueInfo vulkanQueueInfo;
    };
}
