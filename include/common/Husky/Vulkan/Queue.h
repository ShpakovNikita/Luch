#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
{
    struct SemaphoreWaitOperation
    {
        Semaphore* semaphore = nullptr;
        vk::PipelineStageFlags stage = vk::PipelineStageFlagBits::eTopOfPipe;
    };

    struct Submission
    {
        Vector<SemaphoreWaitOperation> waitOperations;
        Vector<Semaphore*> signalSemaphores;
        Vector<CommandBuffer*> commandBuffers;
        Fence* fence = nullptr;
    };

    struct PresentSubmission
    {
        Vector<Semaphore*> waitSemaphores;
        Swapchain* swapchain;
        int32 index;
    };

    class Queue : public BaseObject
    {
        friend GraphicsDevice;
        friend PhysicalDevice;
        friend QueueInfo;
    public:
        explicit Queue(vk::Queue queue);
        ~Queue() = default; // Queues are owned by device, so we don't destroy them in destructor

        vk::Result WaitIdle();
        vk::Result Submit(const Submission& submission);
    protected:
        vk::Queue queue;
    };

    class PresentQueue : public Queue
    {
        friend class GraphicsDevice;
        friend class PhysicalDevice;
    public:
        inline explicit PresentQueue(vk::Queue queue)
            : Queue(queue)
        {
        }

        vk::Result Present(const PresentSubmission& presentSubmission);
    };
}
