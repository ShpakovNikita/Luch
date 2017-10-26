#pragma once

#include <Husky/Types.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Queue.h>

namespace Husky::Vulkan
{
    class Surface;

    using QueueIndex = Husky::uint32;

    struct QueueIndices
    {
        QueueIndex computeQueueFamilyIndex = Husky::Limits<QueueIndex>::max();
        QueueIndex graphicsQueueFamilyIndex = Husky::Limits<QueueIndex>::max();
        QueueIndex presentQueueFamilyIndex = Husky::Limits<QueueIndex>::max();

        Husky::Vector<QueueIndex> uniqueIndices;
    };

    struct QueueInfo
    {
        QueueInfo() = default;

        QueueInfo(QueueInfo&& other) = default;
        QueueInfo& operator=(QueueInfo&& other) = default;

        Queue computeQueue;
        Queue graphicsQueue;
        PresentQueue presentQueue;

        QueueIndices indices;
        Husky::Vector<vk::Queue> uniqueQueues;
    };
}
