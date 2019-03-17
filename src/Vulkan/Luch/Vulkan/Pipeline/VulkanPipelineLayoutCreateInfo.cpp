#include <Luch/Vulkan/Pipeline/VulkanPipelineLayoutCreateInfo.h>

#include <Luch/Vulkan/Shader/VulkanShaderStage.h>

namespace Luch::Vulkan
{
    PipelineLayoutCreateInfo::VulkanPipelineLayoutCreateInfo PipelineLayoutCreateInfo::ToVulkanCreateInfo(const PipelineLayoutCreateInfo& ci)
    {
        VulkanPipelineLayoutCreateInfo createInfo;
        createInfo.setLayouts.reserve(ci.setLayouts.size());
        for(auto setLayout : ci.setLayouts)
        {
            createInfo.setLayouts.push_back(setLayout->GetDescriptorSetLayout());
        }

        createInfo.pushConstantRanges.reserve(ci.pushConstantRanges.size());

        for (const auto& pushConstantRange : ci.pushConstantRanges)
        {
            vk::PushConstantRange range;
            range.setSize(pushConstantRange.size);
            range.setOffset(pushConstantRange.offset);
            range.setStageFlags(ToVulkanShaderStages(pushConstantRange.stages));
            createInfo.pushConstantRanges.push_back(range);
        }

        createInfo.createInfo.setPSetLayouts(createInfo.setLayouts.data());
        createInfo.createInfo.setSetLayoutCount((int32)createInfo.setLayouts.size());

        createInfo.createInfo.setPPushConstantRanges(createInfo.pushConstantRanges.data());
        createInfo.createInfo.setPushConstantRangeCount((int32)createInfo.pushConstantRanges.size());

        return createInfo;
    }

    PipelineLayoutCreateInfo::VulkanPipelineLayoutCreateInfo PipelineLayoutCreateInfo::ToVulkanCreateInfo(const Luch::Graphics::PipelineLayoutCreateInfo& ci)
    {
        VulkanPipelineLayoutCreateInfo createInfo;
        // todo: implement
        return createInfo;
    }
}
