#pragma once

#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/ResourceType.h>

namespace Husky::Graphics
{
    class DescriptorSetBinding
    {
        friend class DescriptorSetLayoutCreateInfo;
    public:
        ResourceType GetType() const { return type; }
        void SetType(ResourceType aType) { type = aType; }
        int32 GetIndex() const { return index; }
    private:
        int32 index = -1;
        ResourceType type;
    };
}
