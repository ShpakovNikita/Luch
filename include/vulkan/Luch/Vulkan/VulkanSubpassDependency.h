#pragma once

#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>
#include <Luch/Vulkan/VulkanSubpassDescription.h>

namespace Luch::Vulkan
{
    struct VulkanSubpassDependency
    {
        VulkanSubpassDescription* srcSubpass = nullptr;
        VulkanSubpassDescription* dstSubpass = nullptr;
        vk::PipelineStageFlags srcStageMask;
        vk::PipelineStageFlags dstStageMask;
        vk::AccessFlags srcAccessMask;
        vk::AccessFlags dstAccessMask;
        bool byRegion = false;
    };
}
