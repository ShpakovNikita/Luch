#include <Luch/Vulkan/VulkanPhysicalDevice.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>
#include <Luch/Vulkan/VulkanSurface.h>
#include <Luch/Vulkan/Common/VulkanFormat.h>
#include <Luch/Vulkan/Queue/VulkanQueue.h>
#include <Luch/Vulkan/Swapchain/VulkanSwapchain.h>

namespace Luch::Vulkan
{
    VulkanPhysicalDevice::VulkanPhysicalDevice(vk::Instance aInstance, vk::SurfaceKHR aSurface
                                               , vk::Optional<const vk::AllocationCallbacks> aAllocationCallbacks)
        : instance(aInstance)
        , surface(aSurface)
        , allocationCallbacks(aAllocationCallbacks)
    {}

    GraphicsResultValue<QueueIndices> ChooseDeviceQueues(vk::PhysicalDevice aPhysicalDevice, vk::SurfaceKHR surface)
    {
        auto queueProperties = aPhysicalDevice.getQueueFamilyProperties();
        Vector<VkBool32> supportsPresent;
        supportsPresent.resize(queueProperties.size());

        for (uint32 i = 0; i < queueProperties.size(); i++)
        {
            auto [result, supports] = aPhysicalDevice.getSurfaceSupportKHR(i, surface);
            if (result != vk::Result::eSuccess)
            {
                return { result };
            }
#if _WIN32
            supports = supports && aPhysicalDevice.getWin32PresentationSupportKHR(i) == VK_TRUE;
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

    // todo: param for device creation
    const std::vector<const char*> requiredDeviceExtensionNames = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    bool CheckDeviceExtensionSupport(vk::PhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(requiredDeviceExtensionNames.begin(), requiredDeviceExtensionNames.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    bool IsDeviceSuitable(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
        auto [result, queues] = ChooseDeviceQueues(device, surface);
        if (result != GraphicsResult::Success)
        {
            return false;
        }

        if (!CheckDeviceExtensionSupport(device))
        {
            return false;
        }

        int width = 640;
        int height = 480;
        GraphicsResultValue<VulkanSwapchainCreateInfo> swapchainCreateInfo =
                VulkanSwapchain::ChooseSwapchainCreateInfo(width, height, device, surface);
        return swapchainCreateInfo.result == GraphicsResult::Success;
    }

    bool VulkanPhysicalDevice::Init()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            LUCH_ASSERT_MSG(false, "failed to find any GPU with Vulkan support.");
            return false;
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            if (IsDeviceSuitable(device, surface)) {
                physicalDevice = device;
                break;
            }
        }

        if (!physicalDevice) {
            LUCH_ASSERT_MSG(false, "failed to find a suitable GPU");
            return false;
        }

        physicalDeviceMemoryProperties = physicalDevice.getMemoryProperties();

        return true;
    }

    GraphicsResultRefPtr<GraphicsDevice> VulkanPhysicalDevice::CreateGraphicsDevice()
    {
        auto [qResult, queueIndices] = ChooseDeviceQueues(physicalDevice, surface);
        if (qResult != GraphicsResult::Success)
        {
            return qResult;
        }

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
        queueCreateInfos.push_back(graphicsCi);
        if (queueIndices.graphicsQueueFamilyIndex != queueIndices.presentQueueFamilyIndex)
        {
            queueCreateInfos.push_back(graphicsCi);
        }
        if (queueIndices.computeQueueFamilyIndex != queueIndices.graphicsQueueFamilyIndex
                && queueIndices.computeQueueFamilyIndex != queueIndices.presentQueueFamilyIndex)
        {
            queueCreateInfos.push_back(computeCi);
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
            auto device = MakeRef<VulkanGraphicsDevice>(this, vulkanDevice, std::move(queueInfo), allocationCallbacks);
            return {GraphicsResult::Success, device};
        }
        else
        {
            vulkanDevice.destroy(allocationCallbacks);
            return { GraphicsResult::UnknownError, nullptr };
        }
    }

    VulkanQueueInfo VulkanPhysicalDevice::ObtainQueueInfo(vk::Device& device, QueueIndices&& indices)
    {
        VulkanQueueInfo queueInfo;

        queueInfo.graphicsQueue = device.getQueue(indices.graphicsQueueFamilyIndex, 0);
        queueInfo.presentQueue = device.getQueue(indices.presentQueueFamilyIndex, 0);
        queueInfo.computeQueue = device.getQueue(indices.computeQueueFamilyIndex, 0);
        queueInfo.indices = indices;

        Set<vk::Queue> uniqueQueues;
        uniqueQueues.insert(queueInfo.graphicsQueue);
        uniqueQueues.insert(queueInfo.presentQueue);
        uniqueQueues.insert(queueInfo.computeQueue);

        std::copy(uniqueQueues.begin(), uniqueQueues.end(), std::back_inserter(queueInfo.uniqueQueues));

        return queueInfo;
    }

    Vector<Format> VulkanPhysicalDevice::GetSupportedDepthStencilFormats(const Vector<Format>& formats) const
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
