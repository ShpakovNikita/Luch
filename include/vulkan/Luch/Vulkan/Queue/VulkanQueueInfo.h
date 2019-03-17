#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/Common/VulkanForwards.h>

namespace Luch::Vulkan
{
    using QueueIndex = Luch::uint32;

    struct QueueIndices
    {
        QueueIndex computeQueueFamilyIndex = Luch::Limits<QueueIndex>::max();
        QueueIndex graphicsQueueFamilyIndex = Luch::Limits<QueueIndex>::max();
        QueueIndex presentQueueFamilyIndex = Luch::Limits<QueueIndex>::max();

        Luch::Vector<QueueIndex> uniqueIndices;
    };

    struct VulkanQueueInfo
    {
        vk::Queue computeQueue;
        vk::Queue graphicsQueue;
        vk::Queue presentQueue;

        QueueIndices indices;
        Luch::Vector<vk::Queue> uniqueQueues;
    };
}
