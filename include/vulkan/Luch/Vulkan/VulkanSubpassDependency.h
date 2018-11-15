#pragma once

#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>

namespace Luch::Vulkan
{
    struct VulkanSubpassDependency
    {
        SubpassDescription* srcSubpass = nullptr;
        SubpassDescription* dstSubpass = nullptr;
        vk::PipelineStageFlags srcStageMask;
        vk::PipelineStageFlags dstStageMask;
        vk::AccessFlags srcAccessMask;
        vk::AccessFlags dstAccessMask;
        bool byRegion = false;
    };
}
