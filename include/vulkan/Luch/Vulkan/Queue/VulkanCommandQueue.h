#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/Common/VulkanForwards.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>
#include <Luch/Graphics/CommandQueue.h>
#include <Luch/Vulkan/Queue/VulkanQueueInfo.h>

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
                CommandList* commandList,
                std::function<void()> completedHandler) override;

        GraphicsResult Present(
                SwapchainTexture* swapchainTexture,
                std::function<void()> presentedHandler) override;

    private:
        GraphicsResultRefPtr<VulkanCommandPool> CreateVulkanCommandPool(
            QueueIndex queueIndex,
            bool transient = false,
            bool canReset = false);

        VulkanGraphicsDevice* device = nullptr;
        VulkanQueueInfo vulkanQueueInfo;
    };
}
