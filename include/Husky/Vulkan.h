#pragma once

#include <vulkan/vulkan.hpp>

#include <Husky/Assert.h>
#include <Husky/Types.h>

#include <Husky/Vulkan/VkUniquePtr.h>
#include <Husky/Vulkan/VkArrayParam.h>
#include <Husky/Vulkan/VkResult.h>

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
