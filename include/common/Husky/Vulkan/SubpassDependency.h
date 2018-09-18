#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Vulkan/VulkanForwards.h>

namespace Husky::Vulkan
{
    struct SubpassDependency
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
