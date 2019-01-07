#pragma once

#include <Luch/Vulkan.h>
#include <Luch/Graphics/ShaderStage.h>

namespace Luch::Vulkan
{
    struct VulkanPushConstantRange
    {
        Graphics::ShaderStage stages;
        int32 offset = 0;
        int32 size = 0;
    };
}
