#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Format.h>

namespace Husky::Vulkan
{
    // TODO move to cpp
    inline vk::Format ToVulkanFormat(Format format)
    {
        switch (format)
        {
        case Format::Undefined:
            return vk::Format::eUndefined;
        case Format::R8G8B8A8Unorm:
            return vk::Format::eR8G8B8A8Unorm;
        case Format::D16Unorm:
            return vk::Format::eD16Unorm;
        case Format::D32Sfloat:
            return vk::Format::eD32Sfloat;
        case Format::S8Uint:
            return vk::Format::eS8Uint;
        case Format::D16UnormS8Uint:
            return vk::Format::eD16UnormS8Uint;
        case Format::D24UnormS8Uint:
            return vk::Format::eD24UnormS8Uint;
        case Format::D32SfloatS8Uint:
            return vk::Format::eD32SfloatS8Uint;
        default:
            HUSKY_ASSERT(false, "Unknown format");
        }
    }

    inline Format FromVulkanFormat(vk::Format format)
    {
        switch (format)
        {
        case vk::Format::eUndefined:
            return Format::Undefined;
        case vk::Format::eR8G8B8A8Unorm:
            return Format::R8G8B8A8Unorm;
        case vk::Format::eD16Unorm:
            return Format::D16Unorm;
        case vk::Format::eD32Sfloat:
            return Format::D32Sfloat;
        case vk::Format::eS8Uint:
            return Format::S8Uint;
        case vk::Format::eD16UnormS8Uint:
            return Format::D16UnormS8Uint;
        case vk::Format::eD24UnormS8Uint:
            return Format::D24UnormS8Uint;
        case vk::Format::eD32SfloatS8Uint:
            return Format::D32SfloatS8Uint;
        default:
            HUSKY_ASSERT(false, "Unknown format");
        }
    }
}
