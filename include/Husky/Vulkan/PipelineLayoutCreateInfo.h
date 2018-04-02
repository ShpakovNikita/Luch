#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Vulkan/DescriptorSetLayout.h>

namespace Husky::Vulkan
{
    class PipelineLayoutCreateInfo
    {
    public:
        struct VulkanPipelineLayoutCreateInfo
        {
            VulkanPipelineLayoutCreateInfo() = default;

            VulkanPipelineLayoutCreateInfo(const VulkanPipelineLayoutCreateInfo& other) = delete;
            VulkanPipelineLayoutCreateInfo& operator=(const VulkanPipelineLayoutCreateInfo& other) = delete;

            VulkanPipelineLayoutCreateInfo(VulkanPipelineLayoutCreateInfo&& other) = default;
            VulkanPipelineLayoutCreateInfo& operator=(VulkanPipelineLayoutCreateInfo&& other) = default;

            vk::PipelineLayoutCreateInfo createInfo;
            Vector<vk::DescriptorSetLayout> setLayouts;
        };

        static VulkanPipelineLayoutCreateInfo ToVulkanCreateInfo(const PipelineLayoutCreateInfo& ci);

        inline PipelineLayoutCreateInfo& WithNSetLayouts(int32 count)
        {
            setLayouts.reserve(count);
            return *this;
        }

        inline PipelineLayoutCreateInfo& AddSetLayout(DescriptorSetLayout* setLayout)
        {
            setLayouts.push_back(setLayout);
            return *this;
        }

        // TODO push constants
    private:
        Vector<DescriptorSetLayout*> setLayouts;
    };
}