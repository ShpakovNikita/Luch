#include <Husky/Vulkan/PipelineCreateInfo.h>
#include <Husky/Vulkan/ShaderModule.h>

namespace Husky::Vulkan
{
    VulkanGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo::ToVkCreateInfo(const GraphicsPipelineCreateInfo& ci)
    {
        VulkanGraphicsPipelineCreateInfo result;

        result.shaderStages.reserve(ci.shaderStages.size());

        for (auto& stage : ci.shaderStages)
        {
            auto& vulkanStage = result.shaderStages.emplace_back();

            vulkanStage.setModule(stage.shaderModule->GetModule());
            vulkanStage.setPName(stage.name.c_str());
            vulkanStage.setStage(ToVulkanShaderStage(stage.stage));
            // TODO vulkanStage.setPSpecializationInfo
        }

        result.bindingDescriptions = ci.vertexInputState.bindingDescriptions;
        result.attributeDescriptions = ci.vertexInputState.attributeDescriptions;

        // TODO
        //result.bindingDescriptions.reserve(ci.vertexInputState.bindingDescriptions.size());
        //for (auto& bindingDescription : ci.vertexInputState.bindingDescriptions)
        //{
        //    auto& vulkanBindingDescription = result.bindingDescriptions.
        //}



        result.createInfo.setStageCount((int32)result.shaderStages.size());
        result.createInfo.setPStages(result.shaderStages.data());
        result.createInfo.setPVertexInputState()

        return result;
    }
}
