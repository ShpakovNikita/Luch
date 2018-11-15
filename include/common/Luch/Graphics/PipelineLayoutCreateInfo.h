#pragma once

#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/DescriptorSetLayout.h>
#include <Luch/Graphics/DescriptorSetLayoutCreateInfo.h>
#include <Luch/Graphics/ShaderStage.h>

namespace Luch::Graphics
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

        inline PipelineLayoutCreateInfo& AddSetLayout(ShaderStage stage, DescriptorSetLayout* descriptorSetLayout)
        {
            auto type = descriptorSetLayout->GetCreateInfo().type;
            switch(type)
            {
            case DescriptorSetType::Texture:
                stages[stage].textureSetLayouts.push_back(descriptorSetLayout);
                break;
            case DescriptorSetType::Buffer:
                stages[stage].bufferSetLayouts.push_back(descriptorSetLayout);
                break;
            case DescriptorSetType::Sampler:
                stages[stage].samplerSetLayouts.push_back(descriptorSetLayout);
                break;
            default:
                LUCH_ASSERT(false);
            }
            return *this;
        }
    };
}
