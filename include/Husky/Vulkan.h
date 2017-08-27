#pragma once

#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

#include <Husky/Assert.h>
#include <Husky/Types.h>

namespace Husky
{

class VulkanDebugDelegate
{
public:
    virtual VkBool32 DebugCallback(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objectType,
        uint64_t object,
        size_t location,
        int32_t messageCode,
        const char* pLayerPrefix,
        const char* pMessage
    ) = 0;
};

}
