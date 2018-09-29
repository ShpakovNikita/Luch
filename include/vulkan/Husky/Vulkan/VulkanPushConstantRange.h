#pragma once

#include <Husky/Vulkan.h>
#include <Husky/ShaderStage.h>

namespace Husky::Vulkan
{
    struct VulkanPushConstantRange
    {
        ShaderStage stages;
        int32 offset = 0;
        int32 size = 0;
    };
}
