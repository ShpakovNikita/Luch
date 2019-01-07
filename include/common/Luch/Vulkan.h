#pragma once
#if LUCH_USE_VULKAN
#include <chrono>

#define NOMINMAX
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>
#undef CreateSemaphore

#include <Luch/Assert.h>
#include <Luch/Types.h>
#include <Luch/RefPtr.h>
#include <Luch/ResultValue.h>

namespace Luch::Vulkan
{

using Timeout = std::chrono::duration<int64, std::nano>;

inline uint64 ToVulkanTimeout(Optional<Timeout> timeout)
{
    if (timeout.has_value())
    {
        return timeout->count();
    }
    else
    {
        return Limits<uint64>::max();
    }
}

inline uint32 ToVulkanSize(size_t size)
{
    if(size > std::numeric_limits<uint32>::max())
    {
        LUCH_ASSERT_MSG(false, "Too big");
    }

    return static_cast<uint32>(size);
}

template<typename Value>
using VulkanResultValue = ResultValue<vk::Result, Value>;

template<typename Value>
using VulkanRefResultValue = ResultValue<vk::Result, RefPtr<Value>>;

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

#endif
