#pragma once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class Device;
    class PhysicalDevice;
    class QueueInfo;

    class Queue
    {
        friend Device;
        friend PhysicalDevice;
        friend QueueInfo;
    public:
        Queue() = default;
        explicit Queue(vk::Queue queue);
        ~Queue() = default; // Queues are owned by device, so we don't destroy them in destructor
        Queue(const Queue& other) = delete;
        Queue& operator=(const Queue& other) = delete;

        vk::Result WaitIdle();
        // Submit
    private:
        Queue(Queue&& other) = default;
        Queue& operator=(Queue&& other) = default;

        vk::Queue queue;
    };
}
