#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Vulkan/DescriptorSetLayout.h>
#include <Husky/Vulkan/PushConstantRange.h>

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

        inline PipelineLayoutCreateInfo& WithNPushConstantRanges(int32 count)
        {
            pushConstantRanges.reserve(count);
            return *this;
        }

        inline PipelineLayoutCreateInfo& AddPushConstantRange(const PushConstantRange& pushConstantRange)
        {
            pushConstantRanges.push_back(pushConstantRange);
            return *this;
        }

        // TODO push constants
    private:
        Vector<DescriptorSetLayout*> setLayouts;
        Vector<PushConstantRange> pushConstantRanges;
    };
}