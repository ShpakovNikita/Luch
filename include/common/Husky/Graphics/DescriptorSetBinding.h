#pragma once

#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/ResourceType.h>
#include <Husky/Graphics/ShaderStage.h>

namespace Husky::Graphics
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

        inline DescriptorSetBinding& AtStage(ShaderStage stage)
        {
            shaderStage = stage;
            return *this;
        }

        int32 GetIndex() const { return index; }
    private:
        int32 index = -1;
        ResourceType type;
        ShaderStage shaderStage;
    };
}
