#pragma once

#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/ShaderStage.h>

namespace Husky::Graphics
{
    struct PipelineLayoutCreateInfo
    {
        struct StageDescriptorSetLayouts
        {
            Vector<DescriptorSetLayout*> textureSetLayouts;
            Vector<DescriptorSetLayout*> bufferSetLayouts;
            Vector<DescriptorSetLayout*> samplerSetLayouts;
        };

        UnorderedMap<ShaderStage, StageDescriptorSetLayouts> stages;
    };
}
