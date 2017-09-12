#include <Husky/Vulkan/PhysicalDevice.h>
#include <Husky/Vulkan/Surface.h>

namespace Husky::Vulkan
{
    inline PhysicalDevice::PhysicalDevice(vk::PhysicalDevice aPhysicalDevice, vk::AllocationCallbacks aAllocationCallbacks)
        : physicalDevice(aPhysicalDevice)
        , allocationCallbacks(aAllocationCallbacks)
        , physicalDeviceMemoryProperties(aPhysicalDevice.getMemoryProperties())
    {
    }

    VulkanResultValue<QueueIndices> PhysicalDevice::ChooseDeviceQueues(Surface * surface)
    {
        static float32 priorities[] = { 1.0f };
        auto queueProperties = physicalDevice.getQueueFamilyProperties();
        Vector<VkBool32> supportsPresent;
        supportsPresent.resize(queueProperties.size());

        for (uint32 i = 0; i < queueProperties.size(); i++)
        {
            auto[result, supports] = physicalDevice.getSurfaceSupportKHR(i, surface->GetSurface());
            if (result != vk::Result::eSuccess)
            {
                return { result, QueueIndices{} };
            }
#if _WIN32
            supports = supports && physicalDevice.getWin32PresentationSupportKHR(i) == VK_TRUE;
#endif

            supportsPresent[i] = supports;
        }

        QueueIndices indices;
        UnorderedSet<int32> uniqueIndices;

        // Find compute queue
        for (uint32 i = 0; i < queueProperties.size(); i++)
        {
            auto& properties = queueProperties[i];

            if ((properties.queueFlags & vk::QueueFlagBits::eCompute) == vk::QueueFlagBits::eCompute)
            {
                indices.computeQueueFamilyIndex = i;
                uniqueIndices.insert(i);
            }
        }

        // Try to find queue that support both graphics and present operation
        bool foundGraphicsAndPresent = false;
        for (uint32 i = 0; i < queueProperties.size(); i++)
        {
            auto& properties = queueProperties[i];

            if (supportsPresent[i] && (properties.queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics)
            {
                indices.graphicsQueueFamilyIndex = i;
                indices.presentQueueFamilyIndex = i;
                uniqueIndices.insert(i);

                foundGraphicsAndPresent = true;
                break;
            }
        }

        // Find separate queues that support graphics and present operation
        if (!foundGraphicsAndPresent)
        {
            for (uint32 i = 0; i < queueProperties.size(); i++)
            {
                auto& properties = queueProperties[i];

                if (supportsPresent[i])
                {
                    indices.presentQueueFamilyIndex = i;
                    uniqueIndices.insert(i);
                }

                if ((properties.queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics)
                {
                    indices.graphicsQueueFamilyIndex = i;
                    uniqueIndices.insert(i);
                }
            }
        }

        std::copy(uniqueIndices.begin(), uniqueIndices.end(), std::back_inserter(indices.uniqueIndices));

        return { vk::Result::eSuccess, indices };
    }

    VulkanResultValue<Device> PhysicalDevice::CreateDevice(
        QueueIndices&& queueIndices,
        const Husky::Vector<const char8*>& requiredDeviceExtensionNames)
    {
        static float32 queuePriorities[] = { 1.0 };

        vk::DeviceQueueCreateInfo graphicsCi;
        graphicsCi.setQueueCount(1);
        graphicsCi.setQueueFamilyIndex(queueIndices.graphicsQueueFamilyIndex);
        graphicsCi.setPQueuePriorities(queuePriorities);

        vk::DeviceQueueCreateInfo presentCi;
        presentCi.setQueueCount(1);
        presentCi.setQueueFamilyIndex(queueIndices.presentQueueFamilyIndex);
        presentCi.setPQueuePriorities(queuePriorities);

        vk::DeviceQueueCreateInfo computeCi;
        computeCi.setQueueCount(1);
        computeCi.setQueueFamilyIndex(queueIndices.computeQueueFamilyIndex);
        computeCi.setPQueuePriorities(queuePriorities);

        Vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        if (queueIndices.computeQueueFamilyIndex == queueIndices.graphicsQueueFamilyIndex
            && queueIndices.graphicsQueueFamilyIndex == queueIndices.presentQueueFamilyIndex)
        {
            queueCreateInfos.reserve(1);
            queueCreateInfos.push_back(graphicsCi);
        }
        else
        {
            if (queueIndices.graphicsQueueFamilyIndex == queueIndices.presentQueueFamilyIndex)
            {
                queueCreateInfos.reserve(2);
                queueCreateInfos.push_back(graphicsCi);
                queueCreateInfos.push_back(computeCi);
            }
            else if (queueIndices.graphicsQueueFamilyIndex == queueIndices.computeQueueFamilyIndex)
            {
                queueCreateInfos.reserve(2);
                queueCreateInfos.push_back(graphicsCi);
                queueCreateInfos.push_back(presentCi);
            }
            else
            {
                queueCreateInfos.reserve(3);
                queueCreateInfos.push_back(graphicsCi);
                queueCreateInfos.push_back(presentCi);
                queueCreateInfos.push_back(computeCi);
            }
        }

        vk::DeviceCreateInfo ci;
        ci.setEnabledExtensionCount(requiredDeviceExtensionNames.size());
        ci.setPpEnabledExtensionNames(requiredDeviceExtensionNames.data());
        ci.setPpEnabledLayerNames(0); // device layers are deprecated
        ci.setQueueCreateInfoCount(queueCreateInfos.size());
        ci.setPQueueCreateInfos(queueCreateInfos.data());

        auto [result, vulkanDevice] = physicalDevice.createDevice(ci, allocationCallbacks);

        if (result == vk::Result::eSuccess)
        {
            auto queueInfo = ObtainQueueInfo(vulkanDevice, std::move(queueIndices));
            auto device = Device{ this, vulkanDevice, std::move(queueInfo), allocationCallbacks };
            return { result, std::move(device) };
        }
        else
        {
            vulkanDevice.destroy(allocationCallbacks);
            return { result };
        }
    }

    QueueInfo PhysicalDevice::ObtainQueueInfo(vk::Device & device, QueueIndices && indices)
    {
        QueueInfo queueInfo;

        auto vulkanGraphicsQueue = device.getQueue(indices.graphicsQueueFamilyIndex, 0);
        auto vulkanPresentQueue = device.getQueue(indices.presentQueueFamilyIndex, 0);
        auto vulkanComputeQueue = device.getQueue(indices.computeQueueFamilyIndex, 0);

        queueInfo.graphicsQueue = Queue{ vulkanGraphicsQueue };
        queueInfo.presentQueue = Queue{ vulkanPresentQueue };
        queueInfo.computeQueue = Queue{ vulkanComputeQueue };
        queueInfo.indices = indices;

        Set<vk::Queue> uniqueQueues;
        uniqueQueues.insert(vulkanGraphicsQueue);
        uniqueQueues.insert(vulkanPresentQueue);
        uniqueQueues.insert(vulkanComputeQueue);

        std::copy(uniqueQueues.begin(), uniqueQueues.end(), std::back_inserter(queueInfo.uniqueQueues));

        return queueInfo;
    }
}