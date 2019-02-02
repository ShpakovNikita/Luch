#include <Luch/Vulkan/VulkanPhysicalDevice.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>
#include <Luch/Vulkan/VulkanSurface.h>
#include <Luch/Vulkan/VulkanFormat.h>
#include <Luch/Vulkan/VulkanQueue.h>
#include <Luch/Vulkan/VulkanSwapchain.h>

namespace Luch::Vulkan
{
    VulkanPhysicalDevice::VulkanPhysicalDevice(vk::Instance aInstance, vk::SurfaceKHR aSurface, Luch::Optional<vk::AllocationCallbacks> aAllocationCallbacks)
        : instance(aInstance)
        , surface(aSurface)
        , callbacks(aAllocationCallbacks)
    {
        if(callbacks.has_value())
        {
            allocationCallbacks = *callbacks;
        }
    }

    struct QueueFamilyIndices {
        int graphicsFamily = -1;
        int presentFamily = -1;

        bool isComplete() {
            return graphicsFamily >= 0 && presentFamily >= 0;
        }
    };

    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (queueFamily.queueCount > 0 && presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
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
        QueueFamilyIndices indices = FindQueueFamilies(device, surface);
        if (!indices.isComplete())
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

    GraphicsResultValue<QueueIndices> VulkanPhysicalDevice::ChooseDeviceQueues(vk::SurfaceKHR surface)
    {
        auto queueProperties = physicalDevice.getQueueFamilyProperties();
        Vector<VkBool32> supportsPresent;
        supportsPresent.resize(queueProperties.size());

        for (uint32 i = 0; i < queueProperties.size(); i++)
        {
            auto [result, supports] = physicalDevice.getSurfaceSupportKHR(i, surface);
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

    GraphicsResultRefPtr<GraphicsDevice> VulkanPhysicalDevice::CreateGraphicsDevice()
    {
        QueueIndices queueIndices; // todo: should it be empty there?

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
