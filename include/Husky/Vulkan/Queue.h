#pragma once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;
    class PhysicalDevice;
    struct QueueInfo;

    class Queue
    {
        friend GraphicsDevice;
        friend PhysicalDevice;
        friend QueueInfo;
    public:
        Queue() = default;

        Queue(Queue&& other) = default;
        Queue& operator=(Queue&& other) = default;

        ~Queue() = default; // Queues are owned by device, so we don't destroy them in destructor

        vk::Result WaitIdle();
        // Submit
    private:
        explicit Queue(vk::Queue queue);

        vk::Queue queue;
    };
}
