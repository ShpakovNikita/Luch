#pragma once

#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/ResourceType.h>

namespace Husky::Graphics
{
    struct DescriptorPoolCreateInfo
    {
        UnorderedMap<ResourceType, int32> descriptorCount;
        int32 maxDescriptorSets = 0;
    };
}
