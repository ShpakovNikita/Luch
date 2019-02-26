#pragma once

#include <Luch/Vulkan.h>
#include <Luch/BaseObject.h>

namespace Luch::Vulkan
{
    class VulkanInstance : public BaseObject
    {
    public:
        bool Init();
        VkInstance GetInstance() { return instance; }

    private:
        bool SetupDebugCallback();
        bool CheckValidationLayerSupport();

        VkInstance instance;
        std::vector<const char*> supportedExtensions;
    };
}
