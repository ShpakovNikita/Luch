#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Flags.h>

namespace Husky::Vulkan
{
    class ShaderModule;
    class Pipeline;
    class PipelineLayout;

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
        ShaderModule* shaderModule;
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
        vk::PrimitiveTopology topology;
        bool primitiveRestartEnabled;
    };

    // TODO tesselation

    struct PipelineViewportStateCreateInfo
    {
        Vector<vk::Viewport> viewports;
        Vector<vk::Rect2D> scissors;
    };

    struct RasterizationDepthBias
    {
        float32 constantFactor;
        float32 clamp;
        float32 slopeFactor;
    };

    struct PipelineRasterizationStateCreateInfo
    {
        bool depthClampEnable;
        bool rasterizerDiscardEnable;
        vk::PolygonMode polygonMode;
        vk::CullModeFlags cullMode;
        vk::FrontFace frontFace;
        Optional<RasterizationDepthBias> depthBias;
        float32 lineWidth;
    };

    struct PipelineMultisampleStateCreateInfo
    {
        int32 rasterizationSamples;
        bool sampleShadingEnable;
        float32  minSampleShading;
        // TODO samplemask
        bool alphaToCoverageEnable;
        bool alphaToOneEnable;
    };

    struct PipelineDepthStencilStateCreateInfo
    {
        bool depthTestEnable;
        bool depthWriteEnable;
        vk::CompareOp depthCompareOp;
        bool depthBoundsTestEnable;
        bool stencilTestEnable;
        vk::StencilOpState front;
        vk::StencilOpState back;
        float32 minDepthBounds;
        float32 maxDepthBounds;
    };

    struct PipelineColorBlendAttachmentState
    {
        bool blendEnable;
        vk::BlendFactor srcColorBlendFactor;
        vk::BlendFactor dstColorBlendFactor;
        vk::BlendOp colorBlendOp;
        vk::BlendFactor srcAlphaBlendFactor;
        vk::BlendFactor dstAlphaBlendFactor;
        vk::BlendOp  alphaBlendOp;
        vk::ColorComponentFlags colorWriteMask;
    };

    struct PipelineColorBlendStateCreateInfo
    {
        bool logicOpEnable;
        vk::LogicOp logicOp;
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
        PipelineLayout* layout;
    };

    vk::GraphicsPipelineCreateInfo ToVulkanGraphicsPipelineCreateInfo(const GraphicsPipelineCreateInfo& ci);
}
