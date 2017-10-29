#pragma once

#include <Husky/Vulkan.h>
#include <Husky/ShaderStage.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;
    class DescriptorSetLayoutCreateInfo;

    class DescriptorSetBinding
    {
        friend class DescriptorSetLayoutCreateInfo;
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

    class DescriptorSetLayoutCreateInfo
    {
    public:
        struct VulkanDescriptorSetLayoutCreateInfo
        {
            VulkanDescriptorSetLayoutCreateInfo() = default;

            VulkanDescriptorSetLayoutCreateInfo(const VulkanDescriptorSetLayoutCreateInfo& other) = delete;
            VulkanDescriptorSetLayoutCreateInfo& operator=(const VulkanDescriptorSetLayoutCreateInfo& other) = delete;

            VulkanDescriptorSetLayoutCreateInfo(VulkanDescriptorSetLayoutCreateInfo&& other) = default;
            VulkanDescriptorSetLayoutCreateInfo& operator=(VulkanDescriptorSetLayoutCreateInfo&& other) = default;

            vk::DescriptorSetLayoutCreateInfo createInfo;
            Vector<vk::DescriptorSetLayoutBinding> bindings;
            Vector<Vector<vk::Sampler>> samplers;
        };

        static VulkanDescriptorSetLayoutCreateInfo ToVulkanCreateInfo(const DescriptorSetLayoutCreateInfo& ci);

        // descriptor set binding must be in a valid state
        // changing the binding after calling this function can lead to undefined behavior
        inline DescriptorSetLayoutCreateInfo& AddBinding(DescriptorSetBinding* binding)
        {
            binding->index = currentBindingIndex;
            currentBindingIndex += binding->count;
            bindings.push_back(binding);
            return *this;
        }
    private:
        Vector<DescriptorSetBinding*> bindings;
        int32 currentBindingIndex = 0;
    };

    class DescriptorSetLayout
    {
        friend class GraphicsDevice;
    public:
        DescriptorSetLayout() = default;

        DescriptorSetLayout(DescriptorSetLayout&& other);
        DescriptorSetLayout& operator=(DescriptorSetLayout&& other);

        ~DescriptorSetLayout();

        vk::DescriptorSetLayout GetDescriptorSetLayout() { return descriptorSetLayout; }
    private:
        DescriptorSetLayout(GraphicsDevice* device, vk::DescriptorSetLayout descriptorSetLayout);
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::DescriptorSetLayout descriptorSetLayout;
    };
}
