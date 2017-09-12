#pragma once

#define NOMINMAX
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

#include <Husky/Assert.h>
#include <Husky/Types.h>
#include <Husky/ResultValue.h>

namespace Husky::Vulkan
{

template<typename Value>
using VulkanResultValue = ResultValue<vk::Result, Value>;

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
