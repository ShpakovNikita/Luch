#include "SampleApplication.h"

#include <Husky/Vulkan.h>
#include <glfw/glfw3.h>

using namespace Husky;

void SampleApplication::Initialize(const Vector<String>& args)
{
    auto [createInstanceResult, createdInstance] = CreateVulkanInstance();
    if (createInstanceResult != vk::Result::eSuccess)
    {
        // TODO
        return;
    }

    instance = createdInstance;

    auto [enumeratePhysicalDevicesResult, physicalDevices] = instance.enumeratePhysicalDevices();
    if (enumeratePhysicalDevicesResult != vk::Result::eSuccess || physicalDevices.empty())
    {
        // TODO
        return;
    }

    physicalDevice = ChoosePhysicalDevice(physicalDevices);

    auto [createDeviceResult, createdDevice] = CreateDevice(physicalDevice);
    if (createDeviceResult != vk::Result::eSuccess)
    {
        // TODO
        return;
    }

    device = createdDevice;


}

void SampleApplication::Deinitialize()
{
    device.destroy();
    instance.destroy();
}

void SampleApplication::Run()
{

}

vk::ResultValue<vk::Instance> SampleApplication::CreateVulkanInstance()
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

    return vk::createInstance(ci);
}

vk::PhysicalDevice SampleApplication::ChoosePhysicalDevice(const Husky::Vector<vk::PhysicalDevice>& devices)
{
    return devices[0];
}

vk::ResultValue<vk::Device> SampleApplication::CreateDevice(vk::PhysicalDevice & physicalDevice)
{
    vk::DeviceCreateInfo ci;
    return physicalDevice.createDevice(ci);
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
    return Husky::Vector<const Husky::char8*>();
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
