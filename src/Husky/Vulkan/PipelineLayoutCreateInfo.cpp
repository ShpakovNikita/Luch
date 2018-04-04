#include <Husky/Vulkan/PipelineLayoutCreateInfo.h>

namespace Husky::Vulkan
{
    PipelineLayoutCreateInfo::VulkanPipelineLayoutCreateInfo PipelineLayoutCreateInfo::ToVulkanCreateInfo(const PipelineLayoutCreateInfo& ci)
    {
        VulkanPipelineLayoutCreateInfo createInfo;
        createInfo.setLayouts.reserve(ci.setLayouts.size());
        for(auto setLayout : ci.setLayouts)
        {
            createInfo.setLayouts.push_back(setLayout->GetDescriptorSetLayout());
        }

        createInfo.createInfo.setPSetLayouts(createInfo.setLayouts.data());
        createInfo.createInfo.setSetLayoutCount((int32)createInfo.setLayouts.size());

        return createInfo;
    }
}