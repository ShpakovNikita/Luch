#pragma once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    struct VulkanSwapchainCreateInfo
    {
        int32 imageCount = 0;
        int32 width = 0;
        int32 height = 0;
        int32 arrayLayers = 1;
        Format format = Format::Undefined;
        vk::ColorSpaceKHR colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
    };
}
