#pragma once

#include <Luch/Vulkan.h>
#include <Luch/Graphics/ShaderStage.h>
#include <Luch/Graphics/DescriptorSetLayoutCreateInfo.h>
#include <Luch/Vulkan/VulkanDescriptorSetBinding.h>

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
        static VulkanDescriptorSetLayoutCreateInfo ToVulkanCreateInfo(const Luch::Graphics::DescriptorSetLayoutCreateInfo& ci);

        // descriptor set binding must be in a valid state
        // changing the binding after calling this function can lead to undefined behavior
        inline DescriptorSetLayoutCreateInfo& AddBinding(VulkanDescriptorSetBinding* binding)
        {
            binding->SetIndex(currentBindingIndex);
            currentBindingIndex += binding->GetCount();
            bindings.push_back(binding);
            return *this;
        }
    private:
        Vector<VulkanDescriptorSetBinding*> bindings;
        int32 currentBindingIndex = 0;
    };
}
