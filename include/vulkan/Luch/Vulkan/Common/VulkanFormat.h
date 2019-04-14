#pragma once

#include <Luch/Vulkan.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Graphics/ResourceType.h>

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
        case Format::RGB8Unorm:
            return vk::Format::eR8G8B8Unorm;
        case Format::RGBA8Unorm:
            return vk::Format::eR8G8B8A8Unorm;
        case Format::BGRA8Unorm:
            return vk::Format::eB8G8R8A8Unorm;
        case Format::BGRA8Unorm_sRGB:
            return vk::Format::eB8G8R8A8Srgb;
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
            return Format::RGB8Unorm;
        case vk::Format::eB8G8R8A8Unorm:
            return Format::BGRA8Unorm;
        case vk::Format::eB8G8R8A8Srgb:
            return Format::BGRA8Unorm_sRGB;
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

    inline vk::DescriptorType ToVulkanType(ResourceType type)
    {
        switch (type)
        {
            case ResourceType::Sampler:
                return vk::DescriptorType::eSampler;
            case ResourceType::Texture:
                return vk::DescriptorType::eSampledImage;
            case ResourceType::UniformBuffer:
                return vk::DescriptorType::eUniformBuffer;
            case ResourceType::ThreadgroupMemory:
                LUCH_ASSERT_MSG(false, "unsupported resource type ResourceType::ThreadgroupMemory");
                return vk::DescriptorType::eStorageBuffer;
        }
    }
}
