#pragma once

#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/ResourceType.h>

namespace Luch::Graphics
{
    struct DescriptorPoolCreateInfo
    {
        UnorderedMap<ResourceType, int32> descriptorCount;
        int32 maxDescriptorSets = 0;
    };
}
