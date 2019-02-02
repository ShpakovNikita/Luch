#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>

namespace Luch::Vulkan
{
    struct VulkanSemaphoreWaitOperation
    {
        VulkanSemaphore* semaphore = nullptr;
        vk::PipelineStageFlags stage = vk::PipelineStageFlagBits::eTopOfPipe;
    };

    struct VulkanSubmission
    {
        Vector<VulkanSemaphoreWaitOperation> waitOperations;
        Vector<VulkanSemaphore*> signalSemaphores;
        Vector<VulkanCommandBuffer*> commandBuffers;
        VulkanFence* fence = nullptr;
    };

    struct VulkanPresentSubmission
    {
        Vector<VulkanSemaphore*> waitSemaphores;
        VulkanSwapchain* swapchain;
        int32 index = 0;
    };

    class VulkanQueue : public BaseObject
    {
        friend class VulkanGraphicsDevice;
        friend class VulkanPhysicalDevice;
        friend struct VulkanQueueInfo;
    public:
        explicit VulkanQueue(vk::Queue queue);
        ~VulkanQueue() = default; // Queues are owned by device, so we don't destroy them in destructor

        vk::Result WaitIdle();
        vk::Result Submit(const VulkanSubmission& submission);
    protected:
        vk::Queue queue;
    };

    class VulkanPresentQueue : public VulkanQueue
    {
        friend class VulkanGraphicsDevice;
        friend class VulkanPhysicalDevice;
    public:
        inline explicit VulkanPresentQueue(vk::Queue queue)
            : VulkanQueue(queue)
        {
        }

        vk::Result Present(const VulkanPresentSubmission& presentSubmission);
    };
}
