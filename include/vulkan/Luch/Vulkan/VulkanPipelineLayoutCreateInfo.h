#pragma once

#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanDescriptorSetLayout.h>
#include <Luch/Vulkan/VulkanPushConstantRange.h>
#include <Luch/Graphics/PipelineLayoutCreateInfo.h>

namespace Luch::Vulkan
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
            Vector<vk::PushConstantRange> pushConstantRanges;
        };

        static VulkanPipelineLayoutCreateInfo ToVulkanCreateInfo(const PipelineLayoutCreateInfo& ci);
        static VulkanPipelineLayoutCreateInfo ToVulkanCreateInfo(const Luch::Graphics::PipelineLayoutCreateInfo& ci);

        inline PipelineLayoutCreateInfo& WithNSetLayouts(int32 count)
        {
            setLayouts.reserve(count);
            return *this;
        }

        inline PipelineLayoutCreateInfo& AddSetLayout(VulkanDescriptorSetLayout* setLayout)
        {
            setLayouts.push_back(setLayout);
            return *this;
        }

        inline PipelineLayoutCreateInfo& WithNPushConstantRanges(int32 count)
        {
            pushConstantRanges.reserve(count);
            return *this;
        }

        inline PipelineLayoutCreateInfo& AddPushConstantRange(const VulkanPushConstantRange& pushConstantRange)
        {
            pushConstantRanges.push_back(pushConstantRange);
            return *this;
        }

        inline PipelineLayoutCreateInfo& AddPushConstantRange(Graphics::ShaderStage stages, int32 size)
        {
            VulkanPushConstantRange range;
            range.stages = stages;
            range.offset = currentPushConstantOffset;
            range.size = size;
            pushConstantRanges.push_back(range);
            currentPushConstantOffset += size;
            return *this;
        }

        // TODO push constants
    private:
        Vector<VulkanDescriptorSetLayout*> setLayouts;
        Vector<VulkanPushConstantRange> pushConstantRanges;
        int32 currentPushConstantOffset = 0;
    };
}
