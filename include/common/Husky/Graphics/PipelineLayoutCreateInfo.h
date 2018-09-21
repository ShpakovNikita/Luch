#pragma once

#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/DescriptorSetLayoutCreateInfo.h>

namespace Husky::Graphics
{
    struct PipelineLayoutCreateInfo
    {
        DescriptorSetLayoutCreateInfo descriptorSetLayout;
    }
}
