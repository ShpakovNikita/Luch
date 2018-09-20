#pragma once

#include <Husky/Graphics/Format.h>
#include <Husky/Graphics/VertexInputRate.h>
#include <Husky/Graphics/PolygonMode.h>
#include <Husky/Graphics/CullMode.h>
#include <Husky/Graphics/FrontFace.h>
#include <Husky/Graphics/StencilOperation.h>
#include <Husky/Graphics/CompareFuntion.h>
#include <Husky/Graphics/GraphicsForwards.h>

namespace Husky::Graphics
{
    struct VertexInputAttributeDescription
    {
        Format format = Format::Undefined;
        int32 offset = 0;
        int32 location = 0;
    };

    struct VertexInputBindingDescription
    {
        int32 stride = 0;
        VertexInputRate inputRate = VertexInputRate::PerVertex;
    };

    struct PipelineVertexInputStateCreateInfo
    {
        Vector<VertexInputAttributeDescription> attributes;
        Vector<VertexInputBindingDescription> bindings;
        PrimitiveTopology primitiveTopology = PrimitiveTopology::TriangleList;
    };

    struct PipelineRasterizationStateCreateInfo
    {
        PolygonMode polygonMode = PolygonMode::Fill;
        CullMode cullMode = CullMode::None;
        FrontFace frontFace = FrontFace::Clockwise;
        float32 depthBiasConstantFactor = 0.0f;
        float32 depthBiasClamp = 0.0f;
        float32 depthBiasSlopeFactor = 0.0f;
        bool depthClampEnable = false;
        bool depthBiasEnable = false;
        bool rasterizerDiscardEnable = false;
    };

    struct PipelineMultisampleStateCreateInfo
    {
        int32 rasterizationSamples = 1;
        bool sampleShadingEnable = false;
        float32  minSampleShading = 0;
        bool alphaToCoverageEnable = false;
        bool alphaToOneEnable = false;
    };

    struct PipelineStencilStateCreateInfo
    {
        StencilOperation depthFailOperation = StencilOperation::Keep;
        StencilOperation stencilFailOperation = StencilOperation::Keep;
        StencilOperation depthStencilPassOperation = StencilOperation::Keep;
        CompareFunction compareFunction = CompareFunction::Never;
        uint32 compareMask = 0;
        uint32 writeMask = 0;
        uint32 reference = 0;
    };

    struct PipelineDepthStencilStateCreateInfo
    {
        bool depthTestEnable = false;
        bool depthWriteEnable = false;
        CompareFunction depthCompareFunction = CompareFunction::Never;
        bool depthBoundsTestEnable = false;
        bool stencilTestEnable = false;
        Format depthFormat = Format::D32Sfloat;
        Format stencilFormat = Format::S8Uint;
        PipelineStencilStateCreateInfo front;
        PipelineStencilStateCreateInfo back;
    };

    struct PipelineColorAttachmentState
    {
        Format format = Format::R8G8B8A8Unorm;
        bool blendEnable = false;
        BlendFactor srcColorBlendFactor = BlendFactor::One;
        BlendFactor dstColorBlendFactor = BlendFactor::Zero;
        BlendOp colorBlendOp = BlendOp::Add;
        BlendFactor srcAlphaBlendFactor = BlendFactor::One;
        BlendFactor dstAlphaBlendFactor = BlendFactor::Zero;
        BlendOp alphaBlendOp = BlendOp::Add;
        ColorComponentFlags colorWriteMask = ColorComponentFlags::All
    };

    struct PipelineColorAttachmentsStateCreateInfo
    {
        Vector<PipelineColorBlendAttachmentState> attachments;
        ColorSNorm blendColor;
    };

    struct PipelineStateCreateInfo
    {
        VertexInputStateCreateInfo vertexInput;
        PipelineVertexInputStateCreateInfo inputAssembler;
        PipelineMultisampleStateCreateInfo multisampling;
        PipelineRasterizationStateCreateInfo rasterization;
        PipelineDepthStencilStateCreateInfo depthStencil;
        PipelineColorAttachmentsStateCreateInfo colorAttachments;
    };
}
