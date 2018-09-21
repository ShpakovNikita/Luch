#pragma once

#include <Husky/Graphics/ShaderStage.h>
#include <Husky/Graphics/GraphicsForwards.h>

namespace Husky::Graphics
{
    enum class ResourceType
    {
        Texture,
        Sampler,
        UniformBuffer,
    };

    class DescriptorSetBinding
    {
        friend class DescriptorSetLayoutCreateInfo;
    public:
        inline DescriptorSetBinding& OfType(ResourceType aType)
        {
            type = aType;
            return *this;
        }

        inline DescriptorSetBinding& WithNBindings(int32 aCount)
        {
            count = aCount;
            return *this;
        }

        inline int32 GetCount() const { return count; }
        inline int32 GetBinding() const { return index; }
    private:
        ResourceType type;
        int32 count = 1;
        int32 index = -1;
        ShaderStage stages;
    };
}

