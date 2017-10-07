#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Flags.h>

namespace Husky::Vulkan
{
    class ShaderModule;
    class Pipeline;
    class PipelineLayout;
    class RenderPass;

    enum class ShaderStage
    {
        Fragment = 1 << 0,
        Geometry = 1 << 1,
        TesselationControl = 1 << 2,
        TesselationEvaluation = 1 << 3,
        Vertex = 1 << 4,
        Compute = 1 << 5,
    };

    enum DynamicState
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
    };

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

    struct RasterizationDepthBias
    {
        float32 constantFactor = 0.0f;
        float32 clamp = 0.0f;
        float32 slopeFactor = 0.0f;
    };

    struct PipelineRasterizationStateCreateInfo
    {
        bool depthClampEnable = false;
        bool rasterizerDiscardEnable = false;
        vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
        vk::CullModeFlags cullMode = vk::CullModeFlagBits::eNone;
        vk::FrontFace frontFace = vk::FrontFace::eClockwise;
        Optional<RasterizationDepthBias> depthBias;
        float32 lineWidth = 1.0f;
    };

    struct PipelineMultisampleStateCreateInfo
    {
        int32 rasterizationSamples = 1;
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
        Vector<DynamicState> dynamicStates;
    };

    struct GraphicsPipelineCreateInfo
    {
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
        PipelineLayout* layout = nullptr;
        RenderPass* renderPass = nullptr;
    };

    vk::GraphicsPipelineCreateInfo ToVulkanGraphicsPipelineCreateInfo(const GraphicsPipelineCreateInfo& ci);
}
