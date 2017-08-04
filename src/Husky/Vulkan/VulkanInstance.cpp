#include <Husky/Vulkan/VulkanInstance.h>

namespace Husky
{

VulkanInstance::VulkanInstance(VkInstance handle)
    : handle(handle, vkDestroyInstance)
{
}

VkCreateResult<VkDebugReportCallbackEXT> VulkanInstance::CreateDebugCallback(VulkanDebugDelegate* delegate, VkDebugReportFlagsEXT flags)
{
    VkDebugReportCallbackCreateInfoEXT ci = {};
    ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    ci.flags = flags;
    ci.pfnCallback = VulkanInstance::GlobalDebugCallback;
    ci.pUserData = static_cast<void*>(delegate);

    VkDebugReportCallbackEXT callbackHandle;
    auto result = vkCreateDebugReportCallbackEXT(handle, &ci, nullptr, &callbackHandle);

    VkUniquePtr<VkDebugReportCallbackEXT> callback{callbackHandle, [&instance = handle](auto handle, auto allocationCallbacks)
    { 
        vkDestroyDebugReportCallbackEXT(instance, handle, allocationCallbacks);
    }};

    return std::make_tuple(result, callback);
}

VkBool32 VulkanInstance::GlobalDebugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objectType,
    uint64_t object,
    size_t location,
    int32_t messageCode,
    const char* pLayerPrefix,
    const char* pMessage,
    void* pUserData)
{
    return static_cast<VulkanDebugDelegate*>(pUserData)->DebugCallback(
        flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
}

VkEnumerateResult<VulkanPhysicalDevice> VulkanInstance::GetPhysicalDevices() const
{
    VkResult result;
    uint32 count;
    result = vkEnumeratePhysicalDevices(handle, &count, nullptr);
    Vector<VkPhysicalDevice> physicalDevices;

    if(result != VK_SUCCESS)
    {
        return std::make_tuple(result, physicalDevices);
    }
    
    do
    {
        physicalDevices.resize(count);
        result = vkEnumeratePhysicalDevices(handle, &count, physicalDevices.data());
    }
    while(result == VK_INCOMPLETE);

    return std::make_tuple(result, move(physicalDevices));
}

}
