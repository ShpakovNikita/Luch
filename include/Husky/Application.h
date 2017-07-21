#pragma once

#include <GLFW/glfw3.h>
#include <Husky/Types.h>
#include <Husky/Version.h>
#include <Husky/Vulkan.h>

namespace Husky
{

    class Application
    {
    public:
        Application() = default;

        void Initialize(const std::vector<String>& args);
        void Deinitialize();
        void Run();


    private:
        VkCreateResult<VkInstance> CreateInstance();

        String applicationName;
        Version applicationVersion;

        String engineName;
        Version engineVersion;

        Vector<const char*> GetRequiredExtensionNames();
        Vector<const char*> GetValidationLayerNames();

        Vector<const char*> requiredExtensionNames;
        Vector<const char*> validationLayerNames;

        VkPtr<VkInstance> instance;
        VkPtr<VkSurfaceKHR> surface;

        VkPtr<VkPhysicalDevice> physicalDevice;
        VkPtr<VkDevice> device;
    };

}
