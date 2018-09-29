#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Graphics/ShaderStage.h>

namespace Husky::Vulkan
{
    class VulkanDescriptorSetBinding
    {
        friend class VulkanDescriptorSetLayoutCreateInfo;
    public:
        inline DescriptorSetBinding& OfType(vk::DescriptorType aType)
        {
            type = aType;
            return *this;
        }

        inline DescriptorSetBinding& WithNBindings(int32 aCount)
        {
            count = aCount;
            return *this;
        }

        inline DescriptorSetBinding& AtStages(ShaderStage aStages)
        {
            stages = aStages;
            return *this;
        }

        inline DescriptorSetBinding& AddImmutableSampler(vk::Sampler sampler)
        {
            immutableSamplers.push_back(sampler);
            return *this;
        }

        inline int32 GetCount() const { return count; }
        inline int32 GetBinding() const { return index; }
    private:
        vk::DescriptorType type;
        int32 count = 1;
        int32 index = -1;
        ShaderStage stages;
        Vector<vk::Sampler> immutableSamplers;
    };
}
