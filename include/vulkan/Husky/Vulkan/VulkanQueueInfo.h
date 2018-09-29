#pragma once

#include <Husky/RefPtr.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/VulkanForwards.h>

namespace Husky::Vulkan
{
    using QueueIndex = Husky::uint32;

    struct QueueIndices
    {
        QueueIndex computeQueueFamilyIndex = Husky::Limits<QueueIndex>::max();
        QueueIndex graphicsQueueFamilyIndex = Husky::Limits<QueueIndex>::max();
        QueueIndex presentQueueFamilyIndex = Husky::Limits<QueueIndex>::max();

        Husky::Vector<QueueIndex> uniqueIndices;
    };

    struct VulkanQueueInfo
    {
        RefPtr<VulkanQueue> computeQueue;
        RefPtr<VulkanQueue> graphicsQueue;
        RefPtr<VulkanPresentQueue> presentQueue;

        QueueIndices indices;
        Husky::Vector<vk::Queue> uniqueQueues;
    };
}
