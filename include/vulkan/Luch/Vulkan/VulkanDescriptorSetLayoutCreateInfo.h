#pragma once

#include <Luch/Vulkan.h>
#include <Luch/ShaderStage.h>
#include <Luch/Vulkan/DescriptorSetBinding.h>

namespace Luch::Vulkan
{
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
}
