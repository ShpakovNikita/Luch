#pragma once

#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/ResourceType.h>
#include <Luch/Graphics/ShaderStage.h>

namespace Luch::Graphics
{
    class DescriptorSetBinding
    {
        friend class DescriptorSetLayoutCreateInfo;
    public:
        static constexpr uint32 UndefinedIndex = -1;

        inline ResourceType GetType() const { return type; }
        inline DescriptorSetBinding& OfType(ResourceType aType)
        {
            type = aType;
            return *this;
        }

        uint32 GetIndex() const { return index; }
    private:
        uint32 index = UndefinedIndex;
        ResourceType type;
    };
}
