#pragma once

#include <Husky/ImageAspects.h>

namespace Husky::Vulkan
{
    inline vk::ImageAspectFlags ToVulkanImageAspectFlags(ImageAspects aspects)
    {
        vk::ImageAspectFlags result;

        if ((aspects & ImageAspects::Color) == ImageAspects::Color)
        {
            result |= vk::ImageAspectFlagBits::eColor;
        }
        if ((aspects & ImageAspects::Depth) == ImageAspects::Depth)
        {
            result |= vk::ImageAspectFlagBits::eDepth;
        }
        if ((aspects & ImageAspects::Stencil) == ImageAspects::Stencil)
        {
            result |= vk::ImageAspectFlagBits::eColor;
        }

        return result;
    }
}
