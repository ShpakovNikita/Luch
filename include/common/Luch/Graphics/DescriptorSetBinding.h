#pragma once

#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/ResourceType.h>

namespace Luch::Graphics
{
    class DescriptorSetBinding
    {
        friend class DescriptorSetLayoutCreateInfo;
    public:
        inline ResourceType GetType() const { return type; }
        inline DescriptorSetBinding& OfType(ResourceType aType)
        {
            type = aType;
            return *this;
        }

        int32 GetIndex() const { return index; }
    private:
        int32 index = -1;
        ResourceType type;
    };
}
