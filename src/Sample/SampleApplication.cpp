#include "SampleApplication.h"

#include <Husky/Vulkan.h>
#include <glfw/glfw3.h>

using namespace Husky;

static VkBool32 StaticDebugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objectType,
    uint64 object,
    size_t location,
    int32 messageCode,
    const char8 * pLayerPrefix,
    const char8 * pMessage,
    void* userData)
{
    static_cast<VulkanDebugDelegate*>(userData)->DebugCallback(flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
}

void SampleApplication::Initialize(const Vector<String>& args)
{
    auto [createInstanceResult, createdInstance] = CreateVulkanInstance(allocationCallbacks);
    if (createInstanceResult != vk::Result::eSuccess)
    {
        // TODO
        return;
    }

    instance = createdInstance;

#ifdef _DEBUG
    auto [createDebugCallbackResult, createdDebugCallback] = CreateDebugCallback(instance, allocationCallbacks);
    if(createDebugCallbackResult != vk::Result::eSuccess)
    {
        // TODO
    }

    debugCallback = createdDebugCallback;
#endif

    auto [enumeratePhysicalDevicesResult, physicalDevices] = instance.enumeratePhysicalDevices();
    if (enumeratePhysicalDevicesResult != vk::Result::eSuccess || physicalDevices.empty())
    {
        // TODO
        return;
    }

    physicalDevice = ChoosePhysicalDevice(physicalDevices);
    auto queueCreateInfo = ChooseDeviceQueue(physicalDevice);
    if (!queueCreateInfo.has_value())
    {
        // TODO
        return;
    }

    auto [createDeviceResult, createdDevice] = CreateDevice(physicalDevice, queueCreateInfo.value(), allocationCallbacks);
    if (createDeviceResult != vk::Result::eSuccess)
    {
        // TODO
        return;
    }

    device = createdDevice;
    queueInfo.queueFamilyIndex = queueCreateInfo->queueFamilyIndex;
    queueInfo.queue = device.getQueue(queueInfo.queueFamilyIndex, 0);

    auto [createCommandPoolResult, createdCommandPool] = CreateCommandPool(device, queueInfo, allocationCallbacks);
    if (createCommandPoolResult != vk::Result::eSuccess)
    {
        // TODO
        return;
    }

    commandPool = createdCommandPool;


}

void SampleApplication::Deinitialize()
{
    device.destroyCommandPool(commandPool, allocationCallbacks);
    device.destroy(allocationCallbacks);
    instance.destroy(allocationCallbacks);
}

void SampleApplication::Run()
{

}

vk::ResultValue<vk::Instance> SampleApplication::CreateVulkanInstance(vk::AllocationCallbacks& allocationCallbacks)
{
    auto requiredExtensions = GetRequiredInstanceExtensionNames();
    auto validationLayers = GetValidationLayerNames();

    auto applicationName = GetApplicationName();

    vk::ApplicationInfo applicationInfo;
    applicationInfo.setApiVersion(VK_MAKE_VERSION(1, 0, 56));
    applicationInfo.setApplicationVersion(VK_MAKE_VERSION(0, 1, 0));
    applicationInfo.setPApplicationName(applicationName.c_str());
    applicationInfo.setEngineVersion(VK_MAKE_VERSION(0, 1, 0));
    applicationInfo.setPEngineName("Husky Engine");

    vk::InstanceCreateInfo ci;
    ci.setPApplicationInfo(&applicationInfo);
    ci.setEnabledLayerCount(validationLayers.size());
    ci.setPpEnabledLayerNames(validationLayers.data());
    ci.setEnabledExtensionCount(requiredExtensions.size());
    ci.setPpEnabledExtensionNames(requiredExtensions.data());

    return vk::createInstance(ci, allocationCallbacks);
}

vk::ResultValue<vk::DebugReportCallbackEXT> SampleApplication::CreateDebugCallback(vk::Instance & instance, vk::AllocationCallbacks & allocationCallbacks)
{
    vk::DebugReportCallbackCreateInfoEXT ci;
    ci.setPfnCallback(StaticDebugCallback);
    ci.setPUserData(this);
    return instance.createDebugReportCallbackEXT(ci, allocationCallbacks);
}

vk::PhysicalDevice SampleApplication::ChoosePhysicalDevice(const Husky::Vector<vk::PhysicalDevice>& devices)
{
    return devices[0];
}

Optional<vk::DeviceQueueCreateInfo> SampleApplication::ChooseDeviceQueue(vk::PhysicalDevice & physicalDevice)
{
    static float32 priorities[] = { 1.0f };
    auto queueProperties = physicalDevice.getQueueFamilyProperties();
    auto requiredQueueFlagBits = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute;

    for (uint32 i = 0; i < queueProperties.size(); i++)
    {
        if ((queueProperties[i].queueFlags & requiredQueueFlagBits) = requiredQueueFlagBits)
        {
            vk::DeviceQueueCreateInfo ci;
            ci.setQueueCount(1);
            ci.setPQueuePriorities(priorities);
            ci.setQueueFamilyIndex(i);
            return ci;
        }
    }

    return {};
}

vk::ResultValue<vk::Device> SampleApplication::CreateDevice(
    vk::PhysicalDevice & physicalDevice,
    vk::DeviceQueueCreateInfo& queueCreateInfo,
    vk::AllocationCallbacks& allocationCallbacks)
{
    auto requiredDeviceExtensionNames = GetRequiredDeviceExtensionNames();

    vk::DeviceCreateInfo ci;
    ci.setEnabledExtensionCount(requiredDeviceExtensionNames.size());
    ci.setPpEnabledExtensionNames(requiredDeviceExtensionNames.data());
    ci.setPpEnabledLayerNames(0); // device layers are deprecated
    ci.setQueueCreateInfoCount(1);
    ci.setPQueueCreateInfos(&queueCreateInfo);

    return physicalDevice.createDevice(ci, allocationCallbacks);
}

vk::ResultValue<vk::CommandPool> SampleApplication::CreateCommandPool(
    vk::Device & device,
    const QueueInfo & info,
    vk::AllocationCallbacks& allocationCallbacks)
{
    vk::CommandPoolCreateInfo ci;
    ci.setQueueFamilyIndex(info.queueFamilyIndex);
    return device.createCommandPool(ci, allocationCallbacks);
}

Vector<const char8*> SampleApplication::GetRequiredInstanceExtensionNames() const
{
    uint32 requiredExtensionsCount = 0;
    const char8** requiredExtensions = glfwGetRequiredInstanceExtensions(&requiredExtensionsCount);

    Vector<const char8*> requiredExtensionNames;
    requiredExtensionNames.reserve(requiredExtensionsCount);

    for(uint32 i = 0; i < requiredExtensionsCount; i++)
    {
        requiredExtensionNames.push_back(requiredExtensions[i]);
    }

    requiredExtensionNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

    return requiredExtensionNames;
}

Husky::Vector<const Husky::char8*> SampleApplication::GetRequiredDeviceExtensionNames() const
{
    Vector<const char8*> requiredExtensionNames;
    return requiredExtensionNames;
}

Vector<const char8*> SampleApplication::GetValidationLayerNames() const
{
    return {"VK_LAYER_LUNARG_standard_validation"};
}

VkBool32 SampleApplication::DebugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objectType,
    uint64_t object,
    size_t location,
    int32_t messageCode,
    const char * pLayerPrefix,
    const char * pMessage)
{
    return VK_TRUE;
}
