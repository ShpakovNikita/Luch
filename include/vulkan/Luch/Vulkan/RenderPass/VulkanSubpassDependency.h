#pragma once

#include <Luch/Vulkan.h>
#include <Luch/Vulkan/Common/VulkanForwards.h>
#include <Luch/Vulkan/RenderPass/VulkanSubpassDescription.h>

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
