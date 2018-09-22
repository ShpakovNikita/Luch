#pragma once

#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/ShaderStage.h>

namespace Husky::Graphics
{
    struct PipelineLayoutCreateInfo
    {
        struct StageDescriptorSetLayouts
        {
            DescriptorSetLayout* textures = nullptr;
            DescriptorSetLayout* buffers = nullptr;
            DescriptorSetLayout* samplers = nullptr;
        };

        UnorderedMap<ShaderStage, StageDescriptorSetLayouts> stages;
    }
}
