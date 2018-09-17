#include <Husky/Vulkan/PipelineCreateInfo.h>
#include <Husky/Vulkan/ShaderModule.h>
#include <Husky/Vulkan/SampleCount.h>
#include <Husky/Vulkan/RenderPass.h>
#include <Husky/Vulkan/PipelineLayout.h>
#include <Husky/Vulkan/ShaderStage.h>

namespace Husky::Vulkan
{
    UniquePtr<VulkanGraphicsPipelineCreateInfo> GraphicsPipelineCreateInfo::ToVulkanCreateInfo(const GraphicsPipelineCreateInfo& ci)
    {
        UniquePtr<VulkanGraphicsPipelineCreateInfo> result = MakeUnique<VulkanGraphicsPipelineCreateInfo>();

        result->shaderStages.reserve(ci.shaderStages.size());

        for (auto& stage : ci.shaderStages)
        {
            auto& vulkanStage = result->shaderStages.emplace_back();

            vulkanStage.setModule(stage.shaderModule->GetModule());
            vulkanStage.setPName(stage.name.c_str());
            vulkanStage.setStage(ToVulkanShaderStage(stage.stage));
        }

        result->bindingDescriptions = ci.vertexInputState.bindingDescriptions;
        result->attributeDescriptions = ci.vertexInputState.attributeDescriptions;

        result->vertexInputState.setVertexBindingDescriptionCount((int32)result->bindingDescriptions.size());
        result->vertexInputState.setPVertexBindingDescriptions(result->bindingDescriptions.data());

        result->vertexInputState.setVertexAttributeDescriptionCount((int32)result->attributeDescriptions.size());
        result->vertexInputState.setPVertexAttributeDescriptions(result->attributeDescriptions.data());

        result->createInfo.setStageCount((int32)result->shaderStages.size());
        result->createInfo.setPStages(result->shaderStages.data());

        result->createInfo.setPVertexInputState(&result->vertexInputState);

        result->inputAssemblyState.setPrimitiveRestartEnable(ci.inputAssemblyState.primitiveRestartEnabled);
        result->inputAssemblyState.setTopology(ci.inputAssemblyState.topology);

        result->createInfo.setPInputAssemblyState(&result->inputAssemblyState);

        // TODO
        result->viewports = ci.viewportState.viewports;
        result->scissors = ci.viewportState.scissors;

        result->viewportState.setViewportCount((int32)result->scissors.size());
        result->viewportState.setPViewports(result->viewports.data());

        result->viewportState.setScissorCount((int32)result->scissors.size());
        result->viewportState.setPScissors(result->scissors.data());

        result->createInfo.setPViewportState(&result->viewportState);

        result->rasterizationState.polygonMode = ci.rasterizationState.polygonMode;
        result->rasterizationState.cullMode = ci.rasterizationState.cullMode;
        result->rasterizationState.frontFace = ci.rasterizationState.frontFace;
        result->rasterizationState.depthBiasClamp = ci.rasterizationState.depthBiasClamp;
        result->rasterizationState.depthBiasConstantFactor = ci.rasterizationState.depthBiasConstantFactor;
        result->rasterizationState.lineWidth = ci.rasterizationState.lineWidth;
        result->rasterizationState.depthClampEnable = ci.rasterizationState.depthClampEnable;
        result->rasterizationState.depthBiasEnable = ci.rasterizationState.depthBiasEnable;
        result->rasterizationState.rasterizerDiscardEnable = ci.rasterizationState.rasterizerDiscardEnable;

        result->createInfo.setPRasterizationState(&result->rasterizationState);

        result->multisampleState.rasterizationSamples = ToVulkanSampleCount(ci.multisampleState.rasterizationSamples);
        result->multisampleState.sampleShadingEnable = ci.multisampleState.sampleShadingEnable;
        result->multisampleState.minSampleShading = ci.multisampleState.minSampleShading;
        // TODO samplemask
        result->multisampleState.alphaToCoverageEnable = ci.multisampleState.alphaToCoverageEnable;
        result->multisampleState.alphaToOneEnable = ci.multisampleState.alphaToOneEnable;

        result->createInfo.setPMultisampleState(&result->multisampleState);

        result->depthStencilState.depthTestEnable = ci.depthStencilState.depthTestEnable;
        result->depthStencilState.depthWriteEnable = ci.depthStencilState.depthWriteEnable;
        result->depthStencilState.depthCompareOp = ci.depthStencilState.depthCompareOp;
        result->depthStencilState.depthBoundsTestEnable = ci.depthStencilState.depthBoundsTestEnable;
        result->depthStencilState.stencilTestEnable = ci.depthStencilState.stencilTestEnable;
        result->depthStencilState.front = ci.depthStencilState.front;
        result->depthStencilState.back = ci.depthStencilState.back;
        result->depthStencilState.minDepthBounds = ci.depthStencilState.minDepthBounds;
        result->depthStencilState.maxDepthBounds = ci.depthStencilState.maxDepthBounds;

        result->createInfo.setPDepthStencilState(&result->depthStencilState);

        result->attachments.reserve((int32)ci.colorBlendState.attachments.size());
        for (auto& attachment : ci.colorBlendState.attachments)
        {
            auto& vulkanAttachment = result->attachments.emplace_back();

            vulkanAttachment.blendEnable = attachment.blendEnable;
            vulkanAttachment.srcColorBlendFactor = attachment.srcColorBlendFactor;
            vulkanAttachment.dstColorBlendFactor = attachment.dstColorBlendFactor;
            vulkanAttachment.colorBlendOp = attachment.colorBlendOp;
            vulkanAttachment.srcAlphaBlendFactor = attachment.srcAlphaBlendFactor;
            vulkanAttachment.dstAlphaBlendFactor = attachment.dstAlphaBlendFactor;
            vulkanAttachment.alphaBlendOp = attachment.alphaBlendOp;
            vulkanAttachment.colorWriteMask = attachment.colorWriteMask;
        }

        result->colorBlendState.logicOpEnable = ci.colorBlendState.logicOpEnable;
        result->colorBlendState.logicOp = ci.colorBlendState.logicOp;
        std::copy(
            ci.colorBlendState.blendConstants.begin(),
            ci.colorBlendState.blendConstants.end(),
            std::begin(result->colorBlendState.blendConstants));
        result->colorBlendState.setAttachmentCount((int32)result->attachments.size());
        result->colorBlendState.setPAttachments(result->attachments.data());

        result->createInfo.setPColorBlendState(&result->colorBlendState);

        if(!ci.dynamicState.dynamicStates.empty())
        {
            result->dynamicStates = ci.dynamicState.dynamicStates;

            result->dynamicState.setDynamicStateCount((int32)result->dynamicStates.size());
            result->dynamicState.setPDynamicStates(result->dynamicStates.data());

            result->createInfo.setPDynamicState(&result->dynamicState);
        }

        result->createInfo.setRenderPass(ci.renderPass->GetRenderPass());
        result->createInfo.setLayout(ci.layout->GetPipelineLayout());

        return result;
    }
}
