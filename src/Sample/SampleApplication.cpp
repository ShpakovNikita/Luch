#include "SampleApplication.h"

#include <Husky/Vulkan.h>

using namespace Husky;

void SampleApplication::Initialize(const Vector<String>& args)
{
    auto requiredExtensions = GetRequiredExtensionNames();
    auto validationLayers = GetValidationLayerNames();

    VulkanInstance instance = VulkanInstance::Create(
        applicationInfo,
        requiredExtensions.begin(), requiredExtensions.end(),
        validationLayers.begin(), validationLayers.end());
    
    {
        auto [result, cInstance] = instance.CreateDebugCallback(this);
        if(result != VK_SUCCESS)
        {
            throw std::exception{};
        }

        debugCallback = std::move(cInstance);
    }

    

    vkCreateDebugReportCallbackEXT()
}

void SampleApplication::Deinitialize()
{
}

void SampleApplication::Run()
{

}

Vector<String> SampleApplication::GetRequiredExtensionNames()
{
    uint32 requiredExtensionsCount = 0;
    char** requiredExtensions = glfwGetRequiredInstanceExtensions(&requiredExtensions);

    Vector<String> names;
    names.reserve(requiredExtensionsCount);

    for(uint32 i = 0; i < requiredExtensionsCount; i++)
    {
        names.emplace_back(requiredExtensions[i]);
    }

    requiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

    return requiredExtensions;
}

Vector<String> GetValidationLayerNames()
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
