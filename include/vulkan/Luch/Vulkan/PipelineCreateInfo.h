#pragma once

#include <Luch/Vulkan.h>
#include <Luch/Graphics/ShaderStage.h>
#include <Luch/UniquePtr.h>
#include <Luch/SampleCount.h>
#include <Luch/Vulkan/VulkanForwards.h>

namespace Luch::Vulkan
{
    /*enum DynamicState
    {
        Viewport,
        Scissor,
        LineWidth,
        DepthBias,
        BlendConstants,
        DepthBounds,
        StencilCompareMask,
        StencilWriteMask,
        StencilReference,
    };*/

    struct PipelineShaderStageCreateInfo
    {
        ShaderModule* shaderModule = nullptr;
        ShaderStage stage;
        String name;
        // TODO specialization
    };

    struct PipelineVertexInputStateCreateInfo
    {
        Vector<vk::VertexInputBindingDescription> bindingDescriptions;
        Vector<vk::VertexInputAttributeDescription> attributeDescriptions;
    };

    struct PipelineInputAssemblyStateCreateInfo
    {
        vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
        bool primitiveRestartEnabled = false;
    };

    // TODO tesselation

    struct PipelineViewportStateCreateInfo
    {
        Vector<vk::Viewport> viewports;
        Vector<vk::Rect2D> scissors;
    };

    struct PipelineRasterizationStateCreateInfo
    {
        vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
        vk::CullModeFlags cullMode = vk::CullModeFlagBits::eNone;
        vk::FrontFace frontFace = vk::FrontFace::eClockwise;
        float32 depthBiasConstantFactor = 0.0f;
        float32 depthBiasClamp = 0.0f;
        float32 depthBiasSlopeFactor = 0.0f;
        float32 lineWidth = 1.0f;
        bool depthClampEnable = false;
        bool depthBiasEnable = false;
        bool rasterizerDiscardEnable = false;
    };

    struct PipelineMultisampleStateCreateInfo
    {
        SampleCount rasterizationSamples = SampleCount::e1;
        bool sampleShadingEnable = false;
        float32  minSampleShading = 0;
        // TODO samplemask
        bool alphaToCoverageEnable = false;
        bool alphaToOneEnable = false;
    };

    struct PipelineDepthStencilStateCreateInfo
    {
        bool depthTestEnable = false;
        bool depthWriteEnable = false;
        vk::CompareOp depthCompareOp = vk::CompareOp::eNever;
        bool depthBoundsTestEnable = false;
        bool stencilTestEnable = false;
        vk::StencilOpState front;
        vk::StencilOpState back;
        float32 minDepthBounds = 0.0f;
        float32 maxDepthBounds = 1.0f;
    };

    struct PipelineColorBlendAttachmentState
    {
        bool blendEnable = false;
        vk::BlendFactor srcColorBlendFactor = vk::BlendFactor::eOne;
        vk::BlendFactor dstColorBlendFactor = vk::BlendFactor::eZero;
        vk::BlendOp colorBlendOp = vk::BlendOp::eAdd;
        vk::BlendFactor srcAlphaBlendFactor = vk::BlendFactor::eOne;
        vk::BlendFactor dstAlphaBlendFactor = vk::BlendFactor::eZero;
        vk::BlendOp alphaBlendOp = vk::BlendOp::eAdd;
        vk::ColorComponentFlags colorWriteMask =
            vk::ColorComponentFlagBits::eR
          | vk::ColorComponentFlagBits::eG
          | vk::ColorComponentFlagBits::eB
          | vk::ColorComponentFlagBits::eA;
    };

    struct PipelineColorBlendStateCreateInfo
    {
        bool logicOpEnable = false;
        vk::LogicOp logicOp = vk::LogicOp::eClear;
        Vector<PipelineColorBlendAttachmentState> attachments;
        Array<float32, 4> blendConstants;
    };

    struct PipelieDynamicStateCreateInfo
    {
        Vector<vk::DynamicState> dynamicStates;
    };

    struct VulkanGraphicsPipelineCreateInfo
    {
        VulkanGraphicsPipelineCreateInfo() = default;

        VulkanGraphicsPipelineCreateInfo(const VulkanGraphicsPipelineCreateInfo& other) = delete;
        VulkanGraphicsPipelineCreateInfo& operator=(const VulkanGraphicsPipelineCreateInfo& other) = delete;

        VulkanGraphicsPipelineCreateInfo(VulkanGraphicsPipelineCreateInfo&& other) = default;
        VulkanGraphicsPipelineCreateInfo& operator=(VulkanGraphicsPipelineCreateInfo&& other) = default;

        vk::GraphicsPipelineCreateInfo createInfo;
        vk::PipelineVertexInputStateCreateInfo vertexInputState;
        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
        //vk::PipelineTessellationStateCreateInfo tessellationState;
        vk::PipelineViewportStateCreateInfo viewportState;
        vk::PipelineRasterizationStateCreateInfo rasterizationState;
        vk::PipelineMultisampleStateCreateInfo multisampleState;
        vk::PipelineDepthStencilStateCreateInfo depthStencilState;
        vk::PipelineColorBlendStateCreateInfo colorBlendState;
        vk::PipelineDynamicStateCreateInfo dynamicState;

        Vector<vk::PipelineShaderStageCreateInfo> shaderStages;
        Vector<vk::VertexInputBindingDescription> bindingDescriptions;
        Vector<vk::VertexInputAttributeDescription> attributeDescriptions;
        Vector<vk::Viewport> viewports;
        Vector<vk::Rect2D> scissors;
        Vector<vk::PipelineColorBlendAttachmentState> attachments;
        Vector<vk::DynamicState> dynamicStates;
    };

    struct GraphicsPipelineCreateInfo
    {
        static UniquePtr<VulkanGraphicsPipelineCreateInfo> ToVulkanCreateInfo(const GraphicsPipelineCreateInfo& ci);

        Vector<PipelineShaderStageCreateInfo> shaderStages;
        PipelineVertexInputStateCreateInfo vertexInputState;
        PipelineInputAssemblyStateCreateInfo inputAssemblyState;
        // TODO tesselation
        PipelineViewportStateCreateInfo viewportState;
        PipelineRasterizationStateCreateInfo rasterizationState;
        PipelineMultisampleStateCreateInfo multisampleState;
        PipelineDepthStencilStateCreateInfo depthStencilState;
        PipelineColorBlendStateCreateInfo colorBlendState;
        PipelieDynamicStateCreateInfo dynamicState;
        VulkanPipelineLayout* layout = nullptr;
        VulkanRenderPass* renderPass = nullptr;
    };
}
