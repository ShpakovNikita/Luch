#pragma once

#include <string_view>

#include <Husky/Types.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/VulkanPhysicalDevice.h>

namespace Husky
{

class VulkanInstance
{
public:
    // TODO allocators
    template<typename LayerIterator, typename ExtensionIterator>
    static VulkanInstance Create(
        const VkApplicationInfo& applicationInfo,
        LayerIterator layersBegin, LayerIterator layersEnd,
        ExtensionIterator extensionsBegin, ExtensionIterator extensionsEnd)
    {
        VkInstance handle;
        VkInstanceCreateInfo ci = {}
        ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        ci.pApplicationInfo = &applicationInfo;
        ci.enabledLayerCount = std::distance(layersBegin, layersEnd);
        ci.ppEnabledLayerNames = &layersBegin[0];
        ci.enabledExtensionCount = std::distance(extensionsBegin, extensionsEnd);
        ci.ppEnabledExtensionNames = &extensionsBegin[0];

        auto result = vkCreateInstance(&info, nullptr, &handle);
        if (result != VK_SUCCESS)
        {
            throw VkException{result, "Instance creation failed"};
        }

        return {handle};
    }

    VulkanInstance(VkInstance handle);

    VulkanInstance() = default;
    VulkanInstance(const VulkanInstance& other) = default;
    VulkanInstance(VulkanInstance&& other) = default;

    VulkanInstance& operator=(const VulkanInstance& other) = default;
    VulkanInstance& operator=(VulkanInstance&& other) = default;

    VkCreateResult<VkDebugReportCallbackEXT> CreateDebugCallback(
        VulkanDebugDelegate* delegate,
        VkDebugReportFlagsEXT flags =
            VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
            VK_DEBUG_REPORT_WARNING_BIT_EXT |
            VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
            VK_DEBUG_REPORT_ERROR_BIT_EXT |
            VK_DEBUG_REPORT_DEBUG_BIT_EXT);

    VkEnumerateResult<VulkanPhysicalDevice> GetPhysicalDevices() const;

    template<typename Procedure>
    Procedure GetProcedureAddress(const StringView& name)
    {
        PFN_vkVoidFunction procedureAddres = vkGetInstanceProcAddr(handle, name.data());
        return static_cast<Procedure>(procedureAddress);
    }
private:
    static VkBool32 VKAPI_CALL GlobalDebugCallback(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objectType,
        uint64_t object,
        size_t location,
        int32_t messageCode,
        const char* pLayerPrefix,
        const char* pMessage,
        void* pUserData);

    VkUniquePtr<VkInstance> handle;
};

}
