#pragma once

#include <chrono>

#define NOMINMAX
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>
#undef CreateSemaphore

#include <Husky/Assert.h>
#include <Husky/Types.h>
#include <Husky/ResultValue.h>


namespace Husky::Vulkan
{

using Timeout = std::chrono::duration<int64, std::nano>;

inline uint64 ToVulkanTimeout(Optional<Timeout> timeout)
{
    if (timeout.has_value())
    {
        return timeout.value().count();
    }
    else
    {
        return Limits<uint64>::max();
    }
}

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
