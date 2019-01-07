#include <Luch/Vulkan/VulkanPhysicalDevice.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>
#include <Luch/Vulkan/VulkanSurface.h>
#include <Luch/Vulkan/VulkanFormat.h>
#include <Luch/Vulkan/VulkanQueue.h>

namespace Luch::Vulkan
{
    VulkanPhysicalDevice::VulkanPhysicalDevice(
        vk::PhysicalDevice aPhysicalDevice,
        Luch::Optional<vk::AllocationCallbacks> aAllocationCallbacks)
        : physicalDevice(aPhysicalDevice)
        , callbacks(aAllocationCallbacks)
        , physicalDeviceMemoryProperties(aPhysicalDevice.getMemoryProperties())
        , allocationCallbacks(nullptr)
    {
        if(callbacks.has_value())
        {
            allocationCallbacks = *callbacks;
        }
    }

    VulkanResultValue<QueueIndices> VulkanPhysicalDevice::ChooseDeviceQueues(VulkanSurface* surface)
    {
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
        UnorderedSet<QueueIndex> uniqueIndices;

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

        return { vk::Result::eSuccess, std::move(indices) };
    }

    VulkanRefResultValue<VulkanGraphicsDevice> VulkanPhysicalDevice::CreateDevice(
        QueueIndices&& queueIndices,
        const Luch::Vector<const char8*>& requiredDeviceExtensionNames)
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
        ci.setEnabledExtensionCount((int32)requiredDeviceExtensionNames.size());
        ci.setPpEnabledExtensionNames(requiredDeviceExtensionNames.data());
        ci.setPpEnabledLayerNames(0); // device layers are deprecated
        ci.setQueueCreateInfoCount((int32)queueCreateInfos.size());
        ci.setPQueueCreateInfos(queueCreateInfos.data());

        auto [result, vulkanDevice] = physicalDevice.createDevice(ci, allocationCallbacks);

        if (result == vk::Result::eSuccess)
        {
            auto queueInfo = ObtainQueueInfo(vulkanDevice, std::move(queueIndices));
            auto device = MakeRef<VulkanGraphicsDevice>(this, vulkanDevice, std::move(queueInfo), callbacks);
            return { result, std::move(device) };
        }
        else
        {
            vulkanDevice.destroy(allocationCallbacks);
            return { result };
        }
    }

    VulkanQueueInfo VulkanPhysicalDevice::ObtainQueueInfo(vk::Device& device, QueueIndices&& indices)
    {
        VulkanQueueInfo queueInfo;

        auto vulkanGraphicsQueue = device.getQueue(indices.graphicsQueueFamilyIndex, 0);
        auto vulkanPresentQueue = device.getQueue(indices.presentQueueFamilyIndex, 0);
        auto vulkanComputeQueue = device.getQueue(indices.computeQueueFamilyIndex, 0);

        queueInfo.graphicsQueue = MakeRef<VulkanQueue>(vulkanGraphicsQueue);
        queueInfo.computeQueue = MakeRef<VulkanQueue>(vulkanComputeQueue);
        queueInfo.presentQueue = MakeRef<VulkanPresentQueue>(vulkanPresentQueue);
        queueInfo.indices = indices;

        Set<vk::Queue> uniqueQueues;
        uniqueQueues.insert(vulkanGraphicsQueue);
        uniqueQueues.insert(vulkanPresentQueue);
        uniqueQueues.insert(vulkanComputeQueue);

        std::copy(uniqueQueues.begin(), uniqueQueues.end(), std::back_inserter(queueInfo.uniqueQueues));

        return queueInfo;
    }

    Vector<Format> VulkanPhysicalDevice::GetSupportedDepthStencilFormats(const Vector<Format>& formats)
    {
        const vk::FormatFeatureFlags requiredBits =
              vk::FormatFeatureFlagBits::eSampledImage
            | vk::FormatFeatureFlagBits::eTransferSrc
            | vk::FormatFeatureFlagBits::eDepthStencilAttachment;

        Vector<Format> supportedFormats;
        for(Format format : formats)
        {
            auto formatProperties = physicalDevice.getFormatProperties(ToVulkanFormat(format));
            if((formatProperties.optimalTilingFeatures & requiredBits) == requiredBits)
            {
                supportedFormats.push_back(format);
            }
        }
        return supportedFormats;
    }
}
