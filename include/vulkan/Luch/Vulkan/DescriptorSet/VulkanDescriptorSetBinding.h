#pragma once

#include <Luch/Vulkan.h>
#include <Luch/Graphics/ShaderStage.h>

namespace Luch::Vulkan
{
    class VulkanDescriptorSetBinding
    {
        friend class DescriptorSetLayoutCreateInfo;
    public:
        inline VulkanDescriptorSetBinding& OfType(vk::DescriptorType aType)
        {
            type = aType;
            return *this;
        }

        inline VulkanDescriptorSetBinding& WithNBindings(int32 aCount)
        {
            count = aCount;
            return *this;
        }

        inline VulkanDescriptorSetBinding& AtStages(Graphics::ShaderStage aStages)
        {
            stages = aStages;
            return *this;
        }

        inline VulkanDescriptorSetBinding& AddImmutableSampler(vk::Sampler sampler)
        {
            immutableSamplers.push_back(sampler);
            return *this;
        }

        inline void SetIndex(int32 aIndex) { index = aIndex; }

        inline int32 GetCount() const { return count; }
        inline int32 GetBinding() const { return index; }
        inline vk::DescriptorType GetType() const { return type; }
        inline Graphics::ShaderStage GetStages() const  { return stages; }

        inline const Vector<vk::Sampler>& GetImmutableSamplers() const
        {
            return immutableSamplers;
        }

    private:
        vk::DescriptorType type;
        int32 count = 1;
        int32 index = -1;
        Graphics::ShaderStage stages;
        Vector<vk::Sampler> immutableSamplers;
    };
}
