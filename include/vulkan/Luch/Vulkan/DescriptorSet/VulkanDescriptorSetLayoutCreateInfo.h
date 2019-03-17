#pragma once

#include <Luch/Vulkan.h>
#include <Luch/Graphics/ShaderStage.h>
#include <Luch/Graphics/DescriptorSetLayoutCreateInfo.h>
#include <Luch/Vulkan/DescriptorSet/VulkanDescriptorSetBinding.h>

using namespace Luch::Graphics;

namespace Luch::Vulkan
{
    struct VulkanDescriptorSetLayoutCreateInfo
    {
    public:
        VulkanDescriptorSetLayoutCreateInfo() = default;

        VulkanDescriptorSetLayoutCreateInfo(const VulkanDescriptorSetLayoutCreateInfo& other) = delete;
        VulkanDescriptorSetLayoutCreateInfo& operator=(const VulkanDescriptorSetLayoutCreateInfo& other) = delete;

        VulkanDescriptorSetLayoutCreateInfo(VulkanDescriptorSetLayoutCreateInfo&& other) = default;
        VulkanDescriptorSetLayoutCreateInfo& operator=(VulkanDescriptorSetLayoutCreateInfo&& other) = default;

        static VulkanDescriptorSetLayoutCreateInfo ToVulkanCreateInfo(const DescriptorSetLayoutCreateInfo& ci);

        vk::DescriptorSetLayoutCreateInfo createInfo;
    private:
        Vector<vk::DescriptorSetLayoutBinding> bindings;
        Vector<Vector<vk::Sampler>> samplers;
    };

}
