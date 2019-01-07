#pragma once

#include <Luch/Vulkan.h>
#include <Luch/Graphics/Format.h>

using namespace Luch::Graphics;
namespace Luch::Vulkan
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
        case Format::B8G8R8A8Unorm:
            return vk::Format::eB8G8R8A8Unorm;
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

        case Format::R8G8Sint:
            return vk::Format::eR8G8Sint;
        case Format::R8G8Uint:
            return vk::Format::eR8G8Uint;
        case Format::R16G16Sint:
            return vk::Format::eR16G16Sint;
        case Format::R16G16Uint:
            return vk::Format::eR16G16Uint;
        case Format::R32G32Uint:
            return vk::Format::eR32G32Uint;
        case Format::R32G32Sfloat:
            return vk::Format::eR32G32Sfloat;

        case Format::R8G8B8Sint:
            return vk::Format::eR8G8B8Sint;
        case Format::R8G8B8Uint:
            return vk::Format::eR8G8B8Uint;
        case Format::R16G16B16Sint:
            return vk::Format::eR16G16B16Sint;
        case Format::R16G16B16Uint:
            return vk::Format::eR16G16B16Uint;
        case Format::R32G32B32Uint:
            return vk::Format::eR32G32B32Uint;
        case Format::R32G32B32Sfloat:
            return vk::Format::eR32G32B32Sfloat;

        case Format::R8G8B8A8Sint:
            return vk::Format::eR8G8B8A8Sint;
        case Format::R8G8B8A8Uint:
            return vk::Format::eR8G8B8A8Uint;
        case Format::R16G16B16A16Sint:
            return vk::Format::eR16G16B16A16Sint;
        case Format::R16G16B16A16Uint:
            return vk::Format::eR16G16B16A16Uint;
        case Format::R32G32B32A32Uint:
            return vk::Format::eR32G32B32A32Uint;
        case Format::R32G32B32A32Sfloat:
            return vk::Format::eR32G32B32A32Sfloat;

        default:
            LUCH_ASSERT_MSG(false, "Unknown format");
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
        case vk::Format::eB8G8R8A8Unorm:
            return Format::B8G8R8A8Unorm;
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
            LUCH_ASSERT_MSG(false, "Unknown format");
        }
    }
}
