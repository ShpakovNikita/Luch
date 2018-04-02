#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;
    class PhysicalDevice;
    class CommandBuffer;
    class Fence;
    class Semaphore;
    class Swapchain;
    struct QueueInfo;

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
        Queue(Queue&& other) = delete;
        Queue(const Queue& other) = delete;
        Queue& operator=(const Queue& other) = delete;
        Queue& operator=(Queue&& other) = delete;

        ~Queue() = default; // Queues are owned by device, so we don't destroy them in destructor

        vk::Result WaitIdle();
        vk::Result Submit(const Submission& submission);
    protected:
        explicit Queue(vk::Queue queue);

        vk::Queue queue;
    };

    class PresentQueue : public Queue
    {
        friend class GraphicsDevice;
        friend class PhysicalDevice;
    public:
        vk::Result Present(const PresentSubmission& presentSubmission);
    protected:
        inline explicit PresentQueue(vk::Queue queue)
            : Queue(queue)
        {
        }
    };
}
