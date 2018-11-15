#pragma once

#include <Luch/Vulkan.h>
#include <Luch/ShaderStage.h>

namespace Luch::Vulkan
{
    struct VulkanPushConstantRange
    {
        ShaderStage stages;
        int32 offset = 0;
        int32 size = 0;
    };
}
