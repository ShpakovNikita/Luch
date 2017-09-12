#pragma once

#include <Husky/Types.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Queue.h>

namespace Husky::Vulkan
{
    class Surface;

    struct QueueIndices
    {
        Husky::uint32 computeQueueFamilyIndex = Husky::Limits<Husky::uint32>::max();
        Husky::uint32 graphicsQueueFamilyIndex = Husky::Limits<Husky::uint32>::max();
        Husky::uint32 presentQueueFamilyIndex = Husky::Limits<Husky::uint32>::max();

        Husky::Vector<Husky::uint32> uniqueIndices;
    };

    struct QueueInfo
    {
        QueueInfo() = default;

        QueueInfo(QueueInfo&& other) = default;
        QueueInfo& operator=(QueueInfo&& other) = default;

        Queue computeQueue;
        Queue graphicsQueue;
        Queue presentQueue;

        QueueIndices indices;
        Husky::Vector<vk::Queue> uniqueQueues;
    };
}
