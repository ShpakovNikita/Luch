#include "Husky/Application.h"

namespace Husky
{

void Application::Initialize(const Vector<String>& args)
{
    requiredExtensionNames = GetRequiredExtensionNames();
    validationLayerNames = GetValidationLayerNames();
    auto [result, instance] = CreateInstance();

}

void Application::Deinitialize()
{
}

void Application::Run()
{
}

VkCreateResult<VkInstance> Application::CreateInstance()
{
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.apiVersion = 0; // 0 means any api version
    applicationInfo.pApplicationName = applicationName.c_str();
    applicationInfo.applicationVersion = VK_MAKE_VERSION(applicationVersion.major, applicationVersion.minor, applicationVersion.patch);
    applicationInfo.pEngineName = engineName.c_str();
    applicationInfo.engineVersion = VK_MAKE_VERSION(engineVersion.major, engineVersion.minor, engineVersion.patch);
    
    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    std::tie(instanceCreateInfo.enabledExtensionCount, instanceCreateInfo.ppEnabledLayerNames) = VkArrayParam(requiredExtensionNames);
    std::tie(instanceCreateInfo.enabledLayerCount, instanceCreateInfo.ppEnabledLayerNames) = VkArrayParam(validationLayerNames);
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    VkInstance instance;
    VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
    return { result, instance };
}

Vector<const char*> Application::GetRequiredExtensionNames()
{
    return Vector<const char*>();
}

Vector<const char*> Application::GetValidationLayerNames()
{
    return Vector<const char*>{"VK_LAYER_LUNARG_standard_validation"};
}

}